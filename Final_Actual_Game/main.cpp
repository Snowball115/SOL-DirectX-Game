#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT

#include <dinput.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dx11.h>
#include <windows.h>
#include <dxerr.h>
#include <xnamath.h>
#include <string>
#include "Camera.h"
#include "text2D.h"
#include "Utilities.h"
#include "Model.h"
#include "DirectionalLight.h"
#include "Input.h"
#include "Skybox.h"
#include "ParticleGenerator.h"
#include "Monster.h"
#include "Player.h"
#include "BattleSystem.h"
#include "Collectible.h"
#include "MovableObject.h"
#include "NPCTrainer.h"

// ******************************
//		GLOBAL VARIABLES
// ******************************
HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;

D3D_DRIVER_TYPE g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pD3DDevice = NULL;
ID3D11DeviceContext* g_pImmediateContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;

ID3D11RenderTargetView* g_pBackBufferRTView = NULL;
ID3D11DepthStencilView* g_pZBuffer;
ID3D11Texture2D* g_pBackBufferTexture;
D3D11_VIEWPORT g_ScreenViewport;

ID3D11Buffer* g_pVertexBuffer;
ID3D11Buffer* g_pConstantBuffer0;
ID3D11VertexShader* g_pVertexShader;
ID3D11PixelShader* g_pPixelShader;
ID3D11InputLayout* g_pInputLayout;

//Copy the vertices into the buffer
D3D11_MAPPED_SUBRESOURCE ms;

ID3D11ShaderResourceView* g_pTexture0;
ID3D11SamplerState* g_pSampler0;

XMMATRIX transpose;
XMMATRIX g_projection, g_world, g_view;

Input* g_pInput;

// SCENE AND GAME OBJECTS
Text2D* g_2DText;
Camera* g_pCamera;
Skybox* g_pSkybox;
DirectionalLight* g_pMainLight;
BattleSystem* g_pBattleSystem;

Model* g_pModelSphereReflective;
Model* g_pModelPlane;
Player* g_pPlayer;
Monster* g_pPlayerMonster;
Monster* g_pEnemyTrainerMonster;
NPCTrainer* g_pEnemyTrainer;

vector<Model*> g_StaticModels;
vector<Monster*> g_MonstersActive;
vector<Collectible*> g_CollectiblesActive;
vector<MovableObject*> g_MovablesActive;
vector<ParticleGenerator*> g_ParticlesActive;

ParticleGenerator* g_pParticleRainbow;

float g_worldMatrixY;
float g_worldMatrixX;
float g_worldMatrixZ = 15;

int g_resX = 1200;
int g_resY = 800;
float currentXscreen;
float currentYscreen;

bool g_lockMouseLook = false;
float g_oldMouseX;
float g_oldMouseY;
float g_mouseX = 0;
float g_mouseY = 0;
float g_rotateAmount = 0.8f;
float g_moveAmount = 0.03f;
float g_jumpAmount = 0.05f;
int g_monsterCount = 8;
int g_itemsCount = 6;
int g_treeCount = 30;
int g_movablesCount = 5;

struct CONSTANT_BUFFER0
{
	XMMATRIX WorldViewProjection; // 64 bytes
	XMVECTOR directional_light_vector;
	XMVECTOR directional_light_colour;
	XMVECTOR ambient_light_vector;
	float scale;			// 4
	float transformX;		// 4
	float transformY;		// 4
	float packing_bytes;	// 4
};

CONSTANT_BUFFER0 cb0_values;

// APP NAME IN TITLE BAR OF WINDOW
char g_AppName[100] = "Main Game\0";

// FORWARD DECLARATIONS
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitialiseD3D();
HRESULT InitialiseGraphics(void);
void ShutdownD3D();
void RenderFrame(void);

