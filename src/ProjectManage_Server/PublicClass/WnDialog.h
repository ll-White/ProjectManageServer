#pragma once
#include <QDialog>

class WnDialog : public QDialog
{
	Q_OBJECT

public:
	WnDialog(QWidget * parent = nullptr, bool bMaxWindow = false,bool MoveforParent = false, Qt::WindowFlags f = Qt::WindowFlags());
	~WnDialog();

	void setMoveforParent(bool MoveParent) { m_bisMoveforParent = MoveParent; };

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *);
	inline void setClickedY(int nY = 40) { m_nClickedY = nY; };

	bool m_isDraggingWindow;
	QPoint m_lastMousePos;
	bool m_bisMaxWindow;

private:
	bool m_bisMoveforParent;
	int m_nClickedY;
};