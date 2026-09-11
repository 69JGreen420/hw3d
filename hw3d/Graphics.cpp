#include "Graphics.h"

// Set the linker settings for us
#pragma comment(lib,"d3d11.lib")

Graphics::Graphics( HWND hWnd )
{
	// Declare buffer information
	// Values are set to their default
	// We don't need scaling as we aren't in full screen
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// create device and front/back buffers, and swap chain and rendering context
	D3D11CreateDeviceAndSwapChain(
		nullptr, // Default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr, // Default software type
		0,
		nullptr, // Default feature level
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr, // Unneeded
		&pContext
	);


}
// Delete deallocated pointers
Graphics::~Graphics()
{
	// Check if pointers are valid and release them if true

	if( pContext != nullptr )
	{
		pContext->Release();
	}
	if( pSwap != nullptr )
	{
		pSwap->Release();
	}
	if( pDevice != nullptr )
	{
		pDevice->Release();
	}
}

void Graphics::EndFrame()
{
	// Use the swap chain to present the frame
	pSwap->Present( 1u,0u );
}
