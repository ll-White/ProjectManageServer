#include "WnShareData.h"

QMutex WnShareData::s_Mutex;

//设备信息
//QMap<QString, WnDeviceBaseInfo> WnShareData::m_mapDeviceBaseInfo;


//void WnShareData::setDeviceBaseInfo(QMap<QString, WnDeviceBaseInfo> & _mapData)
//{
//	s_Mutex.lock();
//	m_mapDeviceBaseInfo = _mapData;
//	s_Mutex.unlock();
//}
//
//WnDeviceBaseInfo WnShareData::getDeviceBaseInfo(QString strDevice)
//{
//	WnDeviceBaseInfo _info;
//	s_Mutex.lock();
//	_info = m_mapDeviceBaseInfo[strDevice];
//	s_Mutex.unlock();
//	return _info;
//}
//
//QMap<QString, WnDeviceBaseInfo> WnShareData::getDeviceBaseInfo()
//{
//	QMutexLocker locker(&s_Mutex);
//	return m_mapDeviceBaseInfo;
//}
