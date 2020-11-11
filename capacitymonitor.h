#ifndef CAPACITYMONITOR_H
#define CAPACITYMONITOR_H

#include <QObject>
#include <QtCore>
#include <QTimer>
#include <QDebug>
#include <QMutexLocker>


#define __STDC_FORMAT_MACROS

#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include "CapacityMonitor_types.h"
#include "CapacityMonitorService.h"
#include "CapacityMonitor_constants.h"

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <thrift/cxxfunctional.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;


#include "databasehelper.h"
#include "rtdbapp.h"

#include "LeastSquaresCurve.h"
#include "LeastSquareSurface.h"

class CapacityMonitor : public QObject
{
    Q_OBJECT
public:
    explicit CapacityMonitor(QObject *parent = 0);
    
    void timerEvent(QTimerEvent *);

    void startCalcModel(stCalcModel calcModel);
	
	int GetHisValue(Tag* pTags,long startTime,long endTime, QString pubIndexCode,int iStep);

    QMap<int, double> calcFhAppearValue(QMap<int, QList<stPointInfo> > mapListPointInfo,QMap<QString, stTfnlCondtion> mapTfnlCondtion);

    list<INPUT_POINT> calcInputPoint(QMap<int, QList<stPointInfo> > mapListPointInfo);
	 
    list<INPUT_POINT3D> calcInputPoint3D(QMap<int, QList<stPointInfo> > mapListPointInfo);
signals:
    
public slots:
    
private:
    int queryModelTimerId_;
	QMutex mutex_;
};

#endif // CAPACITYMONITOR_H
