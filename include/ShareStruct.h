#ifndef SHARDSTRUCT_H
#define SHARDSTRUCT_H

#include <sstream>
#include <iomanip>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

#define TcpEnd "E~n!d@"

//======================================================== 枚举值 ========================================================
//请求命令编码ID （最多到9999）
enum RequestID
{
	Request_UnKnow = 0,						//未知命令

	Request_UserLogin = 100,				//用户登录
	Request_UserInfoOperator = 101,			//用户信息修改
	Request_UserQueryTeam = 102,			//用户信息查询团队列表
	Request_UserSearch = 103,				//用户信息搜索查询

	Request_IndexBase = 200,				//主页基础数据

	Request_TaskQuery = 300,				//任务信息获取
	Request_TaskOperation = 301,			//任务信息修改
	Request_TaskActivityQuery = 310,		//任务动态查询
	Request_TaskActivityOperation = 311,	//任务动态修改

	//任务统计
	Request_TaskStatistic,

	Request_ProjectQuery = 400,				//项目信息获取
	Request_ProjectOperation = 401,			//项目信息修改
	Request_ProjectQueryUser = 402,			//项目信息-用户参与的

	Request_ProjectMemberQuery = 410,		//项目成员信息获取
	Request_ProjectMemberOperation = 411,	//项目成员信息修改

	Request_TeamInfoQuery = 500,			//查询团队信息
	Request_TeamOperation = 501,			//团队信息修改
	Request_TeamMemberQuery = 510,			//团队成员查询
	Request_TeamMemberOperation = 511,		//团队成员修改

	Request_NoteCatalogQuery = 600,			//笔记目录查询
	Request_NoteCatalogOperation = 601,		//笔记目录修改
	Request_NoteQuery = 610,				//笔记信息查询
	Request_NoteOperation = 611				//笔记信息修改
};

//======================================================== 结构体 ========================================================
//请求命令-基类
struct RequestCmd
{
	RequestID	nRequestID;		//请求ID
	QString		strRequest;

	int			nRet;			//返回状态 0=失败 1成功 2其他原因
	QString		strError;		//反馈错误信息

	RequestCmd()
	{
		nRequestID = Request_UnKnow;
		nRet = 0;
	}

	QString getRequestStr()
	{
		std::ostringstream oss;
		oss << std::setw(4) << std::setfill('0') << (int)nRequestID;
		strRequest = QString::fromStdString(oss.str());
		return strRequest;
	}

	//获取发送Cmd命令的字符串
	QString getCmdFromData()
	{
		return QString("%1%2%3").arg(getRequestStr()).arg(getJsonFromData()).arg(TcpEnd);
	};

	//根据数据获取Json字符串
	virtual	QString getJsonFromData() const = 0;
	//根据获取的Json获取具体的数据
	virtual	bool getDataFromJson(const QString &) = 0;
};

//数据操作类型
enum OperationType
{
	Operation_Unknown = 0,			//未知
	Operation_Insert = 1,			//操作-新增
	Operation_Update = 2,			//操作-修改
	Operation_Delete = 3			//操作-删除
};

//用户权限
enum UserPermission
{
	User_Normal = 0,			//普通用户
	User_Manage = 1,			//管理用户
	User_SupManage = 2			//超级用户
};

//用户性别
enum UserGender
{
	Gender_UnKnow = 0,			//性别-未知
	Gender_Man = 1,				//性别-男
	Gender_Woman = 2			//性别-女
};

//用户信息
struct UserInfo
{
	QString			strUserID;			//用户ID
	QString			strPwd;				//用户密码
	QString			strUserName;		//用户名称
	UserPermission	nUserPermission;	//用户权限
	UserGender		nUserGender;		//用户性别
	QString			strUserPhoneNumber;	//用户电话号码
	QDateTime		dtLoginTime;		//登录时间

	UserInfo()
	{
		nUserPermission = User_Normal;
		nUserGender = Gender_UnKnow;
		dtLoginTime = QDateTime::currentDateTime();
	}

	QString getJson() const
	{
		QJsonDocument _doc;
		_doc.setObject(getJsonObject());
		return _doc.toJson();
	}

	QJsonObject getJsonObject() const
	{
		QJsonObject _obj;
		_obj.insert("strUserID", strUserID);
		_obj.insert("strPwd", strPwd);
		_obj.insert("strUserName", strUserName);
		_obj.insert("nUserPermission", (int)nUserPermission);
		_obj.insert("nUserGender", (int)nUserGender);
		_obj.insert("strUserPhoneNumber", strUserPhoneNumber);
		_obj.insert("dtLoginTime", dtLoginTime.toSecsSinceEpoch());
		return _obj;
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QString & strJson)
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_GetProjectInfo");
			return false;
		}
		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_GetProjectInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();
		return getDataFromJson(jsonObj);
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QJsonObject & jsonObj)
	{
		strUserID = jsonObj.value("strUserID").toString();
		strPwd = jsonObj.value("strPwd").toString();
		strUserName = jsonObj.value("strUserName").toString();
		nUserPermission = (UserPermission)jsonObj.value("nUserPermission").toInt();
		nUserGender = (UserGender)jsonObj.value("nUserGender").toInt();
		strUserPhoneNumber = jsonObj.value("strUserPhoneNumber").toString();
		dtLoginTime = QDateTime::fromSecsSinceEpoch(jsonObj.value("dtLoginTime").toInt());
		return true;
	}
};

//用户登录校验反馈
enum UserLoginValidation
{
	Login_Unknown = 0,				//未知（默认）
	ValidationSuccessful = 1,	//校验成功
	ValidationFailure = 2,		//校验失败
	UserNoExist = 3,			//用户不存在
	IncorrectPassword = 4		//密码不正确
};

