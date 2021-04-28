// Simple basecode showing how to create a window and attatch a d3d11surface
#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_GRAPHICS // Enables all Graphics Libraries
#define GATEWARE_ENABLE_MATH	// Enables Math Library 
#define GATEWARE_ENABLE_INPUT // Enables Keyboard/Mouse/Controller Support
// Ignore some GRAPHICS libraries we aren't going to use
#define GATEWARE_DISABLE_GDIRECTX12SURFACE // we have another template for this
#define GATEWARE_DISABLE_GRASTERSURFACE // we have another template for this
#define GATEWARE_DISABLE_GOPENGLSURFACE // we have another template for this
#define GATEWARE_DISABLE_GVULKANSURFACE // we have another template for this
// With what we want & what we don't defined we can include the API
#include "../Gateware/Gateware.h"
#include "renderer.h" // example rendering code (not Gateware code!)
// open some namespaces to compact the code a bit
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;
using namespace DirectX;

IDXGISwapChain* pSwapChain = nullptr;
ID3D11DeviceContext* pDeviceContext = nullptr;
ID3D11RenderTargetView* pTargetView = nullptr;
ID3D11DepthStencilView* pDepth = nullptr;

void CleanUp();

// lets pop a window and use D3D11 to clear to a green screen
int main()
{
	GWindow win;
	GEventReceiver msgs;
	GDirectX11Surface d3d11;
	if (+win.Create(0, 0, 800, 600, GWindowStyle::WINDOWEDBORDERED))
	{
		float clr[] = { 0, 0, 0, 0 }; // start black
		msgs.Create(win, [&]() {
			//if (+msgs.Find(GWindow::Events::RESIZE, true))
			//	clr[2] += 0; // move towards a cyan as they resize
			});

		if (+d3d11.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		{
			Renderer renderer(win, d3d11);
			// main loop (runs until window is closed)
			while (+win.ProcessWindowEvents())
			{
				if (+d3d11.GetImmediateContext((void**)&pDeviceContext) &&
					+d3d11.GetRenderTargetView((void**)&pTargetView) &&
					+d3d11.GetDepthStencilView((void**)&pDepth) &&
					+d3d11.GetSwapchain((void**)&pSwapChain))
				{
					pDeviceContext->ClearRenderTargetView(pTargetView, clr);
					pDeviceContext->ClearDepthStencilView(pDepth, D3D11_CLEAR_DEPTH, 1, 0);
					/*renderer.DrawSkyBox();
					pDeviceContext->ClearDepthStencilView(pDepth, D3D11_CLEAR_DEPTH, 1, 0);*/
					renderer.Update();
					renderer.Render();
					pSwapChain->Present(1, 0);
					//release incremented COM reference counts
					CleanUp();
				}
			}
		}
	}
	return 0; // that's all folks
}

void CleanUp()
{
	if (pDepth) pDepth->Release();
	if (pSwapChain) pSwapChain->Release();
	if (pTargetView) pTargetView->Release();
	if (pDeviceContext) pDeviceContext->Release();
}