#pragma once

#include <QtWidgets/QWidget>
#include <QTimer>

#include "ui_MananWindow.h"
#include "WnTCPServer.h"

class MananWindow : public QWidget
{
    Q_OBJECT

public:
    MananWindow(QWidget *parent = Q_NULLPTR);

private:
	void slotReadLog();

private:
	WnTCPServer * m_pTcpServer = nullptr;
	QTimer * m_pTimerCheckLog = nullptr;

	qint64 m_nLastCheckTask = 0;

    Ui::MananWindowClass ui;
};
