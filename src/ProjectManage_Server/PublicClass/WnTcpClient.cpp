#include <QJsonObject>
#include <QJsonParseError>
#include "WnTcpClient.h"
#include "ShareStructServer.h"

WnTcpClient::WnTcpClient(QObject *parent)
	: QObject(parent)
{
	m_pTcpSocket = new QTcpSocket(this);
	connect(m_pTcpSocket, &QTcpSocket::connected, this, &WnTcpClient::slotOnConnect);
	connect(m_pTcpSocket, &QTcpSocket::disconnected, this, &WnTcpClient::slotOnDisConnect);
	connect(m_pTcpSocket, &QTcpSocket::bytesWritten, this, &WnTcpClient::slotOnBytesWritten);
	connect(m_pTcpSocket, &QTcpSocket::readyRead, this, &WnTcpClient::slotOnRead);
	connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError(QAbstractSocket::SocketError)));
	connect(m_pTcpSocket, &QTcpSocket::stateChanged, this, &WnTcpClient::slotTCPStateChanged);
}

WnTcpClient::~WnTcpClient()
{
}

void WnTcpClient::setServerAddres(QString strIp, int nPort)
{
	m_strIP = strIp;
	m_nPort = nPort;
	m_bConnect = false;
	connectToHost(strIp,nPort);
}

int WnTcpClient::sendTcpAsync(QString strCmd)
{
	if (!m_bConnect)
		reConnect();
	return m_pTcpSocket->write(strCmd.toLatin1());
}

QString WnTcpClient::sendTcpSync(QString, int nWaitBack /*= 500*/)
{
	//if (!m_bConnect)
	//{
	//	reConnect();
	//}

	//QString strFeedBack = "";
	//cmdVal.nWaitBack = nWaitBack;
	//int nID = sendTCPCmd(cmdVal, true);
	//if (nID == -1) return strFeedBack;
	//qint64 tStart = QDateTime::currentDateTime().toMSecsSinceEpoch();
	//while (QDateTime::currentDateTime().toMSecsSinceEpoch() - tStart < cmdVal.nWaitBack)
	//{
	//	QEventLoop loop;
	//	QTimer::singleShot(50, &loop, SLOT(quit()));
	//	loop.exec();

	//	if (m_mapFeedBack.contains(nID))
	//	{
	//		strFeedBack = m_mapFeedBack.value(nID);
	//		break;
	//	}
	//}
	//return strFeedBack;
	return "";
}

void WnTcpClient::slotOnConnect()
{
	int i = 0;
}

void WnTcpClient::slotOnDisConnect()
{
	int i = 0;
}

void WnTcpClient::slotOnBytesWritten(qint64 nWirteLen)
{
	int i = 0;
}

void WnTcpClient::slotOnRead()
{
	QByteArray data = m_pTcpSocket->readAll();
	if (!data.contains(CmdEnd))
	{
		m_RecvData.append(data);
		return;
	}
	QString rStr = QString::fromLocal8Bit(m_RecvData);
	emit sendRecvData(rStr);
}

void WnTcpClient::slotSocketError(QAbstractSocket::SocketError)
{

}

void WnTcpClient::slotTCPStateChanged(QAbstractSocket::SocketState)
{

}

void WnTcpClient::connectToHost(QString ip, int port)
{
	if (m_bConnect)
	{
		if (ip == m_strIP && port == m_nPort)
			return;
		else
			disConnect();
	}

	m_strIP = ip;
	m_nPort = port;

	m_pTcpSocket->connectToHost(m_strIP, m_nPort);
	//if (!m_pTcpClient->waitForConnected(50))
	//{
	//	QString strError = QString("TCP服务连接失败!IP:%1,Port:%2 原因:%3").arg(m_strIP).arg(m_nPort).arg(m_pTcpClient->errorString());
	//	qInfo() << strError;
	//	m_bConnect = false;
	//}
	//else
	//{
	//	m_bConnect = true;
	//}
	//return m_bConnect;
}

void WnTcpClient::disConnect()
{

}