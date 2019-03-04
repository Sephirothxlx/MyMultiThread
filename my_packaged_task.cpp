#include <future>
#include <thread>
#include <iostream>
#include <type_traits>

template<class F> struct my_packaged_task;

template<typename R, typename ...Args>
struct my_packaged_task<R(Args...)> {

	my_packaged_task(R (*f)(Args...)):fptr(f){}

	void operator()(Args... args) {
		prom.set_value((*fptr)(args...));
	}

	std::future<R> get_future(){
		std::future<R> future=prom.get_future();
		return future;
	}

private:
	R (*fptr)(Args... args);

	std::promise<R> prom;
};

int fun1(int a,int b,int c){
	return a+b+c;
}

int main(){
	my_packaged_task<int(int,int,int)> task(fun1);
	std::future<int> ret = task.get_future();
    std::thread th(std::move(task), 10, 20,30);
    int value = ret.get();
    th.join();
    std::cout << value << "\n";
    return 0;
}