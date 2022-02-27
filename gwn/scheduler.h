/*
 * @Author: your name
 * @Date: 2022-01-15 15:05:20
 * @LastEditTime: 2022-01-18 21:15:23
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /gwn/gwn/scheduler.cc
 */
#ifndef __gwn_SCHEDULER_H__
#define __gwn_SCHEDULER_H__

#include <memory>
#include <vector>
#include <list>
#include <iostream>
#include <atomic>  
#include "fiber.h"
#include "thread.h"

namespace gwn{

/**
 * @brief 协程调度器
 * @details 封装的是N-M的协程调度器
 *          内部有一个线程池,支持协程在线程池里面切换
 */
class Scheduler{
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    /**
     * @brief 构造函数
     * @param[in] threads 线程数量
     * @param[in] use_caller 是否使用当前调用线程，将当前线程放入调度器
     * @param[in] name 协程调度器名称
     */
    Scheduler(size_t threads=2,bool use_caller=true,const std::string& name = "");

    ~Scheduler();

    const std::string& getName() const { return m_name;}

    /**
     * @brief 返回当前协程调度器
     */
    static Scheduler* GetThis();

    /**
     * @brief 返回当前协程调度器的调度协程
     */
    static Fiber* GetMainFiber();

    void start();

    void stop();

     /**
     * @brief 调度协程(将回调函数和协程放入任务队列)
     * @param[in] fc 协程或函数
     * @param[in] thread 协程执行的线程id,-1标识任意线程
     */
    template<class FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread);
        }

        if(need_tickle) {
            tickle();
        }
    }

    /**
     * @brief 批量调度协程
     * @param[in] begin 协程数组的开始
     * @param[in] end 协程数组的结束
     */
    template<class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end) {
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;//??
                ++begin;
            }
        }
        if(need_tickle) {
            tickle();
        }
    }
private:
    /**
     * @brief 协程调度启动(无锁)
     */
    template<class FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if(ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }
private:
    struct FiberAndThread
    {
        //协程
       Fiber::ptr fiber;
       //协程执行的函数
       std::function<void()> cb;
       //线程id,不贴切
       int thread;

       FiberAndThread(Fiber::ptr f, int thr)
            :fiber(f),thread(thr){
        }

        //防止智能指针释放失败
        /* @post *f == nullptr*/ 
        FiberAndThread(Fiber::ptr* f, int thr)
            :thread(thr) {
            fiber.swap(*f);
        }

        FiberAndThread(std::function<void()> f, int thr)
            :cb(f), thread(thr) {
        }

        //@post *f = nullptr
        FiberAndThread(std::function<void()>* f, int thr)
            :thread(thr) {
            cb.swap(*f);
        }

        FiberAndThread()
            :thread(-1) {
        }

        void reset(){
            fiber=nullptr;
            cb = nullptr;
            thread = -1;
        }
    };
protected:
    //通知协程调度器有任务
    virtual void tickle();  
    //协程调度函数
    void run();
    //返回是否可以停止
    virtual bool stopping();
    //协程误人误调度执行的时候执行idle协程
    virtual void idle();
    //设置当前协程调度器
    void setThis();
protected:
    std::vector<int> m_threadIds;
    //线程数量
    size_t m_threadCount = 0;
    //工作线程数量
    std::atomic<size_t> m_activeThreadCount = {0};
    /// 空闲线程数量
    std::atomic<size_t> m_idleThreadCount = {0};
    // 是否正在停止
    bool m_stopping = true;
    // 是否自动停止
    bool m_autoStop = false;
    // 主线程id(use_caller)
    pid_t m_rootThread = 0;
private:
    //Mutex
    MutexType m_mutex;
    //线程池
    std::vector<gwn::Thread::ptr> m_threads;
    //待执行的协程队列
    std::list<FiberAndThread> m_fibers;
    //use_caller为true时有效, 调度协程
    Fiber::ptr m_rootFiber;
    // 协程调度器名称
    std::string m_name;
};


}

#endif
