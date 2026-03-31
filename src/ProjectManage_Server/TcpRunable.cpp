#include <QTcpSocket>
#include <QHostAddress>

#include "TcpRunable.h"
#include "WnLog.h"
#include "SqlQueryEx.h"
#include "Shareheader.h"
#include "ShareStructServer.h"

QMap<RequestID, bool (TcpRunable::*)(QString, QTcpSocket*)> TcpRunable::s_mapAnalyseFunc;

TcpRunable::TcpRunable(QTcpSocket * pSocket)
{
	m_pCurrentSocket = pSocket;

	if (s_mapAnalyseFunc.size() == 0)
	{
		//用户登录校验
		s_mapAnalyseFunc[Request_UserLogin] = &TcpRunable::RequestUserLogin;
		//用户信息CURD
		s_mapAnalyseFunc[Request_UserInfoOperator] = &TcpRunable::RequestUserInfoOperator;
		//用户信息查询团队列表
		s_mapAnalyseFunc[Request_UserQueryTeam] = &TcpRunable::RequestUserQueryTeam;
		//用户信息搜索查询
		s_mapAnalyseFunc[Request_UserSearch] = &TcpRunable::RequestUserSearch;

		//任务信息查询
		s_mapAnalyseFunc[Request_TaskQuery] = &TcpRunable::RequestTaskQuery;
		//任务信息修改
		s_mapAnalyseFunc[Request_TaskOperation] = &TcpRunable::RequestTaskOperation;

		//项目信息-用户参与的
		s_mapAnalyseFunc[Request_ProjectQueryUser] = &TcpRunable::RequestProjectQueryUser;

		//用户登录
		s_mapAnalyseFunc[Request_UserLogin] = &TcpRunable::RequestUserLogin;
		//用户信息修改
		s_mapAnalyseFunc[Request_UserInfoOperator] = &TcpRunable::RequestUserInfoOperator;
		//用户信息查询团队列表
		s_mapAnalyseFunc[Request_UserQueryTeam] = &TcpRunable::RequestUserQueryTeam;
		//用户信息搜索查询
		s_mapAnalyseFunc[Request_UserSearch] = &TcpRunable::RequestUserSearch;

		//主页基础数据
		s_mapAnalyseFunc[Request_IndexBase] = &TcpRunable::RequestIndexBase;

		//任务信息获取
		s_mapAnalyseFunc[Request_TaskQuery] = &TcpRunable::RequestTaskQuery;
		//任务信息修改
		s_mapAnalyseFunc[Request_TaskOperation] = &TcpRunable::RequestTaskOperation;
		//任务动态查询
		s_mapAnalyseFunc[Request_TaskActivityQuery] = &TcpRunable::RequestTaskActivityQuery;
		//任务动态修改
		s_mapAnalyseFunc[Request_TaskActivityOperation] = &TcpRunable::RequestTaskActivityOperation;

		//项目信息获取
		s_mapAnalyseFunc[Request_ProjectQuery] = &TcpRunable::RequestProjectQuery;
		//项目信息修改
		s_mapAnalyseFunc[Request_ProjectOperation] = &TcpRunable::RequestProjectOperation;
		//项目信息-用户参与的
		s_mapAnalyseFunc[Request_ProjectQueryUser] = &TcpRunable::RequestProjectQueryUser;

		//项目成员信息获取
		s_mapAnalyseFunc[Request_ProjectMemberQuery] = &TcpRunable::RequestProjectMemberQuery;
		//项目成员信息修改
		s_mapAnalyseFunc[Request_ProjectMemberOperation] = &TcpRunable::RequestProjectMemberOperation;

		//团队信息查询
		s_mapAnalyseFunc[Request_TeamInfoQuery] = &TcpRunable::RequestTeamInfoQuery;
		//团队信息修改
		s_mapAnalyseFunc[Request_TeamOperation] = &TcpRunable::RequestTeamOperation;
		//团队成员查询
		s_mapAnalyseFunc[Request_TeamMemberQuery] = &TcpRunable::RequestTeamMemberQuery;
		//团队成员修改
		s_mapAnalyseFunc[Request_TeamMemberOperation] = &TcpRunable::RequestTeamMemberOperation;

		//笔记目录
		s_mapAnalyseFunc[Request_NoteCatalogQuery] = &TcpRunable::RequestNoteCatalogQuery;
		//笔记目录修改
		s_mapAnalyseFunc[Request_NoteCatalogOperation] = &TcpRunable::RequestNoteCatalogOperation;
		//笔记信息查询
		s_mapAnalyseFunc[Request_NoteQuery] = &TcpRunable::RequestNoteQuery;
		//笔记信息修改
		s_mapAnalyseFunc[Request_NoteOperation] = &TcpRunable::RequestNoteOperation;
	}
}

TcpRunable::~TcpRunable()
{
}

void TcpRunable::run()
{
	if (m_pCurrentSocket == nullptr) return;
	QByteArray RecvData;
	for (;;)
	{
		QByteArray _recv = m_pCurrentSocket->readAll();
		if (_recv.isEmpty())
		{
			WnLog::setShowLog(QString("获取到空数据!"));
			return;
		}

		RecvData.append(_recv);
		if (RecvData.right(6) == TcpEnd) break;
	}
	QString strIp = m_pCurrentSocket->peerAddress().toString().split("::ffff:")[1];

	QString strCmd = QString::fromUtf8(RecvData);
	WnLog::setShowLog(QString("获取IP:%1的命令:%2").arg(strIp).arg(strCmd));
	QList<QString> listCmd = strCmd.split(TcpEnd);
	for (QString _CmdVal : listCmd)
	{
		if (_CmdVal.isEmpty()) continue;
		//任务类型
		RequestID _CmdID = (RequestID)_CmdVal.left(4).toInt();
		_CmdVal = _CmdVal.remove(0, 4);
		if (s_mapAnalyseFunc.contains(_CmdID))
		{
			(this->*s_mapAnalyseFunc[_CmdID])(_CmdVal, m_pCurrentSocket);
		}
	}
}

