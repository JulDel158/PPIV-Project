#include <vector>
#include <DirectXMath.h>
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
	
	struct VertexData
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 uvw;
		DirectX::XMFLOAT3 nrm;
	};

	template <typename T>
	struct MeshData
	{
		std::vector<T> vertices;
		std::vector<int> indicies;
	};

	//Simple function to load data from precompiled obj files into Mesh struct
	MeshData<VertexData> LoadMeshFromHeader(const OBJ_VERT* vertexData, const unsigned int* indexData,
		const int vertexCount, const int indexCount)
	{
		MeshData<VertexData> result;
		//Loading vertex data
		result.vertices.resize(vertexCount);
		for (int i = 0; i < vertexCount; ++i)
		{
			result.vertices[i].pos = (DirectX::XMFLOAT3)vertexData[i].pos;
			result.vertices[i].uvw = (DirectX::XMFLOAT3)vertexData[i].uvw;
			result.vertices[i].nrm = (DirectX::XMFLOAT3)vertexData[i].nrm;
		}
		//Loading index data
		result.indicies.resize(indexCount);
		for (int i = 0; i < indexCount; ++i)
		{
			result.indicies[i] = indexData[i];
		}

		return result;
	}

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

		//loading obj data into mesh
		MeshData<VertexData> pMesh = LoadMeshFromHeader(test_pyramid_data, test_pyramid_indicies, 
			test_pyramid_vertexcount, test_pyramid_indexcount);
		MeshData<VertexData> aMesh = LoadMeshFromHeader(axe1_data, axe1_indicies, axe1_vertexcount, axe1_indexcount);
		
		//making pyramid buffers
		pyramid.CreateBuffers(pDevice, (float*)pMesh.vertices.data(), pMesh.indicies, sizeof(VertexData), pMesh.vertices.size());
		//making axe buffers
		axe.CreateBuffers(pDevice, (float*)aMesh.vertices.data(), aMesh.indicies, sizeof(VertexData), aMesh.vertices.size());

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