// ******************************
//		MAIN
// ******************************
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitialiseWindow(hInstance, nCmdShow)))
	{
		DXTRACE_MSG("Failed to initalise Window");
		return 0;
	}

	if (FAILED(InitialiseD3D()))
	{
		DXTRACE_MSG("Failed to create Device");
		return 0;
	}

	if (FAILED(g_pInput->InitialiseInput()))
	{
		DXTRACE_MSG("Failed to initialise Input");
		return 0;
	}

	if (FAILED(InitialiseGraphics()))
	{
		DXTRACE_MSG("Failed to initialise Graphics");
		return 0;
	}

	//Main msg loop
	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			RenderFrame();
		}
	}

	ShutdownD3D();

	return (int)msg.wParam;
}

// ******************************
//		CREATE WINDOW
// ******************************
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow)
{
	// App name
	char AppName[100] = "DirectX Game\0";

	// Register class
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = AppName;

	if (!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, g_resX, g_resY };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	g_hWnd = CreateWindow(AppName, g_AppName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!g_hWnd) return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

// ******************************
//		MESSAGE CALLBACK
// ******************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_MOUSEMOVE:
			g_oldMouseX = g_mouseX;
			g_oldMouseY = g_mouseY;

			g_mouseX = LOWORD(lParam);
			g_mouseY = HIWORD(lParam);
			
			if (!g_lockMouseLook)
			{
				if ((g_mouseX - g_oldMouseX) > 0) g_pCamera->Rotate(g_rotateAmount, 0);
				else if ((g_mouseX - g_oldMouseX) < 0) g_pCamera->Rotate(-g_rotateAmount, 0);

				if ((g_mouseY - g_oldMouseY) > 0) g_pCamera->Rotate(0, -g_rotateAmount);
				else if ((g_mouseY - g_oldMouseY) < 0) g_pCamera->Rotate(0, g_rotateAmount);
			}
			
			break;
	
		case WM_SIZE:
			currentXscreen = LOWORD(lParam);
			currentYscreen = HIWORD(lParam);

			if (g_pSwapChain)
			{
				RECT rc;
				GetClientRect(g_hWnd, &rc);
				UINT width = rc.right - rc.left;
				UINT height = rc.bottom - rc.top;

				//g_pImmediateContext->OMSetRenderTargets(0, 0, 0);

				// Release all outstanding references to the swap chain's buffers.
				g_pBackBufferRTView->Release();
				// CRASH HERE
				g_pZBuffer->Release();

				HRESULT hr;

				// Preserve the existing buffer count and format.
				// Automatically choose the width and height to match the client rect for HWNDs.
				hr = g_pSwapChain->ResizeBuffers(1, (UINT)currentXscreen, (UINT)currentYscreen, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
				if (FAILED(hr)) return hr;

				// Get buffer and create a render-target-view.
				ID3D11Texture2D* pBuffer;

				hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);
				if (FAILED(hr)) return hr;

				hr = g_pD3DDevice->CreateRenderTargetView(pBuffer, NULL, &g_pBackBufferRTView);
				if (FAILED(hr)) return hr;

				pBuffer->Release();

				//g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, NULL);

				D3D11_TEXTURE2D_DESC depthStencilDesc;
				ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
				depthStencilDesc.Width = (UINT)currentXscreen;
				depthStencilDesc.Height = (UINT)currentYscreen;
				depthStencilDesc.MipLevels = 1;
				depthStencilDesc.ArraySize = 1;
				depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				depthStencilDesc.SampleDesc.Count = 1;
				depthStencilDesc.SampleDesc.Quality = 0;
				depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
				depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
				depthStencilDesc.CPUAccessFlags = 0;
				depthStencilDesc.MiscFlags = 0;

				ID3D11Texture2D* pZBufferTexture;
				hr = (g_pD3DDevice->CreateTexture2D(&depthStencilDesc, NULL, &pZBufferTexture));
				if (FAILED(hr)) return false;

				hr = (g_pD3DDevice->CreateDepthStencilView(pZBufferTexture, 0, &g_pZBuffer));
				if (FAILED(hr)) return false;
				pZBufferTexture->Release();

				g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, g_pZBuffer);

				// Set up the viewport.
				g_ScreenViewport.Width = currentXscreen;
				g_ScreenViewport.Height = currentYscreen;
				g_ScreenViewport.MinDepth = 0.0f;
				g_ScreenViewport.MaxDepth = 1.0f;
				g_ScreenViewport.TopLeftX = 0.0f;
				g_ScreenViewport.TopLeftY = 0.0f;

				g_pImmediateContext->RSSetViewports(1, &g_ScreenViewport);
			}

			return 1;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// ******************************
