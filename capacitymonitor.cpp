#include "capacitymonitor.h"

CapacityMonitor::CapacityMonitor(QObject *parent) :
    QObject(parent)
{
    queryModelTimerId_ = startTimer(10 * 1000);

    QString szFile = qApp->applicationDirPath() + "/rtdb.ini";
    rtdb_InitialFromFile(szFile.toStdString().c_str());
    if(rtdb_Connect())
    {
        qDebug()<<"rtdb Connect Success";
    }
}


void CapacityMonitor::timerEvent(QTimerEvent *timerEvent)
{
    if(timerEvent->timerId() == queryModelTimerId_)
    {
        SingletonDBHelper->queryCalcModel();

        QMap<QString, stCalcModel> mapCalcModel = SingletonDBHelper->getCalcModel();
        QMapIterator<QString, stCalcModel> it(mapCalcModel);
        while (it.hasNext())
        {
            it.next();
            stCalcModel calcModel = it.value();
            if(calcModel.XL_Status == 1 && calcModel.IS_Valid == 1)         //模型已提交，开始计算模型
            {
                QtConcurrent::run(this, &CapacityMonitor::startCalcModel, calcModel);
            }
        }
    }
}


int CapacityMonitor::GetHisValue(Tag* pTags,long startTime,long endTime, QString pubIndexCode,int iStep)
{
	QMutexLocker mutexLocker(&mutex_);
    int iCount = 0;
    std::string fullIndexCode = QString("%1.%2.%3").arg(SingletonConfig->getServiceName()).arg(SingletonConfig->getDeviceName()).arg(pubIndexCode).toStdString();

    bool bReturn = rtdb_GetHisValue(fullIndexCode.c_str(),startTime,endTime,iStep,0,0,0,NULL,&iCount);//统计时间段内点个数

    bReturn = rtdb_GetHisValue(fullIndexCode.c_str(),startTime,endTime,iStep,0,0,0,pTags,&iCount);//取出时间段内所有点值存入pTag
    if(!bReturn)
    {
        char error[100] = {0};
        rtdb_GetError(error);
        qDebug()<<"Err:" + QString((const char *)error) + " fullIndexCode:"<<fullIndexCode.c_str()<<" startTime:"<<startTime<<" endTime:"<<endTime;
        iCount = 0;
    } 
    return iCount;
}


