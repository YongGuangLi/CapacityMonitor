#include <QCoreApplication>
#include <QStringList>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMutexLocker>

#include "capacitymonitor.h"
#include "qtservice.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

QMutex mutex;
void customMessageHandler(QtMsgType type, const char *msg)
{
    QMutexLocker locker(&mutex);
    QString txt;
    QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    switch (type) {
    //调试信息提示
     case QtDebugMsg:
             txt = QString("%1 Debug: %2").arg(datatime).arg(msg);
             break;

     //一般的warning提示
     case QtWarningMsg:
             txt = QString("%1 Warning: %2").arg(datatime).arg(msg);
     break;
     //严重错误提示
     case QtCriticalMsg:
             txt = QString("%1 Critical: %2").arg(datatime).arg(msg);
     break;
     //致命错误提示
     case QtFatalMsg:
             txt = QString("%1 Fatal: %2").arg(datatime).arg(msg);
             abort();
     }
     QDateTime now = QDateTime::currentDateTime();
     QString filepath = qApp->applicationDirPath() + QString("/log%1.txt").arg(now.toString("yyyyMMdd"));
     QFile outFile(filepath);
     outFile.open(QIODevice::WriteOnly | QIODevice::Append);
     QTextStream ts(&outFile);
     ts << txt << endl;
}

class QtCapacityMonitorService : public QtService<QCoreApplication>
{
public:
    QtCapacityMonitorService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, "Capacity Monitor Service")
    {
        setServiceDescription("Capacity Monitor Service service implemented with Qt");
        setServiceFlags(QtServiceBase::CanBeSuspended);
    }

protected:
    void start()
    {
        QCoreApplication *app = application();
        qInstallMsgHandler(customMessageHandler);

        SingletonConfig->initConfigFile(app->applicationDirPath() + "/sysconfig.ini");

        SingletonDBHelper->open(SingletonConfig->getDbIp(), SingletonConfig->getDbPort(), \
                                SingletonConfig->getDbName(), SingletonConfig->getDbUser(), SingletonConfig->getDbPasswd());


        CapacityMonitor *capacityMonitor = new CapacityMonitor();

    }

    void pause()
    {
//    daemon->pause();
    }

    void resume()
    {
//    daemon->resume();
    }

private:
//    HttpDaemon *daemon;
};


int main(int argc, char **argv)
{
#if !defined(Q_OS_WIN)
    // QtService stores service settings in SystemScope, which normally require root privileges.
    // To allow testing this example as non-root, we change the directory of the SystemScope settings file.
    QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, QDir::tempPath());
    qWarning("(Example uses dummy settings file: %s/QtSoftware.conf)", QDir::tempPath().toLatin1().constData());
#endif
//    QtCapacityMonitorService service(argc, argv);


//    return service.exec();
    QCoreApplication a(argc, argv);

    SingletonConfig->initConfigFile(a.applicationDirPath() + "/sysconfig.ini");

    SingletonDBHelper->open(SingletonConfig->getDbIp(), SingletonConfig->getDbPort(), \
                            SingletonConfig->getDbName(), SingletonConfig->getDbUser(), SingletonConfig->getDbPasswd());


	//qInstallMsgHandler(customMessageHandler);

    CapacityMonitor *capacityMonitor = new CapacityMonitor();

    return a.exec();
}