//		CREATE D3D DEVICE AND SWAP CHAIN
// ******************************
HRESULT InitialiseD3D()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; //if program crashes comment this out
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pD3DDevice, &g_featureLevel, &g_pImmediateContext);

		if (SUCCEEDED(hr)) break;
	}

	if (FAILED(hr)) return hr;

	// Get pointer to back buffer texture
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&g_pBackBufferTexture);

	if (FAILED(hr)) return hr;

	// Use the back buffer texture pointer to create the render target view
	hr = g_pD3DDevice->CreateRenderTargetView(g_pBackBufferTexture, NULL, &g_pBackBufferRTView);

	g_pBackBufferTexture->Release();

	if (FAILED(hr)) return hr;

	// Create Z Buffer texture
	D3D11_TEXTURE2D_DESC tex2dDesc;
	ZeroMemory(&tex2dDesc, sizeof(tex2dDesc));

	tex2dDesc.Width = width;
	tex2dDesc.Height = height;
	tex2dDesc.ArraySize = 1;
	tex2dDesc.MipLevels = 1;
	tex2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tex2dDesc.SampleDesc.Count = sd.SampleDesc.Count;
	tex2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex2dDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* pZBufferTexture;
	hr = g_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);
	if (FAILED(hr)) return hr;

	// Create the Z Buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	g_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &g_pZBuffer);
	pZBufferTexture->Release();

	// Set the render target view
	g_pImmediateContext->OMSetRenderTargets(1, &g_pBackBufferRTView, g_pZBuffer);

	// Set the viewport
	g_ScreenViewport.TopLeftX = 0.0f;
	g_ScreenViewport.TopLeftY = 0.0f;
	g_ScreenViewport.Width = static_cast<float>(width);
	g_ScreenViewport.Height = static_cast<float>(height);
	g_ScreenViewport.MinDepth = 0.0f;
	g_ScreenViewport.MaxDepth = 1.0f;

	g_pImmediateContext->RSSetViewports(1, &g_ScreenViewport);

	g_2DText = new Text2D("assets/font1.bmp", g_pD3DDevice, g_pImmediateContext);

	g_pInput = new Input(g_hInst, g_hWnd);

	return S_OK;
}

// ******************************
//		CLEAN UP D3D OBJECTS (release calls should happen in reverse they were created)
// ******************************
void ShutdownD3D() 
{
	if (g_pParticleRainbow) delete g_pParticleRainbow;
	if (g_pPlayerMonster) delete g_pPlayerMonster;
	if (g_pPlayer) delete g_pPlayer;
	if (g_pEnemyTrainer) delete g_pEnemyTrainer;

	for (int i = g_ParticlesActive.size(); i <= 0; i--)
	{
		if (g_ParticlesActive[i]) delete g_ParticlesActive[i];
	}

	for (int i = g_MovablesActive.size(); i <= 0; i--)
	{
		if (g_MovablesActive[i]) delete g_MovablesActive[i];
	}

	for (int i = g_CollectiblesActive.size(); i <= 0; i--)
	{
		if (g_CollectiblesActive[i]) delete g_CollectiblesActive[i];
	}

	for (int i = g_MonstersActive.size(); i <= 0; i--)
	{
		if (g_MonstersActive[i]) delete g_MonstersActive[i];
	}

	for (int i = g_StaticModels.size(); i <= 0; i--)
	{
		if (g_StaticModels[i]) delete g_StaticModels[i];
	}

	if (g_pBattleSystem) delete g_pBattleSystem;
	if (g_pMainLight) delete g_pMainLight;
	if (g_pSkybox) delete g_pSkybox;
	if (g_pCamera) delete g_pCamera;
	if (g_2DText) delete g_2DText;
	if (g_pInput) delete g_pInput;
	if (g_pSampler0) g_pSampler0->Release();
	if (g_pTexture0) g_pTexture0->Release();
	if (g_pConstantBuffer0) g_pConstantBuffer0->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pInputLayout) g_pInputLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pBackBufferRTView) g_pBackBufferRTView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pD3DDevice) g_pD3DDevice->Release();
}

