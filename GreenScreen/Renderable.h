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

	HRESULT CreateBuffers(ID3D11Device* device, vector<int>& indices, float* vertices, int vertexSize, int vertexCount)
	{
		HRESULT hr = S_OK;
		// Create Vertex Buffer
		D3D11_BUFFER_DESC vbd = {};
		D3D11_SUBRESOURCE_DATA vertexData = {};
		vbd.Usage = D3D11_USAGE_DEFAULT;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.ByteWidth = vertexCount * vertexSize;
		vertexData.pSysMem = vertices;
		hr = device->CreateBuffer(&vbd, &vertexData, vertexBuffer.ReleaseAndGetAddressOf());

		// create Index Buffer
		D3D11_BUFFER_DESC ibd = {};
		D3D11_SUBRESOURCE_DATA indexData = {};
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.ByteWidth =  sizeof(int) * (int)indices.size();
		indexData.pSysMem = indices.data();
		hr = device->CreateBuffer(&ibd, &indexData, indexBuffer.GetAddressOf());

		return hr;
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