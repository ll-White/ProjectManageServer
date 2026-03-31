#pragma once

#include <QObject>
#include "WnLog.h"

class WnDebugLog : public QObject
{
	Q_OBJECT

public:
	inline explicit WnDebugLog(QString _msg, QtMsgType _MsgType = QtDebugMsg, QObject *parent = nullptr)
	{
		m_strMsg = _msg;
		m_MsgType = _MsgType;
		if (m_MsgType != QtDebugMsg)
		{
			switch (m_MsgType)
			{
			case QtDebugMsg:
				qDebug() << _msg << "==== Start";
				break;
			case QtWarningMsg:
				qWarning() << _msg << "==== Start";
				break;
			case QtCriticalMsg:
				qCritical() << _msg << "==== Start";
				break;
			case QtFatalMsg:
				qFatal(QString("%1 ==== Start").arg(m_strMsg).toLatin1().data());
				break;
			case QtInfoMsg:
				qInfo() << _msg << "==== Start";
				break;
			default:
				qDebug() << _msg << "==== Start";
				break;
			}
		}
		else
		{
			qDebug() << _msg << "==== Start";
		}
	}

	inline ~WnDebugLog()
	{
		if (m_MsgType != QtDebugMsg)
		{
			switch (m_MsgType)
			{
			case QtDebugMsg:
				qDebug() << m_strMsg << "==== End";
				break;
			case QtWarningMsg:
				qWarning() << m_strMsg << "==== End";
				break;
			case QtCriticalMsg:
				qCritical() << m_strMsg << "==== End";
				break;
			case QtFatalMsg:
				qFatal(QString("%1 ==== End").arg(m_strMsg).toLatin1().data());
				break;
			case QtInfoMsg:
				qInfo() << m_strMsg << "==== End";
				break;
			default:
				qDebug() << m_strMsg << "==== End";
				break;
			}
		}
		else
		{
			qDebug() << m_strMsg << "==== End";
		}
	}

private:
	QString m_strMsg;
	QtMsgType m_MsgType;
};