HRESULT InitialiseGraphics() 
{
	HRESULT hr = S_OK;

	//Load and compile the pixel and vertex shaders - use vs_5_0 to target DX11 hardware only
	ID3DBlob* VS, * PS, * error;
	hr = D3DX11CompileFromFile("shaders.hlsl", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, &error, 0);

	if (error != 0)//Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr)) //Don't fail if error is just a warning
		{
			return hr;
		}
	}

	hr = D3DX11CompileFromFile("shaders.hlsl", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, &error, 0);

	if (error != 0) //Check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr)) //Don't fail if error is just a warning
		{
			return hr;
		}
	}

	//Create shader objects
	hr = g_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &g_pVertexShader);

	if (FAILED(hr))
	{
		return hr;
	}

	hr = g_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &g_pPixelShader);

	if (FAILED(hr))
	{
		return hr;
	}

	//Set the shader objects as active
	g_pImmediateContext->VSSetShader(g_pVertexShader, 0, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, 0, 0);

	//Create and set the input layout object
	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		//Be very careful setting the correct dxgi format and D3D version
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		//NOTE the spelling of COLOR. Again, be careful setting the correct dxgi format (using A32) and correct D3D version
		{"COLOR", 0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = g_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &g_pInputLayout);

	if (FAILED(hr))
	{
		return hr;
	}

	// ******************************
	//		CREATING SCENE/GAME OBJECTS
	// ******************************
	g_pCamera = new Camera(15.0f, 3.0f, 8.0f, 0.0f);

	g_pMainLight = new DirectionalLight(0.0f, 1.0f, -0.5f, 1.0f, 1.0f, 1.0f);

	g_pPlayer = new Player(g_pD3DDevice, g_pImmediateContext);
	g_pPlayer->LoadObjModel((char*)"assets/sphere.obj");
	g_pPlayer->CalculateCollisionData();
	g_pPlayer->SetScale(0.7f, 0.7f, 0.7f);

	g_pParticleRainbow = new ParticleGenerator(g_pD3DDevice, g_pImmediateContext);
	g_pParticleRainbow->SetGeneratorActive(false);
	g_pParticleRainbow->SetPosition(0.0f, 2.0f, 3.0f);
	g_ParticlesActive.push_back(g_pParticleRainbow);

	g_pBattleSystem = new BattleSystem(g_pCamera, g_pPlayer);

	g_pModelSphereReflective = new Model(g_pD3DDevice, g_pImmediateContext);
	g_pModelSphereReflective->LoadObjModel((char*)"assets/sphere.obj");
	g_pModelSphereReflective->UseShader((char*)"reflect_shader.hlsl", (char*)"ModelVS", (char*)"ModelPS");
	g_pModelSphereReflective->AddTexture((char*)"assets/skybox01.dds");
	g_pModelSphereReflective->SetPos(15.0f, 15.0f, 15.0f);
	g_StaticModels.push_back(g_pModelSphereReflective);

	g_pModelPlane = new Model(g_pD3DDevice, g_pImmediateContext);
	g_pModelPlane->LoadObjModel((char*)"assets/Plane.obj");
	g_pModelPlane->UseShader((char*)"plane_shader.hlsl", (char*)"ModelVS", (char*)"ModelPS");
	g_pModelPlane->AddTexture((char*)"assets/Grass.jpg");
	g_pModelPlane->SetPos(55.0f, 0.0f, 55.0f);
	g_pModelPlane->SetScale(1.25f, 1.0f, 1.25f);
	g_StaticModels.push_back(g_pModelPlane);

	// Small map generator
	vector<vector<int>> levelLayout =
	{
		{1,1,1,1,1,1,1,1,1,1},
		{1,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,2},
		{1,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,1},
		{1,1,1,1,1,1,1,1,1,1}
	};

	for (int i = 0; i < levelLayout.size(); i++) 
	{
		for (int j = 0; j < levelLayout[i].size(); j++) 
		{
			switch (levelLayout[i][j])
			{
				case 1:
				{
					Model* wall = new Model(g_pD3DDevice, g_pImmediateContext);
					wall->LoadObjModel((char*)"assets/Cube.obj");
					wall->AddTexture((char*)"assets/RockTexture.jpg");
					// NOT WORKING
					//wall->CalculateCollisionData();
					wall->SetScale(6.0f, 6.0f, 6.0f);
					wall->IncPos(i * 12, 1.0f, j * 12);
					g_StaticModels.push_back(wall);
					break;
				}
					
				case 2:
				{
					Model* waterFallWall = new Model(g_pD3DDevice, g_pImmediateContext);
					waterFallWall->LoadObjModel((char*)"assets/Cube.obj");
					waterFallWall->AddTexture((char*)"assets/RockTexture.jpg");
					waterFallWall->CalculateCollisionData();
					waterFallWall->SetScale(6.0f, 10.0f, 6.0f);
					waterFallWall->IncPos(i * 12, 5.0f, j * 12);
					g_StaticModels.push_back(waterFallWall);

					ParticleGenerator* waterfall = new ParticleGenerator(g_pD3DDevice, g_pImmediateContext);
					waterfall->AddParticles(480);
					waterfall->SetGeneratorActive(false);
					waterfall->SetPosition(waterFallWall->GetPosX(), waterFallWall->GetPosY() + 9.5f, waterFallWall->GetPosZ() - 6.1f);
					waterfall->SetParticleEffect(ParticleGenerator::Effects::WaterFall);
					waterfall->SetGeneratorActive(true);
					g_ParticlesActive.push_back(waterfall);
					break;
				}
			}
		}
	}

	for (int i = 0; i < g_treeCount; i++)
	{
		Model* tree = new Model(g_pD3DDevice, g_pImmediateContext);
		tree->LoadObjModel((char*)"assets/Tree.obj");
		tree->AddTexture((char*)"assets/TreeTexture.jpg");
		tree->CalculateCollisionData();
		tree->SetScale(0.35f, 0.35f, 0.35f);
		tree->SetPos((int)Utilities::RandValue(10.0f, 90.0f), -0.5f, (int)Utilities::RandValue(10.0f, 90.0f));
		g_StaticModels.push_back(tree);
	}

	for (UINT i = 0; i < g_monsterCount; i++)
	{
		Monster* monster = new Monster(g_pD3DDevice, g_pImmediateContext, g_pBattleSystem);
		monster->LoadObjModel((char*)"assets/pointysphere.obj");
		monster->AddTexture((char*)"assets/MonsterTexture.jpg");
		monster->CalculateCollisionData();
		monster->SetPos(Utilities::RandValue(20.0f, 90.0f), 1.35f, Utilities::RandValue(10.0f, 90.0f));
		monster->SetScale(0.5f, 0.5f, 0.5f);
		monster->SetPlayer(g_pPlayer);
		g_MonstersActive.push_back(monster);
	}

	for (UINT i = 0; i < g_itemsCount; i++)
	{
		Collectible* healItem = new Collectible(g_pD3DDevice, g_pImmediateContext);
		healItem->LoadObjModel((char*)"assets/Sphere.obj");
		healItem->AddTexture((char*)"assets/CollectibleTexture.jpg");
		healItem->CalculateCollisionData();
		healItem->SetPos(Utilities::RandValue(20.0f, 90.0f), 1.35f, Utilities::RandValue(10.0f, 90.0f));
		g_CollectiblesActive.push_back(healItem);
	}

	for (UINT i = 0; i < g_movablesCount; i++) 
	{
		MovableObject* rock = new MovableObject(g_pD3DDevice, g_pImmediateContext);
		rock->LoadObjModel((char*)"assets/Cube.obj");
		rock->AddTexture((char*)"assets/RockTexture.jpg");
		rock->CalculateCollisionData();
		rock->SetPos(Utilities::RandValue(10.0f, 90.0f), 1.0f, Utilities::RandValue(10.0f, 90.0f));
		rock->SetRotY(Utilities::RandValue(0.0f, 360.f));
		g_MovablesActive.push_back(rock);
	}

	g_pPlayerMonster = new Monster(g_pD3DDevice, g_pImmediateContext, g_pBattleSystem);
	g_pPlayerMonster->SetStats(10.0f, 2.0f, 1.0f);
	g_pPlayerMonster->LoadObjModel((char*)"assets/pointysphere.obj");
	g_pPlayerMonster->AddTexture((char*)"assets/PlayerTexture.jpg");
	g_pPlayerMonster->SetScale(0.5f, 0.5f, 0.5f);
	g_pBattleSystem->SetPlayerActor(g_pPlayerMonster);
	
	g_pEnemyTrainerMonster = new Monster(g_pD3DDevice, g_pImmediateContext, g_pBattleSystem);
	g_pEnemyTrainerMonster->SetStats(15.0f, 1.5f, 0.5f);
	g_pEnemyTrainerMonster->LoadObjModel((char*)"assets/pointysphere.obj");
	g_pEnemyTrainerMonster->AddTexture((char*)"assets/PlayerTexture.jpg");
	g_pEnemyTrainerMonster->SetScale(0.5f, 0.5f, 0.5f);
	g_pEnemyTrainer = new NPCTrainer(g_pD3DDevice, g_pImmediateContext, g_pBattleSystem, g_pPlayer, g_pEnemyTrainerMonster);
	g_pEnemyTrainer->LoadObjModel((char*)"assets/Sphere.obj");
	g_pEnemyTrainer->AddTexture((char*)"assets/EnemyTrainerTexture.jpg");
	g_pEnemyTrainer->CalculateCollisionData();
	g_pEnemyTrainer->SetScale(0.8f, 0.8f, 0.8f);
	g_pEnemyTrainer->SetPos(Utilities::RandValue(60.0f, 90.0f), 1.8f, Utilities::RandValue(60.0f, 90.0f));

	g_pSkybox = new Skybox(g_pD3DDevice, g_pImmediateContext);
	g_pSkybox->AddTexture((char*)"assets/skybox01.dds");

	return S_OK;
}

