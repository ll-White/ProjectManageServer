#include "WnDialog.h"
#include <QMouseEvent>
#include <QDebug>

WnDialog::WnDialog(QWidget * parent, bool bMaxWindw, bool MoveforParent,Qt::WindowFlags f)
	: QDialog(parent, f), m_bisMaxWindow(bMaxWindw), m_bisMoveforParent(MoveforParent)
{
	m_isDraggingWindow = false;
	m_nClickedY = 40;
}

WnDialog::~WnDialog()
{
}

void WnDialog::mousePressEvent(QMouseEvent *event)
{
	int x = event->pos().x();
	int y = event->pos().y();
	int x1 = 0;
	int y1 = 0;
	int x2 = x1 + this->width();
	int y2 = y1 + m_nClickedY;
	if (x > x1 && x < x2 && y > y1 && y < y2) 
	{
		if (m_bisMoveforParent)
		{
			//qDebug() << "first-global:" << event->globalPos();
			//qDebug() << "first-window:" << event->windowPos();
			//qDebug() << "first-screen:" << event->screenPos();
			//qDebug() << "first-local:" << event->localPos();

			m_isDraggingWindow = true;
		}
		else
		{
			m_lastMousePos = event->pos();
			m_isDraggingWindow = true;
		}
	}
	return QDialog::mousePressEvent(event);
}

void WnDialog::mouseMoveEvent(QMouseEvent *event)
{
	if (m_isDraggingWindow)
	{
		if (m_bisMoveforParent)
		{
			//qDebug() << "first-global:" << event->globalPos();
			//qDebug() << "first-window:" << event->windowPos();		//父类左上角
			//qDebug() << "first-screen:" << event->screenPos();
			//qDebug() << "first-local:" << event->localPos();		//本窗口坐标（左上角）

			this->move(event->globalPos());
		}
		else
		{
			this->move(event->globalPos() - m_lastMousePos);
		}
	}
	//return QWidget::mouseMoveEvent(event);
}

void WnDialog::mouseReleaseEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	m_isDraggingWindow = false;
	return QDialog::mouseReleaseEvent(event);
}

void WnDialog::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (m_bisMaxWindow)
	{
		int x = event->pos().x();
		int y = event->pos().y();
		int x1 = 0;
		int y1 = 0;
		int x2 = x1 + this->width();
		int y2 = y1 + 40;
		if (x > x1 && x < x2 && y > y1 && y < y2)
		{
			if (this->isMaximized())
				showNormal();
			else
				showMaximized();
		}
	}
	//return QDialog::mouseDoubleClickEvent(event);
}

void WnDialog::keyPressEvent(QKeyEvent * event)
{
	if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
		return;
	return QDialog::keyPressEvent(event);
}