//请求命令-用户登录 
struct RequestCmd_UserLogin : RequestCmd
{
	UserInfo sLoginUser;
	UserLoginValidation nValidationStatus;	//用户登录校验结果	

	RequestCmd_UserLogin()
	{
		nRequestID = Request_UserLogin;
		nValidationStatus = UserLoginValidation::Login_Unknown;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nValidationStatus", (int)nValidationStatus);
		_obj.insert("sLoginUser", sLoginUser.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nValidationStatus = (UserLoginValidation)jsonObj.value("nValidationStatus").toInt();
		sLoginUser.getDataFromJson(jsonObj.value("sLoginUser").toObject());
		return true;
	}
};

//请求命令-用户信息修改
struct RequestCmd_UserInfoOperator : RequestCmd
{
	OperationType	nOperationTypeType;		//操作类型
	UserInfo		sUserInfo;				//用户信息

	RequestCmd_UserInfoOperator()
	{
		nRequestID = Request_UserInfoOperator;
		nOperationTypeType = Operation_Unknown;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nOperationTypeType", (int)nOperationTypeType);
		_obj.insert("sUserInfo", sUserInfo.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nOperationTypeType = (OperationType)jsonObj.value("nOperationTypeType").toInt();
		sUserInfo.getDataFromJson(jsonObj.value("sUserInfo").toObject());
		return true;
	}
};

//请求命令-用户信息查询-根据所在团队
struct RequestCmd_UserQueryTeam : RequestCmd
{
	QString strUserID;
	QList<UserInfo> listUserInfo;

	RequestCmd_UserQueryTeam()
	{
		nRequestID = Request_UserQueryTeam;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("strUserID", strUserID);
		QJsonArray _arry;
		for (const UserInfo & _info : listUserInfo)
			_arry.push_back(_info.getJsonObject());
		_obj.insert("listUserInfo", _arry);
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		strUserID = jsonObj.value("strUserID").toString();

		listUserInfo.clear();
		QJsonArray _array = jsonObj.value("listUserInfo").toArray();
		for (const QJsonValue & _val : _array)
		{
			UserInfo info;
			info.getDataFromJson(_val.toObject());
			listUserInfo.push_back(info);
		}

		return true;
	}
};

//任务状态
enum TaskStatus
{
	Task_UnKnow = 0,		//未知	（作为查询条件时表示全部）
	Task_WaitStart = 1,		//任务未开始
	Task_Runing = 2,		//任务进行中
	Task_TimeOut = 3,		//任务已超时
	Task_Completion = 10,	//任务已完成
	Task_Pause = 11,		//任务暂停中
	Task_Abandonment = 12	//任务已废弃
};

//任务等级
enum TaskPriority
{
	Lowest = 0,			//低	灰色
	Low = 1,			//正常	蓝色
	Medium = 3,			//中等优先级 黄色
	High = 4,			//高优先级 橙色
	Critical = 5		//紧急  红色
};

//任务信息
struct TaskInfo
{
	int				nTaskID;			//任务ID
	QString			strProjectName;		//项目名称
	TaskStatus		nTaskStatus;		//任务状态
	QString			strTitle;			//任务标题
	QString			strContent;			//任务内容
	QDateTime		dtDateCreate;		//任务创建日期
	QDateTime		dtDateStart;		//任务开始日期
	QDateTime		dtDateEnd;			//任务结束日期
	QString			strUserExec;		//任务执行人
	QString			strUserCreate;		//任务创建人
	TaskPriority	nTaskPriority;		//任务优先级
	float			fPlannedDuration;	//任务规划时长
	float			fActualDuration;	//任务实际使用
	float			fScore;				//任务实际得分

	TaskInfo()
	{
		nTaskID = -1;
		nTaskStatus = Task_UnKnow;
		nTaskPriority = Lowest;
		fPlannedDuration = 0;
		fActualDuration = 0;
		fScore = 0;
	}

	QString getJson() const
	{
		QJsonDocument _doc;
		_doc.setObject(getJsonObject());
		return _doc.toJson();
	}

	QJsonObject getJsonObject() const
	{
		QJsonObject _obj;
		_obj.insert("nTaskID", nTaskID);
		_obj.insert("strProjectName", strProjectName);
		_obj.insert("nTaskStatus", nTaskStatus);
		_obj.insert("strTitle", strTitle);
		_obj.insert("strContent", strContent);
		_obj.insert("dtDateCreate", dtDateCreate.toString("yyyy-MM-dd hh:mm:ss"));
		_obj.insert("dtDateStart", dtDateStart.toString("yyyy-MM-dd hh:mm:ss"));
		_obj.insert("dtDateEnd", dtDateEnd.toString("yyyy-MM-dd hh:mm:ss"));
		_obj.insert("strUserExec", strUserExec);
		_obj.insert("strUserCreate", strUserCreate);
		_obj.insert("nTaskPriority", nTaskPriority);
		_obj.insert("fPlannedDuration", fPlannedDuration);
		_obj.insert("fActualDuration", fActualDuration);
		_obj.insert("fScore", fScore);
		return _obj;
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QString & strJson)
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_GetProjectInfo");
			return false;
		}
		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_GetProjectInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();
		return getDataFromJson(jsonObj);
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QJsonObject & jsonObj)
	{
		nTaskID = jsonObj.value("nTaskID").toInt();
		strProjectName = jsonObj.value("strProjectName").toString();
		nTaskStatus = (TaskStatus)jsonObj.value("nTaskStatus").toInt();
		strTitle = jsonObj.value("strTitle").toString();
		strContent = jsonObj.value("strContent").toString();
		dtDateCreate = QDateTime::fromString(jsonObj.value("dtDateCreate").toString(), "yyyy-MM-dd hh:mm:ss");
		dtDateStart = QDateTime::fromString(jsonObj.value("dtDateStart").toString(), "yyyy-MM-dd hh:mm:ss");
		dtDateEnd = QDateTime::fromString(jsonObj.value("dtDateEnd").toString(), "yyyy-MM-dd hh:mm:ss");
		strUserExec = jsonObj.value("strUserExec").toString();
		strUserCreate = jsonObj.value("strUserCreate").toString();
		nTaskPriority = (TaskPriority)jsonObj.value("nTaskPriority").toInt();
		fPlannedDuration = jsonObj.value("fPlannedDuration").toDouble();
		fActualDuration = jsonObj.value("fActualDuration").toDouble();
		fScore = jsonObj.value("fScore").toDouble();
		return true;
	}
};

