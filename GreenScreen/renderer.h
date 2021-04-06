#include "VertexShader.h"
#include "PixelShader.h"
#include "axe1.h"
#include "test_pyramid.h"
#include "Renderable.h"
#pragma comment(lib, "d3dcompiler.lib")

// Creation, Rendering & Cleanup
class Renderer
{
	// world, view, projection matrices (constant buffer)
	struct SHADER_VARS
	{
		GW::MATH::GMATRIXF world = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF view = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF projection = GW::MATH::GIdentityMatrixF;
	};

	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// device and target view
	ID3D11DeviceContext* con;
	ID3D11RenderTargetView* view;
	//Renderable object used to load pyramid obj
	Renderable pyramid;
	Renderable axe;

	SHADER_VARS Vars;
	

	
	
	// math library handle
	GW::MATH::GMatrix m;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		ID3D11Device* pDevice = nullptr;
		d3d.GetDevice((void**)&pDevice);

		

		//making pyramid buffers
		pyramid.CreateBuffers(pDevice, test_pyramid_data, test_pyramid_indicies, sizeof(test_pyramid_indicies),
								test_pyramid_vertexcount, test_pyramid_indexcount);

		axe.CreateBuffers(pDevice, axe1_data, axe1_indicies, sizeof(axe1_indicies), 
								axe1_vertexcount, axe1_indexcount);

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

		axe.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader), 
			PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

		//init math stuff
		m.Create();
		// Initializing identity matrix
		//m.IdentityF(Vars.world);
		
		// Initializing view matrix
		m.LookAtLHF(GW::MATH::GVECTORF{ 15.0f, 6.0f, 2.0f }, //eye
					GW::MATH::GVECTORF{ 0,0.0f,0 }, //at
					GW::MATH::GVECTORF{ 0,1,0 }, //up
					Vars.view);
		float ar;
		d3d.GetAspectRatio(ar);
		//Initializing projection matrix
		m.ProjectionDirectXLHF(G_PI_F / 2.0f, ar, 0.01f, 100, Vars.projection);

		// create constant buffer
		pyramid.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
		axe.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));

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

		SHADER_VARS pcb;
		GW::MATH::GVECTORF scale = { 10.0f, 10.f, 10.0f, 1.0f };
		m.ScalingF(pcb.world, scale, pcb.world);
		m.TransposeF(pcb.world, pcb.world);
		m.TransposeF(Vars.view, pcb.view);
		m.TransposeF(Vars.projection, pcb.projection);

		//Preparing to draw pyramid
		con->UpdateSubresource(pyramid.constantBuffer.Get(), 0, nullptr, &pcb, 0, 0);
		pyramid.Bind(con);
		pyramid.Draw(con);

		scale = { 0.3f, 0.3f, 0.3f, 1.0f };
		m.ScalingF(Vars.world, scale, pcb.world);
		m.TransposeF(pcb.world, pcb.world);

		con->UpdateSubresource(axe.constantBuffer.Get(), 0, nullptr, &pcb, 0, 0);
		axe.Bind(con);
		axe.Draw(con);

		// release temp handles
		view->Release();
		con->Release();
	}

	//Use to update things such as camera movement, world matrix, etc
	void Update()
	{
		static float t = 0.0f;
		static ULONGLONG timeStart = 0;
		ULONGLONG timeCur = GetTickCount64();
		if (timeStart == 0)
			timeStart = timeCur;
		t = (timeCur - timeStart) / 1000.0f;
		
		m.RotationYF(Vars.world, t, Vars.world);
	}

	~Renderer()
	{
		// ComPtr will auto release so nothing to do here 
	}
};
