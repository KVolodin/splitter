#pragma once

#include <condition_variable>
#include <mutex>

class TimerInterrupt
{
	std::condition_variable cv_;
	std::mutex				mutex_;
	bool					isAlive_;

public:
	TimerInterrupt() :isAlive_( true ) {};
	template<class R, class P>
	bool sleep( std::chrono::duration<R, P> const& time )
	{
		std::unique_lock<std::mutex> lock( mutex_ );
		return !cv_.wait_for( lock, time, [&] { return !isAlive_; } );
	}
	void interrupt() noexcept
	{
		std::unique_lock<std::mutex> lock( mutex_ );
		isAlive_ = false;
		cv_.notify_all();
	}
};