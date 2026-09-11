// Graphics owns the application's Direct3D 11 device, immediate context, and
// swap chain. It also defines the per-frame boundary used by the renderer,
// exposes the back-buffer target and shared camera matrices, and translates
// Direct3D failures into the engine's exception types.
#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include <cmath>
#include <DirectXMath.h>
#include <array>
#include "GraphicsThrowMacros.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "DepthStencil.h"
#include "RenderTarget.h"

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")


Graphics::Graphics( HWND hWnd,int width,int height )
	:
	width( width ),
	height( height )
{
	// Describe the window-sized swap chain that will provide the back buffer
	// presented to the user at the end of each frame.
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
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

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// The debug layer supplies extra validation messages in non-release builds.
	// The GFX_THROW macros below collect those messages when a call fails.
	HRESULT hr;

	// Create the GPU device, swap chain, and immediate context used for all
	// rendering commands issued through this Graphics object.
	GFX_THROW_INFO( D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		swapCreateFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	) );

	// Turn the swap chain's back-buffer texture into the engine's standard
	// RenderTarget abstraction so render passes can bind it like other targets.
	wrl::ComPtr<ID3D11Texture2D> pBackBuffer;
	GFX_THROW_INFO( pSwap->GetBuffer( 0,__uuidof(ID3D11Texture2D),&pBackBuffer ) );
	pTarget = std::shared_ptr<Bind::RenderTarget>{ new Bind::OutputOnlyRenderTarget( *this,pBackBuffer.Get() ) };
	
	// Set the default viewport to cover the whole window. Individual passes may
	// change other pipeline state, but this is the initial viewport for drawing.
	D3D11_VIEWPORT vp;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	pContext->RSSetViewports( 1u,&vp );
	
	// Connect ImGui's Direct3D backend to the device and context. The Win32
	// backend is initialized elsewhere because it needs the application window.
	ImGui_ImplDX11_Init( pDevice.Get(),pContext.Get() );
}

Graphics::~Graphics()
{
	// Release ImGui's device-side resources before the Direct3D objects owned
	// by this instance are destroyed.
	ImGui_ImplDX11_Shutdown();
}

void Graphics::EndFrame()
{
	// Finish and draw the ImGui command list after the scene has rendered, so
	// the UI appears on top of the frame.
	if( imguiEnabled )
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
	}

	HRESULT hr;
#ifndef NDEBUG
	infoManager.Set();
#endif
	// Present the completed back buffer. Sync interval 1 enables vertical
	// synchronization; device removal is reported with its specific reason.
	if( FAILED( hr = pSwap->Present( 1u,0u ) ) )
	{
		if( hr == DXGI_ERROR_DEVICE_REMOVED )
		{
			throw GFX_DEVICE_REMOVED_EXCEPT( pDevice->GetDeviceRemovedReason() );
		}
		else
		{
			throw GFX_EXCEPT( hr );
		}
	}
}

void Graphics::BeginFrame( float red,float green,float blue ) noexcept
{
	// The color parameters are retained for the public frame API; render-graph
	// passes now clear their own targets instead of clearing here.
	// Start a new ImGui frame before any code records UI for this frame.
	if( imguiEnabled )
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}
	// Unbind resources that may be written as render targets later in the frame.
	// Direct3D rejects simultaneous input/output binding, and these slots can
	// otherwise retain views from the previous frame.
	ID3D11ShaderResourceView* const pNullTex = nullptr;
	pContext->PSSetShaderResources( 0,1,&pNullTex ); // fullscreen input texture
	pContext->PSSetShaderResources( 3,1,&pNullTex ); // shadow map texture
}

void Graphics::DrawIndexed( UINT count ) noxnd
{
	// Submit an indexed draw using the input assembly and shaders currently
	// configured by the active render pass.
	GFX_THROW_INFO_ONLY( pContext->DrawIndexed( count,0u,0u ) );
}

void Graphics::SetProjection( DirectX::FXMMATRIX proj ) noexcept
{
	// Store the projection used by transform constant buffers and render passes.
	projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept
{
	return projection;
}

void Graphics::SetCamera( DirectX::FXMMATRIX cam ) noexcept
{
	// Store the current view/camera transform for consumers that need it while
	// constructing draw data.
	camera = cam;
}

DirectX::XMMATRIX Graphics::GetCamera() const noexcept
{
	return camera;
}

void Graphics::EnableImgui() noexcept
{
	// Allow BeginFrame/EndFrame to advance and render the ImGui frame.
	imguiEnabled = true;
}

void Graphics::DisableImgui() noexcept
{
	// Keep the renderer's frame boundaries active while skipping ImGui work.
	imguiEnabled = false;
}

bool Graphics::IsImguiEnabled() const noexcept
{
	return imguiEnabled;
}

UINT Graphics::GetWidth() const noexcept
{
	return width;
}

UINT Graphics::GetHeight() const noexcept
{
	return height;
}

std::shared_ptr<Bind::RenderTarget> Graphics::GetTarget()
{
	// Return the swap-chain-backed target used to display the final image.
	return pTarget;
}


// Exception helpers below preserve the failing HRESULT and Direct3D debug
// messages, then format them into useful diagnostics for the application.
Graphics::HrException::HrException( int line,const char* file,HRESULT hr,std::vector<std::string> infoMsgs ) noexcept
	:
	Exception( line,file ),
	hr( hr )
{
	// Join all debug-layer messages into one printable string.
	for( const auto& m : infoMsgs )
	{
		info += m;
		info.push_back( '\n' );
	}
	// Avoid leaving a trailing newline in the stored diagnostic.
	if( !info.empty() )
	{
		info.pop_back();
	}
}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if( !info.empty() )
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "Chili Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString( hr );
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription( hr,buf,sizeof( buf ) );
	return buf;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}


const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Chili Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}
Graphics::InfoException::InfoException( int line,const char * file,std::vector<std::string> infoMsgs ) noexcept
	:
	Exception( line,file )
{
	// Join all debug-layer messages into one printable string.
	for( const auto& m : infoMsgs )
	{
		info += m;
		info.push_back( '\n' );
	}
	// Avoid leaving a trailing newline in the stored diagnostic.
	if( !info.empty() )
	{
		info.pop_back();
	}
}


const char* Graphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept
{
	return "Chili Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}
