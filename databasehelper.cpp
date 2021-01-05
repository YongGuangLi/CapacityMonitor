 #include "databasehelper.h"

DataBaseHelper * DataBaseHelper::dbHelp_ = NULL;


#define SQL_SELECT_MODEL   "select * from TB_TFNL_MODEL"

#define SQL_UPDATE_MODEL   "update TB_TFNL_MODEL set XL_STATUS = %1 where ID = '%2'"

#define SQL_SELECT_PUB_INDEDX "SELECT WRITE_BACK_CODE,INDEX_NAME from V_PUB_ALL_INDEX_AND_POINT where FULL_INDEX_CODE = '%1' AND (xtno = 1 )" //or xtno = 4)"

#define SQL_SELECT_TFNL_CONDTION "SELECT INDEX_CODE,UP_LIMT,UP_LIMT_IN,LOW_LIMT,LOW_LIMT_IN from TB_TFNL_CONDTION where MODEL_ID = '%1'"

#define SQL_SELECT_TFNL_SSPGINDEX "SELECT FULL_INDEX_CODE,INDEX_TYPE,INDEX_ORDER,MAX_VALUE,MIN_VALUE from TB_TFNL_SSPGINDEX where MODEL_ID = '%1' and INDEX_TYPE = %2 order by INDEX_ORDER"

#define SQL_INSERT_TFNL_FILTERDATA "INSERT INTO TB_TFNL_FILTERDATA (ID, MODEL_ID,GRCQLL1_VALUE,GRCQLL2_VALUE,FH_VALUE) VALUES ('%1','%2','%3','%4','%5')"

#define SQL_DELETE_TFNL_FILTERDATA "DELETE FROM TB_TFNL_FILTERDATA WHERE MODEL_ID = '%1'"

#define SQL_INSERT_TFNL_CNGKDATA "INSERT INTO TB_TFNL_CNGKDATA (ID, FACTORY_CODE,SET_CODE,FH_VALUE,APPEARNUMBER,MODEL_ID ) VALUES ('%1','%2','%3','%4','%5','%6')"

#define SQL_DELETE_TFNL_CNGKDATA "DELETE FROM TB_TFNL_CNGKDATA WHERE MODEL_ID = '%1'"
 
#define SQL_INSERT_TFNL_EQUATION "INSERT INTO TB_TFNL_EQUATION (ID, MODEL_ID,NM_PARAM_LIST,UPDATE_TIME) VALUES ('%1','%2','%3',to_date('%4', 'yyyy-mm-dd hh24:mi:ss'))"

#define SQL_DELETE_TFNL_EQUATION "DELETE FROM TB_TFNL_EQUATION WHERE MODEL_ID = '%1'"
 
#define SQL_SELECT_TFNL_XNSYRESULT "SELECT GRCQLL_VALUE,TFSX_VALUE,TFXX_VALUE from TB_TFNL_XNSYRESULT where MODEL_ID = '%1'"

#define SQL_UPDATE_PUB_INDEX_VALUE  "update TB_PUB_INDEX_VALUE set UPDATE_TIME = to_date('%1', 'yyyy-mm-dd hh24:mi:ss'), CURRENT_VALUE = %2 where FULL_INDEX_CODE = '%3'"

#define SQL_INSERT_TFNL_ALARM "INSERT INTO TB_TFNL_ALARM(FACTORY_CODE,SET_CODE,FH_VALUE,DDSX_VALUE,DDXX_VALUE,ALARM_TYPE,ALARM_BEGIN_TIME,ALARM_END_TIME) from TB_TFNL_XNSYRESULT where MODEL_ID = '%1'"

DataBaseHelper::DataBaseHelper(QObject *parent) : QObject(parent)
{
}

bool DataBaseHelper::open(QString ip, int port, QString dbName, QString user, QString passwd)
{
    sqlDatabase = QSqlDatabase::addDatabase("QOCI");
    sqlDatabase.setHostName(ip);
    sqlDatabase.setPort(port);
    sqlDatabase.setDatabaseName(dbName);
    sqlDatabase.setUserName(user);
    sqlDatabase.setPassword(passwd);

    bool result = sqlDatabase.open();
    if(result)
        qDebug()<<QString("Database Connect Success:%1").arg(ip);
     else
        qWarning()<<QString("Database Connect Failure, Err:%1").arg(sqlDatabase.lastError().text());

    return result;
}

bool DataBaseHelper::isOpen()
{
    bool result = sqlDatabase.isOpen();

    if(result == false)
    {
        result = sqlDatabase.open();
        if(result == true)
        {
            qDebug()<<QString::fromUtf8("DataBase Reconnect Success");
        }
    }

    return result;
}

