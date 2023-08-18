//
// Created by vini84200 on 8/16/23.
//

#ifndef FILESYNCERCLIENT_RWLOCK_H
#define FILESYNCERCLIENT_RWLOCK_H

#include "../server/ServerState.h"
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <shared_mutex>

template <typename T>
class ReadLock;

template <typename T>
class WriteLock;

template <typename T>
class SharedData {
public:
    SharedData(const T& initialValue) : data(initialValue) {}

    T& getData() {
        return data;
    }

protected:
    T data;
    std::shared_mutex mutex;
    friend ReadLock<T>;
    friend WriteLock<T>;
};

template <typename T>
class ReadLock {
public:
    ReadLock(SharedData<T>& sharedData) : data(sharedData.getData()), lock(sharedData.mutex) {}

    const T& get() const {
        return data;
    }

private:
    const T& data;
    std::shared_lock<std::shared_mutex> lock;
};

template <typename T>
class WriteLock {
public:
    WriteLock(SharedData<T>& sharedData) : data(sharedData.getData()), lock(sharedData.mutex) {}

    T& get() {
        return data;
    }

    void set(const T& newValue) {
        data = newValue;
    }

    T **getPtr() {
        return &data;
    }

    void setPtr(T *newValue) { data = *newValue; }

private:
    T& data;
    std::unique_lock<std::shared_mutex> lock;
};

#endif//FILESYNCERCLIENT_RWLOCK_H
