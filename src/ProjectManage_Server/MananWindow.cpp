#include "MananWindow.h"
#include "WnLog.h"
#include "TaskCheckRunnable.h"
#include <QThreadPool>

MananWindow::MananWindow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

	ui.textEdit->document()->setMaximumBlockCount(300);
	ui.textEdit->setContextMenuPolicy(Qt::NoContextMenu);

	m_pTimerCheckLog = new QTimer(this);
	connect(m_pTimerCheckLog,&QTimer::timeout,this,&MananWindow::slotReadLog);
	m_pTimerCheckLog->start(1000);

	m_pTcpServer = new WnTCPServer;
	m_pTcpServer->ServerListen(7788);
}

void MananWindow::slotReadLog()
{
	QList<QString> listVal = WnLog::getShowLog();
	for (auto strLog : listVal)
	{
		ui.textEdit->append(strLog);
		ui.textEdit->moveCursor(QTextCursor::End);
	}

	if (QDateTime::currentDateTime().toSecsSinceEpoch() - m_nLastCheckTask > 60)
	{
		m_nLastCheckTask = QDateTime::currentDateTime().toSecsSinceEpoch();
		//新增任务过期的计算
		TaskCheckRunnable * pRunable = new TaskCheckRunnable;
		pRunable->setAutoDelete(true);
		QThreadPool::globalInstance()->start(pRunable);
	}
}
