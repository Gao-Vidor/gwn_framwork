#ifndef __gwn_LOG_H__
#define __gwn_LOG_H__

#include <string>
#include <memory>
#include <list>
#include <stdint.h>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include "util.h"
#include "singleton.h"
#include "mutex.h"
#include "thread.h"


//使用流式方式将日志级别level的日志写入到logger
#define gwn_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        gwn::LogEventWrap(gwn::LogEvent::ptr(new gwn::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, gwn::GetThreadId(),\
                gwn::GetFiberId(), time(0), gwn::Thread::GetName()))).getSS()


#define gwn_LOG_DEBUG(logger) gwn_LOG_LEVEL(logger, gwn::LogLevel::DEBUG)

#define gwn_LOG_INFO(logger) gwn_LOG_LEVEL(logger, gwn::LogLevel::INFO)

#define gwn_LOG_WARN(logger) gwn_LOG_LEVEL(logger, gwn::LogLevel::WARN)

#define gwn_LOG_ERROR(logger) gwn_LOG_LEVEL(logger, gwn::LogLevel::ERROR)

#define gwn_LOG_FATAL(logger) gwn_LOG_LEVEL(logger, gwn::LogLevel::FATAL)

//使用格式化的方式将日志级别level写入到logger
#define gwn_LOG_FMT_LEVEL(logger,level,...) \
    if(logger->getLevel() <= level) \
        gwn::LogEventWrap(gwn::LogEvent::ptr(new gwn::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, gwn::GetThreadId(),\
                gwn::GetFiberId(), time(0), gwn::Thread::GetName()))).getEvent()->format(fmt, __VA_ARGS__)

#define gwn_LOG_FMT_DEBUG(logger,fmt,...) gwn_LOG_FMT_LEVEL(logger, gwn::LogLevel::DEBUG, fmt, __VA_ARGS__)

#define gwn_LOG_FMT_INFO(logger, fmt, ...)  gwn_LOG_FMT_LEVEL(logger, gwn::LogLevel::INFO, fmt, __VA_ARGS__)

#define gwn_LOG_FMT_WARN(logger, fmt, ...)  gwn_LOG_FMT_LEVEL(logger, gwn::LogLevel::WARN, fmt, __VA_ARGS__)

#define gwn_LOG_FMT_ERROR(logger, fmt, ...) gwn_LOG_FMT_LEVEL(logger, gwn::LogLevel::ERROR, fmt, __VA_ARGS__)

#define gwn_LOG_FMT_FATAL(logger, fmt, ...) gwn_LOG_FMT_LEVEL(logger, gwn::LogLevel::FATAL, fmt, __VA_ARGS__)

#define gwn_LOG_ROOT() gwn::LoggerMgr::GetInstance()->getRoot()

#define gwn_LOG_NAME(name) gwn::LoggerMgr::GetInstance()->getLogger(name)

namespace gwn
{
    class Logger;
    

    class LogLevel
    {
    public:
        //日志级别
        enum Level {
        /// 未知级别
        UNKNOW = 0,
        /// DEBUG 级别
        DEBUG = 1,
        /// INFO 级别
        INFO = 2,
        /// WARN 级别
        WARN = 3,
        /// ERROR 级别
        ERROR = 4,
        /// FATAL 级别
        FATAL = 5
    };


        //将日志级别转换为文本输出
        static const char *ToString(LogLevel::Level level);
        //将文本转换为日志级别
        static LogLevel::Level FromString(const std::string &str);
    };

    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(std::shared_ptr<Logger> Logger, LogLevel::Level m_level
                , const char *file, int32_t line, uint32_t elapse
                , uint32_t threadid, uint32_t fiberid, uint64_t time
                , const std::string &threadname);

        const char *getFile() const { return m_file; }

        int32_t getLine() const { return m_line; }

        uint32_t getElapse() const { return m_elapse; }

        uint32_t getThreadId() const { return m_threadId; }

        uint32_t getFiberId() const { return m_fiberId; }

        uint64_t getTime() const { return m_time; }

        const std::string &getThreadName() const { return m_threadName; }

        std::string getContent() const { return m_ss.str(); }

        std::shared_ptr<Logger> getLogger() const { return m_logger; }

        LogLevel::Level getLevel() const { return m_level; }

        std::stringstream &getSS() { return m_ss; }

        //格式化写入日志内容
        void format(const char* fmt, ...);

