// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#include "test_pyramid.h"
#include "VertexShader.h"
#include "PixelShader.h"
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11_1.h>
#include <directxmath.h>
#include <directxcolors.h>

using namespace DirectX;

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// device and target view
	ID3D11DeviceContext* con;
	ID3D11RenderTargetView* view;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	//Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

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

	struct SHADER_VARS
	{
		GW::MATH::GMATRIXF w = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF v = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF p = GW::MATH::GIdentityMatrixF;

	}Vars;
	
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


	HRESULT CreateBuffers(ID3D11Device* device)
	{
		HRESULT hr = S_OK;
		// Create Vertex Buffer
		D3D11_SUBRESOURCE_DATA bData = { test_pyramid_data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeof(test_pyramid_data), D3D11_BIND_VERTEX_BUFFER);
		hr = device->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());

		// create Index Buffer
		D3D11_SUBRESOURCE_DATA iData = { test_pyramid_indicies, 0, 0 };
		CD3D11_BUFFER_DESC iDesc(sizeof(test_pyramid_indicies), D3D11_BIND_INDEX_BUFFER);
		hr = device->CreateBuffer(&iDesc, &iData, indexBuffer.GetAddressOf());

		return hr;
	}

	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		ID3D11Device* pDevice;
		d3d.GetDevice((void**)&pDevice);
		
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


		CreateBuffers(pDevice);
		
		// Create Vertex Shader
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif






		// Setting vertex shader
		pDevice->CreateVertexShader(VertexShader, sizeof(VertexShader), nullptr,
			vShader.ReleaseAndGetAddressOf());

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
		
		//Setting pixel Shader
		pDevice->CreatePixelShader(PixelShader, sizeof(PixelShader), nullptr,
			pShader.ReleaseAndGetAddressOf());

		//Setting input layout
		pDevice->CreateInputLayout(format, ARRAYSIZE(format), VertexShader, ARRAYSIZE(VertexShader), 
			vertexFormat.GetAddressOf());

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
		D3D11_SUBRESOURCE_DATA cData = { &Vars, 0, 0 };
		CD3D11_BUFFER_DESC cDesc(sizeof(Vars), D3D11_BIND_CONSTANT_BUFFER);
		pDevice->CreateBuffer(&cDesc, &cData, constantBuffer.GetAddressOf());

		// free temporary handle
		pDevice->Release();
	}
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
		const UINT strides[] = { sizeof(OBJ_VERT) };
		const UINT offsets[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
		con->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		con->VSSetShader(vShader.Get(), nullptr, 0);
		con->PSSetShader(pShader.Get(), nullptr, 0);
		con->IASetInputLayout(vertexFormat.Get());
		// send in the constant buffer
		ID3D11Buffer* const constants[] = { constantBuffer.Get() };
		con->VSSetConstantBuffers(0, 1, constants);
		// to update dynamically *****
		// con->UpdateSubresource()
		// now we can draw
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		con->DrawIndexed(test_pyramid_indexcount, 0, 0);
		// release temp handles
		view->Release();
		con->Release();
	}
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here 
	}
};
