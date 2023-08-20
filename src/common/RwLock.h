//
// Created by vini84200 on 8/16/23.
//

#ifndef FILESYNCERCLIENT_RWLOCK_H
#define FILESYNCERCLIENT_RWLOCK_H

#include "../server/ServerState.h"
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <utility>

template<typename T>
class ReadLock;

template<typename T>
class WriteLock;

template<typename T>
class SharedData {
public:
    SharedData(const T &initialValue) : data(initialValue) {}

    std::condition_variable_any &getCond() { return cond; }

    void notify() { cond.notify_all(); }

    void wait(std::unique_lock<std::shared_mutex> &lock,
              std::function<bool()> predicate) {
        cond.wait(lock, std::move(predicate));
    }

    void wait(std::shared_lock<std::shared_mutex> &lock,
              std::function<bool()> predicate) {
        cond.wait(lock, std::move(predicate));
    }

protected:
    T &getData() { return data; }


protected:
    T data;
    std::shared_mutex mutex;
    std::condition_variable_any cond;
    friend ReadLock<T>;
    friend WriteLock<T>;
};

template<typename T>
class ReadLock {
public:
    ReadLock(SharedData<T> &sharedData)
        : data(sharedData.getData()), lock(sharedData.mutex),
          sharedData(sharedData) {}

    const T &get() const { return data; }

    void wait(std::function<bool()> predicate) { sharedData.wait(this->lock, predicate); }

private:
    const T &data;
    std::shared_lock<std::shared_mutex> lock;
    SharedData<T> &sharedData;
};

template<typename T>
class WriteLock {
public:
    WriteLock(SharedData<T> &sharedData)
        : data(sharedData.getData()), lock(sharedData.mutex),
          sharedData(sharedData) {}

    T &get() {
        sharedData.notify();
        return data;
    }

    void set(const T &newValue) {
        data = newValue;
        sharedData.notify();
    }

    void wait(std::function<bool()> predicate) { sharedData.wait(*this, predicate); }
    void notify() { sharedData.notify(); }

    T **getPtr() { return &data; }

    void setPtr(T *newValue) { data = *newValue; }

private:
    T &data;
    std::unique_lock<std::shared_mutex> lock;
    SharedData<T> &sharedData;
};

#endif//FILESYNCERCLIENT_RWLOCK_H
