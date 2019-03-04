#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <functional>
#include <queue>
#include <stdexcept>
#include <future>
#include <iostream>
#include <chrono>
#include <stdlib.h>

class threadpool{
	bool stop;
	int size;
	std::vector<std::thread> threads;
	std::queue<std::function<void()>>tasks;
	//mu is used to lock queue of tasks
	std::mutex mu;
	std::condition_variable con;
public:
	threadpool(int s):size(s),stop(false){
		for(int i=0;i<s;i++){
			threads.emplace_back([this]{
				while(true){
					std::function<void()> func;
					//get a function to run in thread
					{
						std::unique_lock<std::mutex> lock(this->mu);
						this->con.wait(lock,[this]{return this->stop||!this->tasks.empty();});
						//stop!
						if(this->stop&&this->tasks.empty())
							return;
						func=std::move(this->tasks.front());
						this->tasks.pop();
					}
					func();
				}
			});
		}
	}

	// nocopy
    threadpool(const threadpool&)=delete;
    threadpool& operator=(const threadpool&)=delete;

	template<typename F, typename ...Args>
	std::future<typename std::result_of<F(Args...)>::type> add(F&&f, Args&&... args){
		typedef typename std::result_of<F(Args...)>::type return_type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
	    std::future<return_type> future=task->get_future();

	    //add to queue
	    {
	    	std::unique_lock<std::mutex> lock(mu);
	    	if(stop==true)
            	throw std::runtime_error("threadpool is stopped!");
	    	tasks.emplace([task]{
	    		(*task)();
	    	});
	    }
	    con.notify_one();
	    return future;
	}

	bool isStop(){
		return stop;
	}

	void stopPool(){
		stop=true;
	}

	int getSize(){
		return size;
	}

	~threadpool(){
		{
			std::unique_lock<std::mutex> lock(mu);
			stop=true;
		}
		con.notify_all();
		//execute all the threads
		for(std::thread &x:threads){
			x.join();
		}
	}
};


/**
	Test for threadpool
	run 15 tasks on threadpool, which has 8 threads
	each task will sleep for random time
**/
std::mutex mu;

void task(int i, int time){
	std::unique_lock<std::mutex> lock(mu);
	std::cout << "task " << i << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

int main()
{
    threadpool pool(8);
    std::cout<<"threadpool's size is "<<pool.getSize()<<"\n";
    if(pool.isStop()){
    	std::cout<<"threadpool is stopped."<<"\n";
    }else{
    	std::cout<<"threadpool is running."<<"\n";
    }
    std::cout<<"\n";
    std::vector<std::future<void>> results;
    for(int i=0;i<15;i++) {
        results.emplace_back(pool.add(task,i,rand()%1200));
    }

    for(auto& result: results)
        result.get();
    
    return 0;
}