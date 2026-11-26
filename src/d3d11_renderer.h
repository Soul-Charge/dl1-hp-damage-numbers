#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

// Initializes DX11 Present / ResizeBuffers hooks via a dummy swapchain.
bool D3D11OverlayInit();
void D3D11OverlayShutdown();
