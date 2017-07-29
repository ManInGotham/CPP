#include <mutex>
#include <condition_variable>

class semaphore {
public:
	inline void notify()
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.notify_one();
	}

	inline void wait()
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock);
	}

private:
	std::mutex mtx;
	std::condition_variable cv;
};