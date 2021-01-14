#include "capacitymonitor.h"

CapacityMonitor::CapacityMonitor(QObject *parent) :
    QObject(parent)
{
    queryModelTimerId_ = startTimer(10 * 1000); 

    QString szFile = qApp->applicationDirPath() + "/rtdb.ini";
    rtdb_InitialFromFile(szFile.toStdString().c_str());
    if(rtdb_Connect()) 
        qDebug()<<"rtdb Connect Success"; 
	else 
		qWarning()<<"rtdb Connect Failure"; 

	connect(this, SIGNAL(sendPredictModel(stCalcModel)), this , SLOT(startPredictModel(stCalcModel)));
}

void CapacityMonitor::timerEvent(QTimerEvent *timerEvent)
{
    if(timerEvent->timerId() == queryModelTimerId_)
    {
        QMap<QString, stCalcModel> mapCalcModel = SingletonDBHelper->queryCalcModel();
		 
        QMapIterator<QString, stCalcModel> it(mapCalcModel);
        while (it.hasNext())
        {
            it.next();
            stCalcModel calcModel = it.value();
            if(calcModel.XL_Status == 1 && calcModel.IS_Valid == 1)         //模型已提交，开始计算模型
            {
				if(QThreadPool::globalInstance()->activeThreadCount() < MaxThreadCount)  
					QtConcurrent::run(this, &CapacityMonitor::startCalcModel, calcModel); 
            }
			else if((calcModel.ConditionType == 4 || calcModel.ConditionType == 5)  && calcModel.IS_Valid == 1) //模型已提交，开始计算模型  && calcModel.XL_Status == 3
			{	
				emit sendPredictModel(calcModel);  
			}
        }
    } 
}


int CapacityMonitor::GetHisValue(Tag* pTags,long startTime,long endTime, QString pubIndexCode,int iStep)
{
	QMutexLocker mutexLocker(&mutex_);
	if (!rtdb_CheckConnecting()) 
	{
		qWarning()<<"rtdb DisConnecting";
		return false;
	}
    int iCount = 0;
	std::string strPubIndexCode;

	QString ServiceName = QString("%1.%2").arg(SingletonConfig->getServiceName()).arg(SingletonConfig->getDeviceName()); 
	if(pubIndexCode.contains(ServiceName)) 
		strPubIndexCode = pubIndexCode.toStdString();
	else
		strPubIndexCode = QString("%1.%2").arg(ServiceName).arg(pubIndexCode).toStdString();  

    bool bReturn = rtdb_GetHisValue(strPubIndexCode.c_str(),startTime,endTime,iStep,0,0,0,NULL,&iCount);//统计时间段内点个数
    bReturn = rtdb_GetHisValue(strPubIndexCode.c_str(),startTime,endTime,iStep,0,0,0,pTags,&iCount);//取出时间段内所有点值存入pTag
    if(!bReturn)
    {
        char error[100] = {0};
        rtdb_GetError(error);
        qWarning()<<QString("Err:%1  PubIndexCode:%2  StartTime:%3  EndTime:%4").arg(QString::fromLocal8Bit(error)).arg(QString::fromStdString(strPubIndexCode))
		        .arg(QDateTime::fromTime_t(startTime).toString("yyyy-MM-dd hh:mm:ss"))
				.arg(QDateTime::fromTime_t(endTime).toString("yyyy-MM-dd hh:mm:ss"));
        iCount = 0;
    } 

    return iCount;
}

int CapacityMonitor::GetTagValues(const char* pTagsName,Tag *pTags, int *p_iCount)
{
	QMutexLocker mutexLocker(&mutex_); 
	if (!rtdb_CheckConnecting()) 
	{
		qWarning()<<"rtdb DisConnecting";
		return false;
	}
	bool rslt =  rtdb_GetTagValues(pTagsName, pTags, p_iCount);  
	if (!rslt) 
	{
		char error[100] = {0};
		rtdb_GetError(error);
		qDebug()<<"Err:" + QString::fromLocal8Bit((const char *)error); 
	}
	return *p_iCount; 
}