//请求命令-任务查询 
struct RequestCmd_TaskQuery : RequestCmd
{
	QString				strUserID;						//用户ID
	QString				strProjectName;					//项目名称
	TaskStatus			Conditions_Status;				//筛选条件-任务状态
	int					Conditions_DateStart;			//筛选条件-开始时间
	int					Conditions_DateEnd;				//筛选条件-结束时间
	QString				Conditions_ContentFuzzyQuery;	//筛选条件-任务内容模糊查询
	int					Conditions_Page;				//筛选条件-第几页
	int					Conditions_PageRow;				//筛选条件-一页几行

														//数据返回
	int					nCount;							//数据数量
	QList<TaskInfo>		listTask;						//查询的数据信息

	RequestCmd_TaskQuery()
	{
		nRequestID = Request_TaskQuery;
		Conditions_DateStart = 0;
		Conditions_DateEnd = 0;
		Conditions_Page = 0;
		Conditions_PageRow = 0;
		nCount = 0;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;

		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("strUserID", strUserID);
		_obj.insert("strProjectName", strProjectName);
		_obj.insert("Conditions_Status", (int)Conditions_Status);
		_obj.insert("Conditions_DateStart", Conditions_DateStart);
		_obj.insert("Conditions_DateEnd", Conditions_DateEnd);
		_obj.insert("Conditions_ContentFuzzyQuery", Conditions_ContentFuzzyQuery);
		_obj.insert("Conditions_Page", Conditions_Page);
		_obj.insert("Conditions_PageRow", Conditions_PageRow);
		_obj.insert("nCount", nCount);
		QJsonArray _array;
		for (TaskInfo _info : listTask)
			_array.push_back(_info.getJsonObject());
		_obj.insert("listTask", _array);

		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		strUserID = jsonObj.value("strUserID").toString();
		strProjectName = jsonObj.value("strProjectName").toString();
		Conditions_Status = (TaskStatus)jsonObj.value("Conditions_Status").toInt();
		Conditions_DateStart = jsonObj.value("Conditions_DateStart").toInt();
		Conditions_DateEnd = jsonObj.value("Conditions_DateEnd").toInt();
		Conditions_ContentFuzzyQuery = jsonObj.value("Conditions_ContentFuzzyQuery").toString();
		Conditions_Page = jsonObj.value("Conditions_Page").toInt();
		Conditions_PageRow = jsonObj.value("Conditions_PageRow").toInt();
		nCount = jsonObj.value("nCount").toInt();

		listTask.clear();
		QJsonArray _array = jsonObj.value("listTask").toArray();
		for (QJsonValue _val : _array)
		{
			TaskInfo info;
			info.getDataFromJson(_val.toObject());
			listTask.push_back(info);
		}
		return true;
	}
};

//请求命令-任务操作
struct RequestCmd_TaskOperation : RequestCmd
{
	OperationType	nOperationTypeType;		//操作类型
	TaskInfo		sTaskInfo;				//任务信息

	RequestCmd_TaskOperation()
	{
		nRequestID = Request_TaskOperation;
		nOperationTypeType = Operation_Unknown;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nOperationTypeType", (int)nOperationTypeType);
		_obj.insert("sTaskInfo", sTaskInfo.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nOperationTypeType = (OperationType)jsonObj.value("nOperationTypeType").toInt();
		sTaskInfo.getDataFromJson(jsonObj.value("sTaskInfo").toObject());
		return true;
	}
};

//任务动态
struct TaskActivity
{
	int			nAutoID;						//自增ID
	int			nTaskID;						//任务编号
	QDateTime	dtActivityDateTime;				//时间
	QString		strActivityContent;				//内容
	QString		strCommentUserID;				//评论人ID
	int			nActivityType;					//动态类型 0=状态修改 1=用户评论

	TaskActivity()
	{
		nAutoID = 0;
		nTaskID = 0;
		dtActivityDateTime = QDateTime::currentDateTime();
		nActivityType = 0;
	}

	QString getJson() const
	{
		QJsonDocument _doc;
		_doc.setObject(getJsonObject());
		return _doc.toJson();
	}

