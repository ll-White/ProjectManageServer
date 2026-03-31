#pragma once

#include <atomic>
#include <QString>
#include <QMap>
#include <QList>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

#define CONNECT_MAIN "Connect_Main"

enum DBType
{
	MySql,
	SqlServere,
	SQliter
};

struct DBConfig
{
	DBType	nType;				//数据库类型
	QString strDBIP;			//数据库-IP
	int		nDBPort;			//数据库-端口
	QString strDBName;			//数据库-名称
	QString strDBUser;			//数据库-用户名
	QString strDBPwd;			//数据库-密码
};

//==========宏定义==========
//QThread::currentThread()->setProperty(ThreadSQL.toLatin1(), CONNECT_MAIN);
//QThread::currentThread()->setProperty(ThreadInfo.toLatin1(), "Main");
const QString ThreadSQL = "ThreadSQL";		//线程SQL连接名称
const QString ThreadInfo = "ThreadInfo";	//线程用途说明

class SqlQueryEx
{
public:
	static DBConfig s_DBConfigInfo;
	static QString getNewConnectName();
	static bool InitDatabase(QString strConnect, DBConfig  cfg,QString * pStrError = nullptr);
	static bool SelectDB(QString strSql, QList<QMap<QString, QVariant>> *, QString strConnect, QString * pStrError = nullptr);
	static bool ExecSQL(QString strSql, QString strConnect, QString * pStrError = nullptr);

	//线程中执行SQL语句,调用QtConcurrent::run方法，
	//线程查询SQL,基本使用方法
	//QString strSql = QString();
	//QFutureWatcher<QList<QMap<QString, QVariant>>> * pWatcher = new QFutureWatcher<QList<QMap<QString, QVariant>>>();
	//QObject::connect(pWatcher, &QFutureWatcher<QList<QMap<QString, QVariant>>>::finished, [=]
	//{
	//	QList<QMap<QString, QVariant>> lstVal = pWatcher->result();
	//	for (QMap<QString, QVariant> mapVal : lstVal)
	//	{
	//	}
	//	pWatcher->deleteLater();
	//});
	//pWatcher->setFuture(SqlQueryEx::SelectDB_Thread(strSql));
	static QFuture<bool> ExecSQL_Thread(QString strSql);
	static QFuture<QList<QMap<QString, QVariant>>> SelectDB_Thread(QString strSql);

	//1.主线程使用时需要再main函数调用 QThread::currentThread()->setProperty(ThreadSQL.toLatin1(), "数据库连接名")。主线程使用该类时无需给入连接名，且不会主动关闭连接
	//2.使用继承QThread、MoveToThread 方法时，需要在主线程时调用 QThread线程指针setProperty(ThreadSQL.toLatin1(), "数据库连接名");，线程使用时无需给入连接名，且不会主动关闭连接
	//3.使用QRunable(QThreadTool)、QtConcurrent::run，std::thread 等无法在主线程获取QThread指针时，直接使用，会自动生成连接名，并在该类的析构时自动关闭该连接名并移除
	//4.传入连接名，需自己根据线程识别连接名（不同线程之间连接名应该不相同，否则QSqlDataBase容易出现异常），不会主动关闭连接
	SqlQueryEx(DBConfig  cfg = DBConfig(),QString strConnect = QString());
	~SqlQueryEx();

	//设置获取连接名称
	void setConnectInfo(QString strConnectName);
	inline QString getConnectName() { return m_strConnect; };

	bool exec(QString strSql);
	inline QString getLastError() { return m_Query.lastError().databaseText(); };
	inline bool next() { return m_Query.next(); };
	inline QVariant value(QString strField) { return m_Query.value(strField); };
	inline QVariant value(int nIndex) { return m_Query.value(nIndex); };
	inline int size() { return m_Query.size(); };
	inline int numRowsAffected() { return m_Query.numRowsAffected(); };
	inline QSqlQuery & getQuery() { return m_Query; };
	inline QSqlRecord record() { return m_Query.record(); };

private:
	QSqlQuery m_Query;
	QString m_strConnect;
	DBConfig m_DBConfigInfo;

	static QMutex s_SQLMutex;
	static std::atomic<unsigned int> s_nDBConenctNum;

	bool m_bAutomatic;
};