void CapacityMonitor::startCalcModel(stCalcModel calcModel)
{
    qDebug()<<QString("Start Calculate Model:%1").arg(calcModel.Id); 
	//SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 2);

    uint startTime = calcModel.BeginTime.toTime_t();
    uint endTime = calcModel.EndTime.toTime_t();
    int step = SingletonConfig->getStep();
    int pTagsNum = (endTime - startTime) / step + 1;
    Tag* pTags = (Tag *)malloc(sizeof(Tag)*pTagsNum);


    //1:调峰曲线上限、2:调峰曲线下限、3:纯凝曲线、4:实时评估上限、5:实时评估下限
    switch(calcModel.ConditionType)
    {  
	case 1:
	case 2:
    case 4:
    case 5:
    { 
		//过滤点,key:下标  value:点值
        QMap<int, QList<stPointInfo> > mapListCondtionPointInfo;
		//计算点,key:下标  value:点值
        QMap<int, QList<stPointInfo> > mapListSspgPointInfo;

		//key:点名  value:过滤条件
        QMap<QString, stTfnlCondtion> mapTfnlCondtion;

		//从数据库获取过滤点名
		QList<stTfnlCondtion> listTfnlCondtion = SingletonDBHelper->queryTfnlCondtion(calcModel);
		//从实时库获取过滤点值
		for(int i = 0; i < listTfnlCondtion.size(); ++i)
		{
			stTfnlCondtion tfnlCondtion = listTfnlCondtion.value(i);
			QString fullIndexCode = QString("%1_M%2%3").arg(calcModel.FactoryCode).arg(calcModel.SetCode).arg(tfnlCondtion.IndexCode);
			stPubIndex pubIndex = SingletonDBHelper->queryPubIndexCode(fullIndexCode);

			mapTfnlCondtion[pubIndex.WritebackCodeZ] = tfnlCondtion;

			memset(pTags, 0, sizeof(Tag)*pTagsNum);
			int iCount = GetHisValue(pTags, startTime, endTime, pubIndex.WritebackCodeZ, step);
			qDebug()<<"condtionFullIndexCode:"<<pubIndex.WritebackCodeZ<<" Count:"<<iCount;
			for(int j = 0; j < iCount; ++j)
			{
				stPointInfo pointInfo;
				pointInfo.name = pubIndex.WritebackCodeZ;
				pointInfo.desc = pubIndex.IndexNameC;
				pointInfo.value = pTags[j].fValue;
				pointInfo.timeStamp = pTags[j].lTimeStamp;
                mapListCondtionPointInfo[j].push_back(pointInfo);
			}  
		}

		std::vector<com::thriftcode::data_info> list_data_info;
		//从数据库获取计算点名
		QList<stSspgIndex> listSspgIndex = SingletonDBHelper->queryTfnlSspgIndex(calcModel);
		//从实时库获取计算点值
        for(int i = 0; i < listSspgIndex.size(); ++i)
        {
            stSspgIndex sspgIndex = listSspgIndex.value(i);
			stPubIndex pubIndex = SingletonDBHelper->queryPubIndexCode(sspgIndex.fullIndexCode);

			com::thriftcode::data_info datainfo;
			datainfo.maxValue = pubIndex.maxValue;
			datainfo.minValue = pubIndex.minValue;
			datainfo.colName = pubIndex.IndexNameC.toUtf8().data();
			list_data_info.push_back(datainfo);

			memset(pTags, 0, sizeof(Tag)*pTagsNum);
			int iCount = GetHisValue(pTags, startTime, endTime, pubIndex.WritebackCodeZ, step);
			qDebug()<<"SspgIndexFullIndexCode:"<<pubIndex.WritebackCodeZ<<" Count:"<<iCount;
			for(int j = 0; j < iCount; ++j)
			{
				stPointInfo pointInfo;
				pointInfo.name = pubIndex.WritebackCodeZ;
				pointInfo.desc = pubIndex.IndexNameC;
				pointInfo.value = pTags[j].fValue;
				pointInfo.timeStamp =  pTags[j].lTimeStamp; 
				pointInfo.order = sspgIndex.indexOrder;
                mapListSspgPointInfo[j].push_back(pointInfo);
			}  
        } 

		//按过滤条件过滤计算点
        QMapIterator<int, QList<stPointInfo> > itCondtionPointInfo(mapListCondtionPointInfo);
		while(itCondtionPointInfo.hasNext())
		{
            itCondtionPointInfo.next();
			int index = itCondtionPointInfo.key();
			QList<stPointInfo> liststPointInfo = itCondtionPointInfo.value();
			bool result = true;
			for (int i = 0; i < liststPointInfo.size(); ++i)
			{
				stPointInfo pointInfo = liststPointInfo.value(i);
			 
				stTfnlCondtion tfnlCondtion = mapTfnlCondtion.value(pointInfo.name); 
				if(pointInfo.value < tfnlCondtion.LowLimit || pointInfo.value > tfnlCondtion.UpLimit) 
					result = false; 
			}
			//过滤不符合条件的计算点
//  			if (!result) 
//  				mapListSspgPointInfo.remove(index); 
		}

		qDebug()<<"mapListSspgPointInfoSize:"<<mapListSspgPointInfo.size();
		//1:调峰曲线上限、2:调峰曲线下限
		if(calcModel.ConditionType == 1 || calcModel.ConditionType == 2)
		{
			SingletonDBHelper->insertFilterData(calcModel, mapListSspgPointInfo);

			if(listSspgIndex.size() == 2)   //二维曲线
			{
				std::list<INPUT_POINT> inputData = calcInputPoint(mapListSspgPointInfo);

				LeastSquaresCurve curve;
				std::list<INPUT_POINT> output;
				std::string paramList;  
				int result = curve.ReturnPoint(inputData, output, paramList);
				if(result == 0)
				{
					SingletonDBHelper->insertEquation(calcModel, QString::fromStdString(paramList));
	                SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 3);
				} 
			}
			else if(listSspgIndex.size() == 3)   //三维曲面
			{
				std::list<INPUT_POINT3D> inputData3D = calcInputPoint3D(mapListSspgPointInfo);
				LeastSquareSurface surface;
				std::list<INPUT_POINT3D> out_point; 
				std::string paramList;  
				int result = surface.ReturnPoint(inputData3D, out_point, paramList); 
				if(result == 0)
				{
					SingletonDBHelper->insertEquation(calcModel, QString::fromStdString(paramList));
	                SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 3);
				} 
			}	
		}
		//4:实时评估上限、5:实时评估下限
		else if(calcModel.ConditionType == 4 || calcModel.ConditionType == 5)
		{  				  
			std::vector<double>  list_data;

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
			 
			boost::shared_ptr<TTransport> socket(new TSocket("192.168.19.74", 8000));
			boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
			boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
			com::thriftcode::CapacityMonitorServiceClient client(protocol);
			try
			{
				transport->open();  
				string model;
				client.trainModel(model, calcModel.Id.toStdString(), list_data_info, list_data); 
				qDebug()<<model.c_str();
				transport->close();
//              SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 3);
			}
			catch (TException& tx)
			{
				qCritical()<<tx.what();
			} 
		}
        break;
    }
	case 3:      //纯凝工况
	{  
		//key:点名  value:过滤条件
        QMap<QString, stTfnlCondtion> mapTfnlCondtion;

		//过滤点 key:下标  value:点值
        QMap<int, QList<stPointInfo> > mapListCondtionPointInfo;
		//过滤点
		QList<stTfnlCondtion> listTfnlCondtion = SingletonDBHelper->queryTfnlCondtion(calcModel);
		for(int i = 0; i < listTfnlCondtion.size(); ++i)
		{
			stTfnlCondtion tfnlCondtion = listTfnlCondtion.value(i);
			QString fullIndexCode = QString("%1_M%2%3").arg(calcModel.FactoryCode).arg(calcModel.SetCode).arg(tfnlCondtion.IndexCode);
			stPubIndex pubIndex = SingletonDBHelper->queryPubIndexCode(fullIndexCode);
			 
			mapTfnlCondtion[pubIndex.WritebackCodeZ] = tfnlCondtion;
			qDebug()<<pubIndex.WritebackCodeZ<<pubIndex.IndexNameC<<tfnlCondtion.UpLimit;
			memset(pTags, 0, sizeof(Tag)*pTagsNum);
			int iCount = GetHisValue(pTags, startTime, endTime, pubIndex.WritebackCodeZ, step);
			for(int j = 0; j < iCount; ++j)
			{
				stPointInfo pointInfo;
				pointInfo.name = pubIndex.WritebackCodeZ;
				pointInfo.desc = pubIndex.IndexNameC;
				pointInfo.value = pTags[j].fValue; 
				pointInfo.timeStamp =  pTags[j].lTimeStamp;
                mapListCondtionPointInfo[j].push_back(pointInfo);
			}
		}

		//计算负荷出现次数的百分比
		QMap<int, double> mapFhAppearValue = calcFhAppearValue(mapListCondtionPointInfo, mapTfnlCondtion);
		//纯凝工况数据插入数据库
		SingletonDBHelper->insertCNGKData(calcModel, mapFhAppearValue);

		std::vector<double> list_CNGK_Data = mapFhAppearValue.values().toVector().toStdVector();
		boost::shared_ptr<TTransport> socket(new TSocket("127.0.0.1", 8000));
		boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
		com::thriftcode::CapacityMonitorServiceClient client(protocol);
		try
		{
			transport->open();
		 	string model;
			client.getWeibullDistributionModel(model,list_CNGK_Data);
			SingletonDBHelper->insertEquation(calcModel, QString::fromStdString(model));
			transport->close();
			//SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 3);
		}
		catch (TException& tx)
		{
			qCritical()<<tx.what();
		} 
		break;
	}
	case 6:
	{
		SingletonDBHelper->updateCalcModelStatus(calcModel.Id, 2);
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


QMap<int, double> CapacityMonitor::calcFhAppearValue(QMap<int, QList<stPointInfo> > mapListPointInfo, QMap<QString, stTfnlCondtion> mapTfnlCondtion)
{
    int sum = 0;
    QMap<int, double> mapFhAppearValue;
    QMapIterator<int, QList<stPointInfo> > it(mapListPointInfo);
    while(it.hasNext())
    {
        it.next();
        int GRCQLL1_VALUE,GRCQLL2_VALUE,FH_VALUE;
        stTfnlCondtion GRCQLL1_Condtion,GRCQLL2_Condtion,FH_Condtion;

        QList<stPointInfo> listPointInfo = it.value();

        for(int i = 0; i < listPointInfo.size(); ++i)
        {
            stPointInfo pointInfo = listPointInfo.value(i);
            qDebug()<<"timeStamp:"<<QDateTime::fromTime_t(pointInfo.timeStamp).toString("yyyy-MM-dd hh:mm:ss")<<" desc:"<<pointInfo.desc<<" value:"<<pointInfo.value;

           if(pointInfo.desc.contains(QString::fromLocal8Bit("Ⅰ级")))
           {
               GRCQLL1_VALUE = qCeil(pointInfo.value);
               GRCQLL1_Condtion = mapTfnlCondtion.value(pointInfo.name);
           }
           else if(pointInfo.desc.contains(QString::fromLocal8Bit("ⅠⅠ")))
           {
               GRCQLL2_VALUE = qCeil(pointInfo.value);
               GRCQLL2_Condtion = mapTfnlCondtion.value(pointInfo.name);
           }
           else if(pointInfo.desc.contains(QString::fromLocal8Bit("负荷")))
           {
               FH_VALUE = qCeil(pointInfo.value);
               FH_Condtion = mapTfnlCondtion.value(pointInfo.name);
           }
        }

        if( FH_VALUE < FH_Condtion.UpLimit)        //(GRCQLL1_VALUE == 0) && (GRCQLL2_VALUE == 0) &&
        {
            mapFhAppearValue[FH_VALUE]++;
            sum++;
        }
    }

	//步长为1，计算负荷出现次数的百分比
    QMapIterator<int, double> itFhAppearValue(mapFhAppearValue);
    while(itFhAppearValue.hasNext())
    {
        itFhAppearValue.next();
        int fh = itFhAppearValue.key();
        int count = itFhAppearValue.value();
        double value = (double) count/ sum;
        mapFhAppearValue[fh] = value;
		
        while(!mapFhAppearValue.contains(++fh) && itFhAppearValue.hasNext())
            mapFhAppearValue[fh] = 0;
    }
    return mapFhAppearValue;
}


list<INPUT_POINT> CapacityMonitor::calcInputPoint(QMap<int, QList<stPointInfo> > mapListPointInfo)
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
			{
				inputPoint.y = (double)pointInfo.value; 
			}
			else if(pointInfo.order == 1) //一级供热抽气流量
			{ 
				inputPoint.x = (double)pointInfo.value; 
			}  
		} 

		listInputPoint.push_back(inputPoint); 
	}
	return listInputPoint;
}


