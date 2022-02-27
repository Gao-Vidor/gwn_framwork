/*
 * @Author: your name
 * @Date: 2022-01-12 21:05:36
 * @LastEditTime: 2022-02-27 21:57:45
 * @LastEditors: your name
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /gwn/gwn/noncopyable.h
 */
#ifndef __gwn_NONCOPYABLE_H__
#define __gwn_NONCOPYABLE_H__

namespace gwn{

class Noncopyable {
public:
    Noncopyable() = default;

    ~Noncopyable() = default;

    Noncopyable(const Noncopyable&) = delete;

    Noncopyable& operator=(const Noncopyable&) = delete;
};

}

#endif