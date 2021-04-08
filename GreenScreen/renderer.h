#include <vector>
#include <DirectXMath.h>
#include "VertexShader.h"
#include "PixelShader.h"
#include "axe1.h"
#include "test_pyramid.h"
#include "Renderable.h"
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11_1.h>
#include <directxmath.h>
#include <directxcolors.h>

using namespace DirectX;

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
	
	D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL; // a thing we need for lights 

	// world view proj
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;

	struct ConstantBuffer
	{
		XMMATRIX mWorld;
		XMMATRIX mView;
		XMMATRIX mProjection;
		XMFLOAT4 vLightDir[2];
		XMFLOAT4 vLightColor[2];
		XMFLOAT4 vOutputColor;
	};

	
	
	
	std::vector<uint8_t> load_binary_blob(const char* path)
	{
		std::vector<uint8_t> blob;

		std::fstream file{ path, std::ios_base::in | std::ios_base::binary };

		if (file.is_open())
		{
			file.seekg(0, std::ios_base::end);
			blob.resize(file.tellg());
			file.seekg(0, std::ios_base::beg);

			file.read((char*)blob.data(), blob.size());

			file.close();
		}

		return std::move(blob);
	}

	//struct ShaderBundle
	//{
	//	Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout = nullptr;
	//	Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader = nullptr;
	//	Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader = nullptr;
	//	Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBufferVS = nullptr;
	//	Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBufferPS = nullptr;

	//	HRESULT CreateVertexShaderAndInputLayoutFromFile(
	//		ID3D11Device* device, const char* filename,
	//		D3D11_INPUT_ELEMENT_DESC layout[], UINT numElements)
	//	{
	//		HRESULT hr = S_OK;

	//		auto vs_blob = load_binary_blob(filename);

	//		// Create the vertex shader
	//		hr = device->CreateVertexShader(vs_blob.data(), vs_blob.size(), nullptr,
	//			VertexShader.ReleaseAndGetAddressOf());
	//		if (FAILED(hr))
	//		{
	//			return hr;
	//		}

	//		// Create the input layout
	//		hr = device->CreateInputLayout(layout, numElements, vs_blob.data(),
	//			vs_blob.size(),
	//			InputLayout.ReleaseAndGetAddressOf());
	//		return hr;
	//	}

	//	HRESULT CreatePixelShaderFromFile(ID3D11Device* device, const char* filename)
	//	{
	//		HRESULT hr = S_OK;

	//		auto vs_blob = load_binary_blob(filename);

	//		// Create the pixel shader
	//		hr = device->CreatePixelShader(vs_blob.data(), vs_blob.size(), nullptr,
	//			PixelShader.ReleaseAndGetAddressOf());
	//		return hr;
	//	}

	//	HRESULT CreateConstantBufferVS(ID3D11Device* device, UINT size)
	//	{
	//		return CreateConstantBuffer(device, size, ConstantBufferVS);
	//	}

	//	HRESULT CreateConstantBufferPS(ID3D11Device* device, UINT size)
	//	{
	//		return CreateConstantBuffer(device, size, ConstantBufferPS);
	//	}

	//	HRESULT CreateConstantBuffer(ID3D11Device* device, UINT size,
	//		Microsoft::WRL::ComPtr<ID3D11Buffer>& ConstantBuffer)
	//	{
	//		HRESULT hr = S_OK;
	//		// Create the constant buffer
	//		D3D11_BUFFER_DESC bd = {};
	//		bd.Usage = D3D11_USAGE_DEFAULT;
	//		bd.ByteWidth = size;
	//		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//		bd.CPUAccessFlags = 0;
	//		hr = device->CreateBuffer(&bd, nullptr, &ConstantBuffer);
	//		return hr;
	//	}

	//	void Bind(ID3D11DeviceContext* context)
	//	{
	//		if (ConstantBufferVS)
	//			context->VSSetConstantBuffers(0, 1, ConstantBufferVS.GetAddressOf());
	//		if (ConstantBufferPS)
	//			context->PSSetConstantBuffers(0, 1, ConstantBufferPS.GetAddressOf());
	//		if (InputLayout)
	//			context->IASetInputLayout(InputLayout.Get());
	//		if (VertexShader)
	//			context->VSSetShader(VertexShader.Get(), nullptr, 0);
	//		if (PixelShader)
	//			context->PSSetShader(PixelShader.Get(), nullptr, 0);
	//	}
	//};
	// math library handle
	GW::MATH::GMatrix m;

public:




		//Settting some of the light variables
		XMFLOAT4 vLightDirs[2] =
		{
			XMFLOAT4(-0.577f, 0.577f, -0.577f, 1.0f),
			XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
		};
		XMFLOAT4 vLightColors[2] =
		{
			XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)
		};


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
	//ConstantBuffer cb1;
	//XMMATRIX       g_World;
	//XMMATRIX       g_View;
	//XMMATRIX       g_Projection;
	//ShaderBundle meshShaderBundle;


	//g_World.r[3] = { 0.0f, 1.0f, 3.0f, 2.0f };
	//cb1.mWorld = XMMatrixTranspose(g_World);
	//cb1.mView = XMMatrixTranspose(g_View);
	//cb1.mProjection = XMMatrixTranspose(g_Projection);
	//cb1.vLightDir[0] = vLightDirs[0];
	//cb1.vLightDir[1] = vLightDirs[1];
	//cb1.vLightColor[0] = vLightColors[0];
	//cb1.vLightColor[1] = vLightColors[1];
	//cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
	//con->UpdateSubresource(meshShaderBundle.ConstantBufferVS.Get(), 0, nullptr, &cb1, 0, 0);  //==========================  hey this is the light stuff you wer working on 


		//CreateBuffers(pDevice);
		
		// Create Vertex Shader
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif







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

		// Update our time
		static float t = 0.0f;
		if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			t += (float)XM_PI * 0.0125f;
		}
		else
		{
			static ULONGLONG timeStart = 0;
			ULONGLONG timeCur = GetTickCount64();
			if (timeStart == 0)
				timeStart = timeCur;
			t = (timeCur - timeStart) / 1000.0f;
		}


		//setting lighting stuff and hoping to god it doesn't fuck things up
		XMStoreFloat4(&vLightDirs[0], { -0.577f, 0.577f, -0.577f, 1.0f });
		XMStoreFloat4(&vLightDirs[1], { 0.577f, 0.2577f, -0.577f, 1.0f });

		XMStoreFloat4(&vLightColors[0], { 0.75f, 0.75f, 0.75f, 1.0f });
		//	XMStoreFloat4(&vLightColors[1], { 0.2f, 0.2f, 0.5f, 1.0f });
		XMStoreFloat4(&vLightColors[1], { 0.5f, 0.5f, 1.0f, 1.0f });


		// Rotate the second light around the origin
		XMMATRIX mRotate = XMMatrixRotationY(-1.0f * t);
		XMVECTOR vLightDir = XMLoadFloat4(&vLightDirs[1]);
		// rotares the second light
		//vLightDir = XMVector3Transform(vLightDir, mRotate);
		XMStoreFloat4(&vLightDirs[1], vLightDir);


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