	QJsonObject getJsonObject() const
	{
		QJsonObject _obj;
		_obj.insert("nAutoID", nAutoID);
		_obj.insert("nTaskID", nTaskID);
		_obj.insert("dtActivityDateTime", dtActivityDateTime.toSecsSinceEpoch());
		_obj.insert("strActivityContent", strActivityContent);
		_obj.insert("strCommentUserID", strCommentUserID);
		_obj.insert("nActivityType", nActivityType);
		return _obj;
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QString & strJson)
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_GetProjectInfo");
			return false;
		}
		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_GetProjectInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();
		return getDataFromJson(jsonObj);
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QJsonObject & jsonObj)
	{
		nAutoID = jsonObj.value("nAutoID").toInt();						//自增ID
		nTaskID = jsonObj.value("nTaskID").toInt();						//任务编号
		dtActivityDateTime = QDateTime::fromSecsSinceEpoch(jsonObj.value("dtActivityDateTime").toInt());		//时间
		strActivityContent = jsonObj.value("strActivityContent").toString();						//内容
		strCommentUserID = jsonObj.value("strCommentUserID").toString();						//评论人ID
		nActivityType = jsonObj.value("nActivityType").toInt();
		return true;
	}
};

//请求命令-任务动态查询
struct RequestCmd_TaskActivityQuery : RequestCmd
{
	int nTaskID;
	QList<TaskActivity> listInfo;

	RequestCmd_TaskActivityQuery()
	{
		nRequestID = Request_TaskActivityQuery;
		nTaskID = 0;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;

		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nTaskID", nTaskID);
		QJsonArray _array;
		for (auto & _info : listInfo)
			_array.push_back(_info.getJsonObject());
		_obj.insert("listInfo", _array);

		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nTaskID = jsonObj.value("nTaskID").toInt();

		listInfo.clear();
		QJsonArray _array = jsonObj.value("listInfo").toArray();
		for (const auto & _val : _array)
		{
			TaskActivity info;
			info.getDataFromJson(_val.toObject());
			listInfo.push_back(info);
		}
		return true;
	}
};

//请求命令-任务动态修改
struct RequestCmd_TaskActivityOperation : RequestCmd
{
	OperationType	nOperationTypeType;		//操作类型
	TaskActivity	sTaskActivityInfo;		//任务动态信息

	RequestCmd_TaskActivityOperation()
	{
		nRequestID = Request_TaskActivityOperation;
		nOperationTypeType = Operation_Unknown;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nOperationTypeType", (int)nOperationTypeType);
		_obj.insert("sTaskActivityInfo", sTaskActivityInfo.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nOperationTypeType = (OperationType)jsonObj.value("nOperationTypeType").toInt();
		sTaskActivityInfo.getDataFromJson(jsonObj.value("sTaskActivityInfo").toObject());
		return true;
	}

};

//团队信息
struct TeamInfo
{
	int			nTeamID;				//团队编号
	QString		strTeamName;			//团队名称
	QString		strTeamDescription;		//团队描述
	QDateTime	dtCreateDate;			//创建日期

	TeamInfo()
	{
		nTeamID = 0;
		dtCreateDate = QDateTime::currentDateTime();
	}

	QString getJson() const
	{
		QJsonDocument _doc;
		_doc.setObject(getJsonObject());
		return _doc.toJson();
	}

	QJsonObject getJsonObject() const
	{
		QJsonObject _obj;
		_obj.insert("nTeamID", nTeamID);
		_obj.insert("strTeamName", strTeamName);
		_obj.insert("strTeamDescription", strTeamDescription);
		_obj.insert("dtCreateDate", dtCreateDate.toSecsSinceEpoch());
		return _obj;
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QString & strJson)
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_GetProjectInfo");
			return false;
		}
		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_GetProjectInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();
		return getDataFromJson(jsonObj);
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QJsonObject & jsonObj)
	{
		nTeamID = jsonObj.value("nTeamID").toInt();
		strTeamName = jsonObj.value("strTeamName").toString();
		strTeamDescription = jsonObj.value("strTeamDescription").toString();
		dtCreateDate = QDateTime::fromSecsSinceEpoch(jsonObj.value("dtCreateDate").toInt());
		return true;
	}
};

//请求命令-查询团队信息
struct RequestCmd_TeamInfoQuery : RequestCmd
{
	QString strUserID;						//用户ID （填写的情况下查询该用户所在的团队，不填写的情况下查询所有团队信息）
	QList<TeamInfo> listInfo;

	RequestCmd_TeamInfoQuery()
	{
		nRequestID = Request_TeamInfoQuery;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;

		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("strUserID", strUserID);
		QJsonArray _array;
		for (auto & _info : listInfo)
			_array.push_back(_info.getJsonObject());
		_obj.insert("listInfo", _array);

		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		strUserID = jsonObj.value("strUserID").toString();

		listInfo.clear();
		QJsonArray _array = jsonObj.value("listInfo").toArray();
		for (const auto & _val : _array)
		{
			TeamInfo info;
			info.getDataFromJson(_val.toObject());
			listInfo.push_back(info);
		}
		return true;
	}

};

//请求命令-团队信息修改
struct RequestCmd_TeamOperation : RequestCmd
{
	OperationType	nOperationTypeType;		//操作类型
	QString			strUserID;				//创建的用户ID，默认为管理员
	TeamInfo		sTeamInfo;				//团队信息

	RequestCmd_TeamOperation()
	{
		nRequestID = Request_TeamOperation;
		nOperationTypeType = Operation_Unknown;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("strUserID", strUserID);
		_obj.insert("nOperationTypeType", nOperationTypeType);
		_obj.insert("sTeamInfo", sTeamInfo.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		strUserID = jsonObj.value("strUserID").toString();
		nOperationTypeType = (OperationType)jsonObj.value("nOperationTypeType").toInt();
		sTeamInfo.getDataFromJson(jsonObj.value("sTeamInfo").toObject());
		return true;
	}
};

//团队成员信息
struct TeamMember
{
	int			nAutoID;			//自增编号
	int			nTeamID;			//团队ID
	QString		strUserID;			//用户ID
	int			nProjectPosition;	//团队职别（0=普通成员 1=团队管理者）
	QDateTime	dtJoinDate;			//加入日期

