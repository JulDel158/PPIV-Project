#include <vector>
#include <DirectXMath.h>
#include <d3d11_1.h>
#include <directxcolors.h>
#include "VertexShader.h"
#include "PixelShader.h"
#include "InstancePixelShader.h"
#include "InstanceVertexShader.h"
#include "Renderable.h"
#include "axe2.h"
#include "test_pyramid.h"
#include "Wave_VS.h"
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
		XMFLOAT4 lightColor[3] = { {0.0f, 0.32f, 0.84f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.2f, 0.3f, 0.0f, 0.3f} };
		XMFLOAT4 wave1 = { 1.0f, 1.0f, 0.25f, 30.0f };
		XMFLOAT4 wave2 = { 1.0f, 0.6f, 0.25f, 16.0f };
		XMFLOAT4 wave3 = { 1.0f, 1.3f, 0.25f, 8.0f };
		float specularPow = 10.0f;
		XMFLOAT3 camwpos;
		float specIntent = 0.7f;
		XMFLOAT3 spotPos = { -5.0f, 2.0f, 0.0f };
		
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
		float specIntent = 0.7f;
		XMFLOAT3 spotPos = { -5.0f, 2.0f, 0.0f };
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
	Renderable pyramid;
	Renderable axe;
	Renderable grid;
	Renderable testObj;
	SHADER_VARS Vars;
	SHADER_VARS Camera;
	SHADER_VARS_INSTANCE iVars;

	float prevFrame = clock();
	float dt = 0;

	// math library handle
	GW::MATH::GMatrix m;
	// Input Library
	GW::INPUT::GInput input;
	// resource view for default texture
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texSRV;

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
		MeshData<VertexData> pMesh = LoadMeshFromHeader(test_pyramid_data, test_pyramid_indicies, 
			test_pyramid_vertexcount, test_pyramid_indexcount);
		MeshData<VertexData> aMesh = LoadMeshFromHeader(axe2_data, axe2_indicies, axe2_vertexcount, axe2_indexcount);
		MeshData<VertexData> gMesh = MakePlaneGrid(50, 50);

		MeshData<VertexData> tMesh;
		LoadMeshFromOBJ("../PPIV-Project/GreenScreen/test02.obj", tMesh);
		
		//Setting texture + sampler
		axe.CreateTextureandSampler(pDevice, "../PPIV-Project/GreenScreen/axeTexture.dds");
		pyramid.CreateTextureandSampler(pDevice, "");
		grid.CreateTextureandSampler(pDevice, "");
		testObj.CreateTextureandSampler(pDevice, "");

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
		pyramid.CreateShadersandInputLayout(pDevice, InstanceVertexShader, ARRAYSIZE(InstanceVertexShader),
			InstancePixelShader, ARRAYSIZE(InstancePixelShader), format, ARRAYSIZE(format));

		axe.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader), 
			PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

		testObj.CreateShadersandInputLayout(pDevice, VertexShader, ARRAYSIZE(VertexShader),
			PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));

		grid.CreateShadersandInputLayout(pDevice, Wave_VS, ARRAYSIZE(Wave_VS),
			PixelShader, ARRAYSIZE(PixelShader), format, ARRAYSIZE(format));
		// Wave_VS, ARRAYSIZE(Wave_VS),
		//init math stuff
		m.Create();
		//Vars.time = 0.0f;
		// Initializing view matrix
		m.LookAtLHF(GW::MATH::GVECTORF{ 1.0f, 10.0f, -10.1f }, //eye
					GW::MATH::GVECTORF{ 0.0f, 0.0f, 0.0f }, //at
					GW::MATH::GVECTORF{ 0,1,0 }, //up
					Vars.view);

		//Vars.eye = { Vars.view.row1.x, Vars.view.row1.y, Vars.view.row1.z };
		
		float ar;
		d3d.GetAspectRatio(ar);
		//Initializing projection matrix
		m.ProjectionDirectXLHF(G_PI_F / 2.0f, ar, 0.01f, 100, Vars.projection);

		// create constant buffer
		pyramid.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS_INSTANCE));
		axe.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
		grid.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));
		testObj.CreateConstantBuffer(pDevice, sizeof(SHADER_VARS));

		//setting topology for grid
		//grid.primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

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
		d3d.GetDepthStencilView((void**)&depth);
		// setup the pipeline
		ID3D11RenderTargetView *const views[] = { view };
		con->OMSetRenderTargets(ARRAYSIZE(views), views, depth);

		SHADER_VARS pcb;
		pcb.pLightRad = 5.0f;
		iVars.pLightRad = pcb.pLightRad;
		iVars.dLightdir = pcb.dLightdir;
		pcb.pLightpos = Vars.pLightpos;
		pcb.time = Vars.time;
		m.TransposeF(Vars.projection, pcb.projection);
		m.TransposeF(Vars.view, pcb.view);
		m.TransposeF(pcb.world, pcb.world);


		
		iVars.time = Vars.time;
		m.TransposeF(Vars.projection, iVars.projection);
		m.TransposeF(Vars.view, iVars.view);
		

		GW::MATH::GVECTORF scale = { 10.0f, 10.f, 10.0f, 1.0f };
		m.ScalingF(temp, scale, iVars.world[0]);
		m.TransposeF(iVars.world[0], iVars.world[0]);
		//use this area to move other world matricies
		GW::MATH::GVECTORF translate1 = { -1.0f, 0.0f, 0.0f }; //code that should move the first world matrix in the array to whatever coordinate
		m.TranslatelocalF(temp, translate1, iVars.world[1]);

		m.ScalingF(iVars.world[1], scale, iVars.world[1]);
		m.TransposeF(iVars.world[1], iVars.world[1]);
		
		//drawing grid
		con->UpdateSubresource(grid.constantBuffer.Get(), 0, nullptr, &pcb, 0, 0);
		grid.Bind(con);
		con->PSSetShaderResources(0, 1, texSRV.GetAddressOf());
		grid.Draw(con);

		//GW::MATH::GVECTORF scale = { 10.0f, 10.f, 10.0f, 1.0f };
		m.ScalingF(temp, scale, pcb.world);
		m.TransposeF(pcb.world, pcb.world);

		//drawing pyramid
		con->UpdateSubresource(pyramid.constantBuffer.Get(), 0, nullptr, &iVars, 0, 0);
		pyramid.Bind(con);
		con->PSSetShaderResources(0, 1, texSRV.GetAddressOf());
		//pyramid.Draw(con);
		con->DrawIndexedInstanced(pyramid.iCount,2,0,0, 0);

		//drawing test object
		GW::MATH::GVECTORF translate = { 10.0f, 5.0f, 0.0f };
		m.TranslatelocalF(temp, translate, pcb.world);
		m.TransposeF(pcb.world, pcb.world);
		con->UpdateSubresource(testObj.constantBuffer.Get(), 0, nullptr, &pcb, 0, 0);
		testObj.Bind(con);
		con->PSSetShaderResources(0, 1, texSRV.GetAddressOf());
		testObj.Draw(con);

		//drawing axe
		scale = { 0.3f, 0.3f, 0.3f, 1.0f };
		m.ScalingF(Camera.world, scale, pcb.world);
		m.TransposeF(pcb.world, pcb.world);
		con->UpdateSubresource(axe.constantBuffer.Get(), 0, nullptr, &pcb, 0, 0);
		axe.Bind(con);
		axe.Draw(con);

		// release temp handles
		view->Release();
		depth->Release();
		con->Release();
	}

	//Use to update things such as camera movement, world matrix, etc
	void Update()
	{
		//static float t = 0.0f;
		static ULONGLONG timeStart = 0;
		ULONGLONG timeCur = GetTickCount64();
		if (timeStart == 0)
			timeStart = timeCur;
		Vars.time = (timeCur - timeStart) / 10000.0f;
		//Vars.time += 0.001f;
		/*float dt = (clock() - prevFrame) / 10000.0f;
		prevFrame = clock();
		Vars.time = (1.0f / dt);*/
		m.RotationYF(Camera.world, 0.01f, Camera.world);
		m.RotationYF(Vars.world, 0.001f, Vars.world);
		GW::MATH::GVECTORF tvec = { 0.0f,  2.0f, 0.0f };
		tvec.x += 4.0f;
		m.VectorXMatrixF(Vars.world, tvec, tvec);
		Vars.pLightpos = { tvec.x, tvec.y, tvec.z };
		
		
		GW::MATH::GMATRIXF rotateX;
		GW::MATH::GMATRIXF rotateY;
		float x;
		float y; 

		float keys[4] = { G_KEY_W, G_KEY_A, G_KEY_S, G_KEY_D };
		float wasd[4] = { 0, };
		for (int i = 0; i < 4; i++)
		{
			input.GetState(keys[i], wasd[i]);
		}
		input.GetMouseDelta(x, y);
		m.RotationXF(GW::MATH::GIdentityMatrixF,  y * 0.001f , rotateX);
		m.RotationYF(GW::MATH::GIdentityMatrixF, - x * 0.001f, rotateY);
		m.InverseF(Vars.view, Vars.view);
		/*m.RotationYawPitchRollF(x/360, y/360, 0.0f, rotateX );*/
		m.MultiplyMatrixF(Vars.view, rotateY,Vars.view );
		m.MultiplyMatrixF(Vars.view, rotateX,Vars.view);
		GW::MATH::GMATRIXF move; 
		m.TranslatelocalF(GW::MATH::GIdentityMatrixF,
			GW::MATH::GVECTORF{ wasd[3] - wasd[1], 0, wasd[0] - wasd[2] }, move);
		m.MultiplyMatrixF(Vars.view, move, Vars.view);
		Vars.camwpos = { Vars.view.row1.x, Vars.view.row1.y, Vars.view.row1.z };
		m.InverseF(Vars.view, Vars.view);
		
		iVars.pLightpos = Vars.pLightpos;
		iVars.lightColor[2] = Vars.lightColor[2];
		iVars.camwpos = Vars.camwpos;
		iVars.spotPos = Vars.spotPos;
		iVars.specularPow = Vars.specularPow;
		iVars.view = Vars.view;
	}

	~Renderer()
	{
		// ComPtr will auto release so nothing to do here 
	}
};
