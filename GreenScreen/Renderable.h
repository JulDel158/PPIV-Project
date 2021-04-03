#pragma once
#include <d3dcompiler.h>
#include <d3d11_1.h>
#include <wrl/client.h>
#include <fstream>
#include <vector>
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

	// Default topology to draw triangles
	D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//Shader resources for textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sResourceView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState = nullptr;

	// creates vertex and index buffers using mesh data
	void CreateBuffers(ID3D11Device* device, vector<int>& indices, float* vertices, int vertexSize, int vertexCount)
	{
		// Create Vertex Buffer
		D3D11_BUFFER_DESC vbd = {};
		D3D11_SUBRESOURCE_DATA vertexData = {};
		vbd.Usage = D3D11_USAGE_DEFAULT;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.ByteWidth = vertexCount * vertexSize;
		vertexData.pSysMem = vertices;
		device->CreateBuffer(&vbd, &vertexData, vertexBuffer.ReleaseAndGetAddressOf());

		// create Index Buffer
		D3D11_BUFFER_DESC ibd = {};
		D3D11_SUBRESOURCE_DATA indexData = {};
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.ByteWidth =  sizeof(int) * (int)indices.size();
		indexData.pSysMem = indices.data();
		device->CreateBuffer(&ibd, &indexData, indexBuffer.GetAddressOf());
	}

	void CreateTexture_Sampler(ID3D11Device* device, std::string filename)
	{
		// load texture here -------------------------


		//Creating sampler
		D3D11_SAMPLER_DESC sd = {};
		sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;
		device->CreateSamplerState(&sd, samplerState.GetAddressOf());
	}

	void CreateShadersandInputLayout(ID3D11Device* device, BYTE vertexShader[], BYTE pixelShader[], 
		D3D11_INPUT_ELEMENT_DESC format[], UINT numElements)
	{
		device->CreateVertexShader(vertexShader, sizeof(vertexShader), nullptr, vShader.ReleaseAndGetAddressOf());
		device->CreateInputLayout(format, numElements, vertexShader,
			sizeof(vertexShader), inputLayout.ReleaseAndGetAddressOf());
		device->CreatePixelShader(pixelShader, sizeof(pixelShader), nullptr, pShader.ReleaseAndGetAddressOf());
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