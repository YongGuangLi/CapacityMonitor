#ifndef CONFIGINI_H
#define CONFIGINI_H

#include <QCoreApplication>
#include <QObject>
#include <QFile>
#include <QDebug>
#include <QSettings>
#include <QStringList>
#include <QMutexLocker>

#define SingletonConfig ConfigIni::GetInstance()

class ConfigIni : public QObject
{
    Q_OBJECT
public:
    static ConfigIni *GetInstance();  
	bool initConfigFile(QString);



    QString getDbIp() const;
    int getDbPort() const;
    QString getDbName() const;
    QString getDbUser() const;
    QString getDbPasswd() const;

    int getStep() const;

    QString getServiceName() const;

    QString getDeviceName() const;

private:
    explicit ConfigIni(QObject *parent = 0);
    static ConfigIni* configIni; 
signals:
    
public slots:
    
private:
    QString m_databaseIp;
    int m_databasePort;
    QString m_databaseName;
    QString m_databaseUserName;
    QString m_databasePassWord;

    int m_step;
    QString m_serviceName;
    QString m_deviceName;
};

#endif // CONFIGINI_H
