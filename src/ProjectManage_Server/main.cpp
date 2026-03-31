#include "MananWindow.h"
#include <QtWidgets/QApplication>
#include <QThreadPool>
#include "SqlQueryEx.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	unsigned int numCores = std::thread::hardware_concurrency();
	QThreadPool::globalInstance()->setMaxThreadCount(numCores);

	QThread::currentThread()->setProperty(ThreadSQL.toLatin1(), CONNECT_MAIN);
	QThread::currentThread()->setProperty(ThreadInfo.toLatin1(), "Main");

	SqlQueryEx::s_DBConfigInfo.strDBIP = "127.0.0.1";
	SqlQueryEx::s_DBConfigInfo.nDBPort = 3306;
	SqlQueryEx::s_DBConfigInfo.strDBName = "projectmanage";
	SqlQueryEx::s_DBConfigInfo.strDBUser = "root";
	SqlQueryEx::s_DBConfigInfo.strDBPwd = "123123";

	QString strError;
	if (!SqlQueryEx::InitDatabase(CONNECT_MAIN,SqlQueryEx::s_DBConfigInfo, &strError))
	{
		qDebug() << "连接数据库失败!";
		return 0;
	}

    MananWindow w;
    w.show();
    return a.exec();
}
