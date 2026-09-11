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
private:
	ID3D11Device* pDevice = nullptr;
	IDXGISwapChain* pSwap = nullptr;
	ID3D11DeviceContext* pContext = nullptr;
	ID3D11RenderTargetView* pTarget = nullptr;
};