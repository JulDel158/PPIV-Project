#include <vector>
#include <DirectXMath.h>
#include <d3d11_1.h>
#include <directxcolors.h>
#include "VertexShader.h"
#include "PixelShader.h"
#include "InstancePixelShader.h"
#include "InstanceVertexShader.h"
#include "SkyBoxPixelShader.h"
#include "SkyBoxVertexShader.h"
#include "SeaweedPixel.h"
#include "SeaweedVertex.h"
#include "GeometryShader.h"
#include "Renderable.h"
#include "CUBE.h"
#include "uploads_files_737265_1.h"
#include "Wave_VS.h"
#include "Cannon.h"
#include "lh2.h"
#include "Sun.h"
#include "rocket.h"
#include "moon.h"
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

// Creation, Rendering & Cleanup
class Renderer
{
	// world, view, projection matrices (constant buffer)
	_declspec(align(16))
	struct SHADER_VARS
	{
		GW::MATH::GMATRIXF world = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF view = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF projection = GW::MATH::GIdentityMatrixF;
		float time = 0.0f;
		XMFLOAT3 dLightdir = { -1.0f, 0.0f, 0.0f};
		float pLightRad = 7.5f;
		XMFLOAT3 pLightpos = { 0.0f, 4.5f, 0.0f};
		XMFLOAT4 lightColor[3] = { {0.5f, 0.5f, 0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.2f, 0.3f, 0.0f, 0.3f} };
		XMFLOAT4 wave1 = { 1.0f, 1.0f, 0.25f, 30.0f };
		XMFLOAT4 wave2 = { 1.0f, 0.6f, 0.25f, 16.0f };
		XMFLOAT4 wave3 = { 1.0f, 1.3f, 0.25f, 8.0f };
		float specularPow = 10.0f;
		XMFLOAT3 camwpos;
		float specIntent = 10.0f;
		XMFLOAT3 spotPos = { -5.0f, 2.0f, 0.0f };
		float coneIratio = 1.0f;
		XMFLOAT3 coneDir = { 0.0f, -0.01f, 1.0f };
		float coneOratio = 0.9f;
		float cRatio = 0.2f;
	};

	_declspec(align(16))
	struct SCENE_INFORMATION
	{
		XMFLOAT4X4 View;
		XMFLOAT4X4 Projection;
		XMFLOAT3 gridPos;
		float padding;
		float SeaweedWidth;
		float SeaweedHeight;
	};

	_declspec(align(16))
		struct SHADER_VARS_SKYBOX {
		GW::MATH::GMATRIXF world = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF view = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF projection = GW::MATH::GIdentityMatrixF;
		XMFLOAT4 pos = { 0.0f,0.0f,0.0f,1.0f };

	};

	//the other shader for the instancing
	_declspec(align(16))
	struct SHADER_VARS_INSTANCE
	{
		GW::MATH::GMATRIXF world[10];
		//UINT ID;
		GW::MATH::GMATRIXF view = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF projection = GW::MATH::GIdentityMatrixF;
		float time = 0.0f;
		XMFLOAT3 dLightdir = { -1.0f, 0.0f, 0.0f };
		float pLightRad = 7.5f;
		XMFLOAT3 pLightpos = { 0.0f, 4.5f, 0.0f };
		XMFLOAT4 lightColor[3] = { {0.0f, 0.32f, 0.84f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.6f, 0.6f, 0.2f, 0.3f} };
		XMFLOAT4 wave1 = { 1.0f, 1.0f, 0.25f, 30.0f };
		XMFLOAT4 wave2 = { 1.0f, 0.6f, 0.25f, 16.0f };
		XMFLOAT4 wave3 = { 1.0f, 1.3f, 0.25f, 8.0f };
		float specularPow = 10.0f;
		XMFLOAT3 camwpos;
		float specIntent = 10.0f;
		XMFLOAT3 spotPos = { -5.0f, 2.0f, 0.0f };
		float coneIratio = 1.0f;
		XMFLOAT3 coneDir = { 0.0f, -0.01f, 1.0f };
		float coneOratio = 0.9f;
		float cRatio = 0.2f;
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

	bool LoadMeshFromOBJ(const char* path, MeshData<VertexData>& container)
	{
		//bool result = false;
		std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
		std::vector<XMFLOAT3> temp_vertices, temp_uvs, temp_normals;
		std::vector <XMFLOAT3> out_vertices, out_uvs, out_normals;
		MeshData<VertexData> tempContainer;

		FILE* file = fopen(path, "r");
		if (file == NULL) {
			printf("Impossible to open the file !\n");
			return false;
		}
		while (true)
		{
			char lineHeader[128];
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break; // EOF = End Of File. Quit the loop.
			// else : parse lineHeader
			if (strcmp(lineHeader, "v") == 0) 
			{
				XMFLOAT3 vertex = { 0,0,0 };
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				temp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0)
			{
				XMFLOAT3 uv = { 0,0,0 };
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				uv.z = 0.0f;
				uv.y = 1 - uv.y;
				temp_uvs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0)
			{
				XMFLOAT3 nrm = { 0,0,0 };
				fscanf(file, "%f %f %f\n", &nrm.x, &nrm.y, &nrm.z);
				temp_normals.push_back(nrm);
			}
			else if (strcmp(lineHeader, "f") == 0) 
			{
				//std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",  
					&vertexIndex[0], &uvIndex[0], &normalIndex[0], 											    
					&vertexIndex[1], &uvIndex[1], &normalIndex[1], 												
					&vertexIndex[2], &uvIndex[2], &normalIndex[2]);

				if (matches != 9) {
					printf("File can't be read by our simple parser : ( Try exporting with other options\n");
					return false;
				}

				for (int i = 0; i < 3; ++i)
				{
					vertexIndices.push_back(vertexIndex[i]);
					uvIndices.push_back(uvIndex[i]);
					normalIndices.push_back(normalIndex[i]);
				}
			}
		}

