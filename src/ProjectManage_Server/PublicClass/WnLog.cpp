#include "WnLog.h"
#include <QDir>
#include <QApplication>
#include <QMutex>
#include <QDateTime>
#include <QTextStream>

QMutex WnLog::s_Mutex_General;
QMutex WnLog::s_Mutex_Thread;

QMutex WnLog::s_Mutext_Show;
QList<QString> WnLog::s_listShowLog;

void WnLog::GeneralLog(QString strName, QString strLog)
{
	s_Mutex_General.lock();

	QString _WriteLog = QString("[%1] %2: %3").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(strName).arg(strLog);

	//校验目录是否存在
	int nHour = ((int)(QTime::currentTime().hour() / 6)) + 1;
	QString strFile = QString("%1/Log/%2_General_log_%3.txt").arg(QApplication::applicationDirPath()).arg(QDateTime::currentDateTime().toString("yyyyMMdd")).arg(nHour);

	QFile file(strFile);
	if (!file.exists())
	{
		file.open(QIODevice::WriteOnly);
		file.close();
	}

	file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream text_stream(&file);
	text_stream << _WriteLog << "\r\n";
	file.flush();
	file.close();

	s_Mutex_General.unlock();
}

void WnLog::ThreadLog(QString strName,QString strLog)
{
	s_Mutex_Thread.lock();

	QString _WriteLog = QString("[%1] %2: %3").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(strName).arg(strLog);

	//校验目录是否存在
	int nHour = ((int)(QTime::currentTime().hour() / 6)) + 1;
	QString strFile = QString("%1/Log/%2_Thread_log_%3.txt").arg(QApplication::applicationDirPath()).arg(QDateTime::currentDateTime().toString("yyyyMMdd")).arg(nHour);

	QFile file(strFile);
	if (!file.exists())
	{
		file.open(QIODevice::WriteOnly);
		file.close();
	}

	file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream text_stream(&file);
	text_stream << _WriteLog << "\r\n";
	file.flush();
	file.close();

	s_Mutex_Thread.unlock();
}

void WnLog::setShowLog(QString strLg)
{
	s_Mutext_Show.lock();
	s_listShowLog.push_back(QString("[%1]%2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(strLg));
	if (s_listShowLog.size() > 100)
		s_listShowLog.removeFirst();
	s_Mutext_Show.unlock();
}

QList<QString> WnLog::getShowLog()
{
	QMutexLocker _locker(&s_Mutext_Show);
	QList<QString> tempLog = s_listShowLog;
	s_listShowLog.clear();
	return tempLog;
}

WnLog::WnLog(QObject *parent)
	: QObject(parent)
{
}

WnLog::~WnLog()
{
}

void OutputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	qint64 tStart = QDateTime::currentDateTime().toSecsSinceEpoch();

	static QMutex mutex;
	if (mutex.tryLock(50) == false) return;
	QString strText;
	switch (type)
	{
	case QtWarningMsg:		//警告
		strText = QString("Warning:");
		break;
	case QtCriticalMsg:		//严重的
		strText = QString("Critical:");
		break;
	case QtFatalMsg:		//毁灭的
		strText = QString("Fatal:");
		break;
	case QtInfoMsg:
		strText = QString("Info:");
		break;
	case QtDebugMsg:		//调试		
	default:
		strText = QString("Debug:");
	}

	QString context_info = QString("File:(%1) Line:(%2)").arg(context.file).arg(context.line);
	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_date = QString("%1").arg(current_date_time.toString("yyyy-MM-dd hh:mm:ss:zzz"));
	QString message = QString("%1 %2 %3 %4").arg(strText).arg(context_info).arg(msg).arg(current_date);

	//校验目录是否存在
	QString strDir = QString("%1/Log").arg(QApplication::applicationDirPath());
	QDir* pDir = new QDir(strDir);
	if (!pDir->exists())
		pDir->mkdir(strDir);
	delete pDir;
	pDir = NULL;

	QString strFile = QString("%1/%2_log.txt").arg(strDir).arg(current_date_time.toString("yyyy-MM-dd"));
	QFile file(strFile);
	if (!file.exists())
	{
		file.open(QIODevice::WriteOnly);
		file.close();
	}

	file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream text_stream(&file);
	text_stream << message << "\r\n";
	file.flush();
	file.close();
	mutex.unlock();

	double dbTime = QDateTime::currentDateTime().toSecsSinceEpoch() - tStart;
	if (dbTime > 1000)
	{
		file.open(QIODevice::WriteOnly | QIODevice::Append);
		QTextStream text_stream2(&file);
		text_stream2 << message << "写这句话大于1000毫秒 \r\n";
		file.flush();
		file.close();
	}
}