#include<iostream>
#include "gwn/log.h"
#include "gwn/util.h"

int main(int argc, char** argv){
    gwn::Logger::ptr logger(new gwn::Logger);
    logger->addAppender(gwn::LogAppender::ptr(new gwn::StdoutLogAppender));

    // gwn::LogEvent::ptr event(new gwn::LogEvent(logger,gwn::LogLevel::DEBUG
    //                         ,__FILE__,__LINE__,1,0,0,time(0),"测试"));
    // logger->log(gwn::LogLevel::DEBUG,event);
    //gwn_LOG_DEBUG(logger)<<"DEBUG";

    gwn::FileLogAppender::ptr file_appender(new gwn::FileLogAppender("./log.txt"));
    gwn::LogFormatter::ptr fmt(new gwn::LogFormatter("%d%T%p%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(gwn::LogLevel::ERROR);
    logger->addAppender(file_appender);


    gwn_LOG_DEBUG(logger) << "DEBUG";
    gwn_LOG_INFO(logger) << "INFO";
    gwn_LOG_ERROR(logger) << "ERROR";

    auto loggerTest = gwn::LoggerMgr::GetInstance()->getLogger("测试");
    gwn_LOG_ERROR(loggerTest) << "ERROR";

    return 0;
}