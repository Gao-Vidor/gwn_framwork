/*
 * @Author: your name
 * @Date: 2022-01-15 15:05:20
 * @LastEditTime: 2022-01-18 21:21:02
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE#
 * @FilePath: /gwn/gwn/scheduler.cc
 */
#include "scheduler.h"
#include "log.h"
#include "macro.h"

namespace gwn{
static gwn::Logger::ptr g_logger = gwn_LOG_NAME("system");

//当前调度器的指针
static thread_local Scheduler* t_scheduler = nullptr;
//当前调度协程的指针，代表当前调度器属于拿一个协程
static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool cur_managed, const std::string& name)
    :m_name(name){
    gwn_ASSERT(threads > 0);

    if(cur_managed){
        //将当前线程变为协程
        gwn::Fiber::GetThis(); 
        --threads;
        
        //设置当前调度器指针
        gwn_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        //初始化一个用于调度的协程
        /*
            如果当前线程由由调度器管理，则要初始化一个调度协程
        */ 
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this),0,true));//true代表调度器属于当前协程
        t_scheduler_fiber = m_rootFiber.get();

        //设置线程名称
        gwn::Thread::SetName(m_name);

        
        m_rootThread = gwn::GetThreadId();
        m_threadIds.push_back(m_rootThread);

    }else{
        m_rootThread = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler(){
    gwn_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis(){
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber(){
    return t_scheduler_fiber;
}

//启动协程调度器——》初始化线程池
void Scheduler::start(){
    MutexType::Lock lock(m_mutex);
    if(!m_stopping){
        return;
    }

    m_stopping = false;
    gwn_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for(size_t i=0;i<m_threadCount;++i){
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                            , m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();
}

void Scheduler::tickle(){
    gwn_LOG_INFO(g_logger) << "tickle";
}

void Scheduler::idle(){
    gwn_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        gwn::Fiber::YieldToHold();
    }
}

bool Scheduler::stopping(){
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::setThis(){
    t_scheduler = this;
}

void Scheduler::stop(){
    m_autoStop = true;
    //如果只有一个协程返回
    if(m_rootFiber
        &&m_threadCount==0&&(m_rootFiber->getState() == Fiber::TERM
                || m_rootFiber->getState() == Fiber::INIT)){
        gwn_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if(stopping()){
            return;
        }
    }

    if(m_rootThread != -1) {
        gwn_ASSERT(GetThis() == this);
    } else {
        gwn_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    }

    if(m_rootFiber) {
        tickle();
    }

    if(m_rootFiber) {
        //while(!stopping()) {
        //    if(m_rootFiber->getState() == Fiber::TERM
        //            || m_rootFiber->getState() == Fiber::EXCEPT) {
        //        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        //        gwn_LOG_INFO(g_logger) << " root fiber is term, reset";
        //        t_fiber = m_rootFiber.get();
        //    }
        //    m_rootFiber->call();
        //}
        if(!stopping()) {
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }
}

void Scheduler::run(){
    gwn_LOG_DEBUG(g_logger) << m_name << " run";

    setThis();
    //设置主调度器协程指针便于上下文切换
    if(gwn::GetThreadId() != m_rootThread){
        t_scheduler_fiber = Fiber::GetThis().get();
    }

    //空闲协程
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle,this)));
    Fiber::ptr cb_fiber;

    

    FiberAndThread ft;
    while(true){
        ft.reset();
        bool tickle_me = false; //是否通知其他线程执行任务
        bool is_active = false;
        is_active = (bool)is_active;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()) {
                if(it->thread != -1 && it->thread != gwn::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }

                gwn_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                ft = *it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            tickle_me |= it != m_fibers.end();
        }

        if(tickle_me){
            tickle();
        }

        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEPT)){
            ft.fiber->swapIn();
            --m_activeThreadCount;
            
            if(ft.fiber->getState() == Fiber::READY){
                schedule(ft.fiber);
            }
            else if(ft.fiber->getState() != Fiber::TERM 
                    && ft.fiber->getState() != Fiber::EXCEPT){
                ft.fiber->setState(Fiber::HOLD);
            }
            ft.reset();
        }else if(ft.cb){
            /*
                如果只有一个回调函数，初始化一个新协程
            */
            if(cb_fiber){
                cb_fiber->reset(ft.cb);
            }else{
                cb_fiber.reset(new Fiber(ft.cb));
            }
            
            cb_fiber->swapIn();
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY){
                schedule(cb_fiber);
                cb_fiber.reset();
            }else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM){
                cb_fiber->reset(nullptr);
            }else{//if(cb_fiber->getState() != Fiber::TERM)
                cb_fiber->setState(Fiber::HOLD);
                /*
                    ?????????
                    ?????????
                    ?????????
                */
                cb_fiber.reset();//这个Fiber不是被析构了吗？
            }
            ft.reset();
        }else{
            if(idle_fiber->getState() == Fiber::TERM) {
                gwn_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->setState(Fiber::HOLD);
            }
        }
    }
}
}

