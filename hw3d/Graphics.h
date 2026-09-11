#pragma once
#include "ChiliWin.h"
#include <d3d11.h>

class Graphics
{
public:
	// HWND is a Direct3D device object
	Graphics( HWND hWnd );
	Graphics( const Graphics& ) = delete;
	Graphics& operator=( const Graphics& ) = delete;
	~Graphics();
	void EndFrame();
	void ClearBuffer(float red, float green, float blue) noexcept
	{
		// Clear the back buffer to a specific color
		// 4th parameter is alpha channel, which we don't need
		const float color[] = { red,green,blue,1.0f };
		pContext->ClearRenderTargetView(pTarget, color);
	}
private:
	ID3D11Device* pDevice = nullptr;
	IDXGISwapChain* pSwap = nullptr;
	ID3D11DeviceContext* pContext = nullptr;
	ID3D11RenderTargetView* pTarget = nullptr;
};