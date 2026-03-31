#include <QHostInfo>
#include <QTimer>
#include <QApplication>
#include <QThreadPool>

#include "WnTCPServer.h"
#include "WnLog.h"
#include "TcpRunable.h"

WnTCPServer::WnTCPServer(int nPort)
{
	m_ListenPort = nPort;
	m_TcpServer = new QTcpServer(this);
	connect(m_TcpServer, SIGNAL(newConnection()), this, SLOT(SlotNewConnection()));
	m_bServerState = false;

	//m_pCheckConnect = new QTimer;
	//connect(m_pCheckConnect, &QTimer::timeout, this, &WnTCPServer::slotCheckConnectState);
	//m_pCheckConnect->start(1000);

	//m_pCheckDisConnect = new QTimer;
	//connect(m_pCheckDisConnect, &QTimer::timeout, this, &WnTCPServer::slotCheckDisConnect);
}

WnTCPServer::~WnTCPServer()
{
	for (int i = 0; i < m_ListTcpClient.size(); i++)
	{
		m_ListTcpClient.at(i)->disconnect();
		delete m_ListTcpClient.at(i);
		m_ListTcpClient[i] = nullptr;
	}
	m_ListTcpClient.clear();
}

void WnTCPServer::slotExecFuncFinfish(QString strFun,QString)
{
	////执行完成，发送回去
	//if (m_mapReturnSocker.contains(strFun))
	//{
	//	QString strMsg = QString("FINISH:").append(strFun).append(CmdEnd);
	//	m_mapReturnSocker.value(strFun)->write(strMsg.toUtf8());
	//	m_mapReturnSocker.remove(strFun);
	//}
}

