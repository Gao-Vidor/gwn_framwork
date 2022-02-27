#include "scheduler.h"
#include "fiber.h"
#include "macro.h"
#include "config.h"
#include <atomic>

namespace gwn{

static Logger::ptr g_logger = gwn_LOG_NAME("system");

static thread_local Fiber* t_fiber = nullptr;//非主协程指针
static thread_local Fiber::ptr t_threadFiber = nullptr;//主协程指针

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};


static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
    Config::LookUp<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

class MallocStackAllocator{
public:
    static void* Alloc(size_t size){
        return malloc(size);
    }

    static void Dealloc(void* vp){
        free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId(){
    if(t_fiber){
        return t_fiber->getId();
    }
    return 0;
}

//设置当前执行协程
void Fiber::SetThis(Fiber* f){
    t_fiber = f;
}

//返回当前协程
Fiber::ptr Fiber::GetThis(){
    if(t_fiber){
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);
    gwn_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

Fiber::Fiber(){
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)){
        gwn_ASSERT2(false, "getcontext");
    }

    ++s_fiber_count;

    gwn_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
}


Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller)
    :m_id(++s_fiber_id)
    ,m_cb(cb) {
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx)) {
        gwn_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    
    if(!use_caller) {
        //属于调度协程
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        //属于主协程
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }

    gwn_LOG_DEBUG(g_logger) << " id=" << m_id;
}

Fiber::~Fiber(){
    --s_fiber_count;
    if(m_stack){
        gwn_ASSERT( m_state == TERM
                    || m_state == EXCEPT
                    || m_state == INIT);
        StackAllocator::Dealloc(m_stack);
    }else{
        //主协程没有回调函数，并且执行状态为EXEC
        gwn_ASSERT(!m_cb);
        gwn_ASSERT(m_state == EXEC);
        
        if(t_fiber==this){
            SetThis(nullptr);
        }
    }
    gwn_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id
                              << " total=" << s_fiber_count;
}
//重置协程函数，并重置状态
//INIT，TERM, EXCEPT
void Fiber::reset(std::function<void()> cb){
    gwn_ASSERT(m_stack);
    gwn_ASSERT(m_state == TERM
            || m_state == EXCEPT
            || m_state == INIT);
    m_cb = cb;
    if(getcontext(&m_ctx)){
        gwn_ASSERT2(false, "getcontext");
    }
    
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx,&Fiber::MainFunc,0);
    m_state = INIT;
}

//切换到其他协程
void Fiber::swapIn(){
    SetThis(this);
    gwn_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)){
        gwn_ASSERT2(false, "swapcontext");
    }
    //gwn_LOG_DEBUG(g_logger) << "for debug";
}

//切换到主协程运行
void Fiber::swapOut(){
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
        gwn_ASSERT2(false, "swapcontext");
    }
}


//切换到其他协程
void Fiber::call(){
    SetThis(this);
    gwn_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)){
        gwn_ASSERT2(false, "swapcontext");
    }
    gwn_LOG_DEBUG(g_logger) << "for debug";
}

//切换到主协程运行
void Fiber::back(){
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        gwn_ASSERT2(false, "swapcontext");
    }
}


void Fiber::YieldToReady(){
    Fiber::ptr cur = GetThis();
    gwn_ASSERT(cur->m_state == EXEC);
    cur->m_state = READY;

    cur->swapOut();
    // auto raw_ptr = cur.get();
    // cur.reset();
    // raw_ptr->swapOut();
}

void Fiber::YieldToHold(){
    Fiber::ptr cur = GetThis();
    gwn_ASSERT(cur->m_state == EXEC);
    cur->m_state = HOLD;

    cur->swapOut();
    // auto raw_ptr = cur.get();
    // cur.reset();
    // raw_ptr->swapOut();
}

//返回总协程数
uint64_t Fiber::TotalFibers(){
    return s_fiber_count;
}

void Fiber::MainFunc(){
    Fiber::ptr cur = GetThis();
    try
    {
        cur->m_cb();
        cur->m_cb = nullptr;//防止传递的参数中有智能指针
        cur->m_state = TERM;
    }
    catch(const std::exception& ex)
    {
        cur->m_state = EXCEPT;
        gwn_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << gwn::BacktraceToString();
    }catch (...) {
        cur->m_state = EXCEPT;
        gwn_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << gwn::BacktraceToString();
    }
    
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    gwn_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));

}


void Fiber::CallerMainFunc(){
    Fiber::ptr cur = GetThis();
    try
    {
        cur->m_cb();
        cur->m_cb = nullptr;//防止传递的参数中有智能指针
        cur->m_state = TERM;
    }
    catch(const std::exception& ex)
    {
        cur->m_state = EXCEPT;
        gwn_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << gwn::BacktraceToString();
    }catch (...) {
        cur->m_state = EXCEPT;
        gwn_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << gwn::BacktraceToString();
    }
    
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();

    gwn_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));

}
}
