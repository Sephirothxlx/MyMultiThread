#include <thread>
#include <future>
#include <type_traits>
#include <iostream>

template<typename Function, typename... Args>
std::future<typename std::result_of<Function(Args...)>::type>
my_async( Function&& f, Args&&... args){
	typedef typename std::result_of<Function(Args...)>::type type;
	std::packaged_task <type(Args&&...)>task(f);
    std::future<type> future = task.get_future();
    std::thread t(move(task),(args)...);
    t.join();
    return future;
}

int fun1(int a,int b){
    return a+b;
}

int main(){
    auto x = my_async(fun1, 2,3);
    std::cout<<x.get()<<"\n";
    return 0;
}