float CapacityMonitor::GetIndexCodeValue(QString indexCode)
{ 
	stPubIndex pubIndex = SingletonDBHelper->queryPubIndexCode(indexCode);

	char pTagsName[DEFAULT_TAGNAME_LEN] = {0};  
	strcpy(pTagsName, QString("%1.%2.%3").arg(SingletonConfig->getServiceName()).arg(SingletonConfig->getDeviceName()).arg(pubIndex.WritebackCodeZ).toStdString().c_str());
	int iCount = 1;  
	Tag tag; 
	GetTagValues(pTagsName, &tag, &iCount); 
	return tag.fValue;
}
void CapacityMonitor::startCalcModel(stCalcModel calcModel)
{
    qDebug()<<QString("Start Calculate Model:%1, BeginTime:%2, EndTime:%3").arg(calcModel.Id)
			  .arg(calcModel.BeginTime.toString("yyyy-MM-dd hh:mm:ss"))
			  .arg(calcModel.EndTime.toString("yyyy-MM-dd hh:mm:ss")); 
	SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 2);

    uint startTime = calcModel.BeginTime.toTime_t();
    uint endTime = calcModel.EndTime.toTime_t();
    int step = SingletonConfig->getStep(); 
    int iTagNum = qCeil((endTime - startTime) / (float)step) + 1;  
    Tag* pTags = (Tag *)malloc(sizeof(Tag) * iTagNum);
	 
	//过滤点,key:下标  value:点值
	QMap<int, QList<stPointInfo> > mapListCondtionPointInfo;

	//过滤点,key:点名  value:过滤条件
	QMap<QString, stTfnlCondtion> mapTfnlCondtion;

	//从数据库获取过滤点名
	QList<stTfnlCondtion> listTfnlCondtion = SingletonDBHelper->queryTfnlCondtion(calcModel); 
	//从实时库获取过滤点值
	for(int i = 0; i < listTfnlCondtion.size(); ++i)
	{
		stTfnlCondtion tfnlCondtion = listTfnlCondtion.value(i);

		QString fullIndexCode = QString("%1_M%2%3").arg(calcModel.FactoryCode).arg(calcModel.SetCode).arg(tfnlCondtion.IndexCode);
		stPubIndex pubIndex = SingletonDBHelper->queryPubIndexCode(fullIndexCode);
		  
		//全点名为key，保存过滤条件
		mapTfnlCondtion[pubIndex.WritebackCodeZ] = tfnlCondtion;

		memset(pTags, 0, sizeof(Tag) * iTagNum);
		int iCount = GetHisValue(pTags, startTime, endTime, pubIndex.WritebackCodeZ, step); 
		qDebug()<<"Model:"<<calcModel.Id<<" condtionFullIndexCode:"<<fullIndexCode<<" pubIndexName:"<<pubIndex.WritebackCodeZ<<" Count:"<<iCount<<" UpLimit:"<<tfnlCondtion.UpLimit<<" LowLimit:"<<tfnlCondtion.LowLimit;
		if(iCount == 0)  
		{ 
			qWarning()<<"Model:"<<calcModel.Id<<" Calculate Failure";
			SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 4);
			break;
		}

		for(int j = 0; j < iCount; ++j)
		{
			stPointInfo pointInfo = getPointInfo(pubIndex, pTags[j]);
			//qDebug()<<QDateTime::fromTime_t(pTags[j].lTimeStamp).toString("yyyy-dd-MM hh:mm:ss")<<pTags[j].fValue;
			mapListCondtionPointInfo[pTags[j].lTimeStamp].push_back(pointInfo);
		}  
	}

	//实时评估的参数
	std::vector<com::thriftcode::data_info> list_data_info; 
	std::vector<double>  list_data;

	//计算点,key:下标  value:点值
	QMap<int, QList<stPointInfo> > mapListSspgPointInfo;
	//从数据库获取计算点名
	QList<stSspgIndex> listSspgIndex = SingletonDBHelper->queryTfnlSspgIndex(calcModel, 1); 
	//从实时库获取计算点值
	for(int i = 0; i < listSspgIndex.size(); ++i)
	{
		stSspgIndex sspgIndex = listSspgIndex.value(i);  
		stPubIndex pubIndex = SingletonDBHelper->queryPubIndexCode(sspgIndex.fullIndexCode);

		com::thriftcode::data_info datainfo;
		datainfo.maxValue = sspgIndex.maxValue;
		datainfo.minValue = sspgIndex.minValue;
		datainfo.colName = pubIndex.IndexNameC.toUtf8().data();
		list_data_info.push_back(datainfo);

		memset(pTags, 0, sizeof(Tag) * iTagNum);
		int iCount = GetHisValue(pTags, startTime, endTime, pubIndex.WritebackCodeZ, step);
		qDebug()<<"Model:"<<calcModel.Id<<" SspgIndexFullIndexCode:"<<sspgIndex.fullIndexCode<<" pubIndexName:"<<pubIndex.WritebackCodeZ
			   <<" Count:"<<iCount<<" MaxValue:"<<sspgIndex.maxValue<<" MinValue:"<<sspgIndex.minValue;
		if(iCount == 0)  
		{
			qWarning()<<"Model:"<<calcModel.Id<<" Calculate Failure";
			SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 4);
			break;
		}

		for(int j = 0; j < iCount; ++j)
		{
			stPointInfo pointInfo = getPointInfo(pubIndex,pTags[j], sspgIndex.indexOrder);
			//qDebug()<<QDateTime::fromTime_t(pTags[j].lTimeStamp).toString("yyyy-dd-MM hh:mm:ss");
			mapListSspgPointInfo[pTags[j].lTimeStamp].push_back(pointInfo);
		}  
	} 
	 
	//过滤前记录条数
	qDebug()<<"Model:"<<calcModel.Id<<" FilterBeforeSspgPointInfoSize:"<<mapListSspgPointInfo.size();
	//记录每个过滤条件过滤的个数
	QMap<QString, int> mapFilterCount; 
	//按过滤条件过滤计算点
	QMapIterator<int, QList<stPointInfo> > itSspgPointInfo(mapListSspgPointInfo);
	while(itSspgPointInfo.hasNext())
	{
		itSspgPointInfo.next();
		int timeStamp = itSspgPointInfo.key();
		QList<stPointInfo> listSspgPointInfo = itSspgPointInfo.value(); 
		 
		//qDebug()<<"listSspgPointInfo:"<<listSspgPointInfo.size()<<" listSspgIndex"<<listSspgIndex.size();

		//实时库某时刻的点数 和  计算点数不一致
		if (listSspgPointInfo.size() != listSspgIndex.size()) 
		{ 
			mapFilterCount["size not equal"]++;
			mapListSspgPointInfo.remove(timeStamp);  
			continue;  
		}

		if (!mapListCondtionPointInfo.contains(timeStamp)) 
		{
			mapListSspgPointInfo.remove(timeStamp);
			mapFilterCount["timeStamp not contains"]++;
			continue;   
		}
		  
		QList<stPointInfo> listCondtionPointInfo = mapListCondtionPointInfo.value(timeStamp);
		for (int i = 0; i < listCondtionPointInfo.size(); ++i)
		{
			stPointInfo pointInfo = listCondtionPointInfo.value(i);

			//通过全点名，找到过滤条件
			stTfnlCondtion tfnlCondtion = mapTfnlCondtion.value(pointInfo.name); 
			
			if(pointInfo.value <= tfnlCondtion.LowLimit) 
			{
				//qDebug()<<"remove pointInfo name:"<<pointInfo.desc<<" value:"<<pointInfo.value<<" UpLimit:"<<tfnlCondtion.UpLimit<<" LowLimit:"<<tfnlCondtion.LowLimit;
				mapFilterCount[pointInfo.name + " less"]++;
				mapListSspgPointInfo.remove(timeStamp);  
				break;
			} 
			else if(pointInfo.value >= tfnlCondtion.UpLimit) 
			{
				mapFilterCount[pointInfo.name + " greater"]++;
				mapListSspgPointInfo.remove(timeStamp);  
				break;
			} 
		} 
	}
	//打印过滤条件过滤的点值个数
	QMapIterator<QString, int> itFilterCount(mapFilterCount);
	while(itFilterCount.hasNext())
	{
		itFilterCount.next();
		qDebug()<<QString("Model:%1 FilterParam:%2, FilterCount:%3").arg(calcModel.Id).arg(itFilterCount.key()).arg(itFilterCount.value());
	}
	//过滤后记录条数
	qDebug()<<"Model:"<<calcModel.Id<<" FilterLaterSspgPointInfoSize:"<<mapListSspgPointInfo.size();
 
    switch(calcModel.ConditionType)
    {  
	case 1: //1:调峰曲线上限
	case 2: //2:调峰曲线下限
	{
		SingletonDBHelper->insertFilterData(calcModel, mapListSspgPointInfo);

		if(listSspgIndex.size() == 2)   //二维曲线
		{
			std::list<INPUT_POINT> inputData = getInputPoint(mapListSspgPointInfo); 
			std::list<INPUT_POINT> output;
			std::string paramList;  

			LeastSquaresCurve curve;
			int result = curve.ReturnPoint(inputData, output, paramList);  
			if(result == 0)
			{
				qDebug()<<"Model:"<<calcModel.Id<<" Calculate Success, paramList:"<<paramList.c_str(); 
				if(SingletonDBHelper->insertEquation(calcModel, QString::fromStdString(paramList)))  
					SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 3);
			} 
			else 
			{
				qWarning()<<"Model:"<<calcModel.Id<<" Calculate Failure";
				SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 4);
			}
		}
		else if(listSspgIndex.size() == 3)   //三维曲面
		{
			std::list<INPUT_POINT3D> inputData3D = getInputPoint3D(mapListSspgPointInfo);
			std::list<INPUT_POINT3D> out_point; 
			std::string paramList;  

			LeastSquareSurface surface;
			int result = surface.ReturnPoint(inputData3D, out_point, paramList); 
			if(result == 0)
			{
				qDebug()<<"Model:"<<calcModel.Id<<" Calculate Success, paramList:"<<paramList.c_str(); 
				if(SingletonDBHelper->insertEquation(calcModel, QString::fromStdString(paramList)))
					SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 3);
			} 
			else 
			{
				qWarning()<<"Model:"<<calcModel.Id<<" Calculate Failure";
				SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 4);
			}
		}	
		break;
	}
	case 3:  //3:纯凝曲线
	{   
		//符合过滤条件的点值
		std::vector<double> list_CNGK_Data;

		//计算负荷出现次数的百分比
		QMap<int, double> mapFhAppearPercent = calcFhAppearPercent(mapListCondtionPointInfo, mapTfnlCondtion, list_CNGK_Data);

		qDebug()<<"Model:"<<calcModel.Id<<" list_CNGK_DataSize:"<<list_CNGK_Data.size();

		//纯凝工况数据插入数据库
		SingletonDBHelper->insertCNGKData(calcModel, mapFhAppearPercent);

		boost::shared_ptr<TTransport> socket(new TSocket(SingletonConfig->getPythonServer().toStdString(), 8000));
		boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		com::thriftcode::CapacityMonitorServiceClient client(protocol);
		try
		{
			transport->open();
			string weibullParamList;
			client.getWeibullDistributionModel(weibullParamList,list_CNGK_Data);
			qDebug()<<"Model:"<<calcModel.Id<<" WeibullParamList:"<<weibullParamList.c_str();

			string logisticParamList;
			client.getLogisticDistributionModel(logisticParamList,list_CNGK_Data);
			qDebug()<<"Model:"<<calcModel.Id<<" LogisticParamList:"<<logisticParamList.c_str();

			SingletonDBHelper->insertEquation(calcModel, QString::fromStdString(weibullParamList + ";" + logisticParamList));
			SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 3);
			transport->close();
		}
		catch (TException& tx)
		{ 
			qWarning()<<"Model:"<<calcModel.Id<<" Calculate Failure,Err:"<<tx.what();
			SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 4);
		} 
		break;
	}
	case 4:  //4:实时评估上限
    case 5:  //5:实时评估下限 	
    {   
		if (mapListSspgPointInfo.size() == 0) 
			break; 

		QMapIterator<int, QList<stPointInfo> > itSspgPointInfo(mapListSspgPointInfo);
		while(itSspgPointInfo.hasNext())
		{
			itSspgPointInfo.next();
				 
			QList<stPointInfo> listPointInfo = itSspgPointInfo.value(); 
			for (int i = 0; i < listPointInfo.size(); ++i)
			{
				stPointInfo pointInfo = listPointInfo.value(i); 
				list_data.push_back((double)pointInfo.value); 
			}
		}
			  
		qDebug()<<"Model:"<<calcModel.Id<<" list_data_infoSize:"<<list_data_info.size()<<" list_data_size:"<<list_data.size();

		boost::shared_ptr<TTransport> socket(new TSocket(SingletonConfig->getPythonServer().toStdString(), 8000));
		boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		com::thriftcode::CapacityMonitorServiceClient client(protocol);
		try
		{
			transport->open();   
			qDebug()<<"Model:"<<calcModel.Id<<" send trainModel Request:"<<calcModel.Id; 
			int result = client.trainModel(calcModel.Id.toStdString(), list_data_info, list_data); 
			if (result == 1) 
			{
				qDebug()<<"Model:"<<calcModel.Id<<" trainModel Success:"<<calcModel.Id;
				SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 3); 
			}
			else  
			{ 
				qWarning()<<"Model:"<<calcModel.Id<<" Calculate Failure";
				SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 4);
			}
			transport->close();
		}
		catch (TException& tx)
		{ 
			qWarning()<<"Model:"<<calcModel.Id<<" Calculate Failure,Err:"<<tx.what();
			SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 4);
		}  
		break;
    } 
	case 6:
	{
		//上限输入数据
		std::list<INPUT_POINT> listSxInputData;
		//下限输入数据
		std::list<INPUT_POINT> listXxInputData;

		QList<stXnsyResult> listXnsyResult = SingletonDBHelper->queryXnsyResult(calcModel);
		for (int i = 0; i < listXnsyResult.size(); ++i)
		{
			stXnsyResult xnsyResult = listXnsyResult.value(i);
		 
			INPUT_POINT sxInputData;
			sxInputData.x = xnsyResult.grcqllValue;
			sxInputData.y = xnsyResult.tfsxValue; 
			listSxInputData.push_back(sxInputData);

			INPUT_POINT xxInputData;	
			xxInputData.x = xnsyResult.grcqllValue;
			xxInputData.y = xnsyResult.tfxxValue;
			listXxInputData.push_back(xxInputData);
		}
		LeastSquaresCurve curve;
		std::list<INPUT_POINT> output;

		std::string sxParamList;  
		int result = curve.ReturnPoint(listSxInputData, output, sxParamList);  

		std::string xxParamList;  
		result = curve.ReturnPoint(listXxInputData, output, xxParamList);   
		if(result == 0) 
		{
			SingletonDBHelper->insertEquation(calcModel, QString::fromStdString(sxParamList + ";" + xxParamList));  
			SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 3);
		}
	}
    default:
        break;
    }
 
	if (pTags != NULL)
	{
		free(pTags);
		pTags = NULL;
	}
	 
    qDebug()<<QString("Finish Calculate Model:%1").arg(calcModel.Id);
}

