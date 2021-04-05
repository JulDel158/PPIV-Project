// minimalistic code to draw a single triangle, this is not part of the API.
#include "test_pyramid.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Renderable.h"
#pragma comment(lib, "d3dcompiler.lib")

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// device and target view
	ID3D11DeviceContext* con;
	ID3D11RenderTargetView* view;
	//Renderable object used to load pyramid obj
	Renderable pyramid;

	// world view projection matices (constnat buffer)
	struct SHADER_VARS
	{
		GW::MATH::GMATRIXF w = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF v = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF p = GW::MATH::GIdentityMatrixF;

	}Vars;
	// math library handle
	GW::MATH::GMatrix m;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		ID3D11Device* pDevice;
		d3d.GetDevice((void**)&pDevice);

		//making pyramid buffers
		pyramid.CreateBuffers(pDevice, test_pyramid_data, test_pyramid_indicies, sizeof(test_pyramid_indicies),
								test_pyramid_vertexcount, test_pyramid_indexcount);

		// Create Input Layout
		D3D11_INPUT_ELEMENT_DESC format[] = {
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			}
		};

		//creating v/p shaders and input layout
		pyramid.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
			PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

		//init math stuff
		m.Create();
		m.LookAtLHF(GW::MATH::GVECTORF{ 0.5f, 1.0f, -2.0f }, //eye
					GW::MATH::GVECTORF{ 0,0.5f,0 }, //at
					GW::MATH::GVECTORF{ 0,1,0 }, //up
					Vars.v);
		float ar;
		d3d.GetAspectRatio(ar);
		m.ProjectionDirectXLHF(G_PI_F / 2.0f, ar, 0.01f, 100, Vars.p);

		// create constant buffer
		pyramid.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));

		// free temporary handle
		pDevice->Release();
	}
	//Bind objects to render and update constant buffers 
	void Render()
	{
		// grab the context & render target
		d3d.GetImmediateContext((void**)&con);
		d3d.GetRenderTargetView((void**)&view);
		// setup the pipeline
		ID3D11RenderTargetView *const views[] = { view };
		con->OMSetRenderTargets(ARRAYSIZE(views), views, nullptr);
		//Preparing to draw pyramid
		con->UpdateSubresource(pyramid.constantBuffer.Get(), 0, nullptr, &Vars, 0, 0);
		pyramid.Bind(con);
		pyramid.Draw(con);

		// release temp handles
		view->Release();
		con->Release();
	}
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here 
	}
};
