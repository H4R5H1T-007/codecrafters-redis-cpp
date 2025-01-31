// For now I am considering only functions will be inserted into event loop with no return type (or void return type)
#include "eventLoop.hpp"

EventLoop el;

void EventLoop::run(){
    while(true){
        while(!fileQueue.empty()){
            auto task = fileQueue.front();
            fileQueue.pop();
            task();
        }
        while(!timeQueue.empty()){
            auto tEvent = timeQueue.top();
            if(tEvent.when > std::chrono::system_clock::now()){
                break;
            }
            timeQueue.pop();
            tEvent.callbackFunc();
        }
    }
}

// template<typename T>
// int tempFunc(T t, int seconds){
//     std::cout<<"value of t is: "<<t<<" "<<std::format("Current time: {:%Y-%m-%d %H:%M:%S}", std::chrono::system_clock::now())<<std::endl;
//     ev.timeSubmit(tempFunc<T>, seconds, t, seconds);
//     return 0;
// }

// int main(){
//     // EventLoop ev;
//     ev.fileSubmit(tempFunc<int>, 10, 1);
//     ev.timeSubmit(tempFunc<std::string>, 2, "Hello World", 2);
//     ev.run();
//     return 0;
// }