// Method to draw the custom cube
void DrawShape(XMMATRIX worldMatrix, UINT size)
{
	cb0_values.WorldViewProjection = worldMatrix * g_view * g_projection;
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer0, 0, 0, &cb0_values, 0, 0);
	g_pImmediateContext->Draw(size, 0);
}

// ******************************
//		RENDER OUR FRAME // UPDATE LOOP
// ******************************
void RenderFrame(void)
{
	// Draw all light sources
	g_pMainLight->Draw();

	// ******************************
	//		READ ALL INPUTS
	// ******************************
	g_pInput->ReadInputStates();

	// Destroy window with escape
	if (g_pInput->IsKeyPressed(DIK_ESCAPE)) DestroyWindow(g_hWnd);

	// Normal movement if player is out of battle
	if (!g_pBattleSystem->IsActive())
	{
		g_lockMouseLook = false;
		if (g_pInput->IsKeyPressed(DIK_W)) g_pCamera->Forward(g_moveAmount);
		if (g_pInput->IsKeyPressed(DIK_S)) g_pCamera->Forward(-g_moveAmount);
		if (g_pInput->IsKeyPressed(DIK_A)) g_pCamera->Strafe(g_moveAmount);
		if (g_pInput->IsKeyPressed(DIK_D)) g_pCamera->Strafe(-g_moveAmount);
		//if (g_pInput->IsKeyPressed(DIK_SPACE)) g_pCamera->Up(g_jumpAmount);
		//if (g_pInput->IsKeyPressed(DIK_LCONTROL)) g_pCamera->Up(-g_jumpAmount);
		if (g_pInput->IsKeyPressed(DIK_1)) g_pMainLight->IncRot(0.01f, 0.0f, 0.0f);
		if (g_pInput->IsKeyPressed(DIK_2)) g_pMainLight->IncRot(-0.01f, 0.0f, 0.0f);
		if (g_pInput->IsKeyPressed(DIK_3)) g_pMainLight->SetColor(Utilities::RandValue(0.0f, 1.0f), Utilities::RandValue(0.0f, 1.0f), Utilities::RandValue(0.0f, 1.0f));
		if (g_pInput->IsKeyPressed(DIK_4)) g_pMainLight->SetColor(1.0f, 1.0f, 1.0f);
	}
	else
	{
		// Lock player camera
		g_lockMouseLook = true;

		// Battle camera rotation
		g_pCamera->RotateAroundTwoObjects(g_pPlayerMonster->GetPosition(), g_pBattleSystem->GetEnemyActor()->GetPosition(), 0.01f);

		//// If player is in battle draw his monster
		//g_pPlayerMonster->DrawIfAlive(&g_view, &g_projection);

		// If player is in battle
		if (g_pBattleSystem->IsPlayerTurn())
		{
			if (g_pInput->IsKeyPressed(DIK_1)) g_pBattleSystem->GetPlayerInput(1);
			if (g_pInput->IsKeyPressed(DIK_2)) g_pBattleSystem->GetPlayerInput(2);
			if (g_pInput->IsKeyPressed(DIK_3)) g_pBattleSystem->GetPlayerInput(3);
			if (g_pInput->IsKeyPressed(DIK_4)) g_pBattleSystem->GetPlayerInput(4);
		}
	}

	// TEST
	g_pPlayerMonster->DrawIfAlive(&g_view, &g_projection, g_pCamera->GetPosition());

	// ******************************
	//		SET WVP / PROJECTION / ZBUFFER
	// ******************************
	// Create wvp projection	
	g_world *= XMMatrixTranslation(g_worldMatrixX, g_worldMatrixY, g_worldMatrixZ);

	// Set FOV and Clipping planes here
	g_projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (float)g_resX / (float)g_resY, 1.0f, 200.0f);
	g_view = g_pCamera->GetViewMatrix();
	cb0_values.WorldViewProjection = g_world * g_view * g_projection;

	// Clear back buffer and choose color (ONLY IF SKYBOX IS NOT DRAWN)
	float rgba_clear_colour[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pBackBufferRTView, rgba_clear_colour);
	g_pImmediateContext->ClearDepthStencilView(g_pZBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Select which primitive type to use
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Need this for drawing text
	g_pImmediateContext->VSSetShader(g_pVertexShader, 0, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, 0, 0);
	g_pImmediateContext->IASetInputLayout(g_pInputLayout);

	// ******************************
	//		DRAW MODELS AND RUN STUFF
	// ******************************
	g_pBattleSystem->Run();

	g_pSkybox->Draw(g_view, g_projection, g_pCamera->GetPosX(), g_pCamera->GetPosY(), g_pCamera->GetPosZ());

	g_pEnemyTrainer->Draw(&g_view, &g_projection);
	g_pEnemyTrainer->CalculateLighting(XMLoadFloat4(&g_pMainLight->GetLightColor()), XMLoadFloat4(&g_pMainLight->GetAmbientLightColor()), XMLoadFloat4(&g_pMainLight->GetLightVector()));
	g_pEnemyTrainer->RunTrainerAI();

	// Set player position to camera position if out of battle
	if (!g_pBattleSystem->IsActive())
	{
		g_pPlayer->SetPos(g_pCamera->GetPosition());
	}

	// COLLISION TEST
	for (UINT i = 0; i < g_StaticModels.size(); i++)
	{
		if (g_pPlayer->CheckCollision(g_StaticModels[i])) 
		{
			g_pCamera->SetPosX(g_pCamera->GetPosition().x - g_pCamera->GetDX() * 0.5f);
			g_pCamera->SetPosZ(g_pCamera->GetPosition().z - g_pCamera->GetDZ() * 0.5f);
		}
	}

	for (UINT i = 0; i < g_MovablesActive.size(); i++)
	{
		if (g_MovablesActive[i]->CheckCollision(g_pPlayer))
		{
			g_MovablesActive[i]->Move(g_pCamera);
		}
	}

	// Draw all static models
	for (UINT i = 0; i < g_StaticModels.size(); i++)
	{
		g_StaticModels[i]->Draw(&g_view, &g_projection);
		g_StaticModels[i]->CalculateLighting(XMLoadFloat4(&g_pMainLight->GetLightColor()), XMLoadFloat4(&g_pMainLight->GetAmbientLightColor()), XMLoadFloat4(&g_pMainLight->GetLightVector()));
	}

	// Draw all NPC monsters
	for (UINT i = 0; i < g_MonstersActive.size(); i++)
	{
		g_MonstersActive[i]->DrawIfAlive(&g_view, &g_projection, g_pCamera->GetPosition());
		g_MonstersActive[i]->CalculateLighting(XMLoadFloat4(&g_pMainLight->GetLightColor()), XMLoadFloat4(&g_pMainLight->GetAmbientLightColor()), XMLoadFloat4(&g_pMainLight->GetLightVector()));
		g_MonstersActive[i]->RunAI();
	}

	// Draw all movable objects
	for (UINT i = 0; i < g_MovablesActive.size(); i++)
	{
		g_MovablesActive[i]->Draw(&g_view, &g_projection);
		g_MovablesActive[i]->CalculateLighting(XMLoadFloat4(&g_pMainLight->GetLightColor()), XMLoadFloat4(&g_pMainLight->GetAmbientLightColor()), XMLoadFloat4(&g_pMainLight->GetLightVector()));
	}

	// Draw all collectibles
	for (UINT i = 0; i < g_CollectiblesActive.size(); i++)
	{
		g_CollectiblesActive[i]->DrawIfNotPickedUp(&g_view, &g_projection);
		g_CollectiblesActive[i]->CalculateLighting(XMLoadFloat4(&g_pMainLight->GetLightColor()), XMLoadFloat4(&g_pMainLight->GetAmbientLightColor()), XMLoadFloat4(&g_pMainLight->GetLightVector()));

		// Pick up item by the player
		if (g_CollectiblesActive[i]->CheckCollision(g_pPlayer))
		{
			g_CollectiblesActive[i]->PickUp(g_pPlayer);
			g_pPlayer->AddItemToInv();
		}
	}

	// Draw all particles
	for (UINT i = 0; i < g_ParticlesActive.size(); i++)
	{
		g_ParticlesActive[i]->Draw(&g_view, &g_projection, g_pCamera->GetPosition());
	}

	// ******************************
	//		DRAW UI TEXT
	// ******************************
	g_2DText->AddText("HP " + std::to_string((int)g_pPlayerMonster->GetHealth()), -1.0f, 0.8f, 0.07f);
	g_2DText->AddText("Items " + std::to_string(g_pPlayer->GetItemsInInvSize()), -1.0f, 0.7f, 0.07f);

	if (g_pBattleSystem->IsActive())
	{
		g_2DText->AddText("BATTLE", -0.3f, 1.0f, 0.09f);
		g_2DText->AddText("EnemyHP " + std::to_string((int)g_pBattleSystem->GetEnemyActor()->GetHealth()), 0.3f, 0.8f, 0.07f);

		if (g_pBattleSystem->IsPlayerTurn())
		{
			g_2DText->AddText("Player", -0.25f, 0.9f, 0.07f);
			g_2DText->AddText("1 Attack", 0.35f, -0.65f, 0.07f);
			g_2DText->AddText("2 Heal  ", 0.35f, -0.75f, 0.07f);
			g_2DText->AddText("3 Flee  ", 0.35f, -0.85f, 0.07f);
		}
		else
		{
			g_2DText->AddText("Enemy", -0.22f, 0.9f, 0.07f);
		}
	}

	if (g_pEnemyTrainer->GetChaseState())
	{
		g_2DText->AddText("Hey you lets battle", -0.9f, -0.9f, 0.08f);
	}
	
	g_2DText->RenderText();

	// Display what just has been rendered
	g_pSwapChain->Present(0, 0);
}