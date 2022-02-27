/*
 * @Author: your name
 * @Date: 2022-01-14 22:46:42
 * @LastEditTime: 2022-01-15 13:42:39
 * @LastEditors: your name
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /gwn/tests/fiber_test.cc
 */
#include "gwn/gwn.h"

gwn::Logger::ptr g_logger = gwn_LOG_ROOT();

void run_in_fiber() {
    gwn_LOG_INFO(g_logger) << "run_in_fiber begin";
    gwn::Fiber::YieldToHold();
    gwn_LOG_INFO(g_logger) << "run_in_fiber end";
    gwn::Fiber::YieldToHold();
}

void test_fiber() {
    gwn_LOG_INFO(g_logger) << "main begin -1";
    {
        gwn::Fiber::GetThis();
        gwn_LOG_INFO(g_logger) << "main begin";
        gwn::Fiber::ptr fiber(new gwn::Fiber(run_in_fiber));
        fiber->swapIn();
        gwn_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        gwn_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    gwn_LOG_INFO(g_logger) << "main after end2";
}

int main(int argc, char** argv) {
    gwn::Thread::SetName("main");

    std::vector<gwn::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i) {
        thrs.push_back(gwn::Thread::ptr(
                    new gwn::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    return 0;
}
