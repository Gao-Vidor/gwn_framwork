#ifndef __gwn_THREAD_H
#define __gwn_THREAD_H

#include<functional>
#include <string>
#include <memory>
#include "noncopyable.h"
#include "pthread.h"
#include "mutex.h"

namespace gwn{
class Thread : Noncopyable{
public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(std::function<void()> cb, const std::string& name);

    ~Thread();

    pid_t getId() const {return m_id;}

    const std::string& getName() const {return m_name;}

    void join();

    static Thread* GetThis();

    static const std::string& GetName();

    static void SetName(const std::string& name);
private:
    static void* run(void* arg);
private:
    //线程id，全局唯一
    pid_t m_id = -1;
    //进程内唯一的，不同进程内可能相同
    pthread_t m_thread = 0;
    //线程执行函数
    std::function<void()> m_cb;
    //线程名称
    std::string m_name;

    gwn::Semaphore m_semaphore;
};
}


#endif