#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include "Shareheader.h"

class WnTCPServer : public QObject
{
	Q_OBJECT
public:
	WnTCPServer(int nPort = 7788);
	~WnTCPServer();

	inline QList<QTcpSocket*>& getAllClient() { return m_ListTcpClient; };
	void ServerListen(int nPort);
	void setDisConnect(QString strIp,int nPort);
public slots:
	void SlotNewConnection();
	void SlotDisconnected();
	void slotReadyRead();
	void slotExecFuncFinfish(QString,QString); 
private:
	//数据解析
	bool JsonAnalysis(QString, QTcpSocket*);
signals:
	//发送新链接
	void sendNewConnect(QString);
	//发送断开连接
	void sendDisConnect(QString);

private:
	int m_ListenPort;		//监听端口
	QList<QTcpSocket*> m_ListTcpClient;					//当前监听的Socket
	QTcpServer * m_TcpServer;		//Qt TCPServer
	bool m_bServerState;			//TCP服务监听状态

	QMap<QTcpSocket *, qint64> m_mapSocketHear;		//心跳时间记录（现已弃用）
};