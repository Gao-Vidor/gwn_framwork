/*
 * @Author: your name
 * @Date: 2022-01-18 13:25:12
 * @LastEditTime: 2022-02-27 21:30:10
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /gwn/tests/schedul_test.cc
 */



#include "gwn/gwn.h"

gwn::Logger::ptr g_logger = gwn_LOG_ROOT();

void test_fiber() {
    static int s_count = 5;
    gwn_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    // sleep(1);
    // if(--s_count >= 0) {
    //     gwn::Scheduler::GetThis()->schedule(&test_fiber, gwn::GetThreadId());
    // }
}

int main(int argc,char** args){
    gwn_LOG_INFO(g_logger) << "main";
    gwn::Scheduler sc(1, false, "test");
    sc.start();

    gwn_LOG_INFO(g_logger) << "schedul";
    sc.schedule(test_fiber);
    sc.stop();
    gwn_LOG_INFO(g_logger) << "over";
    return 0;
}