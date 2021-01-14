#include "configini.h"

ConfigIni* ConfigIni::configIni = NULL;

ConfigIni *ConfigIni::GetInstance()
{
    if(configIni == NULL) 
        configIni = new ConfigIni();
    
    return configIni;
}

ConfigIni::ConfigIni(QObject *parent) : QObject(parent)
{

}
 
bool ConfigIni::initConfigFile(QString fileName)
{
	bool result = false;
	if(QFile::exists(fileName))
	{
		result = true;
		QSettings* settings_ = new QSettings(fileName,QSettings::IniFormat);
        settings_->setIniCodec("UTF-8");

        settings_->beginGroup("database");
        m_databaseIp =  settings_->value("database_ip").toString();
        m_databasePort =  settings_->value("database_port").toInt();
        m_databaseName =  settings_->value("database_dbname").toString();
        m_databaseUserName =  settings_->value("database_user").toString();
        m_databasePassWord = settings_->value("database_pwd").toString();
        settings_->endGroup();

        settings_->beginGroup("config");
        m_step =  settings_->value("step").toInt();
        m_serviceName =  settings_->value("ServiceName").toString();
        m_deviceName =  settings_->value("DeviceName").toString(); 
		m_pythonServer =  settings_->value("PythonServer").toString();
        settings_->endGroup();

		settings_->beginGroup("alarm");
		m_fhIndexCode =  settings_->value("FH").toString();
		m_ddsxIndexCode =  settings_->value("DDSX").toString();
		m_ddxxIndexCode =  settings_->value("DDXX").toString();  
		settings_->endGroup();


		settings_->deleteLater();
    }

    return result;
}



QString ConfigIni::getDbIp() const
{
    return m_databaseIp;
}

int ConfigIni::getDbPort() const
{
    return m_databasePort;
}

QString ConfigIni::getDbName() const
{
    return m_databaseName;
}

QString ConfigIni::getDbUser() const
{
    return m_databaseUserName;
}

QString ConfigIni::getDbPasswd() const
{
    return m_databasePassWord;
}

int ConfigIni::getStep() const
{
    return m_step;
}

QString ConfigIni::getServiceName() const
{
    return m_serviceName;
}

QString ConfigIni::getDeviceName() const
{
    return m_deviceName;
}

QString ConfigIni::getPythonServer() const
{
	return m_pythonServer;
}


QString ConfigIni::getFHIndexCode() const
{
	return m_fhIndexCode;
}


QString  ConfigIni::getDDSXIndexCode() const
{
	return m_ddsxIndexCode; 
}

QString  ConfigIni::getDDXXIndexCode() const
{ 
	return m_ddxxIndexCode; 
}