		for (int i = 0; i < vertexIndices.size(); ++i)
		{
			VertexData entry;
			//pos
			unsigned int vIndex = (vertexIndices[i] - 1);
			entry.pos = temp_vertices[vIndex];
			// uvs
			unsigned int uIndex = (uvIndices[i] - 1);
			entry.uvw = temp_uvs[uIndex];
			//nrm
			unsigned int nIndex = (normalIndices[i] - 1);
			entry.nrm = temp_normals[nIndex];

			tempContainer.vertices.push_back(entry);
			tempContainer.indicies.push_back(i);
		}

		container.vertices.clear();
		container.indicies.clear();
		for (int i = 0, x = 0; i < tempContainer.vertices.size(); ++i)
		{
			bool unique = true;
			for (int j = 0; j < container.vertices.size(); ++j)
			{
				//if the vertex is not unique, add index and break out for next vertex
				if ((tempContainer.vertices[i].pos.x) == (container.vertices[j].pos.x) &&
					(tempContainer.vertices[i].pos.y) == (container.vertices[j].pos.y) &&
					(tempContainer.vertices[i].pos.z) == (container.vertices[j].pos.z) &&
					(tempContainer.vertices[i].uvw.x) == (container.vertices[j].uvw.x) &&
					(tempContainer.vertices[i].uvw.y) == (container.vertices[j].uvw.y) &&
					(tempContainer.vertices[i].nrm.x) == (container.vertices[j].nrm.x) &&
					(tempContainer.vertices[i].nrm.y) == (container.vertices[j].nrm.y) &&
					(tempContainer.vertices[i].nrm.z) == (container.vertices[j].nrm.z))
				{
					unique = false;
					container.indicies.push_back(tempContainer.indicies[j]);
					break;
				}
			}

			if (unique)
			{
				//container.indicies.push_back(container.vertices.size());
				container.vertices.push_back(tempContainer.vertices[i]);
				container.indicies.push_back(x++);
			}
		}
		//container = tempContainer;
		return true;
	}

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

	//plane grid to be drawn with triangles instead of lines
	MeshData<VertexData> MakePlaneGrid(int width, int depth, float spacing = 1)
	{
		MeshData<VertexData> grid;
		int size = width * depth;
		//for testing purposes
		//spacing = 1.0f;
		float xLines = static_cast<float>(width) / spacing;
		float zLines = static_cast<float>(depth) / spacing;

		float x = -width / 2.0f;
		float z = -depth / 2.0f;

		for (int i = 0; i < zLines; ++i)
		{
			x = -width / 2.0f;
			for (int j = 0; j < xLines; ++j)
			{
				VertexData v1 = { {x, 0, z}, {0,0,0,}, {0,1,0,} };
				grid.vertices.push_back(v1);
				x += spacing;
			}
			z += spacing;
		}

		for (int i = 0; i < (zLines - 1); ++i)
		{
			for (int j = 0; j < (xLines - 1); ++j)
			{
				grid.indicies.push_back(j + (i * width));
				grid.indicies.push_back(j + ((i+1) * width));
				grid.indicies.push_back((j+1) + ((i + 1) * width));

				grid.indicies.push_back(j + (i * width));
				grid.indicies.push_back((j + 1) + ((i + 1) * width));
				grid.indicies.push_back((j + 1) + (i * width));
			}
		}

		return grid;
	}

