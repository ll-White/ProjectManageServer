#pragma once

#include <QObject>
#include <QDebug>
#include <QMutex>

//#if _MSC_VER >= 1600  
//#pragma execution_character_set("utf-8")  
//#endif

class WnLog : public QObject
{
	Q_OBJECT

public:
	static void GeneralLog(QString, QString);		//综合日志     参数：名称、日志
	static void ThreadLog(QString, QString);		//线程日志		参数：名称、日志
	
	static void setShowLog(QString);				//设置日志 - 展示在主画面
	static QList<QString> getShowLog();				//获取日志 - 展示在主画面
private:
	WnLog(QObject *parent);
	~WnLog();

private:

	static QMutex s_Mutex_General;
	static QMutex s_Mutex_Thread;

	static QList<QString> s_listShowLog;
	static QMutex s_Mutext_Show;
};

//输出日志信息
extern void OutputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg);