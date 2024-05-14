#pragma once
#include "lockqueue.h"
#include <string>

// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
// 定义宏 LOG_INFO，用于输出 INFO 级别的日志信息
// logmsgformat: 格式化字符串，类似于 printf 函数的格式化字符串
// ...: 可变参数，对应格式化字符串中的参数
//当宏的内容跨越多行时,（\）来指示宏定义尚未结束的情况
//使用snprintf函数将格式化字符串和可变参数拼接成日志消息，最后调用logger.Log()方法输出日志。
//##__VA_ARGS__ 表示可变数量的参数列表
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

// 定义日志级别
enum LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
};

// Mprpc框架提供的日志系统
class Logger
{
public:
    // 获取日志的单例
    static Logger& GetInstance();
    // 设置日志级别 
    void SetLogLevel(LogLevel level);
    // 写日志
    void Log(std::string msg);
private:
    int m_loglevel; // 记录日志级别
    LockQueue<std::string>  m_lckQue; // 日志缓冲队列

//强制要求只能通过默认构造函数来创建Logger对象，从而避免了不必要的资源拷贝或移动操作。
    Logger();
    Logger(const Logger&) = delete; //表示不允许使用拷贝构造函数创建对象。
    Logger(Logger&&) = delete;
};