	MeshData<VertexData> MakeGrid(float size = 0, float spacing = 0)
	{
		MeshData<VertexData> grid;
		if (size == 0 || spacing == 0)
		{
			size = 100.0f + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / 100.0f));
			spacing = 0.5f + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / 4.5f));
		}

		const float lines = size / spacing;
		float x = -size / 2.0f;
		float z = x;
		const float cz = x;
		const float cx = x;
		int i = 0;

		for (int p = 0; p < lines; ++p)
		{
			VertexData v1 = { {cx,0,z}, {0,0,0}, {0,0,0} };
			VertexData v2 = { {cx + size, 0, z}, {0,0,0}, {0,0,0} };
			VertexData v3 = { {x,0,cz}, {0,0,0}, {0,0,0} };
			VertexData v4 = { {x, 0, cz + size}, {0,0,0}, {0,0,0} };
			grid.vertices.push_back(v1);
			grid.indicies.push_back(i++);
			grid.vertices.push_back(v2);
			grid.indicies.push_back(i++);
			grid.vertices.push_back(v3);
			grid.indicies.push_back(i++);
			grid.vertices.push_back(v4);
			grid.indicies.push_back(i++);
			z += spacing;
			x += spacing;
		}
		return grid;
	}

	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// device and target view
	ID3D11DeviceContext* con;
	ID3D11RenderTargetView* view;
	ID3D11DepthStencilView* depth;
	//Renderable object used to load pyramid obj
	
	Renderable skyBox;

	//island objects
	Renderable cannon;
	Renderable lh;
	Renderable water;
	Renderable chest;
	Renderable island;
	Renderable seaweed;

	//space objects
	Renderable sun;
	Renderable rocket;
	Renderable planet;
	Renderable moon;


	GW::MATH::GMATRIXF rx = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMATRIXF ry = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMATRIXF rz = GW::MATH::GIdentityMatrixF;

	SHADER_VARS Vars;
	SHADER_VARS Camera;
	SHADER_VARS_INSTANCE iVars;
	SHADER_VARS_SKYBOX skyboxSV;
	SCENE_INFORMATION scene;
	bool done = true;

#define MAX_SEAWEED_COUNT 50 //we'll start with a conservative number to not break things
	//taking this rand float stuff cause it's too good to not have
#define RAND_FLOAT(min,max) (((max)-(min))*(rand()/float(RAND_MAX))+(min))

	//will likely be taken out soon v
	//actually bringing the geometry shader and the other shaders it needs - side note, I should prob either make sure I release them later or just have be com pointers
	ID3D11PixelShader* ps_Seaweed;
	ID3D11VertexShader* vs_Seaweed;
	ID3D11GeometryShader* gs_Seaweed;
	ID3D11InputLayout* il_Seaweed; //using the il to preface
	

	bool regenerate = true;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState; //other stuff for function

	//buffers for the seaweed
	Microsoft::WRL::ComPtr<ID3D11Buffer> seaweedLocations;
	Microsoft::WRL::ComPtr<ID3D11Buffer> seaweedDirections;

	float prevFrame = clock();
	float dt = 0;
	float aspectRatio;
	float _aspectRatio;
	float fov = 2.0f;
	float zNear = 0.01f;
	float zFar = 10000;
	// math library handle
	GW::MATH::GMatrix m;
	// Input Library
	GW::INPUT::GInput input;
	// resource view for default texture
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> etex;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyBoxSRV;

	float X = 20.0f;
	float Y = 10.0f;
	float Z = -1.1f;
	float rX = 0.0f; 
	float rY = 0.0f;
	float rZ = 0.0f; 

