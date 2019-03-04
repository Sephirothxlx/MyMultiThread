#include <mutex>
#include <condition_variable> 
#include <memory>
#include <functional>

template<typename R>
class my_future{
	typedef typename std::unique_ptr<R>::type ptrtype;
	ptrtype result;
	std::mutex mu;
	std::condition_variable cond;
	bool status=false;
	std::function<R()> f;

	void run(){
		f();
		status=true;
	}

public:
	void set_result(R&&r){
		*result=r;
	}

	R& get(){
		run();
		std::unique_lock<std::mutex> lock(mu);
		while(!status){
			cond.wait(lock);
		}
		return *result;
	}
};

template<typename R>
class promise{
	std::shared_ptr<my_future<R>> mf;

public:
	my_future<R> get_future(){
		return mf;
	}

	void set_value(R&& r){
		mf->set_result(r);
	}
};