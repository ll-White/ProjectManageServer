#include "SqlQueryEx.h"

#include <QThread>
#include <QMutexLocker>
#include "WnLog.h"

QMutex SqlQueryEx::s_SQLMutex;
DBConfig SqlQueryEx::s_DBConfigInfo;

std::atomic<unsigned int> SqlQueryEx::s_nDBConenctNum(0);

QString SqlQueryEx::getNewConnectName()
{
	QMutexLocker mutexlocker(&s_SQLMutex);
	QString strConnect = QString("TempDBConnect_%1").arg(s_nDBConenctNum++);
	if (s_nDBConenctNum >= 0xfffffff)
		s_nDBConenctNum = 1;
	return strConnect;
}


bool ThreadExecSQL(QString _Sql)
{
	SqlQueryEx query(SqlQueryEx::s_DBConfigInfo);
	if (!query.exec(_Sql))
	{
		WnLog::GeneralLog("ThreadExecSQL", QString("SQL执行失败,原因:%1 SQL:%2").arg(query.getLastError()).arg(_Sql));
		return false;
	}
	return true;
}

QList<QMap<QString, QVariant>> ThreadSelectDB(QString _Sql)
{
	QList<QMap<QString, QVariant>> listVal;
	SqlQueryEx query(SqlQueryEx::s_DBConfigInfo);
	if (!query.exec(_Sql))
		WnLog::GeneralLog("ThreadSelectDB", QString("SQL执行失败,原因:%1 SQL:%2").arg(query.getLastError()).arg(_Sql));
	while (query.next())
	{
		QMap<QString, QVariant> tempVal;
		for (int i = 0; i < query.record().count(); i++)
		{
			QString strFieldName = query.record().fieldName(i);
			QVariant vVal = query.record().value(i);
			tempVal[strFieldName] = vVal;
		}
		listVal.push_back(tempVal);
	}
	return listVal;
}

bool SqlQueryEx::InitDatabase(QString strConnect, DBConfig cfg, QString * pStrError)
{
	QMutexLocker mutexlocker(&s_SQLMutex);
	if (cfg.strDBIP.isEmpty())
	{
		QString strError = "数据库连接信息错误!";
		qCritical() << strError;
		if (pStrError)
			*pStrError = strError;
		return false;
	}

	if (cfg.nType == DBType::SqlServere)
	{
		QSqlDatabase db;
		if (QSqlDatabase::contains(strConnect))
			db = QSqlDatabase::database(strConnect);
		else
			db = QSqlDatabase::addDatabase("QODBC", strConnect);
		db.setDatabaseName(QString("DRIVER={SQL SERVER};SERVER=%1,1433;DATABASE=%2;UID=%3;PWD=%4;").arg(cfg.strDBIP).arg(cfg.strDBName).arg(cfg.strDBUser).arg(cfg.strDBPwd));
		if (!db.open())
		{
			QString str = db.lastError().databaseText();
			qDebug() << QString("MarkInfo写入时连接SQLServer数据库链接失败：ip:%1, 数据库名：%2").arg(cfg.strDBIP).arg(cfg.strDBName);
		}
	}
	else if(cfg.nType == DBType::MySql)
	{
		QSqlDatabase db;
		if (QSqlDatabase::contains(strConnect))
		{
			db = QSqlDatabase::database(strConnect);
			if (db.isOpen()) return true;
		}
		else
		{
			db = QSqlDatabase::addDatabase("QMYSQL", strConnect);
		}
		db.setHostName(cfg.strDBIP);
		db.setDatabaseName(cfg.strDBName);
		db.setUserName(cfg.strDBUser);
		db.setPort(cfg.nDBPort);
		db.setPassword(cfg.strDBPwd);
		db.setConnectOptions("MYSQL_OPT_RECONNECT=1");

		qDebug() << "初始化连接:" << strConnect;

		if (!db.open())
		{
			QString strError = QString("数据库链接失败, IP:%1 ,Port:%2 ,连接名:%3 原因:%4").arg(cfg.strDBIP).arg(cfg.nDBPort).arg(strConnect).arg(db.lastError().databaseText());
			qCritical() << strError;
			if (pStrError)
				*pStrError = strError;
			return false;
		}
	}
	return true;
}

bool SqlQueryEx::SelectDB(QString strSql, QList<QMap<QString, QVariant>> * pListAllRecord, QString strConnect, QString * pStrError)
{
	QString strThreadInfo = QThread::currentThread()->property("ThreadInfo").toString();
	QMutexLocker Mutexlocker(&s_SQLMutex);
	if (!QSqlDatabase::database(strConnect).isOpen())
	{
		qInfo() << "数据库查询-连接名:" << strConnect << ",数据库未打开!";
		QSqlDatabase::database(strConnect).close();
		if (!QSqlDatabase::database(strConnect).open())
		{
			QString strError = QString("数据库查询-连接名:%1，打开数据库失败,原因:%2").arg(strConnect).arg(QSqlDatabase::database(strConnect).lastError().databaseText());
			qInfo() << strError;
			if (pStrError)
				*pStrError = strError;

			return false;
		}
	}

	if (pListAllRecord == nullptr) return false;
	pListAllRecord->clear();
	QSqlQuery query(QSqlDatabase::database(strConnect));
	if (!query.exec(strSql))
	{
		QString strError = QString("数据库查询-连接名:%1,，初次执行失败,原因:%2").arg(strConnect).arg(query.lastError().databaseText());
		qInfo() << strError;
		QSqlDatabase::database(strConnect).close();
		if (!QSqlDatabase::database(strConnect).open())
		{
			strError = QString("数据库查询-连接名:%1,，打开数据库失败,原因:%2").arg(strConnect).arg(query.lastError().databaseText());
			qInfo() << strError;
			if (pStrError)
				*pStrError = strError;
			return false;
		}
		if (!query.exec(strSql))
		{
			strError = QString("数据库查询-连接名:%1,，再次执行失败,原因:%2").arg(strConnect).arg(query.lastError().databaseText());
			qInfo() << strError;
			if (pStrError)
				*pStrError = strError;
			return false;
		}
	}
	while (query.next())
	{
		QMap<QString, QVariant> tempVal;
		for (int i = 0; i < query.record().count(); i++)
		{
			QString strFieldName = query.record().fieldName(i);
			QVariant vVal = query.record().value(i);
			tempVal[strFieldName] = vVal;
		}
		pListAllRecord->push_back(tempVal);
	}
	return true;
}