void DataBaseHelper::close()
{
    if(sqlDatabase.isOpen())
        sqlDatabase.close();
}

QMap<QString, stCalcModel>  DataBaseHelper::queryCalcModel()
{
	QMutexLocker mutexLocker(&mutex_); 
	QMap<QString, stCalcModel> mapCalcModel;

    QSqlQuery query(SQL_SELECT_MODEL);
    if(query.lastError().isValid()) 
        qWarning()<<query.lastError().text(); 
		 

    while(query.next())
    {
        stCalcModel calcModel;
        calcModel.Id = query.value(0).toString();
        calcModel.FactoryCode = query.value(1).toString();
        calcModel.SetCode = query.value(2).toString();
        calcModel.BeginTime = query.value(3).toDateTime();
        calcModel.EndTime = query.value(4).toDateTime();
        calcModel.ConditionType = query.value(5).toInt();
        calcModel.AlarmValue = query.value(6).toInt();
        calcModel.XL_Status = query.value(7).toInt();
        calcModel.IS_Valid = query.value(8).toInt();

        mapCalcModel[calcModel.Id] = calcModel;
    }

    return mapCalcModel;
}

bool DataBaseHelper::updateCalcModelStatus(QString Id, int status)
{
	QMutexLocker mutexLocker(&mutex_); 
    bool result = true;

    QSqlQuery query(QString(SQL_UPDATE_MODEL).arg(status).arg(Id));
    if(query.lastError().isValid())
    {
        qWarning()<<query.lastError().text();
        result = false;
    }

    return result;
}
  

stPubIndex DataBaseHelper::queryPubIndexCode(QString fullIndexCode)
{
	QMutexLocker mutexLocker(&mutex_); 
    stPubIndex pubIndex;
    QSqlQuery query(QString(SQL_SELECT_PUB_INDEDX).arg(fullIndexCode));
    if(query.lastError().isValid())
    {
        qWarning()<<query.lastError().text();
    } 

    while(query.next())
    {
        pubIndex.WritebackCodeZ = query.value(0).toString();//.remove("_Z", Qt::CaseSensitive); 
		pubIndex.IndexNameC = query.value(1).toString(); 
    }

    return pubIndex;
}

QList<stTfnlCondtion> DataBaseHelper::queryTfnlCondtion(stCalcModel calcModel)
{
	QMutexLocker mutexLocker(&mutex_); 
    QList<stTfnlCondtion> listTfnlCondtion;

    QSqlQuery query(QString(SQL_SELECT_TFNL_CONDTION).arg(calcModel.Id));
    if(query.lastError().isValid())
    {
        qWarning()<<query.lastError().text();
    }
	 
    while(query.next())
    {
        stTfnlCondtion tfnlCondtion;
        tfnlCondtion.IndexCode = query.value(0).toString();
        tfnlCondtion.UpLimit = query.value(1).toInt();
        tfnlCondtion.UpLimitIn = query.value(2).toInt();
        tfnlCondtion.LowLimit = query.value(3).toInt();
        tfnlCondtion.LowLimitIn = query.value(4).toInt();

        listTfnlCondtion.push_back(tfnlCondtion);
    }

    return listTfnlCondtion;
}

/*
	* @date      2020-09-30
	* @param     
	* @return    
	* @brief     IndexType:0 预测结果点  IndexType:1 INDEX_ORDER最大为训练结果点
*/
QList<stSspgIndex> DataBaseHelper::queryTfnlSspgIndex(stCalcModel calcModel, int IndexType)
{
	QMutexLocker mutexLocker(&mutex_); 
    QList<stSspgIndex> listSspgIndex;

    QSqlQuery query(QString(SQL_SELECT_TFNL_SSPGINDEX).arg(calcModel.Id).arg(IndexType));
    if(query.lastError().isValid())
    {
        qWarning()<<query.lastError().text();
    }  
    while(query.next())
    {
		stSspgIndex sspgIndex;
		sspgIndex.fullIndexCode = query.value(0).toString();
		sspgIndex.IndexType = query.value(1).toInt();
		sspgIndex.indexOrder = query.value(2).toInt();
		sspgIndex.maxValue = query.value(3).toInt();
		sspgIndex.minValue = query.value(4).toInt();
		 
		listSspgIndex.push_back(sspgIndex);  
    }

    return listSspgIndex;
}

