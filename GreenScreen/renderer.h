// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#include "test_pyramid.h"
#pragma comment(lib, "d3dcompiler.lib")

//basic file loader (3DCC REFERENCE)
std::vector<uint8_t> loadBlob(const char* filename)
{
	std::vector<uint8_t> blob;
	std::fstream file{ filename, std::ios_base::in | std::ios_base::binary };

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

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	//Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	// world view proj
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;
	struct SHADER_VARS
	{
		GW::MATH::GMATRIXF w = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF v = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF p = GW::MATH::GIdentityMatrixF;

	}Vars;
	// math library handle
	GW::MATH::GMatrix m;


public:

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
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);
		
		CreateBuffers(creator);
		
		// Create Vertex Shader
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;
		std::vector<uint8_t> blob = loadBlob("VertexShader.h");
		creator->CreateVertexShader(blob.data(), blob.size(), nullptr,
			vertexShader.ReleaseAndGetAddressOf());

		// Create Input Layout
		D3D11_INPUT_ELEMENT_DESC format[] = {
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"UVW", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"NRM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			}
		};
		creator->CreateInputLayout(format, ARRAYSIZE(format), blob.data(),
			blob.size(), vertexFormat.ReleaseAndGetAddressOf());

		blob = loadBlob("PixelShader.h");
		creator->CreatePixelShader(blob.data(), blob.size(), nullptr,
			pixelShader.ReleaseAndGetAddressOf());


		/*if (SUCCEEDED(D3DCompile(vertexShaderSource, strlen(vertexShaderSource),
			nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0, 
			vsBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			creator->CreateVertexShader(vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
		}
		else
			std::cout << (char*)errors->GetBufferPointer() << std::endl;*/

		// Create Pixel Shader
		/*Microsoft::WRL::ComPtr<ID3DBlob> psBlob; errors.Reset();
		if (SUCCEEDED(D3DCompile(pixelShaderSource, strlen(pixelShaderSource),
			nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0, 
			psBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
		}
		else
			std::cout << (char*)errors->GetBufferPointer() << std::endl;*/

		
		/*creator->CreateInputLayout(format, ARRAYSIZE(format), 
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), 
			vertexFormat.GetAddressOf());*/

		
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
		creator->CreateBuffer(&cDesc, &cData, constantBuffer.GetAddressOf());

		// free temporary handle
		creator->Release();
	}
	void Render()
	{
		// grab the context & render target
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
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
		con->VSSetShader(vertexShader.Get(), nullptr, 0);
		con->PSSetShader(pixelShader.Get(), nullptr, 0);
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
