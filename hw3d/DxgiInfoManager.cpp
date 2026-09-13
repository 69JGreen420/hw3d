#include "DxgiInfoManager.h"
#include "Window.h"
#include "Graphics.h"
#include <dxgidebug.h>
#include <memory>

#pragma comment(lib, "dxguid.lib")

#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Graphics::HrException( __LINE__,__FILE__,hr )

DxgiInfoManager::DxgiInfoManager()
{
	// define function signature of DXGIGetDebugInterface
	typedef HRESULT (WINAPI* DXGIGetDebugInterface)(REFIID,void **);

	// load the dll that contains the function DXGIGetDebugInterface
	const auto hModDxgiDebug = LoadLibraryEx( "dxgidebug.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32 );
	if( hModDxgiDebug == nullptr )
	{
		throw CHWND_LAST_EXCEPT();
	}

	// get address of DXGIGetDebugInterface in dll
	const auto DxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(
		reinterpret_cast<void*>(GetProcAddress( hModDxgiDebug,"DXGIGetDebugInterface" ))
	);
	if( DxgiGetDebugInterface == nullptr )
	{
		throw CHWND_LAST_EXCEPT();
	}

	HRESULT hr;

	// Call the DLL to get the debug interface (IDXGIInfoQueue) that we can then use to retrieve messages.
	GFX_THROW_NOINFO( DxgiGetDebugInterface( __uuidof(IDXGIInfoQueue),reinterpret_cast<void**>(&pDxgiInfoQueue) ) );
}

DxgiInfoManager::~DxgiInfoManager()
{
	if( pDxgiInfoQueue != nullptr )
	{
		pDxgiInfoQueue->Release();
	}
}

void DxgiInfoManager::Set() noexcept
{
	// set the index (next) so that the next all to GetMessages()
	// will only get errors generated after this call
	next = pDxgiInfoQueue->GetNumStoredMessages( DXGI_DEBUG_ALL );
}

std::vector<std::string> DxgiInfoManager::GetMessages() const
{
	std::vector<std::string> messages;
	const auto end = pDxgiInfoQueue->GetNumStoredMessages( DXGI_DEBUG_ALL );

	// Loop through messages in the queue, get the size of each message, 
	// allocate memory for it, and then get the message itself.
	for( auto i = next; i < end; i++ )
	{
		HRESULT hr;
		SIZE_T messageLength;
		// get the size of message i in bytes
		GFX_THROW_NOINFO( pDxgiInfoQueue->GetMessage( DXGI_DEBUG_ALL,i,nullptr,&messageLength ) );
		// allocate memory for message
		auto bytes = std::make_unique<byte[]>( messageLength );
		auto pMessage = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
		// get the message and push its description into the vector
		GFX_THROW_NOINFO( pDxgiInfoQueue->GetMessage( DXGI_DEBUG_ALL,i,pMessage,&messageLength ) );
		messages.emplace_back( pMessage->pDescription );
	}
	return messages;
}