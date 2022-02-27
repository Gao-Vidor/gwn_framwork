#ifndef __gwn_UTIL_H__
#define __gwn_UTIL_H__


#include <cxxabi.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>

namespace gwn{
    //返回当前线程ID
    pid_t GetThreadId();

    //返回当前协程ID
    uint32_t GetFiberId();

    /**
     * @brief 获取当前的调用栈
     * @param[out] bt 保存调用栈
     * @param[in] size 最多返回层数
     * @param[in] skip 跳过栈顶的层数
     */
    void Backtrace(std::vector<std::string>& bt,int size=64,int skip=1);

    std::string BacktraceToString(int size=64,int skip=2,const std::string& prefix = "");
}

#endif