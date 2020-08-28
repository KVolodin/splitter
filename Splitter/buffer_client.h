#pragma once

//////////////////////////////////////////////////////////////////////////
#include <list>
#include "timer_interrupt.h"

//////////////////////////////////////////////////////////////////////////

class Buffer
{
	std::unique_ptr<TimerInterrupt>						sleepTimerPut_;
	std::unique_ptr<TimerInterrupt>						sleepTimerGet_;
	std::unique_ptr<std::mutex>							mutex_;
	std::list < std::shared_ptr<std::vector<uint8_t>> > pData_;	

	void put_( std::shared_ptr<std::vector<uint8_t>> pData )
	{
		std::lock_guard<std::mutex> lock( *mutex_ );
		pData_.push_back( pData );
	};
	std::shared_ptr<std::vector<uint8_t>> get_() noexcept
	{
		std::lock_guard<std::mutex> lock( *mutex_ );
		auto ret = pData_.front();
		pData_.pop_front();
		return ret;
	};

public:
	Buffer()
	{
		mutex_			= std::make_unique<std::mutex>();
		sleepTimerPut_	= std::make_unique<TimerInterrupt>();
		sleepTimerGet_	= std::make_unique<TimerInterrupt>();
	};

	const int getCount() const noexcept
	{
		std::lock_guard<std::mutex> lock( *mutex_ );
		return pData_.size();
	};

	const int put( std::shared_ptr<std::vector<uint8_t>> pData, const int nMaxBuffer, const int nTimeOutMsec )
	{
		int _error = 0;
		if ( getCount() < nMaxBuffer )
			put_( pData );
		else
		{
			sleepTimerPut_->sleep( std::chrono::milliseconds( nTimeOutMsec ) );
			if ( getCount() >= nMaxBuffer )
			{
				++_error;
				std::lock_guard<std::mutex> lock( *mutex_ );
				pData_.pop_front();
			}

			put_( pData );
		}
		return _error;
	};

	std::shared_ptr<std::vector<uint8_t>> get( const int nTimeOutMsec )  noexcept
	{
		if ( getCount() > 0 )
			return get_();
		else
		{
			sleepTimerGet_->sleep( std::chrono::milliseconds( nTimeOutMsec ) );
			if ( getCount() == 0 )
				return nullptr;
			else
				return get_();
		}
	};
	void clear() noexcept
	{
		std::lock_guard<std::mutex> lock( *mutex_ );
		pData_.clear();
	}
	void interrupt()
	{
		sleepTimerGet_->interrupt();
		sleepTimerPut_->interrupt();
	}
};