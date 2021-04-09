#include <vector>
#include <DirectXMath.h>
#include "VertexShader.h"
#include "PixelShader.h"
#include "Renderable.h"
#include "axe1.h"
#include "test_pyramid.h"
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

	bool LoadMeshFromOBJ(const char* path, MeshData<VertexData>& container)
	{
		//bool result = false;
		std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
		std::vector<XMFLOAT3> temp_vertices, temp_uvs, temp_normals;
		std::vector <XMFLOAT3> out_vertices, out_uvs, out_normals;

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
				XMFLOAT3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				temp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0)
			{
				XMFLOAT3 uv;
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				uv.z = 0.0f;
				temp_uvs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0)
			{
				XMFLOAT3 nrm;
				fscanf(file, "%f %f %f\n", &nrm.x, &nrm.y, &nrm.z);
				temp_normals.push_back(nrm);
			}
			else if (strcmp(lineHeader, "f") == 0) {
				std::string vertex1, vertex2, vertex3;
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
			unsigned int vertexIndex = vertexIndices[i] - 1;
			XMFLOAT3 vertex = temp_vertices[vertexIndex];
			entry.pos = vertex;
			// uvs
			unsigned int uvIndex = uvIndices[i] - 1;
			XMFLOAT3 uvw = temp_uvs[uvIndex];
			entry.uvw = uvw;
			//nrm
			unsigned int nrmIndex = normalIndices[i] - 1;
			XMFLOAT3 nrm = temp_uvs[nrmIndex];
			entry.nrm = nrm;

			container.vertices.push_back(entry);
			container.indicies.push_back(i);
		}
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
			VertexData line1 = { {cx,0,z}, {0,0,0}, {0,0,0} };
			VertexData line2 = { {cx + size, 0, z}, {0,0,0}, {0,0,0} };
			VertexData line3 = { {x,0,cz}, {0,0,0}, {0,0,0} };
			VertexData line4 = { {x, 0, cz + size}, {0,0,0}, {0,0,0} };
			grid.vertices.push_back(line1);
			grid.indicies.push_back(i++);
			grid.vertices.push_back(line2);
			grid.indicies.push_back(i++);
			grid.vertices.push_back(line3);
			grid.indicies.push_back(i++);
			grid.vertices.push_back(line4);
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
	//Renderable object used to load pyramid obj
	Renderable pyramid;
	Renderable axe;
	Renderable grid;
	Renderable testObj;
	SHADER_VARS Vars;

	// world view proj
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;

	// math library handle
	GW::MATH::GMatrix m;

public:

	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		srand(time(NULL));
		win = _win;
		d3d = _d3d;
		ID3D11Device* pDevice = nullptr;
		d3d.GetDevice((void**)&pDevice);

		//loading obj data into mesh
		MeshData<VertexData> pMesh = LoadMeshFromHeader(test_pyramid_data, test_pyramid_indicies, 
			test_pyramid_vertexcount, test_pyramid_indexcount);
		MeshData<VertexData> aMesh = LoadMeshFromHeader(axe1_data, axe1_indicies, axe1_vertexcount, axe1_indexcount);
		MeshData<VertexData> gMesh = MakeGrid();
		MeshData<VertexData> tMesh;
		LoadMeshFromOBJ("../PPIV-Project/GreenScreen/test02.obj", tMesh);
		
		//making pyramid index and vertex buffers
		pyramid.CreateBuffers(pDevice, (float*)pMesh.vertices.data(), &pMesh.indicies, sizeof(VertexData), pMesh.vertices.size());
		//making axe buffers
		axe.CreateBuffers(pDevice, (float*)aMesh.vertices.data(), &aMesh.indicies, sizeof(VertexData), aMesh.vertices.size());
		//making grid buffers
		grid.CreateBuffers(pDevice, (float*)gMesh.vertices.data(), &gMesh.indicies, sizeof(VertexData), gMesh.vertices.size());
		//test mesh buffers
		testObj.CreateBuffers(pDevice, (float*)tMesh.vertices.data(), &tMesh.indicies, sizeof(VertexData), tMesh.vertices.size());

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

		grid.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
			PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

		testObj.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
			PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

		//init math stuff
		m.Create();
		
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
		grid.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
		testObj.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));

		//setting topology for grid
		grid.primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

		// free temporary handle
		pDevice->Release();
	}
	//Bind objects to render and update constant buffers 
	void Render()
	{
		GW::MATH::GMATRIXF temp;
		m.IdentityF(temp);

		// grab the context & render target
		d3d.GetImmediateContext((void**)&con);
		d3d.GetRenderTargetView((void**)&view);
		// setup the pipeline
		ID3D11RenderTargetView *const views[] = { view };
		con->OMSetRenderTargets(ARRAYSIZE(views), views, nullptr);

		SHADER_VARS pcb;
		
		m.TransposeF(pcb.world, pcb.world);
		m.TransposeF(Vars.view, pcb.view);

		//drawing grid
		m.TransposeF(Vars.projection, pcb.projection);
		con->UpdateSubresource(grid.constantBuffer.Get(), 0, nullptr, &pcb, 0, 0);
		grid.Bind(con);
		grid.Draw(con);

		GW::MATH::GVECTORF scale = { 10.0f, 10.f, 10.0f, 1.0f };
		m.ScalingF(temp, scale, pcb.world);
		m.TransposeF(pcb.world, pcb.world);

		//drawing pyramid
		con->UpdateSubresource(pyramid.constantBuffer.Get(), 0, nullptr, &pcb, 0, 0);
		pyramid.Bind(con);
		pyramid.Draw(con);

		//drawing test object
		GW::MATH::GVECTORF translate = { 0.0f, -5.0f, 10.0f };
		m.TranslatelocalF(temp, translate, pcb.world);
		m.TransposeF(pcb.world, pcb.world);
		con->UpdateSubresource(testObj.constantBuffer.Get(), 0, nullptr, &pcb, 0, 0);
		testObj.Bind(con);
		testObj.Draw(con);

		//drawing axe
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
		/*static float t = 0.0f;
		static ULONGLONG timeStart = 0;
		ULONGLONG timeCur = GetTickCount64();
		if (timeStart == 0)
			timeStart = timeCur;
		t = (timeCur - timeStart) / 1000.0f;
		
		m.RotationYF(Vars.world, t, Vars.world);*/



		// Rotate the second light around the origin
		XMMATRIX mRotate = XMMatrixRotationY(-1.0f * t); //now should be able to rotate lights at the same kind of time for the axe
		XMVECTOR vLightDir = XMLoadFloat4(&vLightDirs[1]);
		// rotates the second light
		vLightDir = XMVector3Transform(vLightDir, mRotate);
		XMStoreFloat4(&vLightDirs[1], vLightDir);
	}

	~Renderer()
	{
		// ComPtr will auto release so nothing to do here 
	}
};