QList<stXnsyResult> DataBaseHelper::queryXnsyResult(stCalcModel calcModel)
{
	QMutexLocker mutexLocker(&mutex_); 
	QList<stXnsyResult> listXnsyResult;

	QSqlQuery query(QString(SQL_SELECT_TFNL_XNSYRESULT).arg(calcModel.Id));
	if(query.lastError().isValid())
	{
		qWarning()<<query.lastError().text();
	} 

	while(query.next())
	{
		stXnsyResult xnsyResult;
		xnsyResult.grcqllValue = query.value(0).toDouble();
		xnsyResult.tfsxValue = query.value(1).toDouble();
		xnsyResult.tfxxValue = query.value(2).toDouble();

		listXnsyResult.push_back(xnsyResult);
	}

	return listXnsyResult; 
}


bool DataBaseHelper::insertFilterData(stCalcModel calcModel, QMap<int, QList<stPointInfo> > mapListPointInfo)
{
	QMutexLocker mutexLocker(&mutex_); 
    bool result = false;
    if(QSqlDatabase::database().transaction())
	{
		QSqlQuery query;
		result = query.exec(QString(SQL_DELETE_TFNL_FILTERDATA).arg(calcModel.Id)); 

        QMapIterator<int, QList<stPointInfo> > it(mapListPointInfo);
        while(it.hasNext())
        {
            it.next();
            QString GRCQLL1_VALUE,GRCQLL2_VALUE,FH_VALUE;
            QList<stPointInfo> listPointInfo = it.value();
            for(int i = 0; i < listPointInfo.size(); ++i)
            {
                stPointInfo pointInfo = listPointInfo.value(i);   
                if(pointInfo.order == 0) 
					FH_VALUE = QString::number(pointInfo.value); 
				else if(pointInfo.order == 1) 
					GRCQLL1_VALUE = QString::number(pointInfo.value); 
                else if(pointInfo.order == 2) 
					GRCQLL2_VALUE = QString::number(pointInfo.value); 
            } 
            QSqlQuery query;
            query.exec(QString(SQL_INSERT_TFNL_FILTERDATA).arg(QUuid::createUuid()).arg(calcModel.Id).arg(GRCQLL1_VALUE).arg(GRCQLL2_VALUE).arg(FH_VALUE));
        }
        result = QSqlDatabase::database().commit();
        if(!result)
        {
            qDebug() << QSqlDatabase::database().lastError(); //提交
            if(!QSqlDatabase::database().rollback())
                qDebug() << QSqlDatabase::database().lastError(); //回滚
        }
    }
    return result;
}

bool DataBaseHelper::insertCNGKData(stCalcModel calcModel, QMap<int, double> mapFhAppearPercent)
{
	QMutexLocker mutexLocker(&mutex_); 
    bool result = false;

    QSqlQuery query;
    result = query.exec(QString(SQL_DELETE_TFNL_CNGKDATA).arg(calcModel.Id));

    QMapIterator<int, double> itAppearPercent(mapFhAppearPercent);
    while(itAppearPercent.hasNext())
    {
        itAppearPercent.next();
        result = query.exec(QString(SQL_INSERT_TFNL_CNGKDATA).arg(QUuid::createUuid()).arg(calcModel.FactoryCode)
                   .arg(calcModel.SetCode).arg(QString::number(itAppearPercent.key())).arg(QString::number(itAppearPercent.value())).arg(calcModel.Id));
    }

    return result;
}

bool DataBaseHelper::insertEquation(stCalcModel calcModel, QString paramList)
{
	QMutexLocker mutexLocker(&mutex_); 
	bool result = false;

	QSqlQuery query; 
	result = query.exec(QString(SQL_DELETE_TFNL_EQUATION).arg(calcModel.Id)); 

	QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	result = query.exec(QString(SQL_INSERT_TFNL_EQUATION).arg(QUuid::createUuid()) .arg(calcModel.Id).arg(paramList).arg(date)); 
	return result; 
}



bool DataBaseHelper::updatePubIndexValue(double value, QString fullIndexCode)
{  
	QMutexLocker mutexLocker(&mutex_); 
	bool result = true;

	QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	QSqlQuery query(QString(SQL_UPDATE_PUB_INDEX_VALUE).arg(date).arg(value).arg(fullIndexCode)); 
	if(query.lastError().isValid())
	{
		qWarning()<<query.lastError().text();
		result = false;
	}

	return result;
}


bool DataBaseHelper::updtaeTFNLAlarm(stCalcModel calcModel,double fhValue, double ddsxValue, double ddxxValue, int alarmType) 
{
	QMutexLocker mutexLocker(&mutex_); 
	bool result = false;

	QSqlQuery query;  

	QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	return result; 

}
DataBaseHelper *DataBaseHelper::GetInstance()
{
    if(dbHelp_ == NULL)
    {
        dbHelp_ = new DataBaseHelper();
    }
    return dbHelp_;
}