list<INPUT_POINT3D> CapacityMonitor::calcInputPoint3D(QMap<int, QList<stPointInfo> > mapListPointInfo)
{
	list<INPUT_POINT3D> listInputPoint3D;
    QMapIterator<int, QList<stPointInfo> > it(mapListPointInfo);
	while(it.hasNext())
	{
		it.next(); 

		INPUT_POINT3D inputPoint3D;
		QList<stPointInfo> listPointInfo = it.value();
		if (listPointInfo.size() != 3)
		{
			continue;
		}
		for(int i = 0; i < listPointInfo.size(); ++i)
		{
			stPointInfo pointInfo = listPointInfo.value(i);
			qDebug()<<"timeStamp:"<<QDateTime::fromTime_t(pointInfo.timeStamp).toString("yyyy-MM-dd hh:mm:ss")<<"desc:"<<pointInfo.desc<<" value:"<<pointInfo.value;

			if (pointInfo.order == 0)   //负荷
			{
				inputPoint3D.z = pointInfo.value;
			}
			else if(pointInfo.order == 1)  //一级供热抽气流量
			{ 
				inputPoint3D.x = pointInfo.value;
			}  
			else if(pointInfo.order == 2)  //二级供热抽气流量y
			{ 
				inputPoint3D.y = pointInfo.value;
			}  
		}  
		qDebug()<<inputPoint3D.z<<inputPoint3D.x<<inputPoint3D.y;
		listInputPoint3D.push_back(inputPoint3D); 
	}
	return listInputPoint3D;
}
