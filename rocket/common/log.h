#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H
#include<memory>
#include<vector>
#include<string>
#include<queue>
#include"rocket/common/config.h"
#include"rocket/common/mutex.h"

namespace rocket{
enum LogLevel{
    Unkown,
    Debug,
    Info,
    Error
};
template<typename... Args>
std::string formatString(const char*str,Args&&... args)
{
    int size = snprintf(nullptr,0,str,args...);//返回字符串长度
    std::string result;
    if(size>0)
    {
        result.resize(size);
        snprintf(&result[0],size+1,str,args...);//str 表示格式字符串，args是源字符串
    }
    return result;

}
#define DEBUGLOG(str,...)\
    if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Debug)\
    {\
    rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Debug))->toString()+"["+std::string(__FILE__)+":"+std::to_string(__LINE__)+"]\t"+ rocket::formatString(str,##__VA_ARGS__)+"\n");\
    rocket::Logger::GetGlobalLogger()->log();\
    }\

#define INFOLOG(str, ...)\
    if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Info)\
    {\
    rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Info))->toString()+"["+std::string(__FILE__)+":"+std::to_string(__LINE__)+"]\t"+ rocket::formatString(str,##__VA_ARGS__)+"\n");\
    rocket::Logger::GetGlobalLogger()->log();\
    }\

#define ERRORLOG(str, ...)\
    if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Error)\
    {\
    rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Error))->toString()+"["+std::string(__FILE__)+":"+std::to_string(__LINE__)+"]\t"+ rocket::formatString(str,##__VA_ARGS__)+"\n");\
    rocket::Logger::GetGlobalLogger()->log();\
    }\

std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(const std::string &log_level);
class Logger{
    public:
    typedef std::shared_ptr<Logger> s_ptr;
    void pushLog(const std::string& msg);//使用队列存放logmsg
    void log();
    Logger(LogLevel level):m_set_level(level){}
    public:
    LogLevel getLogLevel() const{
        return m_set_level;
    }
    static Logger* GetGlobalLogger();   
    static void InitGlobalLogger();
    private:
    LogLevel m_set_level;
    std::queue<std::string> m_buffer;
    Mutex m_mutex;
};

class LogEvent{
    public:
    LogEvent (LogLevel level): m_level(level){};
    std::string getFileName()
    const{
        return m_file_name;
    }
    LogLevel geLogLevel()
    const{
        return m_level;
    }
    std::string toString();
    private:
    std::string m_file_name;//文件名
    std::string m_file_line;//行号
    int m_pid;//进程号
    int m_thread_id;//线程号
    LogLevel m_level;//日志级别
    
};

}
#endif