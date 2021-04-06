#pragma once
#include <d3dcompiler.h>
#include <d3d11_1.h>
#include <wrl/client.h>
#include <fstream>
#include <vector>
#include "DDSTextureLoader.h"
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;
//using Microsoft::WRL::ComPtr;

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

class Renderable
{
public:
	//Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer = nullptr;
	
	//Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	inputLayout = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer = nullptr;

	UINT vSize = 0;
	int vCount = 0;
	int iCount = 0;

	// Default topology to draw triangles
	D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//Shader resources for textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sResourceView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState = nullptr;

	// creates vertex and index buffers using mesh data
	void CreateBuffers(ID3D11Device* device, const OBJ_VERT* vertices, const unsigned int* indices, 
		SIZE_T indexSize, int vertexCount, int indexCount)
	{
		vSize = sizeof(OBJ_VERT);
		vCount = vertexCount;
		iCount = indexCount;
		
		// Create Vertex Buffer
		D3D11_SUBRESOURCE_DATA vData = {vertices, 0, 0};
		CD3D11_BUFFER_DESC vDesc(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
		vDesc.Usage = D3D11_USAGE_DEFAULT;
		vDesc.CPUAccessFlags = 0;
		vDesc.ByteWidth = vCount * vSize;
		device->CreateBuffer(&vDesc, &vData, vertexBuffer.ReleaseAndGetAddressOf());
		
		// create Index Buffer
		D3D11_SUBRESOURCE_DATA iData = {indices, 0, 0 };
		CD3D11_BUFFER_DESC iDesc(indexCount, D3D11_BIND_INDEX_BUFFER);
		iDesc.Usage = D3D11_USAGE_DEFAULT;
		iDesc.CPUAccessFlags = 0;
		iDesc.ByteWidth = sizeof(int) * iCount;
		device->CreateBuffer(&iDesc, &iData, indexBuffer.ReleaseAndGetAddressOf());
	}

	void CreateTexture_Sampler(ID3D11Device* device, std::string filename)
	{
		// load texture here -------------------------
		std::wstring wstr = std::wstring(filename.begin(), filename.end());
		const wchar_t* wcstr = wstr.c_str();
		CreateDDSTextureFromFile(device, wcstr, nullptr,
			sResourceView.ReleaseAndGetAddressOf());

		//Creating sampler
		D3D11_SAMPLER_DESC sd = {};
		sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;
		device->CreateSamplerState(&sd, samplerState.ReleaseAndGetAddressOf());
	}

	void CreateShadersandInputLayout(ID3D11Device* device, const BYTE* vertexShader, SIZE_T vByteLength, 
		const BYTE* pixelShader, SIZE_T pByteLength,
		D3D11_INPUT_ELEMENT_DESC format[], UINT nElements)
	{
		device->CreateVertexShader(vertexShader, vByteLength, nullptr, vShader.ReleaseAndGetAddressOf());
		device->CreatePixelShader(pixelShader, pByteLength, nullptr, pShader.ReleaseAndGetAddressOf());
		device->CreateInputLayout(format, nElements, vertexShader, 
			vByteLength, inputLayout.ReleaseAndGetAddressOf());
		
	}

	void CreateConstantBuffer(ID3D11Device* device, UINT size)
	{
		CD3D11_BUFFER_DESC cbd = {};
		cbd.Usage = D3D11_USAGE_DEFAULT;
		cbd.ByteWidth = size;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = 0;
		device->CreateBuffer(&cbd, nullptr, constantBuffer.ReleaseAndGetAddressOf());
	}

	void Bind(ID3D11DeviceContext* dContext)
	{
		if (constantBuffer) {
			dContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
			dContext->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
		}

		if (inputLayout)
			dContext->IASetInputLayout(inputLayout.Get());
		if (vShader)
			dContext->VSSetShader(vShader.Get(), nullptr, 0);
		if(pShader)
			dContext->PSSetShader(pShader.Get(), nullptr, 0);
		if(sResourceView)
			dContext->PSSetShaderResources(0, 1, sResourceView.GetAddressOf());
		if(samplerState)
			dContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());
		/*UINT strides = vSize;*/
		if (vertexBuffer)
		{
			const UINT strides[] = { sizeof(OBJ_VERT) };
			const UINT offsets[] = { 0 };
			ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
			dContext->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
		}
		if(indexBuffer)
			dContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		dContext->IASetPrimitiveTopology(primitiveTopology);
	}

	//Draws object (must bind first)
	void Draw(ID3D11DeviceContext* dContext)
	{
		if (indexBuffer)
			dContext->DrawIndexed(iCount, 0, 0);
		else if (vertexBuffer)
			dContext->Draw(vCount, 0);
	}

	Renderable();
	~Renderable();

private:

};

Renderable::Renderable()
{
}

Renderable::~Renderable()
{
}