bool SqlQueryEx::ExecSQL(QString strSql, QString strConnect, QString * pStrError)
{
	QString strThreadInfo = QThread::currentThread()->property("ThreadInfo").toString();
	QMutexLocker Mutexlocker(&s_SQLMutex);
	if (!QSqlDatabase::database(strConnect).isOpen())
	{
		qInfo() << "数据库执行-连接名:" << strConnect << ",数据库未打开!";
		QSqlDatabase::database(strConnect).close();
		if (!QSqlDatabase::database(strConnect).open())
		{
			QString strError = QString("数据库执行-连接名:%1，打开数据库失败,原因:%2").arg(strConnect).arg(QSqlDatabase::database(strConnect).lastError().databaseText());
			if (pStrError)
				*pStrError = strError;
			return false;
		}
	}
	QSqlQuery query(QSqlDatabase::database(strConnect));
	if (!query.exec(strSql))
	{
		QString strError = QString("数据库执行-连接名:%1,，初次执行失败,原因:%2").arg(strConnect).arg(query.lastError().databaseText());
		qInfo() << strError;
		QSqlDatabase::database(strConnect).close();
		if (!QSqlDatabase::database(strConnect).open())
		{
			strError = QString("数据库执行-连接名:%1,，打开数据库失败,原因:%2").arg(strConnect).arg(query.lastError().databaseText());
			qInfo() << strError;
			if (pStrError)
				*pStrError = strError;
			return false;
		}
		if (!query.exec(strSql))
		{
			strError = QString("数据库执行-连接名:%1,，再次执行失败,原因:%2").arg(strConnect).arg(query.lastError().databaseText());
			qInfo() << strError;
			if (pStrError)
				*pStrError = strError;
			return false;
		}
	}
	return true;
}

QFuture<bool> SqlQueryEx::ExecSQL_Thread(QString strSql)
{
	return QtConcurrent::run(ThreadExecSQL, strSql);
}

QFuture<QList<QMap<QString, QVariant>>> SqlQueryEx::SelectDB_Thread(QString strSql)
{
	return QtConcurrent::run(ThreadSelectDB, strSql);
}

SqlQueryEx::SqlQueryEx(DBConfig cfg,QString strConnect)
{
	if (cfg.strDBIP.isEmpty())
		m_DBConfigInfo = SqlQueryEx::s_DBConfigInfo;
	else
		m_DBConfigInfo = cfg;
	m_bAutomatic = false;
	if (strConnect.isEmpty())
	{
		m_strConnect = QThread::currentThread()->property(ThreadSQL.toLatin1()).toString();
		if (m_strConnect.isEmpty() || m_strConnect == "" || m_strConnect.length() == 0)
		{
			m_bAutomatic = true;
			m_strConnect = QString("TempDBConnect_%1").arg(s_nDBConenctNum++);
			if (s_nDBConenctNum >= 0xffffff)
				s_nDBConenctNum = 1;
			InitDatabase(m_strConnect, m_DBConfigInfo);
		}
	}
	else
	{
		m_strConnect = strConnect;
	}
	m_Query = QSqlQuery(QSqlDatabase::database(m_strConnect));
}

SqlQueryEx::~SqlQueryEx()
{
	if (m_bAutomatic)
	{
		QSqlDatabase::database(m_strConnect).close();
		QSqlDatabase::removeDatabase(m_strConnect);
		m_bAutomatic = false;
	}
}

void SqlQueryEx::setConnectInfo(QString strConnectName)
{
	if (m_bAutomatic)
	{
		QSqlDatabase::database(m_strConnect).close();
		QSqlDatabase::removeDatabase(m_strConnect);
		m_bAutomatic = false;
	}
	m_strConnect = strConnectName;
	m_Query = QSqlQuery(QSqlDatabase::database(m_strConnect));
}

bool SqlQueryEx::exec(QString strSql)
{
	qint64 nTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
	QString strThreadInfo = QThread::currentThread()->property(ThreadInfo.toLatin1().data()).toString();
	if (!m_Query.exec(strSql))
	{
		qInfo() << "SqlQueryEx-执行SQL语句失败,原因:" << m_Query.lastError().databaseText();
		QSqlDatabase::database(m_strConnect).close();
		if (InitDatabase(m_strConnect,m_DBConfigInfo))
			qInfo() << "SqlQueryEx-打开数据库连接失败:" << m_strConnect;
		qInfo() << "SqlQueryEx-进行重连后再次执行";
		return m_Query.exec(strSql);
	}
	qint64 nTime2 = QDateTime::currentDateTime().toMSecsSinceEpoch();
	if (nTime2 - nTime > 2000)
		qInfo() << "读取数据库时间过长:" << nTime2 - nTime;

	return true;
}

