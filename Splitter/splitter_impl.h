#pragma once

//////////////////////////////////////////////////////////////////////////
#include <unordered_map>
#include <vector>
#include <shared_mutex>
#include <atomic>

//////////////////////////////////////////////////////////////////////////
#include "splitter_interface.h"
#include "buffer_client.h"

enum class Status
{
	StatusStoped,
	StatusFlush,
	StatusProcessed
};

enum ErrorCode
{
	NoError,
	MoreThanOneMissData,
	NotClientFound,
	NoData,
	isClosed,
	isFlushed,
};

class SplitterImpl final : public ISplitter
{
	std::vector<int>				orderClients_;
	std::unordered_map<int,Buffer>	clients_;

	std::shared_mutex				mutex_;

	std::atomic<Status>				currentStatus_;

	int								uniqId_;
	int								nMaxBuffers_;
	int								nMaxClients_;

public:
	SplitterImpl	( _In_ int _nMaxBuffers, _In_ int _nMaxClients );
	~SplitterImpl	();

	bool	SplitterInfoGet			( _Out_ int* _pnMaxBuffers, _Out_ int* _pnMaxClients ) override;
	int		SplitterPut				( _In_ const std::shared_ptr<std::vector<uint8_t>>& _pVecPut, _In_ int _nTimeOutMsec ) override;
	int		SplitterFlush			() override;
	bool	SplitterClientAdd		( _Out_ int* _pnClientID ) override;
	bool	SplitterClientRemove	( _In_ int _nClientID ) override;
	bool	SplitterClientGetCount	( _Out_ int* _pnCount ) override;
	bool	SplitterClientGetByIndex( _In_ int _nIndex, _Out_ int* _pnClientID, _Out_ int* _pnLatency ) override;
	int		SplitterGet				( _In_ int _nClientID, _Out_ std::shared_ptr<std::vector<uint8_t>>& _pVecGet, _In_ int _nTimeOutMsec ) override;
	void	SplitterClose			() override;
};