public:

	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		srand(time(NULL));
		win = _win;
		d3d = _d3d;
		ID3D11Device* pDevice = nullptr;
		d3d.GetDevice((void**)&pDevice);

		// create input lib
		input.Create(_win);
		
		//loading obj data into mesh
		MeshData<VertexData> gMesh = MakePlaneGrid(500, 500);
		MeshData<VertexData> seaweedMesh = MakePlaneGrid(100, 100,3);
		
		//canon
		MeshData<VertexData> cMesh = LoadMeshFromHeader(Cannon_data, Cannon_indicies, Cannon_vertexcount, Cannon_indexcount);

		//island with palms
		MeshData<VertexData> iMesh = LoadMeshFromHeader(uploads_files_737265_1_data, uploads_files_737265_1_indicies,
														uploads_files_737265_1_vertexcount, uploads_files_737265_1_indexcount);
		//treasure chest
		MeshData<VertexData> tMesh;
		LoadMeshFromOBJ("../PPIV-Project/GreenScreen/test02.obj", tMesh);

		//lighthouse
		MeshData<VertexData> lhMesh = LoadMeshFromHeader(lh2_data, lh2_indicies, lh2_vertexcount, lh2_indexcount);

		//sun
		MeshData<VertexData> snMesh = LoadMeshFromHeader(Sun_data, Sun_indicies, Sun_vertexcount, Sun_indexcount);
		MeshData<VertexData> pnMesh = snMesh;
		for (int i = 0; i < Sun_vertexcount; ++i)
		{
			snMesh.vertices[i].nrm = { snMesh.vertices[i].nrm.x * -1.0f, snMesh.vertices[i].nrm.y * -1.0f, snMesh.vertices[i].nrm.z * -1.0f };
		}

		//moon
		MeshData<VertexData> mnMesh = LoadMeshFromHeader(moon_data, moon_indicies, moon_vertexcount, moon_indexcount);

		//rocket
		MeshData<VertexData> roMesh = LoadMeshFromHeader(rocket_data, rocket_indicies, rocket_vertexcount, rocket_indexcount);

		//skybox
		MeshData<VertexData> skyBoxMesh;
		LoadMeshFromOBJ("../PPIV-Project/GreenScreen/CUBE.obj", skyBoxMesh);
		
		//Setting texture + sampler of objects
		water.CreateTextureandSampler(pDevice, "");
		chest.CreateTextureandSampler(pDevice, "../PPIV-Project/GreenScreen/TreasureChestTexture.dds");
		skyBox.CreateTextureandSampler(pDevice, "../PPIV-Project/GreenScreen/TestSkyBoxOcean.dds");
		seaweed.CreateTextureandSampler(pDevice, "");
		cannon.CreateTextureandSampler(pDevice, "");
		island.CreateTextureandSampler(pDevice, "");
		lh.CreateTextureandSampler(pDevice, "");
		sun.CreateTextureandSampler(pDevice, "");
		moon.CreateTextureandSampler(pDevice, "");
		planet.CreateTextureandSampler(pDevice, "../PPIV-Project/GreenScreen/2k_mars.dds");
		rocket.CreateTextureandSampler(pDevice, "../PPIV-Project/GreenScreen/SSBox.dds");

		//default texture (single pixel)
		{
			const uint32_t pixel = 0xFFFFFFFF;
			D3D11_SUBRESOURCE_DATA initData = { &pixel, sizeof(uint32_t), 0 };
			D3D11_TEXTURE2D_DESC pd = {};
			pd.SampleDesc.Count = pd.MipLevels = pd.ArraySize = pd.Width = pd.Height = 1;
			pd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			pd.Usage = D3D11_USAGE_IMMUTABLE;
			pd.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
			pDevice->CreateTexture2D(&pd, &initData, tex.GetAddressOf());

			D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
			srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvd.Texture2D.MipLevels = 1;

			pDevice->CreateShaderResourceView(tex.Get(),
				&srvd, texSRV.GetAddressOf());
		}

		//initializing skybox texture for second scene
		string skyboxfn = "../PPIV-Project/GreenScreen/SSBox.dds";
		std::wstring wstr = std::wstring(skyboxfn.begin(), skyboxfn.end());
		const wchar_t* wcstr = wstr.c_str();
		CreateDDSTextureFromFile(pDevice, wcstr, nullptr,
			skyBoxSRV.ReleaseAndGetAddressOf());

		skyboxfn = "../PPIV-Project/GreenScreen/2k_earth_daymap.dds";
		wstr = std::wstring(skyboxfn.begin(), skyboxfn.end());
		wcstr = wstr.c_str();
		CreateDDSTextureFromFile(pDevice, wcstr, nullptr,
			etex.ReleaseAndGetAddressOf());


		////making pyramid index and vertex buffers
		water.CreateBuffers(pDevice, (float*)gMesh.vertices.data(), &gMesh.indicies, sizeof(VertexData), gMesh.vertices.size());
		//test mesh buffers
		chest.CreateBuffers(pDevice, (float*)tMesh.vertices.data(), &tMesh.indicies, sizeof(VertexData), tMesh.vertices.size());
		//skybox buffers
		skyBox.CreateBuffers(pDevice, (float*)skyBoxMesh.vertices.data(), &skyBoxMesh.indicies, sizeof(VertexData), skyBoxMesh.vertices.size());
		//seaweed buffers
		seaweed.CreateBuffers(pDevice, (float*)seaweedMesh.vertices.data(), &seaweedMesh.indicies, sizeof(VertexData), seaweedMesh.vertices.size());

		island.CreateBuffers(pDevice, (float*)iMesh.vertices.data(), &iMesh.indicies, sizeof(VertexData), iMesh.vertices.size());

		cannon.CreateBuffers(pDevice, (float*)cMesh.vertices.data(), &cMesh.indicies, sizeof(VertexData), cMesh.vertices.size());

		lh.CreateBuffers(pDevice, (float*)lhMesh.vertices.data(), &lhMesh.indicies, sizeof(VertexData), lhMesh.vertices.size());

		sun.CreateBuffers(pDevice, (float*)snMesh.vertices.data(), &snMesh.indicies, sizeof(VertexData), snMesh.vertices.size());

		moon.CreateBuffers(pDevice, (float*)mnMesh.vertices.data(), &mnMesh.indicies, sizeof(VertexData), mnMesh.vertices.size());

		planet.CreateBuffers(pDevice, (float*)pnMesh.vertices.data(), &pnMesh.indicies, sizeof(VertexData), pnMesh.vertices.size());

		rocket.CreateBuffers(pDevice, (float*)roMesh.vertices.data(), &roMesh.indicies, sizeof(VertexData), roMesh.vertices.size());

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

		//using renderable code
		seaweed.CreateGeometryShader(pDevice,GeometryShader, ARRAYSIZE(GeometryShader));
		seaweed.CreateShadersandInputLayout(pDevice, SeaweedVertex, ARRAYSIZE(SeaweedVertex),
		SeaweedPixel, ARRAYSIZE(SeaweedPixel), format, ARRAYSIZE(format));

		//creating v/p shaders and input layout
		{
			chest.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
				PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

			cannon.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
				PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

			island.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
				PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

			water.CreateShadersandInputLayout(pDevice, Wave_VS, ARRAYSIZE(Wave_VS),
				PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

			skyBox.CreateShadersandInputLayout(pDevice, SkyBoxVertexShader, ARRAYSIZE(SkyBoxVertexShader),
				SkyBoxPixelShader, ARRAYSIZE(SkyBoxPixelShader), format, ARRAYSIZE(format));

			lh.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
				PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

			sun.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
				PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

			moon.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
				PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

			planet.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
				PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

			rocket.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
				PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));
		}

		//init math stuff
		m.Create();
		// Initializing view matrix
		m.LookAtLHF(GW::MATH::GVECTORF{ 1.0f, 10.0f, -10.1f }, //eye
					GW::MATH::GVECTORF{ 0.0f, 0.0f, 0.0f }, //at
					GW::MATH::GVECTORF{ 0,1,0 }, //up
					Vars.view);
		
		d3d.GetAspectRatio(aspectRatio);
		//Initializing projection matrix
		m.ProjectionDirectXLHF(G_PI_F / fov, aspectRatio, zNear, zFar, Vars.projection);

		// create constant buffer
		{
			water.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
			cannon.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
			chest.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
			island.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
			skyBox.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS_SKYBOX));
			seaweed.CreateConstantBuffer(pDevice, sizeof(SCENE_INFORMATION));
			lh.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
			sun.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
			moon.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
			planet.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
			rocket.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
		}
		//setting topology for the seaweed
		seaweed.primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

		{
		//initializing the values of multiple instances for the other world matricies should now create a nomrmal pyramid at (2,2,2)
		//iVars.ID = 0;
		iVars.world[0].row1 = { 1.0f,0.0f,0.0f,0.0f };
		iVars.world[0].row2 = { 0.0f,1.0f,0.0f,0.0f };
		iVars.world[0].row3 = { 0.0f,0.0f,1.0f,0.0f };
		iVars.world[0].row4 = { 0.0f,0.0f,0.0f,1.0f };

		//setting up the others in a way that isn't absolute cancer like above
		
			iVars.world[1] = iVars.world[0];
			iVars.world[2] = iVars.world[0];
			iVars.world[3] = iVars.world[0];
			iVars.world[4] = iVars.world[0];
			iVars.world[5] = iVars.world[0];
			iVars.world[6] = iVars.world[0];
			iVars.world[7] = iVars.world[0];
			iVars.world[8] = iVars.world[0];
			iVars.world[9] = iVars.world[0];
		}
		// free temporary handle
		pDevice->Release();
	}