        //格式化写入日志内容
        void format(const char* fmt, va_list al);
    private:
        const char *m_file = nullptr;     //文件名
        int32_t m_line = 0;               //行号
        int32_t m_elapse = 0;             //程序开始到现在的毫秒数
        uint32_t m_threadId = 0;          //线程ID
        uint32_t m_fiberId = 0;           //协程ID
        uint64_t m_time = 0;              //时间戳
        std::string m_threadName;         //线程名称
        std::stringstream m_ss;           //日志内容流
        std::shared_ptr<Logger> m_logger; //日志器
        LogLevel::Level m_level;          //日志级别
    };

    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        /*
         * @brief 构造函数
         * @param[in] pattern 格式模板
         * @details 
         *  %m 消息
         *  %p 日志级别
         *  %r 累计毫秒数
         *  %c 日志名称
         *  %t 线程id
         *  %n 换行
         *  %d 时间
         *  %f 文件名
         *  %l 行号
         *  %T 制表符
         *  %F 协程id
         *  %N 线程名称
         *
         *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
        */
        LogFormatter(const std::string &pattern);

        //返回格式化后日志文本
        std::string format(std::shared_ptr<Logger> Logger,LogLevel::Level level, LogEvent::ptr event);
        std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    public:
        //日志内容项格式化
        class FormatItem{
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            //析构函数
            virtual ~FormatItem() {}
            //格式化日志到流
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        //初始化，接卸日志模板
        void init();
        //是否有错误
        bool isError() const {return m_error;}
        const std::string getPattern() const { return m_pattern;}
    private:
        std::string m_pattern; //日志格式模式
        std::vector<FormatItem::ptr> m_items;//日志格式解析后格式
        bool m_error = false;//是否有错误
    };

    class LogEventWrap{
    public:
        LogEventWrap(LogEvent::ptr e);

        ~LogEventWrap();
        
        LogEvent::ptr getEvent() const {return m_event;}

        std::stringstream& getSS();//获取日志内容流
    private:
        LogEvent::ptr m_event;//日志事件
    };

    class LogAppender
    {
    friend class Logger;
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        typedef Spinlock MutexType;

        virtual ~LogAppender() {}
        //写入日志
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        virtual std::string toYamlString() = 0;

        void setFormatter(LogFormatter::ptr val);

        LogFormatter::ptr getFormatter();

        LogLevel::Level getLevel() const { return m_level;}

        void setLevel(LogLevel::Level val) { m_level = val;}
    protected:
        // 日志级别
        LogLevel::Level m_level = LogLevel::DEBUG;
        //是否拥有自己的日志格式器
        bool m_hasFormatter = false;
        //日志格式器
        LogFormatter::ptr m_formatter;
        MutexType m_mutex;
    };

    

    class Logger : public std::enable_shared_from_this<Logger>
    {
    friend class LoggerManager;
    public:
        typedef std::shared_ptr<Logger> ptr;
        typedef Spinlock MutexType;


        Logger(const std::string& name="root");

        //写日志
        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);

        void info(LogEvent::ptr event);

        void warn(LogEvent::ptr event);

        void error(LogEvent::ptr event);

        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);

        void delAppender(LogAppender::ptr appender);

        void clearAppenders();

        LogLevel::Level getLevel() const { return m_level;}

        void setLevel(LogLevel::Level val) { m_level = val;}

        const std::string& getName() const { return m_name;}

        const Logger::ptr getRoot() const { return m_root; }
        //void setRoot(Logger::ptr root) const { m_root = root; }


        /*
            设置日志格式器
        */
        void setFormatter(LogFormatter::ptr val);
        /*
            设置日志格式模板        
        */
        void setFormatter(const std::string& val);

        LogFormatter::ptr getFormatter();

        // /**
        //  * @brief 将日志器的配置转成YAML String
        //  */
        std::string toYamlString();
    private:
        std::string m_name;                      //日志名称
        LogLevel::Level m_level;                 //日志级别
        std::list<LogAppender::ptr> m_appenders; //日志目标集合
        LogFormatter::ptr m_formatter;           //日志格式器
        Logger::ptr m_root;                      //主日志器
        MutexType m_mutex;
    };

    class StdoutLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        std::string toYamlString() override;
    };

    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string& filename);
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        std::string toYamlString() override;


        //重新打开日志文件
        bool reopen();
    private:
        //文件路径
        std::string m_filename;
        //文件流
        std::ofstream m_filestream;
        //上次重新打开时间
        uint64_t m_lastTime = 0;
    };

    class LoggerManager
    {
    public:
        typedef Spinlock MutexType;   

        LoggerManager();

        Logger::ptr getLogger(const std::string& name);

        void init();

        Logger::ptr getRoot() const {return m_root;}

        std::string toYamlString();

    private:
        std::map<std::string,Logger::ptr> m_loggers;

        Logger::ptr m_root;

        MutexType m_mutex;
    };

    typedef gwn::Singleton<LoggerManager> LoggerMgr;

}

#endif