	TeamMember()
	{
		nAutoID = 0;
		nTeamID = 0;
		nProjectPosition = 0;
	}

	QString getJson() const
	{
		QJsonDocument _doc;
		_doc.setObject(getJsonObject());
		return _doc.toJson();
	}

	QJsonObject getJsonObject() const
	{
		QJsonObject _obj;
		_obj.insert("nAutoID", nAutoID);
		_obj.insert("nTeamID", nTeamID);
		_obj.insert("strUserID", strUserID);
		_obj.insert("nProjectPosition", nProjectPosition);
		_obj.insert("dtJoinDate", dtJoinDate.toSecsSinceEpoch());
		return _obj;
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QString & strJson)
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_GetProjectInfo");
			return false;
		}
		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_GetProjectInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();
		return getDataFromJson(jsonObj);
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QJsonObject & jsonObj)
	{
		nAutoID = jsonObj.value("nAutoID").toInt();
		nTeamID = jsonObj.value("nTeamID").toInt();
		strUserID = jsonObj.value("strUserID").toString();
		nProjectPosition = jsonObj.value("nProjectPosition").toInt();
		dtJoinDate = QDateTime::fromSecsSinceEpoch(jsonObj.value("dtJoinDate").toInt());
		return true;
	}
};

//请求命令-团队成员查询
struct RequestCmd_TeamMemberQuery : RequestCmd
{
	int nTeamID;
	QList<TeamMember> listInfo;

	RequestCmd_TeamMemberQuery()
	{
		nRequestID = Request_TeamMemberQuery;
		nTeamID = 0;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;

		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nTeamID", nTeamID);
		QJsonArray _array;
		for (auto & _info : listInfo)
			_array.push_back(_info.getJsonObject());
		_obj.insert("listInfo", _array);

		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nTeamID = jsonObj.value("nTeamID").toInt();

		listInfo.clear();
		QJsonArray _array = jsonObj.value("listInfo").toArray();
		for (const auto & _val : _array)
		{
			TeamMember info;
			info.getDataFromJson(_val.toObject());
			listInfo.push_back(info);
		}
		return true;
	}
};

//请求命令-团队成员修改
struct RequestCmd_TeamMemberOperation : RequestCmd
{
	OperationType	nOperationTypeType;		//操作类型
	TeamMember		sTeamMemberInfo;		//任务动态信息

	RequestCmd_TeamMemberOperation()
	{
		nRequestID = Request_TeamMemberOperation;
		nOperationTypeType = Operation_Unknown;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nOperationTypeType", (int)nOperationTypeType);
		_obj.insert("sTeamMemberInfo", sTeamMemberInfo.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nOperationTypeType = (OperationType)jsonObj.value("nOperationTypeType").toInt();
		sTeamMemberInfo.getDataFromJson(jsonObj.value("sTeamMemberInfo").toObject());
		return true;
	}
};

//项目状态
enum ProjectInfoStatus
{
	Project_UnKnow = 0,		//未知	（作为查询条件时表示全部）
	Project_Wait = 1,		//任务未开始
	Project_Runing = 2,		//任务进行中
	Project_Completion = 3,	//任务已完成
	Project_Pause = 4,		//任务暂停中
	Project_Abandonment = 5	//任务已废弃
};

//项目信息表
struct ProjectInfo
{
	QString					strProjectName;				//项目名称			--唯一主键
	QString					strProjenctDescription;		//项目描述
	QString					strManangeUser;				//项目负责人		-- 用户表-UserID
	QString					strProjectProgress;			//项目进展说明
	QString					strUrgency;					//项目紧急出来项
	int						nProjectTask;				//项目任务数
	int						nProjectTaskCompletion;		//项目任务完成数
	int						nProjectTaskExecution;		//项目任务执行数
	QDateTime				dtProjectCreateDate;		//项目创建时间
	ProjectInfoStatus		nProjectStatus;				//项目状态			-- 0=待启动 1=进行中 2=已结束

	ProjectInfo()
	{
		nProjectTask = 0;
		nProjectTaskCompletion = 0;
		nProjectTaskExecution = 0;
		dtProjectCreateDate = QDateTime::currentDateTime();
		nProjectStatus = Project_UnKnow;
	}

	QString getJson() const
	{
		QJsonDocument _doc;
		_doc.setObject(getJsonObject());
		return _doc.toJson();
	}

	QJsonObject getJsonObject() const
	{
		QJsonObject _obj;
		_obj.insert("strProjectName", strProjectName);
		_obj.insert("strProjenctDescription", strProjenctDescription);
		_obj.insert("strManangeUser", strManangeUser);
		_obj.insert("strProjectProgress", strProjectProgress);
		_obj.insert("strUrgency", strUrgency);
		_obj.insert("nProjectTask", nProjectTask);
		_obj.insert("nProjectTaskCompletion", nProjectTaskCompletion);
		_obj.insert("nProjectTaskExecution", nProjectTaskExecution);
		_obj.insert("dtProjectCreateDate", dtProjectCreateDate.toSecsSinceEpoch());
		_obj.insert("nProjectStatus", (int)nProjectStatus);
		return _obj;
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QString & strJson)
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_GetProjectInfo");
			return false;
		}
		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_GetProjectInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();
		return getDataFromJson(jsonObj);
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QJsonObject & jsonObj)
	{
		strProjectName = jsonObj.value("strProjectName").toString();
		strProjenctDescription = jsonObj.value("strProjenctDescription").toString();
		strManangeUser = jsonObj.value("strManangeUser").toString();
		strProjectProgress = jsonObj.value("strProjectProgress").toString();
		strUrgency = jsonObj.value("strUrgency").toString();
		nProjectTask = jsonObj.value("nProjectTask").toInt();
		nProjectTaskCompletion = jsonObj.value("nProjectTaskCompletion").toInt();
		nProjectTaskExecution = jsonObj.value("nProjectTaskExecution").toInt();
		dtProjectCreateDate = QDateTime::fromSecsSinceEpoch(jsonObj.value("dtProjectCreateDate").toInt());
		nProjectStatus = (ProjectInfoStatus)jsonObj.value("nProjectStatus").toInt();
		return true;
	}
};