void CapacityMonitor::startPredictModel(stCalcModel calcModel)
{    
	QString fullIndexCode = QString("%1_M%2_%3").arg(calcModel.FactoryCode).arg(calcModel.SetCode).arg("FH");
	float fhValue = GetIndexCodeValue(fullIndexCode);
	 
	if (calcModel.ConditionType == 4)
	{
		QString ddsxIndexCode = QString("%1_M%2_%3").arg(calcModel.FactoryCode).arg(calcModel.SetCode).arg(SingletonConfig->getDDSXIndexCode());
		float ddsxValue = GetIndexCodeValue(ddsxIndexCode);
		qDebug()<<fhValue<<ddsxValue;
		if(qAbs(ddsxValue - fhValue) / ddsxValue * 100 < calcModel.AlarmValue)
		{ 
			if(!mapCalcModelAlarm.contains(calcModel.Id))
			{ 
				QString alarmID = QUuid::createUuid();
				mapCalcModelAlarm[calcModel.Id] = alarmID;

				SingletonDBHelper->insertTFNLAlarm(alarmID, calcModel, fhValue, ddsxValue, 0, 1);
			}
		} 
		else
		{
			if(mapCalcModelAlarm.contains(calcModel.Id))
			{
				QString alarmID = mapCalcModelAlarm.value(calcModel.Id);

				SingletonDBHelper->updateTFNLAlarm(alarmID);
				mapCalcModelAlarm.remove(calcModel.Id);
			}
		}
	}
	else if(calcModel.ConditionType == 5)
	{
		QString ddxxIndexCode = QString("%1_M%2_%3").arg(calcModel.FactoryCode).arg(calcModel.SetCode).arg(SingletonConfig->getDDXXIndexCode());
		float ddxxValue = GetIndexCodeValue(ddxxIndexCode); 
		qDebug()<<fhValue<<ddxxValue;
		if(qAbs(ddxxValue - fhValue) / ddxxValue * 100 < calcModel.AlarmValue)
		{ 
			if(!mapCalcModelAlarm.contains(calcModel.Id))
			{ 
				QString alarmID = QUuid::createUuid();
				mapCalcModelAlarm[calcModel.Id] = alarmID;

				SingletonDBHelper->insertTFNLAlarm(alarmID, calcModel, fhValue, 0, ddxxValue, 2);
			}
		} 
		else
		{
			if(mapCalcModelAlarm.contains(calcModel.Id))
			{
				QString alarmID = mapCalcModelAlarm.value(calcModel.Id);

				SingletonDBHelper->updateTFNLAlarm(alarmID);
				mapCalcModelAlarm.remove(calcModel.Id);
			}
		} 
	}
	/*
	//保存实时值
	std::vector<double> list_data;
	std::vector<com::thriftcode::data_info>  list_data_info;

	//从数据库获取计算点名
	QList<stSspgIndex> listSspgIndex = SingletonDBHelper->queryTfnlSspgIndex(calcModel , 1);

	//预测只需要结果点的上下限，不用实时点值
	int iTagNum = listSspgIndex.size() - 1;  
	  
	Tag* pTags = (Tag *)malloc(sizeof(Tag) * iTagNum); 
	memset(pTags, 0, sizeof(Tag) * iTagNum);
	 
	char *pTagsName = (char *)malloc(DEFAULT_TAGNAME_LEN * iTagNum); 
	memset(pTagsName, 0, iTagNum * DEFAULT_TAGNAME_LEN);    

	//从实时库获取计算点值
	for(int i = 0; i < listSspgIndex.size(); ++i)
	{
		stSspgIndex sspgIndex = listSspgIndex.value(i);
		stPubIndex pubIndex = SingletonDBHelper->queryPubIndexCode(sspgIndex.fullIndexCode);
		 
		com::thriftcode::data_info datainfo;
		datainfo.maxValue = sspgIndex.maxValue;
		datainfo.minValue = sspgIndex.minValue;
		datainfo.colName = pubIndex.IndexNameC.toUtf8().data();
		list_data_info.push_back(datainfo);
			
		//预测只需要结果点的上下限，不用实时点值
		if (i == (listSspgIndex.size() - 1))
			continue;

		std::string fullIndexCode = QString("%1.%2.%3").arg(SingletonConfig->getServiceName()).arg(SingletonConfig->getDeviceName()).arg(pubIndex.WritebackCodeZ).toStdString();
		strcpy(pTagsName + i * DEFAULT_TAGNAME_LEN, fullIndexCode.c_str());	
	} 

	//从实时库获取实时值
	int iCount = GetTagValues(pTagsName, pTags, &iTagNum);  
	for(int j = 0; j < iCount; ++j) 
		list_data.push_back( pTags[j].fValue);   
	   
	boost::shared_ptr<TTransport> socket(new TSocket(SingletonConfig->getPythonServer().toStdString(), 8000));
	boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
	com::thriftcode::CapacityMonitorServiceClient client(protocol);
	try
	{
		transport->open();   

		std::vector<double> vecPredictValue; 
		client.predict(vecPredictValue, calcModel.Id.toStdString(), list_data_info, list_data);    

		QList<stSspgIndex> listSspgIndexResult = SingletonDBHelper->queryTfnlSspgIndex(calcModel , 0); 
		for(int i = 0; i < vecPredictValue.size(); ++i)
		{
			double predictValue =  vecPredictValue.at(i);
			stSspgIndex sspgIndex = listSspgIndexResult.value(i); 

			if(SingletonDBHelper->updatePubIndexValue(predictValue, sspgIndex.fullIndexCode))
				qDebug()<<"Model:"<<calcModel.Id<<" Update PubIndexValue Success:"<<sspgIndex.fullIndexCode<<predictValue; 
			else 
				qWarning()<<"Model:"<<calcModel.Id<<" Update PubIndexValue Failure:"<<sspgIndex.fullIndexCode<<predictValue;  
		}

		transport->close();
	}
	catch (TException& tx)
	{  
		qWarning()<<"Model:"<<calcModel.Id<<" Predict Failure,Err:"<<tx.what();
		SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 4);
	} 

	if (pTags != NULL)
	{
		free(pTags);
		pTags = NULL;
	}
	
	if (pTagsName != NULL)
	{
		free(pTagsName);
		pTagsName = NULL;
	} 
	*/
}

 
QMap<int, double> CapacityMonitor::calcFhAppearPercent(QMap<int, QList<stPointInfo> > mapListPointInfo, QMap<QString, stTfnlCondtion> mapTfnlCondtion, std::vector<double>& list_CNGK_Data)
{
    int sum = 0;
    QMap<int, double> mapFhAppearPercent;
    QMapIterator<int, QList<stPointInfo> > it(mapListPointInfo);
    while(it.hasNext())
    {
        it.next();
        double GRCQLL1_VALUE = 0,GRCQLL2_VALUE = 0,FH_VALUE = 0;
        stTfnlCondtion GRCQLL1_Condtion,GRCQLL2_Condtion,FH_Condtion;

        QList<stPointInfo> listPointInfo = it.value();

        for(int i = 0; i < listPointInfo.size(); ++i)
        {
            stPointInfo pointInfo = listPointInfo.value(i);
			//qDebug()<<"timeStamp:"<<QDateTime::fromTime_t(pointInfo.timeStamp).toString("yyyy-MM-dd hh:mm:ss")<<" desc:"<<pointInfo.desc<<" value:"<<pointInfo.value;

			if(pointInfo.desc.contains(QString::fromLocal8Bit("Ⅰ级")) || pointInfo.desc.contains(QString::fromLocal8Bit("一级")))
			{
				GRCQLL1_VALUE = pointInfo.value;
				GRCQLL1_Condtion = mapTfnlCondtion.value(pointInfo.name);
			}
			else if(pointInfo.desc.contains(QString::fromLocal8Bit("ⅠⅠ")) || pointInfo.desc.contains(QString::fromLocal8Bit("二级")))
			{
				GRCQLL2_VALUE = pointInfo.value;
				GRCQLL2_Condtion = mapTfnlCondtion.value(pointInfo.name);
			}
			else if(pointInfo.desc.contains(QString::fromLocal8Bit("负荷")))
			{
				FH_VALUE = pointInfo.value;
				FH_Condtion = mapTfnlCondtion.value(pointInfo.name);
			}
		}
		
		if(((FH_VALUE >= FH_Condtion.LowLimit) && (FH_VALUE <= FH_Condtion.UpLimit)) && (GRCQLL1_VALUE <= 3) && (GRCQLL2_VALUE <= 3))    
        {
            mapFhAppearPercent[qCeil(FH_VALUE)]++;
            sum++;
			list_CNGK_Data.push_back(FH_VALUE);
        }
    }

	//步长为1，计算负荷出现次数的百分比
    QMapIterator<int, double> itFhAppearPercent(mapFhAppearPercent);
    while(itFhAppearPercent.hasNext())
    {
        itFhAppearPercent.next();
        int fh = itFhAppearPercent.key();
        int count = itFhAppearPercent.value();
        double appearPercent = (double) count/ sum;
        mapFhAppearPercent[fh] = appearPercent;
		
        while(!mapFhAppearPercent.contains(++fh) && itFhAppearPercent.hasNext())
            mapFhAppearPercent[fh] = 0;
    }
    return mapFhAppearPercent;
}


