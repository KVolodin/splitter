#include "splitter_impl.h"

SplitterImpl::SplitterImpl( _In_ int _nMaxBuffers, _In_ int _nMaxClients )
	: uniqId_			( 0 )
	, nMaxBuffers_		( _nMaxBuffers )
	, nMaxClients_		( _nMaxClients )
	, currentStatus_	( Status::StatusProcessed )
{

}

SplitterImpl::~SplitterImpl()
{

}

bool SplitterImpl::SplitterInfoGet( _Out_ int* _pnMaxBuffers, _Out_ int* _pnMaxClients )
{
	if (!_pnMaxBuffers || !_pnMaxClients )
		return false;

	*_pnMaxBuffers = nMaxBuffers_;
	*_pnMaxClients = nMaxClients_;

	return true;
}

int SplitterImpl::SplitterPut( _In_ const std::shared_ptr<std::vector<uint8_t>>& _pVecPut, _In_ int _nTimeOutMsec )
{
	std::shared_lock<std::shared_mutex> lock( mutex_ );
	int _error(0);
	for (auto &_client : clients_)
	{
		if ( currentStatus_ == Status::StatusStoped )
			return isClosed;
		else if(currentStatus_ == Status::StatusFlush )
			return isFlushed;

		_error += _client.second.put( _pVecPut, nMaxBuffers_, _nTimeOutMsec );			
	}
	if ( _error > 0 )
		return MoreThanOneMissData;

	return NoError;
}

int SplitterImpl::SplitterGet( _In_ int _nClientID, _Out_ std::shared_ptr<std::vector<uint8_t>>& _pVecGet, _In_ int _nTimeOutMsec )
{
	if ( currentStatus_ == Status::StatusStoped )
		return isClosed;
	else if ( currentStatus_ == Status::StatusFlush )
		return isFlushed;

	std::shared_lock<std::shared_mutex> lock( mutex_ );
	auto _client = clients_.find( _nClientID );
	if ( _client == clients_.end() )
		return NotClientFound;

	auto _buffer = _client->second.get( _nTimeOutMsec );
	if ( !_buffer )
		return NoData;
	else
		_pVecGet = _buffer;

	return NoError;
}

int SplitterImpl::SplitterFlush()
{
	if( currentStatus_ == Status::StatusFlush )
		return isFlushed;

	currentStatus_ = Status::StatusFlush;
	for ( auto& it : clients_ )
	{
		it.second.interrupt();
		it.second.clear();
	}

	currentStatus_ = Status::StatusProcessed;

	return NoError;
}

bool SplitterImpl::SplitterClientAdd( _Out_ int* _pnClientID )
{
	std::lock_guard<std::shared_mutex> lock( mutex_ );
	if ( !_pnClientID || static_cast<int>( clients_.size() ) >= nMaxClients_ )
		return false;

	*_pnClientID = uniqId_;
	orderClients_.push_back( uniqId_ );
	clients_.emplace( uniqId_++ , Buffer() );
	
	return true;
}

bool SplitterImpl::SplitterClientRemove( _In_ int _nClientID )
{
	std::lock_guard<std::shared_mutex> lock( mutex_ );

	auto _client = clients_.find( _nClientID );
	if ( _client == clients_.end() )
		return false;

	auto _orderClient = std::find( orderClients_.begin(), orderClients_.end(), _nClientID );
	if ( _orderClient == orderClients_.end() )
		return false;
	_client->second.interrupt();
	clients_.erase( _client );
	orderClients_.erase( _orderClient );

	return true;
}

bool SplitterImpl::SplitterClientGetCount( _Out_ int* _pnCount )
{
	std::shared_lock<std::shared_mutex> lock( mutex_ );
	if ( !_pnCount )
		return false;

	*_pnCount = clients_.size();
	return true;
}

bool SplitterImpl::SplitterClientGetByIndex( _In_ int _nIndex, _Out_ int* _pnClientID, _Out_ int* _pnLatency )
{
	std::shared_lock<std::shared_mutex> lock( mutex_ );
	if ( !_pnClientID || !_pnLatency ||
		static_cast<int> (orderClients_.size()) <= _nIndex )
		return false;	
	auto _client = clients_.find( orderClients_[_nIndex] );
	if ( _client == clients_.end() )
		return false;
	*_pnClientID = _client->first;
	*_pnLatency = _client->second.getCount();

	return true;
}

void SplitterImpl::SplitterClose()
{
	currentStatus_ = Status::StatusStoped;
	for ( auto& it : clients_ )
	{
		it.second.interrupt();
		it.second.clear();
	}

	std::unique_lock<std::shared_mutex> lock( mutex_ );
	clients_.clear();
	orderClients_.clear();
}