//请求命令-项目信息
struct RequestCmd_ProjectQuery : RequestCmd
{
	int nProjectID;
	ProjectInfo info;

	RequestCmd_ProjectQuery()
	{
		nRequestID = Request_ProjectQuery;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nProjectID", nProjectID);
		_obj.insert("info", info.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nProjectID = jsonObj.value("nProjectID").toInt();

		info.getDataFromJson(jsonObj.value("nProjectID").toObject());
		return true;
	}
};

//请求命令-用户参与的项目列表信息
struct RequestCmd_ProjectQueryUser : RequestCmd
{
	QString				strUserID;
	ProjectInfoStatus	Conditions_Status;				//筛选条件-项目状态
	QString				Conditions_ContentFuzzyQuery;	//筛选条件-任务内容模糊查询

	QList<ProjectInfo>	listInfo;

	RequestCmd_ProjectQueryUser()
	{
		nRequestID = Request_ProjectQueryUser;
		Conditions_Status = Project_UnKnow;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("strUserID", strUserID);
		_obj.insert("Conditions_Status", (int)Conditions_Status);
		_obj.insert("Conditions_ContentFuzzyQuery", Conditions_ContentFuzzyQuery);
		QJsonArray _arry;
		for (const auto & _info : listInfo)
			_arry.push_back(_info.getJsonObject());
		_obj.insert("listInfo", _arry);
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		strUserID = jsonObj.value("strUserID").toString();
		Conditions_Status = (ProjectInfoStatus)jsonObj.value("Conditions_Status").toInt();
		Conditions_ContentFuzzyQuery = jsonObj.value("Conditions_ContentFuzzyQuery").toString();

		listInfo.clear();
		QJsonArray _array = jsonObj.value("listInfo").toArray();
		for (const QJsonValue & _val : _array)
		{
			ProjectInfo info;
			info.getDataFromJson(_val.toObject());
			listInfo.push_back(info);
		}

		return true;
	}
};

//请求命令-项目信息操作
struct RequestCmd_ProjectOperation : RequestCmd
{
	OperationType	nOperationTypeType;		//操作类型
	ProjectInfo		sProjectInfo;			//项目信息

	RequestCmd_ProjectOperation()
	{
		nRequestID = Request_ProjectOperation;
		nOperationTypeType = Operation_Unknown;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nOperationTypeType", (int)nOperationTypeType);
		_obj.insert("sProjectInfo", sProjectInfo.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nOperationTypeType = (OperationType)jsonObj.value("nOperationTypeType").toInt();
		sProjectInfo.getDataFromJson(jsonObj.value("sProjectInfo").toObject());
		return true;
	}
};

//项目与对应的成员
struct ProjectMember
{
	int			nAutoID;				//自增编号
	QString		strProjectName;			//项目名称
	QString		strUserID;				//用户ID
	QDateTime	dtJoinDate;				//加入时间
	int			nProjectPosition;		//项目职位 0=普通成员 1=项目管理者

	ProjectMember()
	{
		nAutoID = 0;
		dtJoinDate = QDateTime::currentDateTime();
		nProjectPosition = 0;
	}

	QString getJson() const
	{
		QJsonDocument _doc;
		_doc.setObject(getJsonObject());
		return _doc.toJson();
	}

	QJsonObject getJsonObject() const
	{
		QJsonObject _obj;
		_obj.insert("nAutoID", nAutoID);
		_obj.insert("strProjectName", strProjectName);
		_obj.insert("strUserID", strUserID);
		_obj.insert("dtJoinDate", dtJoinDate.toSecsSinceEpoch());
		_obj.insert("nProjectPosition", nProjectPosition);
		return _obj;
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QString & strJson)
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_GetProjectInfo");
			return false;
		}
		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_GetProjectInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();
		return getDataFromJson(jsonObj);
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QJsonObject & jsonObj)
	{
		nAutoID = jsonObj.value("nAutoID").toInt();
		strProjectName = jsonObj.value("strProjectName").toInt();
		strUserID = jsonObj.value("strUserID").toString();
		dtJoinDate = QDateTime::fromSecsSinceEpoch(jsonObj.value("dtJoinDate").toInt());
		nProjectPosition = jsonObj.value("nProjectPosition").toInt();
		return true;
	}
};

//请求命令-项目成员查询
struct RequestCmd_ProjectMemberQuery : RequestCmd
{
	QString strProjectName;
	QList<ProjectMember> listInfo;

	RequestCmd_ProjectMemberQuery()
	{
		nRequestID = Request_ProjectMemberQuery;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;

		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("strProjectName", strProjectName);
		QJsonArray _array;
		for (auto & _info : listInfo)
			_array.push_back(_info.getJsonObject());
		_obj.insert("listInfo", _array);

		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		strProjectName = jsonObj.value("strProjectName").toString();

		listInfo.clear();
		QJsonArray _array = jsonObj.value("listInfo").toArray();
		for (const auto & _val : _array)
		{
			ProjectMember info;
			info.getDataFromJson(_val.toObject());
			listInfo.push_back(info);
		}
		return true;
	}
};

