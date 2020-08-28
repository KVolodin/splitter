#include "pch.h"
#include "CppUnitTest.h"
#include <algorithm>
#include "../Splitter/splitter_impl.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestSplitter
{
	TEST_CLASS( UnitTestSplitter )
	{
	public:
		int _nMaxBuffers = 2;
		int _nMaxClients = 2;
		std::shared_ptr<ISplitter> splitter_ = std::make_shared<SplitterImpl>( _nMaxBuffers, _nMaxClients );

		TEST_METHOD( TestGetInfo )
		{
			auto maxBuffers = 0;
			auto maxClients = 2;
			splitter_->SplitterInfoGet( &maxBuffers, &maxClients );

			Assert::AreEqual( _nMaxBuffers, maxBuffers );
			Assert::AreEqual( _nMaxClients, maxClients );
		}

		TEST_METHOD( TestClient )
		{
			int _uniqId( -1 );
			int _count( 0 );
			constexpr auto _maxCount( 2000 );

			for ( int i = 0; i < _maxCount; ++i )
				splitter_->SplitterClientAdd( &_uniqId );
			splitter_->SplitterClientGetCount( &_count );
			Assert::AreEqual( std::min( _nMaxClients, _maxCount ), _count );

			for ( int i = 0; i < _maxCount; ++i )
			{
				int id( 0 );
				int countDel( 0 );
				if ( splitter_->SplitterClientGetByIndex( i, &id, &countDel ) )
				{
					splitter_->SplitterClientRemove( id );
					--i;
				}
			}
			splitter_->SplitterClientGetCount( &_count );
			Assert::AreEqual( 0, _count );
		};

		TEST_METHOD( TestBuffer )
		{
			Buffer buff;
			constexpr int nTimeOutMsec = 50;
			std::shared_ptr<std::vector<uint8_t>> pData = std::make_shared<std::vector<uint8_t>>();
			pData->assign( { 1,2,3,4,5,6 } );
			buff.put( pData, _nMaxBuffers, nTimeOutMsec );

			auto _data = buff.get( nTimeOutMsec );
			bool _equalSize = _data->size() == pData->size();
			Assert::IsTrue( _equalSize );

			if ( _equalSize )
			{
				for ( size_t i = 0; i < pData->size(); ++i )
					Assert::AreEqual( _data->at( i ), pData->at( i ) );
			}

			Assert::IsTrue( buff.get( nTimeOutMsec ) == nullptr );
		}
	};
}