QString TcpRunable::EncryptMD5(QString strPwd)
{
	QCryptographicHash hash(QCryptographicHash::Md5);                               // 使用md5加密
	hash.addData(strPwd.toStdString().c_str(), strPwd.length());        // 添加数据
	QByteArray array = hash.result();           // 获取MD5加密后的数据
	QString strNewPwd = array.toHex();          // 转十六进制
	strNewPwd = strNewPwd.toLower();
	return strNewPwd;
}

bool TcpRunable::RequestUserLogin(QString strJson, QTcpSocket* pSocket)
{
	RequestCmd_UserLogin RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strPwd_MD5 = EncryptMD5(RequestCmd.sLoginUser.strPwd);
	QString strSql = QString("select * from %1 where UserID = '%2'").arg(TB_USERTABLE).arg(RequestCmd.sLoginUser.strUserID);
	if (!query.exec(strSql))
	{
		QString strError = query.getLastError();
		RequestCmd.nValidationStatus = ValidationFailure;
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
	}
	RequestCmd.nRet = 1;
	if (query.next())
	{
		//先检验密码
		QString strPwd = query.value("UserPwd").toString();
		if (strPwd != strPwd_MD5)
		{
			RequestCmd.nValidationStatus = IncorrectPassword;
		}
		else
		{
			RequestCmd.nValidationStatus = ValidationSuccessful;

			RequestCmd.sLoginUser.strUserID = query.value("UserID").toString();			//用户ID
			RequestCmd.sLoginUser.strUserName = query.value("UserName").toString();		//用户名称
			RequestCmd.sLoginUser.nUserPermission = (UserPermission)query.value("UserType").toInt();	//用户权限
			RequestCmd.sLoginUser.nUserGender = (UserGender)query.value("UserGender").toInt();		//用户性别
			RequestCmd.sLoginUser.strUserPhoneNumber = query.value("UserPhoneNumber").toString();	//用户电话号码
			RequestCmd.sLoginUser.dtLoginTime = QDateTime::currentDateTime();		//登录时间
		}
	}
	else
	{
		RequestCmd.nValidationStatus = UserNoExist;
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

//用户信息CURD
bool TcpRunable::RequestUserInfoOperator(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_UserInfoOperator RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strSql;
	if (RequestCmd.nOperationTypeType == Operation_Insert)
	{
		//用户密码
		QString strPwd = EncryptMD5(RequestCmd.sUserInfo.strPwd);
		strSql = QString("insert into %1(UserID,UserPwd,UserName,UserGender,UserPhoneNumber)values('%2','%3','%4','%5','%6')")
			.arg(TB_USERTABLE).arg(RequestCmd.sUserInfo.strUserID).arg(strPwd).arg(RequestCmd.sUserInfo.strUserName).arg(RequestCmd.sUserInfo.nUserGender).arg(RequestCmd.sUserInfo.strUserPhoneNumber);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Update)
	{
		//用户密码
		QString strPwd = EncryptMD5(RequestCmd.sUserInfo.strPwd);
		strSql = QString("update %1 set UserPwd = '%2',UserName = '%3',UserGender = '%4',UserPhoneNumber = '%5' where UserID = '%6'")
			.arg(TB_USERTABLE).arg(strPwd).arg(RequestCmd.sUserInfo.strUserName).arg(RequestCmd.sUserInfo.nUserGender).arg(RequestCmd.sUserInfo.strUserPhoneNumber).arg(RequestCmd.sUserInfo.strUserID);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Delete)
	{
		strSql = QString("delete from %1 where UserID = '%2'").arg(TB_USERTABLE).arg(RequestCmd.sUserInfo.strUserID);
	}

	if (!query.exec(strSql))
	{
		RequestCmd.nRet = query.getQuery().lastError().nativeErrorCode().toInt();
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	else
	{
		RequestCmd.nRet = 1;
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	pSocket->waitForBytesWritten(2000);
	return RequestCmd.nRet;
}

//用户信息查询团队列表
bool TcpRunable::RequestUserQueryTeam(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_UserQueryTeam RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	QString strSql = QString("select u.* from %1 u join %2 tm on tm.UserID = u.UserID where tm.TeamID = (select TeamID from %2 where UserID = '%3')").arg(TB_USERTABLE).arg(TB_TEAMMEMBER).arg(RequestCmd.strUserID);
	SqlQueryEx query;
	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	RequestCmd.nRet = 1;
	RequestCmd.listUserInfo.clear();
	while (query.next())
	{
		UserInfo info;
		info.strUserID = query.value("UserID").toString();							//用户ID
		info.strUserName = query.value("UserName").toString();						//用户名称
		info.nUserPermission = (UserPermission)query.value("UserType").toInt();		//用户权限
		info.nUserGender = (UserGender)query.value("UserGender").toInt();			//用户性别
		info.strUserPhoneNumber = query.value("UserPhoneNumber").toString();		//用户电话号码
		RequestCmd.listUserInfo.push_back(info);
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

bool TcpRunable::RequestUserSearch(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_UserSearch RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	QString strSql;
	if (RequestCmd.nExactMatch == 1)
	{
		strSql = QString("select u.*,t.* from %1 u \
						left join %2 tm on u.UserID = tm.UserID \
						left join %3 t on tm.TeamID = t.TeamID \
						where u.UserID = '%2'").arg(TB_USERTABLE).arg(TB_TEAMMEMBER).arg(TB_TEAM).arg(RequestCmd.strSearchText);
	}
	else
	{
		strSql = QString("select u.*,t.* from %1 u \
						left join %2 tm on u.UserID = tm.UserID \
						left join %3 t on tm.TeamID = t.TeamID \
						where u.UserID like '%%4%'").arg(TB_USERTABLE).arg(TB_TEAMMEMBER).arg(TB_TEAM).arg(RequestCmd.strSearchText);
		strSql += QString(" or UserName like '%%1%' ").arg(RequestCmd.strSearchText);
		strSql += QString(" or UserPhoneNumber like '%%1%' ").arg(RequestCmd.strSearchText);
	}
	SqlQueryEx query;
	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	RequestCmd.nRet = 1;
	while (query.next())
	{
		UserAssociatedInfo info;
		info.sUserInfo.strUserID = query.value("UserID").toString();							//用户ID
		info.sUserInfo.strUserName = query.value("UserName").toString();						//用户名称
		info.sUserInfo.nUserPermission = (UserPermission)query.value("UserType").toInt();		//用户权限
		info.sUserInfo.nUserGender = (UserGender)query.value("UserGender").toInt();				//用户性别
		info.sUserInfo.strUserPhoneNumber = query.value("UserPhoneNumber").toString();			//用户电话号码
		info.sTeamInfo.nTeamID = query.value("TeamID").toInt();									//团队编号
		info.sTeamInfo.strTeamName = query.value("TeamName").toString();						//团队名称
		info.sTeamInfo.strTeamDescription = query.value("TeamDescription").toString();			//团队描述
		info.sTeamInfo.dtCreateDate = query.value("TeamCreateTime").toDateTime();				//创建日期
		RequestCmd.listUserAssociatedInfo.push_back(info);
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

//主页基础数据
bool TcpRunable::RequestIndexBase(QString strJson, QTcpSocket * pSocket)
{
	return false;
}

//任务查询
bool TcpRunable::RequestTaskQuery(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_TaskQuery RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	//分析筛选条件
	QString strConditions;
	//用户ID
	if (!RequestCmd.strUserID.isEmpty())
		strConditions.append(QString("and UserExec = '%1'").arg(RequestCmd.strUserID));
	//项目ID
	if (!RequestCmd.strProjectName.isEmpty())
		strConditions.append(QString("and ProjectName = '%1'").arg(RequestCmd.strProjectName));
	//筛选条件-任务状态
	if (RequestCmd.Conditions_Status != 0)
		strConditions.append(QString("and TaskStatus = '%1'").arg(RequestCmd.Conditions_Status));
	//筛选条件-开始时间
	if (RequestCmd.Conditions_DateStart > 0)
		strConditions.append(QString("and StartDate >= '%1'").arg(QDateTime::fromTime_t(RequestCmd.Conditions_DateStart).toString("yyyy-MM-dd hh:mm:ss")));
	//筛选条件-结束结束
	if (RequestCmd.Conditions_DateEnd > 0)
		strConditions.append(QString("and StartDate <= '%1'").arg(QDateTime::fromTime_t(RequestCmd.Conditions_DateEnd).toString("yyyy-MM-dd hh:mm:ss")));
	//筛选条件-任务内容模糊查询
	if (!RequestCmd.Conditions_ContentFuzzyQuery.isEmpty())
	{
		strConditions.append(QString("and TaskContent like '%%1%'").arg(RequestCmd.Conditions_ContentFuzzyQuery));
		strConditions.append(QString("and TaskTitle like '%%1%'").arg(RequestCmd.Conditions_ContentFuzzyQuery));
	}
	// 筛选条件 - 第几页、一页几行

	QString strSql = QString("select count(*) as count from %1 where 1=1 %2").arg(TB_TASK).arg(strConditions);
	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	if (query.next())
	{
		RequestCmd.nCount = query.value("count").toInt();
	}

	//实际数据查询
	int _Page = RequestCmd.Conditions_Page - 1;
	if (_Page < 0)
		_Page = 0;
	int nRowStart = _Page * RequestCmd.Conditions_PageRow;
	int nRowEnd = nRowStart + RequestCmd.Conditions_PageRow;

	strSql = QString("select * from %1 where 1=1 %2 order by CreateDate desc limit %3,%4").arg(TB_TASK).arg(strConditions).arg(nRowStart).arg(nRowEnd);
	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	RequestCmd.nCount = 0;
	RequestCmd.nRet = 1;
	while (query.next())
	{
		TaskInfo _info;
		_info.nTaskID = query.value("TaskID").toInt();								//任务ID
		_info.strProjectName = query.value("ProjectName").toString();			//项目ID
		_info.nTaskStatus = (TaskStatus)query.value("TaskStatus").toInt();			//任务状态
		_info.strTitle = query.value("TaskTitle").toString();						//任务标题
		_info.strContent = query.value("TaskContent").toString();					//任务内容
		_info.dtDateCreate = query.value("CreateDate").toDateTime();				//任务创建日期
		_info.dtDateStart = query.value("StartDate").toDateTime();					//任务开始日期
		_info.dtDateEnd = query.value("EndDate").toDateTime();						//任务结束日期
		_info.strUserExec = query.value("UserExec").toString();						//任务执行人
		_info.strUserCreate = query.value("UserCreate").toString();					//任务创建人
		_info.nTaskPriority = (TaskPriority)query.value("TaskPriority").toInt();	//任务优先级
		_info.fPlannedDuration = query.value("PlannedDuration").toFloat();			//任务规划时长
		_info.fActualDuration = query.value("ActualDuration").toFloat();			//任务实际使用
		_info.fScore = query.value("Score").toFloat();								//任务实际得分
		RequestCmd.listTask.push_back(_info);

		if (RequestCmd.nCount == 0)
			RequestCmd.nCount = query.value("count").toInt();
	}
	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

//任务信息修改
bool TcpRunable::RequestTaskOperation(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_TaskOperation RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strSql;
	if (RequestCmd.nOperationTypeType == Operation_Insert)
	{
		if (RequestCmd.sTaskInfo.nTaskStatus < Task_Completion)
		{
			if (QDateTime::currentDateTime() < RequestCmd.sTaskInfo.dtDateStart)
				RequestCmd.sTaskInfo.nTaskStatus = Task_WaitStart;
			else if (RequestCmd.sTaskInfo.dtDateStart <= QDateTime::currentDateTime() && QDateTime::currentDateTime() < RequestCmd.sTaskInfo.dtDateEnd)
				RequestCmd.sTaskInfo.nTaskStatus = Task_Runing;
			else if (RequestCmd.sTaskInfo.dtDateEnd <= QDateTime::currentDateTime())
				RequestCmd.sTaskInfo.nTaskStatus = Task_TimeOut;
		}

		strSql = QString("insert into %1(ProjectName,TaskStatus,TaskTitle,TaskContent,CreateDate,StartDate,EndDate,UserExec,UserCreate,TaskPriority,PlannedDuration,ActualDuration,Score)values \
			('%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','%12','%13','%14')")
			.arg(TB_TASK).arg(RequestCmd.sTaskInfo.strProjectName).arg(RequestCmd.sTaskInfo.nTaskStatus).arg(RequestCmd.sTaskInfo.strTitle).arg(RequestCmd.sTaskInfo.strContent).arg(RequestCmd.sTaskInfo.dtDateCreate.toString("yyyy-MM-dd hh:mm:ss"))
			.arg(RequestCmd.sTaskInfo.dtDateStart.toString("yyyy-MM-dd hh:mm:ss")).arg(RequestCmd.sTaskInfo.dtDateEnd.toString("yyyy-MM-dd hh:mm:ss")).arg(RequestCmd.sTaskInfo.strUserExec).arg(RequestCmd.sTaskInfo.strUserCreate).arg(RequestCmd.sTaskInfo.nTaskPriority)
			.arg(RequestCmd.sTaskInfo.fPlannedDuration).arg(RequestCmd.sTaskInfo.fActualDuration).arg(RequestCmd.sTaskInfo.fScore);

		if (!query.exec(strSql))
		{
			int i = 0;
			QString strInfo = query.getLastError();
		}

		strSql = QString("select TaskID from %1 where ProjectName = '%2' and TaskTitle = '%3' and CreateDate = '%4' and UserExec = '%5' and UserCreate = '%6'")
			.arg(TB_TASK).arg(RequestCmd.sTaskInfo.strProjectName).arg(RequestCmd.sTaskInfo.strTitle).arg(RequestCmd.sTaskInfo.dtDateCreate.toString("yyyy-MM-dd hh:mm:ss")).arg(RequestCmd.sTaskInfo.strUserExec).arg(RequestCmd.sTaskInfo.strUserCreate);
		if (!query.exec(strSql))
		{
			int i = 0;
			QString strInfo = query.getLastError();
		}
		int nTaskID = 0;
		if (query.next())
		{
			nTaskID = query.value("TaskID").toInt();
		}
		QString strActivityContent = QString("创建了任务");

		strSql = QString("insert into %1(TaskID,ActivityDateTime,ActivityContent,CommentUserID,ActivityType)values('%2','%3','%4','%5','%6');")
			.arg(TB_TASKACTIVITY).arg(nTaskID).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
			.arg(strActivityContent).arg(RequestCmd.sTaskInfo.strUserCreate).arg("0");

	}
	else if (RequestCmd.nOperationTypeType == Operation_Update)
	{
		if (RequestCmd.sTaskInfo.nTaskStatus < Task_Completion)
		{
			if (QDateTime::currentDateTime() < RequestCmd.sTaskInfo.dtDateStart)
				RequestCmd.sTaskInfo.nTaskStatus = Task_WaitStart;
			else if (RequestCmd.sTaskInfo.dtDateStart <= QDateTime::currentDateTime() && QDateTime::currentDateTime() < RequestCmd.sTaskInfo.dtDateEnd)
				RequestCmd.sTaskInfo.nTaskStatus = Task_Runing;
			else if (RequestCmd.sTaskInfo.dtDateEnd <= QDateTime::currentDateTime())
				RequestCmd.sTaskInfo.nTaskStatus = Task_TimeOut;
		}

		strSql = QString("update %1 set ProjectName = '%2',TaskStatus = '%3',TaskTitle = '%4',TaskContent = '%5',CreateDate = '%6',StartDate = '%7',EndDate = '%8',UserExec = '%9',UserCreate = '%10',TaskPriority = '%11', \
			PlannedDuration = '%12',ActualDuration = '%13',Score = '%14' where TaskID = '%15';")
			.arg(TB_TASK).arg(RequestCmd.sTaskInfo.strProjectName).arg(RequestCmd.sTaskInfo.nTaskStatus).arg(RequestCmd.sTaskInfo.strTitle).arg(RequestCmd.sTaskInfo.strContent).arg(RequestCmd.sTaskInfo.dtDateCreate.toString("yyyy-MM-dd hh:mm:ss"))
			.arg(RequestCmd.sTaskInfo.dtDateStart.toString("yyyy-MM-dd hh:mm:ss")).arg(RequestCmd.sTaskInfo.dtDateEnd.toString("yyyy-MM-dd hh:mm:ss")).arg(RequestCmd.sTaskInfo.strUserExec).arg(RequestCmd.sTaskInfo.strUserCreate).arg(RequestCmd.sTaskInfo.nTaskPriority)
			.arg(RequestCmd.sTaskInfo.fPlannedDuration).arg(RequestCmd.sTaskInfo.fActualDuration).arg(RequestCmd.sTaskInfo.fScore).arg(RequestCmd.sTaskInfo.nTaskID);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Delete)
	{
		strSql = QString("delete from %1 where TaskID = '%2';").arg(TB_TASK).arg(RequestCmd.sTaskInfo.nTaskID);
	}

	//修改项目中的任务信息->使用了task表的触发器

	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	else
	{
		RequestCmd.nRet = 1;
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	pSocket->waitForBytesWritten(2000);
	return RequestCmd.nRet;
}

//任务动态查询
bool TcpRunable::RequestTaskActivityQuery(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_TaskActivityQuery RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strSql = QString("select * from %1 where TaskID = %2 order by ActivityDateTime").arg(TB_TASKACTIVITY).arg(RequestCmd.nTaskID);
	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	RequestCmd.nRet = 1;
	RequestCmd.listInfo.clear();
	while (query.next())
	{
		TaskActivity info;
		info.nAutoID = query.value("AutoID").toInt();									//自增ID
		info.nTaskID = query.value("TaskID").toInt();									//任务编号
		info.dtActivityDateTime = query.value("ActivityDateTime").toDateTime();			//时间
		info.strActivityContent = query.value("ActivityContent").toString();			//内容
		info.strCommentUserID = query.value("CommentUserID").toString();				//评论人ID
		info.nActivityType = query.value("ActivityType").toInt();						//动态类型 0=状态修改 1=用户评论
		RequestCmd.listInfo.push_back(info);
	}
	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

//任务动态修改
bool TcpRunable::RequestTaskActivityOperation(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_TaskActivityOperation RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strSql;
	if (RequestCmd.nOperationTypeType == Operation_Insert)
	{
		strSql = QString("insert into %1(TaskID,ActivityDateTime,ActivityContent,CommentUserID,ActivityType)values('%2','%3','%4','%5','%6')")
			.arg(TB_TASKACTIVITY).arg(RequestCmd.sTaskActivityInfo.nTaskID).arg(RequestCmd.sTaskActivityInfo.dtActivityDateTime.toString("yyyy-MM-dd hh:mm:ss"))
			.arg(RequestCmd.sTaskActivityInfo.strActivityContent).arg(RequestCmd.sTaskActivityInfo.strCommentUserID).arg(RequestCmd.sTaskActivityInfo.nActivityType);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Update)
	{
		strSql = QString("update %1 set ActivityDateTime = '%2',ActivityContent = '%3',CommentUserID = '%4',ActivityType = '%5' where AutoID = '%6'")
			.arg(TB_TASKACTIVITY).arg(RequestCmd.sTaskActivityInfo.dtActivityDateTime.toString("yyyy-MM-dd hh:mm:ss"))
			.arg(RequestCmd.sTaskActivityInfo.strActivityContent).arg(RequestCmd.sTaskActivityInfo.strCommentUserID).arg(RequestCmd.sTaskActivityInfo.nActivityType).arg(RequestCmd.sTaskActivityInfo.nAutoID);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Delete)
	{
		strSql = QString("delete from %1 where AutoID = '%2'").arg(TB_TASKACTIVITY).arg(RequestCmd.sTaskActivityInfo.nAutoID);
	}

	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	else
	{
		RequestCmd.nRet = 1;
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	pSocket->waitForBytesWritten(2000);
	return RequestCmd.nRet;
}

//项目信息获取
bool TcpRunable::RequestProjectQuery(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_ProjectQuery RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	QString strSql = QString("select * from %1 where ProjectID = '%2'").arg(TB_PROJECTINFO).arg(RequestCmd.nProjectID);
	SqlQueryEx query;
	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	RequestCmd.nRet = 1;
	if (query.next())
	{
		RequestCmd.info.strProjectName = query.value("ProjectName").toString();							//项目名称
		RequestCmd.info.strProjenctDescription = query.value("ProjenctDescription").toString();			//项目描述
		RequestCmd.info.strManangeUser = query.value("ManangeUser").toString();							//项目负责人		-- 用户表-UserID
		RequestCmd.info.strProjectProgress = query.value("ProjectProgress").toString();					//项目进展说明
		RequestCmd.info.nProjectTask = query.value("ProjectTask").toInt();								//项目任务数
		RequestCmd.info.nProjectTaskCompletion = query.value("ProjectTaskCompletion").toInt();			//项目任务完成数
		RequestCmd.info.nProjectTaskExecution = query.value("ProjectTaskExecution").toInt();			//项目任务执行数
		RequestCmd.info.dtProjectCreateDate = query.value("ProjectCreateDate").toDateTime();			//项目创建时间
		RequestCmd.info.nProjectStatus = (ProjectInfoStatus)query.value("ProjectStatus").toInt();		//项目状态
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

//项目信息修改
bool TcpRunable::RequestProjectOperation(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_ProjectOperation RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strSql;
	if (RequestCmd.nOperationTypeType == Operation_Insert)
	{
		strSql = QString("insert into %1(ProjectName,ProjenctDescription,ManangeUser,ProjectCreateDate,ProjectStatus)values('%2','%3','%4','%5','%6');")
			.arg(TB_PROJECTINFO).arg(RequestCmd.sProjectInfo.strProjectName).arg(RequestCmd.sProjectInfo.strProjenctDescription).arg(RequestCmd.sProjectInfo.strManangeUser)
			.arg(RequestCmd.sProjectInfo.dtProjectCreateDate.toString("yyyy-MM-dd hh:mm:ss")).arg((int)RequestCmd.sProjectInfo.nProjectStatus);
		strSql += QString("insert into %1(ProjectName,UserID,ProjectPosition,JoinDate)values('%2','%3','%4','%5')")
			.arg(TB_PROJECTMEMBER).arg(RequestCmd.sProjectInfo.strProjectName).arg(RequestCmd.sProjectInfo.strManangeUser).arg("1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	}
	else if (RequestCmd.nOperationTypeType == Operation_Update)
	{
		strSql = QString("update %1 set ProjenctDescription = '%2',ManangeUser = '%3',ProjectProgress = '%4',ProjectUrgency = '%5',ProjectCreateDate = '%6',ProjectStatus = '%7' where ProjectName = '%8'")
			.arg(TB_PROJECTINFO).arg(RequestCmd.sProjectInfo.strProjenctDescription).arg(RequestCmd.sProjectInfo.strManangeUser)
			.arg(RequestCmd.sProjectInfo.strProjectProgress).arg(RequestCmd.sProjectInfo.strUrgency).arg(RequestCmd.sProjectInfo.dtProjectCreateDate.toString("yyyy-MM-dd hh:mm:ss"))
			.arg((int)RequestCmd.sProjectInfo.nProjectStatus).arg(RequestCmd.sProjectInfo.strProjectName);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Delete)
	{
		strSql = QString("delete from %1 where ProjectName = '%2'").arg(TB_PROJECTINFO).arg(RequestCmd.sProjectInfo.strProjectName);
	}

	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	else
	{
		RequestCmd.nRet = 1;
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	pSocket->waitForBytesWritten(2000);
	return RequestCmd.nRet;
}

//项目信息列表-用户参与的
bool TcpRunable::RequestProjectQueryUser(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_ProjectQueryUser RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	QString strSql = QString("select p.* from %1 p join %2 pm on p.ProjectName = pm.ProjectName join %3 u on u.UserID = pm.UserID where u.UserID = '%4'")
		.arg(TB_PROJECTINFO).arg(TB_PROJECTMEMBER).arg(TB_USERTABLE).arg(RequestCmd.strUserID);
	//筛选条件-项目状态
	if (RequestCmd.Conditions_Status != Project_UnKnow)
		strSql += QString(" and ProjectStatus = %1").arg(RequestCmd.Conditions_Status);
	if (!RequestCmd.Conditions_ContentFuzzyQuery.isEmpty())
		strSql += QString(" and p.ProjectName like '%%1%'").arg(RequestCmd.Conditions_ContentFuzzyQuery);

	SqlQueryEx query;
	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	RequestCmd.nRet = 1;
	RequestCmd.listInfo.clear();
	while (query.next())
	{
		ProjectInfo info;
		info.strProjectName = query.value("ProjectName").toString();						//项目名称
		info.strProjenctDescription = query.value("ProjenctDescription").toString();		//项目描述
		info.strManangeUser = query.value("ManangeUser").toString();						//项目负责人		-- 用户表-UserID
		info.strProjectProgress = query.value("ProjectProgress").toString();				//项目进展说明
		info.strUrgency = query.value("ProjectUrgency").toString();							//项目紧急出来项
		info.nProjectTask = query.value("ProjectTask").toInt();								//项目任务数
		info.nProjectTaskCompletion = query.value("ProjectTaskCompletion").toInt();			//项目任务完成数
		info.nProjectTaskExecution = query.value("ProjectTaskExecution").toInt();			//项目任务执行数
		info.dtProjectCreateDate = query.value("ProjectCreateDate").toDateTime();			//项目创建时间
		info.nProjectStatus = (ProjectInfoStatus)query.value("ProjectStatus").toInt();		//项目状态
		RequestCmd.listInfo.push_back(info);
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

//项目成员信息获取
bool TcpRunable::RequestProjectMemberQuery(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_ProjectMemberQuery RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	QString strSql = QString("select * from %1 where ProjectName = '%2'").arg(TB_PROJECTMEMBER).arg(RequestCmd.strProjectName);
	SqlQueryEx query;
	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	RequestCmd.nRet = 1;
	RequestCmd.listInfo.clear();
	while (query.next())
	{
		ProjectMember info;
		info.nAutoID = query.value("AutoID").toInt();						//自增编号
		info.strProjectName = query.value("ProjectName").toString();		//项目名称
		info.strUserID = query.value("UserID").toString();					//用户ID
		info.dtJoinDate =  query.value("JoinDate").toDateTime();			//加入时间
		info.nProjectPosition = query.value("ProjectPosition").toInt();		//项目职位 0=普通成员 1=项目管理者
		RequestCmd.listInfo.push_back(info);
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

//项目成员信息修改
bool TcpRunable::RequestProjectMemberOperation(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_ProjectMemberOperation RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strSql;
	if (RequestCmd.nOperationTypeType == Operation_Insert)
	{
		strSql = QString("insert into %1(ProjectName,UserID,ProjectPosition,JoinDate)values('%2','%3','%4','%5')")
			.arg(TB_PROJECTMEMBER).arg(RequestCmd.sProjectMember.strProjectName).arg(RequestCmd.sProjectMember.strUserID).arg(RequestCmd.sProjectMember.nProjectPosition).arg(RequestCmd.sProjectMember.dtJoinDate.toString("yyyy-MM-dd hh:mm:ss"));
	}
	else if (RequestCmd.nOperationTypeType == Operation_Update)
	{
		strSql = QString("update %1 set ProjectName = '%2',UserID = '%3',ProjectPosition = '%4',JoinDate = '%5' where AutoID = '%6'")
			.arg(TB_PROJECTMEMBER).arg(RequestCmd.sProjectMember.strProjectName).arg(RequestCmd.sProjectMember.strUserID).arg(RequestCmd.sProjectMember.nProjectPosition).arg(RequestCmd.sProjectMember.dtJoinDate.toString("yyyy-MM-dd hh:mm:ss")).arg(RequestCmd.sProjectMember.nAutoID);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Delete)
	{
		strSql = QString("delete from %1 where AutoID = '%2'").arg(TB_PROJECTMEMBER).arg(RequestCmd.sProjectMember.nAutoID);
	}

	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	else
	{
		RequestCmd.nRet = 1;
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	pSocket->waitForBytesWritten(2000);
	return RequestCmd.nRet;
}

//查询团队信息
bool TcpRunable::RequestTeamInfoQuery(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_TeamInfoQuery RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strSql;

	if (RequestCmd.strUserID.isEmpty())
	{
		strSql = QString("select * from %1 order by TeamCreateTime desc").arg(TB_TEAM);
	}
	else
	{
		strSql = QString("select t.* from %1 t \
			join %2 tm on tm.TeamID = t.TeamID  \
			join %3 u on u.UserID = tm.UserID \
			where u.UserID = '%4'")
			.arg(TB_TEAM).arg(TB_TEAMMEMBER).arg(TB_USERTABLE).arg(RequestCmd.strUserID);
	}

	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	RequestCmd.nRet = 1;
	RequestCmd.listInfo.clear();
	while (query.next())
	{
		TeamInfo info;
		info.nTeamID = query.value("TeamID").toInt();							//团队编号
		info.strTeamName = query.value("TeamName").toString();					//团队名称
		info.strTeamDescription = query.value("TeamDescription").toString();	//团队描述
		info.dtCreateDate = query.value("TeamCreateTime").toDateTime();			//创建日期
		RequestCmd.listInfo.push_back(info);
	}
	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

//团队信息修改
bool TcpRunable::RequestTeamOperation(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_TeamOperation RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strSql;
	if (RequestCmd.nOperationTypeType == Operation_Insert)
	{
		strSql = QString("insert into %1(TeamName,TeamDescription,TeamCreateTime)values('%2','%3','%4');")
			.arg(TB_TEAM).arg(RequestCmd.sTeamInfo.strTeamName).arg(RequestCmd.sTeamInfo.strTeamDescription).arg(RequestCmd.sTeamInfo.dtCreateDate.toString("yyyy-MM-dd hh:mm:ss"));
		if (!query.exec(strSql))
		{

		}
		strSql = QString("select TeamID from %1 where TeamName = '%2' and TeamDescription = '%3' and TeamCreateTime = '%4'")
			.arg(TB_TEAM).arg(RequestCmd.sTeamInfo.strTeamName).arg(RequestCmd.sTeamInfo.strTeamDescription).arg(RequestCmd.sTeamInfo.dtCreateDate.toString("yyyy-MM-dd hh:mm:ss"));
		if (!query.exec(strSql))
		{

		}
		int nTeamID = 0;
		if (query.next())
		{
			nTeamID = query.value("TeamID").toInt();
		}
		if (nTeamID > 0)
		{
			//插入团队成员数据
			strSql = QString("insert into %1(TeamID,UserID,TeamPosition,JoinDate)values('%2','%3','1',now())")
				.arg(TB_TEAMMEMBER).arg(nTeamID).arg(RequestCmd.strUserID);
		}
	}
	else if (RequestCmd.nOperationTypeType == Operation_Update)
	{
		strSql = QString("update %1 set TeamName = '%2',TeamDescription = '%3',TeamCreateTime = '%4' where TeamID = '%5'")
			.arg(TB_TEAM).arg(RequestCmd.sTeamInfo.strTeamName).arg(RequestCmd.sTeamInfo.strTeamDescription).arg(RequestCmd.sTeamInfo.dtCreateDate.toString("yyyy-MM-dd hh:mm:ss")).arg(RequestCmd.sTeamInfo.nTeamID);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Delete)
	{
		strSql = QString("delete from %1 where TeamID = '%2'").arg(TB_TEAM).arg(RequestCmd.sTeamInfo.nTeamID);
	}

	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	else
	{
		RequestCmd.nRet = 1;
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	pSocket->waitForBytesWritten(2000);
	return RequestCmd.nRet;
}

//团队成员查询
bool TcpRunable::RequestTeamMemberQuery(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_TeamMemberQuery RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	QString strSql = QString("select * from %1 where TeamID = '%2'").arg(TB_TEAMMEMBER).arg(RequestCmd.nTeamID);
	SqlQueryEx query;
	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	RequestCmd.nRet = 1;
	RequestCmd.listInfo.clear();
	while (query.next())
	{
		TeamMember info;
		info.nAutoID = query.value("AutoID").toInt();					//自增编号
		info.nTeamID = query.value("TeamID").toInt();					//团队ID
		info.strUserID = query.value("UserID").toString();				//用户ID
		info.nProjectPosition = query.value("TeamPosition").toInt();	//团队职别（0=普通成员 1=团队管理者）
		info.dtJoinDate = query.value("JoinDate").toDateTime();			//加入日期
		RequestCmd.listInfo.push_back(info);
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

//团队成员修改
bool TcpRunable::RequestTeamMemberOperation(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_TeamMemberOperation RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strSql;
	if (RequestCmd.nOperationTypeType == Operation_Insert)
	{
		strSql = QString("insert into %1(TeamID,UserID,TeamPosition,JoinDate)values('%2','%3','%4','%5')")
			.arg(TB_TEAMMEMBER).arg(RequestCmd.sTeamMemberInfo.nTeamID).arg(RequestCmd.sTeamMemberInfo.strUserID).arg(RequestCmd.sTeamMemberInfo.nProjectPosition).arg(RequestCmd.sTeamMemberInfo.dtJoinDate.toString("yyyy-MM-dd hh:mm:ss"));
	}
	else if (RequestCmd.nOperationTypeType == Operation_Update)
	{
		strSql = QString("update %1 set TeamPosition = '%2',JoinDate = '%3' where AutoID = '%4'")
			.arg(TB_TEAMMEMBER).arg(RequestCmd.sTeamMemberInfo.nProjectPosition).arg(RequestCmd.sTeamMemberInfo.dtJoinDate.toString("yyyy-MM-dd hh:mm:ss")).arg(RequestCmd.sTeamMemberInfo.nAutoID);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Delete)
	{
		strSql = QString("delete from %1 where AutoID = '%2'").arg(TB_TEAMMEMBER).arg(RequestCmd.sTeamMemberInfo.nAutoID);
	}

	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	else
	{
		RequestCmd.nRet = 1;
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	pSocket->waitForBytesWritten(2000);
	return RequestCmd.nRet;
}

//笔记目录
bool TcpRunable::RequestNoteCatalogQuery(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_NoteCatalogQuery RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	QString strSql = QString("select * from %1 where UserID = '%2'").arg(TB_NOTECATALOG).arg(RequestCmd.strUserID);
	SqlQueryEx query;
	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	RequestCmd.nRet = 1;
	RequestCmd.listNoteCatalog.clear();
	while (query.next())
	{
		NoteCatalog info;
		info.nAutoNode = query.value("AutoNode").toInt();		//节点ID
		info.strUserID = query.value("UserID").toString();		//用户ID
		info.nParentNode = query.value("ParentNode").toInt();	//父类节点ID	（0为原节点）
		info.strNoteName = query.value("NodeName").toString();	//节点名称
		RequestCmd.listNoteCatalog.push_back(info);
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

//笔记目录修改
bool TcpRunable::RequestNoteCatalogOperation(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_NoteCatalogOperation RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strSql;
	if (RequestCmd.nOperationTypeType == Operation_Insert)
	{
		strSql = QString("insert into %1(UserID,ParentNode,NodeName)values('%2','%3','%4')")
			.arg(TB_NOTECATALOG).arg(RequestCmd.sNoteCatalog.strUserID).arg(RequestCmd.sNoteCatalog.nParentNode).arg(RequestCmd.sNoteCatalog.strNoteName);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Update)
	{
		strSql = QString("update %1 set ParentNode = '%2',NodeName = '%3' where AutoNode = '%4'")
			.arg(TB_NOTECATALOG).arg(RequestCmd.sNoteCatalog.nParentNode).arg(RequestCmd.sNoteCatalog.strNoteName).arg(RequestCmd.sNoteCatalog.nAutoNode);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Delete)
	{
		strSql = QString("delete from %1 where ParentNode = '%2';delete from %1 where AutoNode = '%2'").arg(TB_NOTECATALOG).arg(RequestCmd.sNoteCatalog.nAutoNode);
	}

	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	else
	{
		RequestCmd.nRet = 1;
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	pSocket->waitForBytesWritten(2000);
	return RequestCmd.nRet;
}

//笔记信息查询
bool TcpRunable::RequestNoteQuery(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_NoteQuery RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	QString strSql = QString("select * from %1 where CatalogNode = '%2'").arg(TB_NOTE).arg(RequestCmd.nNoteID);
	SqlQueryEx query;
	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	RequestCmd.nRet = 1;
	if (query.next())
	{
		RequestCmd.sNoteInfo.nNotoID = query.value("NotoID").toInt();						//笔记编号（自增主键）
		RequestCmd.sNoteInfo.strNoteTitle = query.value("NoteTitle").toString();			//笔记标题
		RequestCmd.sNoteInfo.strNoteContent = query.value("NoteContent").toString();		//笔记内容
		RequestCmd.sNoteInfo.nCatalogNode = query.value("CatalogNode").toInt();				//目录节点
		RequestCmd.sNoteInfo.dtCreateDateTime = query.value("CreateDateTime").toDateTime();	//创建时间
		RequestCmd.sNoteInfo.dtUpdateDateTime = query.value("UpdateDateTime").toDateTime();	//更新时间
		RequestCmd.sNoteInfo.nRemind = query.value("Remind").toInt();						//是否提醒
		RequestCmd.sNoteInfo.dtRemindDateTime = query.value("RemindDateTime").toDateTime();	//提醒时间
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	return pSocket->waitForBytesWritten(2000);
}

//笔记信息修改
bool TcpRunable::RequestNoteOperation(QString strJson, QTcpSocket * pSocket)
{
	RequestCmd_NoteOperation RequestCmd;
	RequestCmd.getDataFromJson(strJson);

	SqlQueryEx query;
	QString strSql;
	if (RequestCmd.nOperationTypeType == Operation_Insert)
	{
		strSql = QString("insert into %1(NoteTitle,NoteContent,CatalogNode,CreateDateTime,UpdateDateTime,Remind,RemindDateTime)values('%2','%3','%4','%5','%6','%7','%8');")
			.arg(TB_NOTE).arg(RequestCmd.sNoteInfo.strNoteTitle).arg(RequestCmd.sNoteInfo.strNoteContent).arg(RequestCmd.sNoteInfo.nCatalogNode)
			.arg(RequestCmd.sNoteInfo.dtCreateDateTime.toString("yyyy-MM-dd hh:mm:ss")).arg(RequestCmd.sNoteInfo.dtUpdateDateTime.toString("yyyy-MM-dd hh:mm:ss"))
			.arg(RequestCmd.sNoteInfo.nRemind).arg(RequestCmd.sNoteInfo.dtRemindDateTime.toString("yyyy-MM-dd hh:mm:ss"));

		strSql += QString("update %1 set NodeName = '%2' where AutoNode = '%3'")
			.arg(TB_NOTECATALOG).arg(RequestCmd.sNoteInfo.strNoteTitle).arg(RequestCmd.sNoteInfo.nCatalogNode);
	}
	else if (RequestCmd.nOperationTypeType == Operation_Update)
	{
		strSql = QString("update %1 set NoteTitle = '%2',NoteContent = '%3',CatalogNode = '%4',CreateDateTime = '%5',UpdateDateTime = '%6',Remind = '%7',RemindDateTime = '%8' where NotoID = '%9';")
			.arg(TB_NOTE).arg(RequestCmd.sNoteInfo.strNoteTitle).arg(RequestCmd.sNoteInfo.strNoteContent).arg(RequestCmd.sNoteInfo.nCatalogNode)
			.arg(RequestCmd.sNoteInfo.dtCreateDateTime.toString("yyyy-MM-dd hh:mm:ss")).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
			.arg(RequestCmd.sNoteInfo.nRemind).arg(RequestCmd.sNoteInfo.dtRemindDateTime.toString("yyyy-MM-dd hh:mm:ss")).arg(RequestCmd.sNoteInfo.nNotoID);

		strSql += QString("update %1 set NodeName = '%2' where AutoNode = '%3'")
			.arg(TB_NOTECATALOG).arg(RequestCmd.sNoteInfo.strNoteTitle).arg(RequestCmd.sNoteInfo.nCatalogNode);

	}
	else if (RequestCmd.nOperationTypeType == Operation_Delete)
	{
		strSql = QString("delete from %1 where CatalogNode = '%2'").arg(TB_NOTE).arg(RequestCmd.sNoteInfo.nNotoID);
	}

	if (!query.exec(strSql))
	{
		RequestCmd.nRet = 0;
		RequestCmd.strError = query.getLastError();
		qDebug() << RequestCmd.strError;
	}
	else
	{
		RequestCmd.nRet = 1;
	}

	QString strCmd = RequestCmd.getCmdFromData();
	WnLog::setShowLog(QString("返回数据:%1").arg(strCmd));
	pSocket->write(strCmd.toUtf8().data());
	pSocket->waitForBytesWritten(2000);
	return RequestCmd.nRet;
}