//请求命令-项目成员修改
struct RequestCmd_ProjectMemberOperation : RequestCmd
{
	OperationType	nOperationTypeType;		//操作类型
	ProjectMember	sProjectMember;			//团队成员信息

	RequestCmd_ProjectMemberOperation()
	{
		nRequestID = Request_TeamMemberOperation;
		nOperationTypeType = Operation_Unknown;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nOperationTypeType", (int)nOperationTypeType);
		_obj.insert("sProjectMember", sProjectMember.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nOperationTypeType = (OperationType)jsonObj.value("nOperationTypeType").toInt();
		sProjectMember.getDataFromJson(jsonObj.value("sProjectMember").toObject());
		return true;
	}
};

//笔记目录
struct NoteCatalog
{
	int						nAutoNode;				//节点ID
	QString					strUserID;				//用户ID
	int						nParentNode;			//父类节点ID	（0为原节点）
	QString					strNoteName;			//节点名称
	QList<NoteCatalog *>	listChildNote;			//子节点数据指针

	NoteCatalog()
	{
		nAutoNode = -1;
		nParentNode = 0;
	}

	QString getJson() const
	{
		QJsonDocument _doc;
		_doc.setObject(getJsonObject());
		return _doc.toJson();
	}

	QJsonObject getJsonObject() const
	{
		QJsonObject _obj;
		_obj.insert("nAutoNode", nAutoNode);
		_obj.insert("strUserID", strUserID);
		_obj.insert("nParentNode", nParentNode);
		_obj.insert("strNoteName", strNoteName);
		return _obj;
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QString & strJson)
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_GetProjectInfo");
			return false;
		}
		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_GetProjectInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();
		return getDataFromJson(jsonObj);
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QJsonObject & jsonObj)
	{
		nAutoNode = jsonObj.value("nAutoNode").toInt();
		strUserID = jsonObj.value("strUserID").toString();
		nParentNode = jsonObj.value("nParentNode").toInt();
		strNoteName = jsonObj.value("strNoteName").toString();
		return true;
	}
};

//请求命令-获取笔记目录
struct RequestCmd_NoteCatalogQuery : RequestCmd
{
	QString strUserID;
	QList<NoteCatalog> listNoteCatalog;

	RequestCmd_NoteCatalogQuery()
	{
		nRequestID = Request_NoteCatalogQuery;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("strUserID", strUserID);
		QJsonArray _arry;
		for (const NoteCatalog & _info : listNoteCatalog)
			_arry.push_back(_info.getJsonObject());
		_obj.insert("listInfo", _arry);
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();
		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		strUserID = jsonObj.value("strUserID").toString();
		listNoteCatalog.clear();
		QJsonArray _array = jsonObj.value("listInfo").toArray();
		for (const QJsonValue & _val : _array)
		{
			NoteCatalog info;
			info.getDataFromJson(_val.toObject());
			listNoteCatalog.push_back(info);
		}
		return true;
	}
};

//请求命令-笔记目录修改
struct RequestCmd_NoteCatalogOperation : RequestCmd
{
	OperationType	nOperationTypeType;		//操作类型
	NoteCatalog		sNoteCatalog;			//笔记目录信息

	RequestCmd_NoteCatalogOperation()
	{
		nRequestID = Request_NoteCatalogOperation;
		nOperationTypeType = Operation_Unknown;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nOperationTypeType", (int)nOperationTypeType);
		_obj.insert("sNoteCatalog", sNoteCatalog.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nOperationTypeType = (OperationType)jsonObj.value("nOperationTypeType").toInt();
		sNoteCatalog.getDataFromJson(jsonObj.value("sNoteCatalog").toObject());
		return true;
	}
};

//笔记信息
struct NoteInfo
{
	int			nNotoID;			//笔记编号（自增主键）
	QString		strNoteTitle;		//笔记标题
	QString		strNoteContent;		//笔记内容
	int			nCatalogNode;		//目录节点
	QDateTime	dtCreateDateTime;	//创建时间
	QDateTime	dtUpdateDateTime;	//更新时间
	int			nRemind;			//是否提醒
	QDateTime	dtRemindDateTime;	//提醒时间

	NoteInfo()
	{
		nNotoID = 0;
		nCatalogNode = 0;
		nRemind = 0;

		dtCreateDateTime = QDateTime::currentDateTime();
		dtUpdateDateTime = QDateTime::currentDateTime();
		dtRemindDateTime = QDateTime::currentDateTime();
	}

	QString getJson() const
	{
		QJsonDocument _doc;
		_doc.setObject(getJsonObject());
		return _doc.toJson();
	}

	QJsonObject getJsonObject() const
	{
		QJsonObject _obj;
		_obj.insert("nNotoID", nNotoID);										//笔记编号（自增主键）
		_obj.insert("strNoteTitle", strNoteTitle);								//笔记标题
		_obj.insert("strNoteContent", strNoteContent);							//笔记内容
		_obj.insert("nCatalogNode", nCatalogNode);								//目录节点
		_obj.insert("dtCreateDateTime", dtCreateDateTime.toSecsSinceEpoch());	//创建时间
		_obj.insert("dtUpdateDateTime", dtUpdateDateTime.toSecsSinceEpoch());	//更新时间
		_obj.insert("nRemind", nRemind);										//是否提醒
		_obj.insert("dtRemindDateTime", dtRemindDateTime.toSecsSinceEpoch());	//提醒时间
		return _obj;
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QString & strJson)
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_GetProjectInfo");
			return false;
		}
		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_GetProjectInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();
		return getDataFromJson(jsonObj);
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QJsonObject & jsonObj)
	{
		nNotoID = jsonObj.value("nNotoID").toInt();
		strNoteTitle = jsonObj.value("strNoteTitle").toString();
		strNoteContent = jsonObj.value("strNoteContent").toString();
		nCatalogNode = jsonObj.value("nCatalogNode").toInt();
		dtCreateDateTime = QDateTime::fromSecsSinceEpoch(jsonObj.value("dtCreateDateTime").toInt());
		dtUpdateDateTime = QDateTime::fromSecsSinceEpoch(jsonObj.value("dtUpdateDateTime").toInt());
		nRemind = jsonObj.value("nRemind").toInt();
		dtRemindDateTime = QDateTime::fromSecsSinceEpoch(jsonObj.value("dtRemindDateTime").toInt());
		return true;
	}
};