void WnTCPServer::SlotNewConnection()
{
	QTcpSocket * pSocket = m_TcpServer->nextPendingConnection();
	m_ListTcpClient.append(pSocket);
	m_mapSocketHear.insert(pSocket, QDateTime::currentDateTime().toSecsSinceEpoch());
	connect(pSocket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
	connect(pSocket, SIGNAL(disconnected()), this, SLOT(SlotDisconnected()));

	QString strIP = pSocket->peerAddress().toString().split("::ffff:")[1];
	int nPort = pSocket->peerPort();

	QString strLog = QString("新连接:%1:%2").arg(strIP).arg(nPort);
	WnLog::setShowLog(strLog);

	emit sendNewConnect(QString("%1:%2").arg(strIP).arg(nPort));

	//断开连接 3秒检测
	//if (m_mapDisConnect.contains(strIP))
	//	m_mapDisConnect.remove(strIP);
	//if (m_mapDisConnect.size() == 0)
	//	m_pCheckDisConnect->stop();
}

void WnTCPServer::SlotDisconnected()
{
	//由于disconnected信号并未提供SocketDescriptor，所以需要遍历寻找
	QTcpSocket * pSocker = dynamic_cast<QTcpSocket*>(sender());
	if (pSocker == NULL)
		return;

	QString strIP = pSocker->peerAddress().toString().split("::ffff:")[1];
	int nPort = pSocker->peerPort();

	m_mapSocketHear.remove(pSocker);
	m_ListTcpClient.removeOne(pSocker);

	WnLog::setShowLog(QString("工作台:%1:%2 主动断开连接!").arg(strIP).arg(nPort));
	pSocker->deleteLater();

	emit sendDisConnect(strIP);

	//m_pCheckDisConnect->start(300);
	//m_mapDisConnect[strIP] = QDateTime::currentDateTime().toMSecsSinceEpoch();
	//m_mapDisConnect.remove(strIP);
}

void WnTCPServer::slotReadyRead()
{
	QTcpSocket * pSocket = dynamic_cast<QTcpSocket*>(sender());
	if (pSocket == NULL)
		return;
	
	TcpRunable * pRunable = new TcpRunable(pSocket);
	pRunable->setAutoDelete(true);
	QThreadPool::globalInstance()->start(pRunable);

	//QByteArray _data = pSocket->readAll();
	//QString strIp = pSocket->peerAddress().toString().split("::ffff:")[1];
	//WnLog::setShowLog(QString("获取IP:%1的命令").arg(strIp));
	//JsonAnalysis(_data, pSocket);

	//QStringList lstBuf = _data.split(";");
	//bool bSuc = false;
	//for (int i = 0; i < lstBuf.size(); i++)
	//{
	//	if (lstBuf.at(i).isEmpty()) continue;
	//	m_ReceiverData[strIp].push_back(lstBuf.at(i));
	//	//解析数据
	//	bSuc = JsonAnalysis(lstBuf.at(i), pSocker);
	//}
	//m_mapData[pSocker]->clear();
	//if (bSuc)
	//{
	//	QString strReturn = QString("{\"stat\":\"%1\"}").arg((int)bSuc);
	//	pSocker->write(strReturn.toUtf8());
	//}
}

void WnTCPServer::ServerListen(int nPort)
{
	if (m_ListenPort != nPort)
	{
		m_ListenPort = nPort;
		m_TcpServer->close();
	}
	m_bServerState = m_TcpServer->listen(QHostAddress::Any, m_ListenPort);
	if (!m_bServerState)
	{
		QMessageBox::information(nullptr,"提示",QString("TCP服务启动失败!端口号:%1 程序退出!").arg(m_ListenPort));
		qInfo() << "服务监听启动失败! Port:" << m_ListenPort;
		QTimer * pTimer = new QTimer;
		connect(pTimer,&QTimer::timeout, [=]{ qApp->quit(); });
		pTimer->start(1000);
	}
}

void WnTCPServer::setDisConnect(QString strIp, int nPort)
{
	//获取Socket指针
	QTcpSocket * _pTcpSocket = nullptr;
	for (int i = 0; i < m_ListTcpClient.size(); i++)
	{
		QString _strIp = m_ListTcpClient.at(i)->peerAddress().toString().split("::ffff:")[1];
		int _nPort = m_ListTcpClient.at(i)->peerPort();

		if (_strIp == strIp && _nPort == nPort)
		{
			_pTcpSocket = m_ListTcpClient.at(i);
			break;
		}
	}

	if (_pTcpSocket == nullptr) 
	{
		WnLog::setShowLog(QString("主动断开客户端连接时,未找到Soket IP:%1 Port:%2").arg(strIp).arg(nPort));
		return;
	}

	//m_mapSocketHear.remove(_pTcpSocket);
	//m_ListTcpClient.removeOne(_pTcpSocket);
	//m_mapData.remove(_pTcpSocket);
	//m_mapReturnSocker.remove(strIp);

	WnLog::setShowLog(QString("主动断开客户端连接 IP:%1 Port:%2").arg(strIp).arg(nPort));
	_pTcpSocket->close();
	_pTcpSocket->abort();
}

bool WnTCPServer::JsonAnalysis(QString strCmd, QTcpSocket * pSocket)
{
	//QJsonParseError parseJsonErr;
	//QJsonDocument jsonDoc = QJsonDocument::fromJson(strCmd.toUtf8(), &parseJsonErr);
	//if (!parseJsonErr.error == QJsonParseError::NoError)
	//{
	//	WnLog::setShowLog(QString("解析json错误！json:%1").arg(strCmd));
	//	return false;
	//}
	//QJsonObject jsonObj = jsonDoc.object();
	//QStringList lstKey = jsonObj.keys();

	//if (!jsonObj.contains("TYPE"))
	//{
	//	WnLog::setShowLog(QString("Json命令错误,无命令类型! Json:%1").arg(strCmd));
	//	return false;
	//}

	////任务类型
	//Cmd_Type nType = (Cmd_Type)jsonObj.value("TYPE").toInt();
	//switch (nType)
	//{
	//case UserInfoCheck:
	//{
	//	Cmd_CheckUserInfo _info;
	//	_info.getDataFromJson(strCmd);
	//	_info.nCheckVal = 1;
	//	QString strVal = _info.getCmdJson();
	//	strVal.append(CmdEnd);
	//	int nWrite = pSocket->write(strVal.toUtf8());
	//	pSocket->waitForBytesWritten(500);

	//	WnLog::setShowLog(QString("获取到用户检测指令并返回"));
	//}
	//break;
	//default:
	//	break;
	//}

	//switch (nType)
	//{
	//case cmdPLC:
	//{
	//	QString sCmd = jsonObj.value("DATA").toString();
	//	RecordLog(QString("获取PLC命令:%1").arg(sCmd));
	//	emit sendCmdToPLC(sCmd,true);
	//}
	//break;
	//case cmdNewTask:
	//{
	//	QJsonDocument jsonTaskDoc;
	//	QJsonObject jsonObjTask = jsonObj.value("DATA").toObject();
	//	jsonTaskDoc.setObject(jsonObjTask);
	//	QString jsonData = jsonTaskDoc.toJson();
	//	qInfo() << "--------------新任务"<< jsonData;
	//	WnRectTaskinfo task;
	//	task.JsonToData(jsonData);

	//	RecordLog(QString("获取到新%1任务，设备:%2     开始点：%3，结束点：%4").arg(g_lisTaskType[task.TaskType]).arg(task.DeviceNo).arg(task.StartPoint).arg(task.EndPoint));
	//	emit sendNewTask(task);
	//}
	//break;
	//case cmdData:
	//{
	//	QString sFunName = jsonObj.value("FUNNAME").toString();
	//	QJsonDocument docs;
	//	docs.setObject(jsonObj.value("DATA").toObject());
	//	emit sendDealWithData(sFunName, docs.toJson());
	//	//记录传入的Soket 指针 和执行的方法 用于外部执行完成后发生信号回来接收后，返回完成信息到对应的工作站中
	//	m_mapReturnSocker.insert(sFunName, pSocket);
	//}
	//break;
	//case cmdControlDevice://控制设备
	//{
	//	QString sCmd = jsonObj.value("DATA").toString();
	//	QString strControlIP = "";
	//	bool bControl = sCmd.toInt();
	//	if (bControl)
	//		strControlIP = pSocket->peerAddress().toString().split("::ffff:")[1];
	//	//通知主界面保存到数据库并写入Redis
	//	RecordLog(QString("控制设备改变，新IP:").append(strControlIP));
	//	emit sendChangeControl(strControlIP);
	//}
	//break;
	//case cmdUpdateYardNo:	//修改料场名称
	//{
	//	QString sYardNo = jsonObj.value("DATA").toString();
	//	RecordLog("更新料场:" + sYardNo);
	//	emit sendUpdateYardNo(sYardNo);
	//}
	//break;
	//case  cmdUpdateTask:     //修改任务
	//{
	//	QString strCmd = jsonObj.value("DATA").toString();
	//	QStringList lstData = strCmd.split(":");
	//	if (lstData.size() == 2)
	//		emit sendUpdateTask(lstData.at(0).toInt(), lstData.at(1).toInt());
	//}
	//break;
	//case cmdParam:
	//{
	//	QString sCmd = jsonObj.value("DATA").toString();
	//	RecordLog(QString("获取参数命令:%1").arg(sCmd));
	//	emit sendParmCmd(sCmd);
	//}
	//break;
	//case cmdHeart:
	//{
	//	m_mapSocketHear[pSocket] = QDateTime::currentDateTime().toSecsSinceEpoch();
	//}
	//break;
	//case cmdHandoffPLC: //修改PLC地址
	//{
	//	//QJsonObject jObjDate = jsonObj.value("DATA").toObject();
	//	//QString DeviceNo = jObjDate.value("DEVICENO").toString();
	//	//QString IP = jObjDate.value("IP").toString();
	//	//emit sendHandoffPLC(DeviceNo, IP);

	//	QString DeviceName = jsonObj.value("NAME").toString();
	//	QString IP = jsonObj.value("IP").toString();
	//	emit sendHandoffPLC(DeviceName, IP);
	//}
	//break;
	//default:
	//	return false;
	//}
	return true;
}
