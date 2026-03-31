#pragma once
#include <QMutex>
#include "Shareheader.h"

class WnShareData
{
public:
	////设备信息
	//static void setDeviceBaseInfo(QMap<QString, WnDeviceBaseInfo> &);
	//static WnDeviceBaseInfo getDeviceBaseInfo(QString);
	//static QMap<QString, WnDeviceBaseInfo> getDeviceBaseInfo();

private:
	static QMutex s_Mutex;

	//设备信息
	//static QMap<QString, WnDeviceBaseInfo> m_mapDeviceBaseInfo;

};
