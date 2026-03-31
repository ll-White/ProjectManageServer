#include "TaskCheckRunnable.h"
#include "SqlQueryEx.h"
#include "WnLog.h"

TaskCheckRunnable::TaskCheckRunnable()
{
}

TaskCheckRunnable::~TaskCheckRunnable()
{
}

void TaskCheckRunnable::run()
{
	//检测到时间启动、超时等检查
	
	//待启动
	QString strSql = QString("update %1 set TaskStatus = %2 where TaskStatus < %3 and Now() < StartDate;").arg(TB_TASK).arg(Task_WaitStart).arg(Task_Completion);
	//进行中
	strSql += QString("update %1 set TaskStatus = %2 where TaskStatus < %3 and StartDate < Now() and Now() < EndDate;").arg(TB_TASK).arg(Task_Runing).arg(Task_Completion);
	//已超时的任务
	strSql += QString("update %1 set TaskStatus = %2 where TaskStatus < %3 and EndDate < Now();").arg(TB_TASK).arg(Task_TimeOut).arg(Task_Completion);

	SqlQueryEx query;
	if (!query.exec(strSql))
	{
		QString strError = query.getLastError();
		qDebug() << strError;
		WnLog::setShowLog(strError);
	}
}
