/*
 * @Author: your name
 * @Date: 2022-01-11 10:31:20
 * @LastEditTime: 2022-01-16 09:20:14
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /gwn/gwn/util.cc
 */
#include "util.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <execinfo.h>

#include "log.h"
#include "fiber.h"

namespace gwn
{
    static gwn::Logger::ptr g_logger = gwn_LOG_NAME("system");

    pid_t GetThreadId()//这个函数"可以"改成Thread类的静态函数
    {
        return syscall(SYS_gettid);
    }

    uint32_t GetFiberId()
    {
        return gwn::Fiber::GetFiberId();
        return 0;
    }

    void Backtrace(std::vector<std::string>& bt, int size, int skip){
        void** array = (void**)malloc((sizeof(void*)*size));
        size_t s = ::backtrace(array,size);

        char** strings = backtrace_symbols(array,s);
        if(strings == NULL){
            gwn_LOG_ERROR(g_logger) << "backtrace_synbols error";
            return;
        }

        for(size_t i=skip;i<s;++i){
            bt.push_back(strings[i]);
        }

        free(strings);
        free(array);
    }

    std::string BacktraceToString(int size,int skip,const std::string& prefix){
        std::vector<std::string> bt;
        Backtrace(bt,size,skip);
        std::stringstream ss;
        for(size_t i=0;i<bt.size();++i){
            ss<<prefix<<bt[i]<<std::endl;
        }
        return ss.str();
    }
}