private:
	void scene1()
	{
		GW::MATH::GMATRIXF im;
		m.IdentityF(im);
		GW::MATH::GVECTORF scale = { 1.5f, 1.5f, 1.5f, 1.0f };
		GW::MATH::GVECTORF translate = { 0.0f, 2.5f, 0.0f };
		SHADER_VARS cb = Vars;
		m.TransposeF(Vars.projection, cb.projection);
		m.TransposeF(Vars.view, cb.view);

		//Drawing Skybox
		m.TransposeF(Vars.projection, skyboxSV.projection);
		m.TransposeF(Vars.view, skyboxSV.view);
		m.TransposeF(skyboxSV.world, skyboxSV.world);
		con->UpdateSubresource(skyBox.constantBuffer.Get(), 0, nullptr, &skyboxSV, 0, 0);
		skyBox.Bind(con);
		skyBox.Draw(con);
		con->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 0);

		//island draw
		m.ScalingF(im, scale, cb.world);
		m.TranslatelocalF(cb.world, translate, cb.world);
		m.TransposeF(cb.world, cb.world);
		con->UpdateSubresource(island.constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		island.Bind(con);
		con->PSSetShaderResources(0, 1, texSRV.GetAddressOf());
		island.Draw(con);

		//cannon draw
		scale = { 2.5f, 2.5f, 2.5f , 1 };
		m.ScalingF(im, scale, cb.world);
		translate = { 0,1.5f,0 };
		m.TranslatelocalF(cb.world, translate, cb.world);
		m.TransposeF(cb.world, cb.world);
		con->UpdateSubresource(cannon.constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		cannon.Bind(con);
		cannon.Draw(con);

		//light house
		scale = { 3.0f,3.0f,3.0f, 1 };
		m.ScalingF(im, scale, cb.world);
		translate = { 200, -55, -40 };
		m.TranslatelocalF(cb.world, translate, cb.world);
		m.TransposeF(cb.world, cb.world);
		con->UpdateSubresource(lh.constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		lh.Bind(con);
		lh.Draw(con);

		//treasure chest
		translate = {4.85f, -0.55f, -11.5f };
		m.TranslatelocalF(im, translate, cb.world);
		m.TransposeF(cb.world, cb.world);
		con->UpdateSubresource(chest.constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		chest.Bind(con);
		
		chest.Draw(con);

		//water
		m.TransposeF(im, cb.world);
		con->UpdateSubresource(water.constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		water.Bind(con);
		con->PSSetShaderResources(0, 1, texSRV.GetAddressOf());
		water.Draw(con);

		//seaweed?
		scene.Projection = *reinterpret_cast<XMFLOAT4X4*>(&cb.projection);
		scene.View = *reinterpret_cast<XMFLOAT4X4*>(&cb.view);
		scene.SeaweedHeight = 4.0f;
		scene.SeaweedWidth = 0.5f;
		scene.gridPos = XMFLOAT3(0.0f, 1.0f, 0.0f);
		con->UpdateSubresource(seaweed.constantBuffer.Get(), 0, nullptr, &scene, 0, 0);
		seaweed.Bind(con);
		con->DrawIndexedInstanced(seaweed.iCount, 2, 0, 0, 0);
		seaweed.UnBind(con);
	}


	void scene2()
	{
		GW::MATH::GMATRIXF im;
		m.IdentityF(im);
		GW::MATH::GVECTORF scale = { 1.5f, 1.5f, 1.5f, 1.0f };
		GW::MATH::GVECTORF translate = { 0.0f, 2.5f, 0.0f };
		SHADER_VARS cb = Vars;
		m.TransposeF(Vars.projection, cb.projection);
		m.TransposeF(Vars.view, cb.view);
		cb.lightColor[0] = { 0,0,0,0 };
		cb.lightColor[1] = { 1,1,0,1 };
		cb.lightColor[2] = { 0,0,0,0 };
		cb.pLightpos = { 0,0,0 };
		cb.pLightRad = 100000000;

		//Skybox
		con->UpdateSubresource(skyBox.constantBuffer.Get(), 0, nullptr, &skyboxSV, 0, 0);
		skyBox.Bind(con);
		con->PSSetShaderResources(0, 1, skyBoxSRV.GetAddressOf());
		skyBox.Draw(con);
		con->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 0);

		//sun
		scale = { 8,8,8,1 };
		m.ScalingF(im, scale, cb.world);
		m.TransposeF(cb.world, cb.world);
		con->UpdateSubresource(sun.constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		sun.Bind(con);
		con->PSSetShaderResources(0, 1, texSRV.GetAddressOf());
		sun.Draw(con);

		//rocket
		cb.world = rz;
		m.MultiplyMatrixF(cb.world, rx, cb.world);
		translate = { 500, 0, 0, 1};
		
		m.TranslatelocalF(cb.world, translate, cb.world);
		m.RotationXF(cb.world, 1.0f, cb.world);
		scale = { 0.8f,0.8f,0.8f,1 };
		m.ScalingF(cb.world, scale, cb.world);
		m.TransposeF(cb.world, cb.world);
		con->UpdateSubresource(rocket.constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		rocket.Bind(con);
		rocket.Draw(con);

		//mars
		cb.world = ry;
		//m.MultiplyMatrixF(cb.world, ry, cb.world);
		scale = { 3,3,3,1 };
		m.ScalingF(cb.world, scale, cb.world);
		translate = { 0, 0, 50, 1 };
		m.TranslatelocalF(cb.world, translate, cb.world);
		m.TransposeF(cb.world, cb.world);
		con->UpdateSubresource(planet.constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		planet.Bind(con);
		planet.Draw(con);

		//earth
		cb.world = ry;
		//m.MultiplyMatrixF(cb.world, ry, cb.world);
		scale = { 5,5,5,1 };
		m.ScalingF(cb.world, scale, cb.world);
		translate = { 0, 0, -30, 1 };
		m.TranslatelocalF(cb.world, translate, cb.world);
		m.TransposeF(cb.world, cb.world);
		con->UpdateSubresource(planet.constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		planet.Bind(con);
		con->PSSetShaderResources(0, 1, etex.GetAddressOf());
		planet.Draw(con);

		//moon
		GW::MATH::GMATRIXF temp;
		m.TransposeF(cb.world, temp);
		cb.world = im;
		scale = { 0.01f,0.01f,0.01f,1 };
		m.ScalingF(cb.world, scale, cb.world);
		m.MultiplyMatrixF(cb.world, ry, cb.world);

		translate = { 0,0,1000,1 };
		m.TranslatelocalF(cb.world, translate, cb.world);
		

		m.MultiplyMatrixF(cb.world, temp, cb.world);
		m.TransposeF(cb.world, cb.world);
		con->UpdateSubresource(moon.constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
		moon.Bind(con);
		con->PSSetShaderResources(0, 1, texSRV.GetAddressOf());
		moon.Draw(con);
	}

public:

	//Bind objects to render and update constant buffers 
	void Render()
	{
		GW::MATH::GMATRIXF temp;
		m.IdentityF(temp);
		
		// grab the context & render target
		d3d.GetImmediateContext((void**)&con);
		d3d.GetRenderTargetView((void**)&view);
		d3d.GetDepthStencilView((void**)&depth);
		// setup the pipeline
		ID3D11RenderTargetView *const views[] = { view };
		con->OMSetRenderTargets(ARRAYSIZE(views), views, depth);
		
		//setting first view port and draw calls
		D3D11_VIEWPORT viewportOne;
		viewportOne.Width = 400;
		viewportOne.Height = 600;
		viewportOne.MinDepth = 0.0f;
		viewportOne.MaxDepth = 1.0f;

		unsigned int topLX = 0;
		unsigned int topLY = 0;
		win.GetClientTopLeft(topLX, topLY);

		viewportOne.TopLeftX = (float)topLX;
		viewportOne.TopLeftY = (float)topLY;
	    con->RSSetViewports(1, &viewportOne);
		
		scene1();
	
		//second viewport and draw calls
		D3D11_VIEWPORT viewportTwo;
		viewportTwo.Width = 400;
		viewportTwo.Height = 600;
		viewportTwo.MinDepth = 0.0f;
		viewportTwo.MaxDepth = 1.0f;

		topLX = 401;
		topLY = 0;
		viewportTwo.TopLeftX = (float)topLX;
		viewportTwo.TopLeftY = (float)topLY;

		con->RSSetViewports(1, &viewportTwo);
		
		scene2();

		// release temp handles
		view->Release();
		depth->Release();
		con->Release();
	}

	//Use to update things such as camera movement, world matrix, etc
	void Update()
	{
		static ULONGLONG timeStart = 0;
		ULONGLONG timeCur = GetTickCount64();

		if (timeStart == 0)
			timeStart = timeCur;

		Vars.time = (timeCur - timeStart) / 10000.0f;

		m.RotationYF(ry, 0.01f, ry);
		m.RotationXF(rx, 0.01f, rx);
		m.RotationZF(rz, 0.01f, rz);
		//m.RotationYF(Vars.world, 0.001f, Vars.world);
		GW::MATH::GVECTORF tvec = { 8.0f,  2.0f, 0.0f };
		GW::MATH::GVECTORF svec = tvec;
		//tvec.x += 4.0f;
		svec.x -= 3.0f;
		m.VectorXMatrixF(Camera.world, svec, svec);
		m.VectorXMatrixF(Camera.world, tvec, tvec);
		Vars.pLightpos = { tvec.x, tvec.y, tvec.z };
		Vars.spotPos = { -tvec.x, tvec.y, -tvec.z };
		//Vars.coneDir = { svec.x, svec.y, svec.z };
		
		iVars.coneDir = Vars.coneDir;
		GW::MATH::GMATRIXF rotateX;
		GW::MATH::GMATRIXF rotateY;
		float x;
		float y; 

		float keys[6] = { G_KEY_W, G_KEY_A, G_KEY_S, G_KEY_D, G_KEY_Q, G_KEY_E };
		float wasd[6] = { 0, };
		for (int i = 0; i < 6; i++)
		{
			input.GetState(keys[i], wasd[i]);
		}
		if (input.GetMouseDelta(x, y) == GW::GReturn::REDUNDANT) { x = y = 0; }
		m.RotationXF(GW::MATH::GIdentityMatrixF, y * 0.01f, rotateX);
		m.RotationYF(GW::MATH::GIdentityMatrixF, x * 0.01f, rotateY);
		m.InverseF(Vars.view, Vars.view);
		// m.RotationYawPitchRollF(x / 360, y / 360, 0.0f, rotateX); /
		//m.MultiplyMatrixF(rotateX, rotateY, rotateX);
		GW::MATH::GVECTORF save = Vars.view.row4;
		m.MultiplyMatrixF(rotateX, Vars.view, Vars.view);
		m.MultiplyMatrixF(Vars.view, rotateY, Vars.view);
		Vars.view.row4 = save;
		GW::MATH::GMATRIXF move;
		m.TranslatelocalF(GW::MATH::GIdentityMatrixF,
			GW::MATH::GVECTORF{ wasd[3] - wasd[1], wasd[4] - wasd[5], wasd[0] - wasd[2] }, move);
		m.MultiplyMatrixF(move, Vars.view, Vars.view);
		Vars.camwpos = { Vars.view.row1.x, Vars.view.row1.y, Vars.view.row1.z };
		GW::MATH::GVECTORF position = Vars.view.row4;
		m.InverseF(Vars.view, Vars.view);

		skyboxSV.world = (GW::MATH::GMATRIXF&)XMMatrixTranslation(position.x, position.y, position.z);
		
		iVars.pLightpos = Vars.pLightpos;
		iVars.lightColor[2] = Vars.lightColor[2];
		iVars.camwpos = Vars.camwpos;
		iVars.spotPos = Vars.spotPos;
		iVars.specularPow = Vars.specularPow;
		iVars.view = Vars.view;
		iVars.specIntent = Vars.specIntent;
		iVars.pLightRad = Vars.pLightRad;
		iVars.dLightdir = Vars.dLightdir;

		UpdateFOVandPlane(d3d, &Vars.projection);
		
	}

	void UpdateProjection(GW::GRAPHICS::GDirectX11Surface _d3d, GW::MATH::GMATRIXF* projectionMatrix)
	{
		GW::GReturn results = _d3d.GetAspectRatio(aspectRatio);
		_aspectRatio = aspectRatio;
		aspectRatio = 1 / aspectRatio;
		projectionMatrix->row1 = { ((1 / tanf((0.5 * fov) * G_PI_F / 180.0f)) * (aspectRatio)), (0.0f), (0.0f), (0.0f) };
		projectionMatrix->row2 = { (0.0f), (1 / tanf((0.5f * fov) * G_PI_F / 180.0f)), (0.0f), (0.0f) };
		projectionMatrix->row3 = { (0.0f), (0.0f), (zFar / (zFar - zNear)), (1.0f) };
		projectionMatrix->row4 = { (0.0f), (0.0f), (-(zFar * zNear) / (zFar - zNear)), (0.0f) };
	}

	void UpdateFOVandPlane(GW::GRAPHICS::GDirectX11Surface _d3d, GW::MATH::GMATRIXF* projectionMatrix)
	{
		//Increase FOV
		if (GetAsyncKeyState(VK_UP))
		{
			fov += 0.1f;
			if (fov > 140)
			{
				fov = 140;
			}
			UpdateProjection(_d3d, projectionMatrix);
		}
		//Decrease FOV
		else if (GetAsyncKeyState(VK_DOWN))
		{
			fov -= 0.1f;
			if (fov < 40)
			{
				fov = 40;
			}
			UpdateProjection(_d3d, projectionMatrix);
		}

		//Increase ZNear
		if (GetAsyncKeyState(VK_NUMPAD8))
		{
			zNear += 0.01f;
			UpdateProjection(_d3d, projectionMatrix);
		}
		//Decrease ZNear
		else if (GetAsyncKeyState(VK_NUMPAD2))
		{
			zNear -= 0.01f;
			if (zNear <= 0)
			{
				zNear = 0.01f;
			}
			UpdateProjection(_d3d, projectionMatrix);
		}
		//Increase ZFar
		if (GetAsyncKeyState(VK_NUMPAD6))
		{
			zFar += 0.01f;
			UpdateProjection(_d3d, projectionMatrix);
		}
		//Decrease ZFar
		else if (GetAsyncKeyState(VK_NUMPAD4))
		{
			zFar -= 0.01f;
			if (zFar < zNear)
			{
				zFar = zNear + 0.01f;
			}
			UpdateProjection(_d3d, projectionMatrix);
		}
	};


	void CreateSeaweed() {

		ID3D11Device* pDevice = nullptr;
		d3d.GetDevice((void**)&pDevice);
		//geo shader stuff
		static unsigned int numVert = 0;
		static unsigned int numIndex = 0;

		if (!seaweedLocations || !seaweedDirections) {
			XMFLOAT4 seaweedPositions[MAX_SEAWEED_COUNT];
			XMFLOAT4 seaweedDirectionsdone[MAX_SEAWEED_COUNT]; //yes i know basically named it the same thing, fight me
			//looping to create the random XZ positions
			for (unsigned int i = 0; i < MAX_SEAWEED_COUNT; i++) {
				seaweedPositions[i] = XMFLOAT4(RAND_FLOAT(-5, 5), 0, RAND_FLOAT(-5, 5), RAND_FLOAT(0.2, 1));
			}
			//loop for the random directions even though I'm not really intersted in it right now -.-
			for (unsigned int i = 0; i < MAX_SEAWEED_COUNT; i++) {
				seaweedDirectionsdone[i] = XMFLOAT4(RAND_FLOAT(-0.5, 0.5), RAND_FLOAT(1.0, 2.5), RAND_FLOAT(-5, 5), RAND_FLOAT(0.2, 1)); //ideally we don't need to worry abotu this
				XMStoreFloat4(&seaweedDirectionsdone[i], XMVector4Normalize(XMLoadFloat4(&seaweedDirectionsdone[i])));
				//we'll rotate the seaweed but only a bit to make it seem like it's being affected by the waves more than just following basically
				seaweedDirectionsdone[i].w = RAND_FLOAT(XMConvertToRadians(-2), XMConvertToRadians(-1));
			}
			//now comes the hard part - i'm doing it the non-normal way that we have been doing it cause I'm not sure how the geoshaders will take our current layout so sorry
			//is this shameless? yes. Do I care? A little but not at the moment, what works works, other details can be fixed later
			D3D11_BUFFER_DESC descriptor;
			ZeroMemory(&descriptor, sizeof(descriptor));
			descriptor.ByteWidth = sizeof(XMFLOAT4) * MAX_SEAWEED_COUNT;
			descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			//creating location vertex buffer
			D3D11_SUBRESOURCE_DATA contents;
			ZeroMemory(&contents, sizeof(contents));
			contents.pSysMem = seaweedPositions;
			pDevice->CreateBuffer(&descriptor, &contents, &seaweedLocations);
			//doing the same for the directions even though we aren't focusing on it
			contents.pSysMem = seaweedDirectionsdone;
			pDevice->CreateBuffer(&descriptor, &contents, &seaweedDirections);

			D3D11_RASTERIZER_DESC rasterizerDesc;
			ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
			rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE; //now the seaweed won't disappear even if we manage to get behind it
			rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
			pDevice->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());
		//	
		//	//ok so assuming that we have set everything, lets hope to god that this will work or else we are uber fucked
		//	//for now let's just put it on everything
		//	how did we even get here....

			pDevice->Release();
			
		}
	}

	~Renderer()
	{
		// ComPtr will auto release so nothing to do here 
	}
};
