#pragma once

#include <QRunnable>
#include "ShareStructServer.h"

class TaskCheckRunnable : public QRunnable
{

public:
	TaskCheckRunnable();
	~TaskCheckRunnable();

protected:
	void run() override;
};