//请求命令-获取笔记详细信息
struct RequestCmd_NoteQuery : RequestCmd
{
	QString strUserID;
	int nNoteID;
	NoteInfo sNoteInfo;

	RequestCmd_NoteQuery()
	{
		nRequestID = Request_NoteQuery;
		nNoteID = 0;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("strUserID", strUserID);
		_obj.insert("nNoteID", nNoteID);
		_obj.insert("sNoteInfo", sNoteInfo.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		strUserID = jsonObj.value("strUserID").toString();
		nNoteID = (RequestID)jsonObj.value("nNoteID").toInt();
		sNoteInfo.getDataFromJson(jsonObj.value("sNoteInfo").toObject());

		return true;
	}
};

//请求命令-笔记详细信息修改
struct RequestCmd_NoteOperation : RequestCmd
{
	OperationType	nOperationTypeType;		//操作类型
	NoteInfo		sNoteInfo;				//笔记信息

	RequestCmd_NoteOperation()
	{
		nRequestID = Request_NoteOperation;
		nOperationTypeType = Operation_Unknown;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("nOperationTypeType", (int)nOperationTypeType);
		_obj.insert("sNoteInfo", sNoteInfo.getJsonObject());
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();
		nOperationTypeType = (OperationType)jsonObj.value("nOperationTypeType").toInt();
		sNoteInfo.getDataFromJson(jsonObj.value("sNoteInfo").toObject());
		return true;
	}
};

//用户及其他信息
struct UserAssociatedInfo
{
	UserInfo sUserInfo;
	TeamInfo sTeamInfo;

	QString getJson() const
	{
		QJsonDocument _doc;
		_doc.setObject(getJsonObject());
		return _doc.toJson();
	}

	QJsonObject getJsonObject() const
	{
		QJsonObject _obj;
		_obj.insert("sUserInfo", sUserInfo.getJsonObject());
		_obj.insert("sTeamInfo", sTeamInfo.getJsonObject());
		return _obj;
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QString & strJson)
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_GetProjectInfo");
			return false;
		}
		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_GetProjectInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();
		return getDataFromJson(jsonObj);
	}

	//根据获取的Json获取具体的数据
	bool getDataFromJson(const QJsonObject & jsonObj)
	{
		sUserInfo.getDataFromJson(jsonObj.value("sUserInfo").toObject());
		sTeamInfo.getDataFromJson(jsonObj.value("sTeamInfo").toObject());
		return true;
	}
};

//请求命令-用户信息搜索
struct RequestCmd_UserSearch : RequestCmd
{
	QString						strSearchText;			//查找文本（如果为精确查找，则直接为用户ID）
	int							nExactMatch;			//精确查找（匹配）
	QList<UserAssociatedInfo>	listUserAssociatedInfo;	//用户信息

	RequestCmd_UserSearch()
	{
		nRequestID = Request_UserSearch;
		nExactMatch = 0;
	}

	//根据数据获取Json字符串
	virtual QString getJsonFromData() const override
	{
		QJsonDocument _doc;
		QJsonObject _obj;
		_obj.insert("nRequestID", nRequestID);
		_obj.insert("nRet", nRet);
		_obj.insert("strError", strError);
		_obj.insert("strSearchText", strSearchText);
		_obj.insert("nExactMatch", nExactMatch);
		QJsonArray _arry;
		for (const auto & _info : listUserAssociatedInfo)
			_arry.push_back(_info.getJsonObject());
		_obj.insert("listUserAssociatedInfo", _arry);
		_doc.setObject(_obj);
		return _doc.toJson();
	}

	//根据获取的Json获取具体的数据
	virtual bool getDataFromJson(const QString & strJson) override
	{
		if (strJson.isEmpty())
		{
			qDebug() << QString("%1 Json为空,").arg("Cmd_CheckUserInfo");
			return false;
		}

		QJsonParseError parseJsonErr;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &parseJsonErr);
		if (!parseJsonErr.error == QJsonParseError::NoError)
		{
			qDebug() << QString("%1 Json解析错误!Json:%2").arg("Cmd_CheckUserInfo").arg(strJson);
			return false;
		}
		QJsonObject jsonObj = jsonDoc.object();

		nRequestID = (RequestID)jsonObj.value("nRequestID").toInt();
		nRet = jsonObj.value("nRet").toInt();
		strError = jsonObj.value("strError").toString();

		strSearchText = jsonObj.value("strSearchText").toString();
		nExactMatch = jsonObj.value("nExactMatch").toInt();

		listUserAssociatedInfo.clear();
		QJsonArray _array = jsonObj.value("listUserAssociatedInfo").toArray();
		for (const QJsonValue & _val : _array)
		{
			UserAssociatedInfo info;
			info.getDataFromJson(_val.toObject());
			listUserAssociatedInfo.push_back(info);
		}
		return true;
	}
};

#endif