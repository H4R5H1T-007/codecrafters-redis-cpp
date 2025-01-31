#pragma once
#include <queue>
#include <functional>
#include <chrono>

using TimeStamp = std::chrono::system_clock::time_point;

struct timeEvent{
    TimeStamp when;
    std::function<void()> callbackFunc;
};

struct CompareTimeEvent {
    bool operator()(const timeEvent& a, const timeEvent& b) {
        return a.when > b.when; // Earlier timestamp has higher priority
    }
};

extern class EventLoop{
    private:

    std::queue<std::function<void()>> fileQueue;
    std::priority_queue<timeEvent, std::vector<timeEvent>, CompareTimeEvent> timeQueue;

    public:

    template<typename F, typename ...Args>
    void fileSubmit(F&& func, Args&&... args){
        fileQueue.push(
            [
                func = std::forward<F>(func),
                ...args = std::forward<Args>(args)
            ] () {
                // func(args...);
                std::invoke(func, args...);
            }
        );
    }

    template<typename F, typename ...Args>
    void timeSubmit(F&& func, long long int afterMilliSeconds, Args&&... args){
        timeQueue.push({
            std::chrono::system_clock::now() + std::chrono::milliseconds(afterMilliSeconds),
            [
                func = std::forward<F>(func),
                ...args = std::forward<Args>(args)
            ] () {
                std::invoke(func, args...);
            }
        });
    }

    void run();
} el;