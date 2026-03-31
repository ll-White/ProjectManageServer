#include <QtNetwork/QHostAddress>
#include <QtNetwork/QHostInfo>
#include <QDir>
#include <QNetworkConfigurationManager>

#include "ShareHeader.h"
#include "WnTcpClient.h"

//全局-锁
QMutex  g_mutexlock;

QString GetLocalIp(QString strIpComp)
{
	QString strIp = "127.0.0.1";
	QHostInfo hostInfo = QHostInfo::fromName(QHostInfo::localHostName());
	foreach(QHostAddress address, hostInfo.addresses())
	{
		if (address.protocol() == QAbstractSocket::IPv4Protocol)
		{
			if (address.toString().contains(strIpComp))
			{
				strIp = address.toString();
				break;
			}
		}
	}
	return strIp;
}

QList<QString> GetLocalIPs()
{
	QList<QString> lstIP;
	lstIP.push_back("127.0.0.1");
	QHostInfo hostInfo = QHostInfo::fromName(QHostInfo::localHostName());
	foreach(QHostAddress address, hostInfo.addresses())
	{
		if (address.protocol() == QAbstractSocket::IPv4Protocol)
			lstIP.push_back(address.toString());
	}
	return lstIP;
}

void initGlobalData()
{
	//WnSqlQueryEx query;
	////设备类型
	//g_DeviceType.clear();
	//QString strSql = QString("select * from %1").arg(TB_DEVICETYPE);
	//if (!query.exec(strSql))
	//	qInfo() << "设备类型数据查询失败!原因:" << query.getLastError();
	//g_DeviceType.push_back("未知");
	//while (query.next())
	//	g_DeviceType.push_back(query.value("TypeName").toString());

	////设备状态
	//g_DeviceStatus.clear();
	//strSql = QString("select * from %1").arg(TB_DEVICESTATUS);
	//if (!query.exec(strSql))
	//	qInfo() << "设备状态数据查询失败!原因:" << query.getLastError();
	//g_DeviceStatus.push_back("未知");
	//while (query.next())
	//	g_DeviceStatus.push_back(query.value("StatusName").toString());

	////任务类型
	//g_TaskType.clear();
	//strSql = QString("select * from %1").arg(TB_TASKTYPE);
	//if (!query.exec(strSql))
	//	qInfo() << "任务类型数据查询失败!原因:" << query.getLastError();
	//g_TaskType.push_back("未知");
	//while (query.next())
	//	g_TaskType.push_back(query.value("TypeName").toString());

	////任务状态
	//g_TaskStatus.clear();
	//strSql = QString("select * from %1").arg(TB_TASKSTATUS);
	//if (!query.exec(strSql))
	//	qInfo() << "任务状态数据查询失败!原因:" << query.getLastError();
	//g_TaskStatus.push_back("未知");
	//while (query.next())
	//	g_TaskStatus.push_back(query.value("StatusName").toString());

	//g_QuTypeMode = { "未知", "正常", "开堆" };

	QMap<int, QString> g_MapTaskStatus{ { -1, "终止" },{ 0, "等待" },{ 1, "调车" },{ 2, "运行" },{ 3, "暂停" },{ 4, "完成" } };
}

int RGB2Int(int * rgb)
{
	return rgb[2] << 16 | rgb[1] << 8 | rgb[0];
}

void Int2RGB(int & nRGB, int * RGB)
{
	RGB[0] = (nRGB & 0x0000FF);
	RGB[1] = (nRGB & 0x00FF00) >> 8;
	RGB[2] = (nRGB & 0xFF0000) >> 16;
}