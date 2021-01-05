#ifndef DATABASEHELPER_H
#define DATABASEHELPER_H

#include "configini.h"

#include <QObject>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDateTime>
#include <QString>
#include <QFileInfo>
#include <QUuid>
#include <QTimer>
#include <QMutexLocker>
#include <QMutex>

typedef struct{
    QString Id;
    QString FactoryCode;
    QString SetCode;
    QDateTime BeginTime;
    QDateTime EndTime;
    int ConditionType;  //1:调峰曲线上限、2:调峰曲线下限、3:纯凝曲线、4:实时评估上限、5:实时评估下限
    int AlarmValue;     
    int XL_Status;      //模型状态(0：模型初始化配置中 1:模型提交，2:模型计算中 3:计算完成，4:计算失败)
    int IS_Valid;
}stCalcModel;

typedef struct{
    QString WritebackCodeZ;
	QString IndexNameC;
	int minValue;
	int maxValue;
}stPubIndex;

typedef struct{
	QString fullIndexCode;
	int IndexType;
	int indexOrder;  
	int maxValue;
	int minValue;
}stSspgIndex;

  
typedef struct{
    QString IndexCode;
    int UpLimit;
    int UpLimitIn;
    int LowLimit;
    int LowLimitIn;
}stTfnlCondtion;

typedef struct{
	double grcqllValue;
	double tfxxValue;
	double tfsxValue;
}stXnsyResult;

typedef struct{
    QString ID;
    QString	setCode;
    QString pointCode;
	QString	name;
	QString	desc;
    float	value;
    QString	status;
    long	timeStamp; 
    int order; 
}stPointInfo;


#define SingletonDBHelper DataBaseHelper::GetInstance()

class DataBaseHelper : public QObject
{
    Q_OBJECT
public:
    static DataBaseHelper *GetInstance();

    bool open(QString ip,int port, QString dbName, QString user, QString passwd);

    bool isOpen();

    void close();

    QMap<QString, stCalcModel> queryCalcModel();

    /**
    * @date      2020-09-04
    * @param
    * @return
    * @brief     修改计算模型状态
    */
    bool updateCalcModelStatus(QString Id ,int status);
	 
    stPubIndex queryPubIndexCode(QString fullIndexCode);

    QList<stTfnlCondtion> queryTfnlCondtion(stCalcModel);

    QList<stSspgIndex> queryTfnlSspgIndex(stCalcModel, int IndexType);

	QList<stXnsyResult> queryXnsyResult(stCalcModel);

    bool insertFilterData(stCalcModel, QMap<int, QList<stPointInfo> > mapListPointInfo);

    bool insertCNGKData(stCalcModel, QMap<int, double> mapFhAppearPercent);

	bool insertEquation(stCalcModel, QString);

	bool updatePubIndexValue(double value, QString fullIndexCode);

	bool updtaeTFNLAlarm(stCalcModel,double , double, double, int);
public:

signals:

private:
    explicit DataBaseHelper(QObject *parent = 0);
    static DataBaseHelper * dbHelp_;

    QSqlDatabase sqlDatabase;
    QString ip_;
    int port_;
    QString dbName_;
    QString user_;
    QString passwd_;


    QMutex mutex_;
signals:

public slots:

private:
    
};

#endif // DATABASEHELPER_H