list<INPUT_POINT> CapacityMonitor::getInputPoint(QMap<int, QList<stPointInfo> > mapListPointInfo)
{
	list<INPUT_POINT> listInputPoint;
    QMapIterator<int, QList<stPointInfo> > it(mapListPointInfo);
	while(it.hasNext())
	{
		it.next(); 
		 
		INPUT_POINT inputPoint;
		QList<stPointInfo> listPointInfo = it.value();
		for(int i = 0; i < listPointInfo.size(); ++i)
		{
			stPointInfo pointInfo = listPointInfo.value(i); 
			//qDebug()<<"desc:"<<pointInfo.desc<<" value:"<<pointInfo.value;
			if (pointInfo.order == 0)   //负荷 
				inputPoint.y = (double)pointInfo.value;  
			else  //供热抽气流量,可能是1级也可能是2级
				inputPoint.x = (double)pointInfo.value;  
		} 

		listInputPoint.push_back(inputPoint); 
	}
	return listInputPoint;
}


list<INPUT_POINT3D> CapacityMonitor::getInputPoint3D(QMap<int, QList<stPointInfo> > mapListPointInfo)
{
	list<INPUT_POINT3D> listInputPoint3D;
    QMapIterator<int, QList<stPointInfo> > it(mapListPointInfo);
	while(it.hasNext())
	{
		it.next(); 

		INPUT_POINT3D inputPoint3D;
		QList<stPointInfo> listPointInfo = it.value();
		if (listPointInfo.size() != 3) 
			continue;
	 
		for(int i = 0; i < listPointInfo.size(); ++i)
		{
			stPointInfo pointInfo = listPointInfo.value(i);
			//qDebug()<<"timeStamp:"<<QDateTime::fromTime_t(pointInfo.timeStamp).toString("yyyy-MM-dd hh:mm:ss")<<"desc:"<<pointInfo.desc<<" value:"<<pointInfo.value;

			if (pointInfo.order == 0)   //负荷 
				inputPoint3D.z = pointInfo.value; 
			else if(pointInfo.order == 1)  //一级供热抽气流量 
				inputPoint3D.x = pointInfo.value; 
			else if(pointInfo.order == 2)  //二级供热抽气流量y 
				inputPoint3D.y = pointInfo.value; 
		}  
	 
		listInputPoint3D.push_back(inputPoint3D); 
	}
	return listInputPoint3D;
}



stPointInfo CapacityMonitor::getPointInfo(stPubIndex pubIndex, Tag tag, int order)
{
	stPointInfo pointInfo;
	pointInfo.name = pubIndex.WritebackCodeZ;
	pointInfo.desc = pubIndex.IndexNameC;
	pointInfo.value = tag.fValue; 
	pointInfo.timeStamp = tag.lTimeStamp; 
	pointInfo.order = order;
	return pointInfo;
}
