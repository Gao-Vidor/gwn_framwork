#include "gwn/gwn.h"
#include "gwn/mutex.h"

gwn::Logger::ptr g_logger = gwn_LOG_ROOT();
  
int count = 0;
gwn::Mutex s_mutex;

void fun1(){
    gwn_LOG_INFO(g_logger)<<"name: "<< gwn::Thread::GetName()<<" "
                            <<"this.name: "<<gwn::Thread::GetThis()->getName()<<" "
                            <<"this.id: "<<gwn::Thread::GetThis()->getId()<<" ";

    for(int i = 0; i < 10; ++i) {
        //gwn::RWMutex::ReadLock lock(s_mutex);
        gwn::Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void fun2(){

}

int main(int argc, char **argv){
    gwn_LOG_INFO(g_logger)<<"thread test begin";
    std::vector<gwn::Thread::ptr> thrs;
    for(int i=0;i<5;++i){
        gwn::Thread::ptr thr(new gwn::Thread(&fun1,"name_"+std::to_string(i)));
        thrs.push_back(thr);
    }

    for(int i=0;i<5;++i){
        thrs[i]->join();
    }

    gwn_LOG_INFO(g_logger)<<"thread test end";
    return 0;
}