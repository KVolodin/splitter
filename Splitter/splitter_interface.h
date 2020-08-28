#pragma once
#include <sal.h>
#include <memory>
#include <vector>

class ISplitter
{
public:
	ISplitter() {};
	virtual ~ISplitter() {};

	virtual bool SplitterInfoGet			( _Out_ int* _pnMaxBuffers, _Out_ int* _pnMaxClients ) = 0;
	virtual int	SplitterPut					( _In_ const std::shared_ptr<std::vector<uint8_t>>& _pVecPut, _In_ int _nTimeOutMsec ) = 0;
	virtual int	SplitterFlush				() = 0;
	virtual bool SplitterClientAdd			( _Out_ int* _pnClientID ) = 0;
	virtual bool SplitterClientRemove		( _In_ int _nClientID ) = 0;
	virtual bool SplitterClientGetCount		( _Out_ int* _pnCount ) = 0;
	virtual bool SplitterClientGetByIndex	( _In_ int _nIndex, _Out_ int* _pnClientID, _Out_ int* _pnLatency ) = 0;
	virtual int	SplitterGet					( _In_ int _nClientID, _Out_ std::shared_ptr<std::vector<uint8_t>>& _pVecGet, _In_ int _nTimeOutMsec ) = 0;
	virtual void SplitterClose				() = 0;
private:

};