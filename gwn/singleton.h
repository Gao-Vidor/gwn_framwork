#ifndef __gwn_SINGLETON_H__
#define __gwn_SINGLETON_H__

#include <memory>

namespace gwn
{
    namespace
    {
        template <class T, class X, int N>
        T &GetInstanceX()
        {
            static T v;
            return v;
        }

        template <class T, class X, int N>
        std::shared_ptr<T> GetInstancePtr()
        {
            static std::shared_ptr<T> v(new T);
            return v;
        }
    }

    //T 类型 X为了创造多个实例对应的Tag N同一个tag创造多个实例索引
    template <class T, class X = void, int N = 0>
    class Singleton
    {
    public:
        static T *GetInstance()
        {
            static T v;
            return &v;
        }
    };

    template <class T, class X = void, int N = 0>
    class SingletonPtr
    {
    public:
        static std::shared_ptr<T> GetInstance()
        {
            static std::shared_ptr<T> v(new T);
            return v;
        }
    };
}

#endif