#ifndef KSSHAREHEADER_H
#define KSSHAREHEADER_H
//公用头文件
#include <QMutex>
#include <QMutexLocker>
#include <QMessageBox>
#include <QDateTime>
#include <qDebug>

#include <QSqlError>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>
#include <QThread>
#include <cmath>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")  
#endif 

#define _DEBUGLOG 0					//Debug 日志

#define TcpEnd "E~n!d@"

//==========宏定义==========

//π的值
constexpr float PI = std::atan(1.0f) * 4;
//读取指定位的值
#define GET_BIT(value,bit) (value&(1<<bit))		

//======================================================== 全局参数 ========================================================
extern QMutex  g_Mutexlock;					//全局线程锁（主要用于初始化数据库函数）

//======================================================== 全局方法 ========================================================
//获取本地IP
extern QString GetLocalIp(QString strIpComp = "");
//获取本地全部IP
extern QList<QString> GetLocalIPs();
//初始化全局基本信息
extern void initGlobalData();					
//RGB转Int
extern int RGB2Int(int * rgb);					
//int转RGB
extern void Int2RGB(int & nRGB, int * RGB);	
#endif