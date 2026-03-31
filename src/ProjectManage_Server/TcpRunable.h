#pragma once

#include <QRunnable>
#include "ShareStructServer.h"

class QTcpSocket;
class TcpRunable : public QRunnable
{

public:
	TcpRunable(QTcpSocket *);
	~TcpRunable();

protected:
	void run() override;
	//MD5加密
	QString EncryptMD5(QString strPwd);

protected:
	//用户登录校验
	bool RequestUserLogin(QString strJson, QTcpSocket * pSocket);
	//用户信息CURD
	bool RequestUserInfoOperator(QString strJson, QTcpSocket * pSocket);
	//用户信息查询团队列表
	bool RequestUserQueryTeam(QString strJson, QTcpSocket * pSocket);
	//用户信息搜索查询
	bool RequestUserSearch(QString strJson, QTcpSocket * pSocket);

	//主页基础数据
	bool RequestIndexBase(QString strJson, QTcpSocket * pSocket);

	//任务信息查询
	bool RequestTaskQuery(QString strJson, QTcpSocket * pSocket);
	//任务信息修改
	bool RequestTaskOperation(QString strJson, QTcpSocket * pSocket);
	//任务动态查询
	bool RequestTaskActivityQuery(QString strJson, QTcpSocket * pSocket);
	//任务动态修改
	bool RequestTaskActivityOperation(QString strJson, QTcpSocket * pSocket);

	//项目信息获取
	bool RequestProjectQuery(QString strJson, QTcpSocket * pSocket);
	//项目信息修改
	bool RequestProjectOperation(QString strJson, QTcpSocket * pSocket);
	//项目信息列表-用户参与的
	bool RequestProjectQueryUser(QString strJson, QTcpSocket * pSocket);
	//项目成员信息获取
	bool RequestProjectMemberQuery(QString strJson, QTcpSocket * pSocket);
	//项目成员信息修改
	bool RequestProjectMemberOperation(QString strJson, QTcpSocket * pSocket);

	//查询团队信息
	bool RequestTeamInfoQuery(QString strJson, QTcpSocket * pSocket);
	//团队信息修改
	bool RequestTeamOperation(QString strJson, QTcpSocket * pSocket);
	//团队成员查询
	bool RequestTeamMemberQuery(QString strJson, QTcpSocket * pSocket);
	//团队成员修改
	bool RequestTeamMemberOperation(QString strJson, QTcpSocket * pSocket);

	//笔记目录
	bool RequestNoteCatalogQuery(QString strJson, QTcpSocket * pSocket);
	//笔记目录修改
	bool RequestNoteCatalogOperation(QString strJson, QTcpSocket * pSocket);
	//笔记信息查询
	bool RequestNoteQuery(QString strJson, QTcpSocket * pSocket);
	//笔记信息修改
	bool RequestNoteOperation(QString strJson, QTcpSocket * pSocket);
	
protected:
	QTcpSocket * m_pCurrentSocket = nullptr;
	static QMap<RequestID, bool (TcpRunable::*)(QString, QTcpSocket*)> s_mapAnalyseFunc;
};
