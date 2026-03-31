#pragma once

#include <QObject>
#include <QTcpSocket>

#include "Shareheader.h"

#define CmdEnd "#End!"

class WnTcpClient : public QObject
{
	Q_OBJECT

public:
	static WnTcpClient * getInstance();

	WnTcpClient(QObject *parent = nullptr);
	~WnTcpClient();

	//设置服务端地址
	void setServerAddres(QString strIp, int nPort);
	//发送Tcp命令-异步
	int sendTcpAsync(QString);
	//发送Tcp命令-同步步
	QString sendTcpSync(QString, int nWaitBack = 500);
	//连接到服务器
	void connectToHost(QString ip, int port);
	//重连服务器
	void reConnect() { connectToHost(m_strIP, m_nPort); };
	//断开连接服务器
	void disConnect();
signals:
	void sendRecvData(QString);						//信号 - 发送获取到的数据

private slots:
	void slotOnConnect();									//槽函数 - 服务器连接成功
	void slotOnDisConnect();								//槽函数 - 服务器断开连接
	void slotOnBytesWritten(qint64);						//槽函数 - 写入信息
	void slotOnRead();										//槽函数 - 获取到信息
	void slotSocketError(QAbstractSocket::SocketError);		//槽函数 - TCP故障信息
	void slotTCPStateChanged(QAbstractSocket::SocketState);	//槽函数 - TCP状态信息改变

private:
	//TCP-Socket 指针
	QTcpSocket * m_pTcpSocket;
	//TCP服务端IP地址
	QString			m_strIP;
	//TCP服务端断开
	int				m_nPort;
	//TCP服务端是否连接
	bool			m_bConnect;
	//读取到的数据
	QByteArray		m_RecvData;

	static WnTcpClient * s_TcpClient;
};
