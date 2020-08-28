// Splitter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "splitter_impl.h"
#include "timer_interrupt.h"

std::shared_ptr<ISplitter> SplitterCreate( _In_ int _nMaxBuffers, _In_ int _nMaxClients )
{
	return std::make_shared<SplitterImpl>( _nMaxBuffers , _nMaxClients );
}
int main()
{
	using namespace std::literals::chrono_literals;

	constexpr auto countBuffer = 10;
	constexpr auto countClient = 10;

    auto _splitter = SplitterCreate( 2, countClient  );
	std::thread					_clientsPut;
	std::vector<std::thread>	_clientsGet;

    for (long i = 0; i < countClient; ++i)
    {
        int _id = 0;
        _splitter->SplitterClientAdd( &_id );      
    }
	bool finishWrite = false;

	std::mutex _mutex;
	_clientsPut = std::thread( [&_splitter,&countBuffer,&finishWrite,&_mutex] ()
	{
		for ( uint8_t i = 0; i < countBuffer; ++i )
		{
			auto vec = std::make_shared<std::vector<uint8_t>>();
			vec->push_back( i + 1 );
			if ( _splitter->SplitterPut( vec, 50 ) != NoError )
			{
				std::lock_guard<std::mutex> lock( _mutex );
				std::cout << "Error write " << (int) i- 1 << "\n";
			}
			std::this_thread::sleep_for( 100ms );
		}
		finishWrite = true;
	} );

	for ( int i = 0; i < countClient; ++i )
	{
		_clientsGet.push_back( std::thread( [&_splitter, &countBuffer, i, &_mutex, &finishWrite] ()
		{
			std::shared_ptr<std::vector<uint8_t>> vec;
			int id( 0 );
			int countDel( 0 );
			if ( !_splitter->SplitterClientGetByIndex( i, &id, &countDel ) )
				return;

			while ( !finishWrite )
			{
				if ( _splitter->SplitterGet( id, vec, 50 ) != NoError )
					continue;
				{
					std::lock_guard<std::mutex> lock( _mutex );
					if ( id == 0 )
						std::cout << "Client - " << i << " buffer = " << (int) vec->front() << "\n";
				}

				if ( (int) vec->front() == 3 && id == 0 )
					std::this_thread::sleep_for( 500ms );
			}
		} ) );
	}
	
    _clientsPut.join();

	for ( auto& it : _clientsGet )
		it.join();
}