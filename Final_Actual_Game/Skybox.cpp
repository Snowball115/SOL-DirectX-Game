#include "Skybox.h"

Skybox::Skybox(ID3D11Device* device, ID3D11DeviceContext* context)
{
	m_pD3DDevice = device;
	m_pImmediateContext = context;

	InitSkybox();
}

Skybox::~Skybox()
{
	if (m_pTexture0) m_pTexture0->Release();
	if (m_pSampler0) m_pSampler0->Release();
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pVShader) m_pVShader->Release();
	if (m_pPShader) m_pPShader->Release();
	if (m_pRasterSolid) m_pRasterSolid->Release();
	if (m_pRasterSkyBox) m_pRasterSkyBox->Release();
	if (m_pDepthWriteSolid) m_pDepthWriteSolid->Release();
	if (m_pDepthWriteSkyBox) m_pDepthWriteSkyBox->Release();
}

// Initialise the skyboxes DirectX components
HRESULT Skybox::InitSkybox()
{
	D3D11_RASTERIZER_DESC rasterizer_desc;
	ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));

	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	m_pD3DDevice->CreateRasterizerState(&rasterizer_desc, &m_pRasterSolid);

	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_FRONT;
	m_pD3DDevice->CreateRasterizerState(&rasterizer_desc, &m_pRasterSkyBox);

	D3D11_DEPTH_STENCIL_DESC DSDecsc;
	ZeroMemory(&DSDecsc, sizeof(DSDecsc));
	DSDecsc.DepthEnable = true;
	DSDecsc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DSDecsc.DepthFunc = D3D11_COMPARISON_LESS;
	m_pD3DDevice->CreateDepthStencilState(&DSDecsc, &m_pDepthWriteSolid);
	DSDecsc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_pD3DDevice->CreateDepthStencilState(&DSDecsc, &m_pDepthWriteSkyBox);

	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));
	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;					// Can use UpdateSubresource() to update
	constant_buffer_desc.ByteWidth = 80;								// MUST be a multiple of 16, calculate from CB struct
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		// Use a constant buffer
	m_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &m_pConstantBuffer);

	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &skybox_cb_values, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//Set up and create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;										//Allows use by CPU and GPU
	bufferDesc.ByteWidth = sizeof(POS_COL_TEX_NORM_VERTEX) * sizeof(verticesCube);		//Set the total size of the buffer (in this case, 3 vertices)
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;							//Set the type of buffer to vertex buffer
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;							//Allow access by the CPU
	m_pD3DDevice->CreateBuffer(&bufferDesc, NULL, &m_pVertexBuffer);		//Create the buffer

	//Lock the buffer to allow writing
	m_pImmediateContext->Map(m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);

	//Copy the data
	memcpy(ms.pData, verticesCube, sizeof(verticesCube));

	//Unlock the buffer
	m_pImmediateContext->Unmap(m_pVertexBuffer, NULL);

	//Load and compile the pixel and vertex shaders - use vs_5_0 to target DX11 hardware only
	ID3DBlob* VS, * PS, * error;
	D3DX11CompileFromFile("sky_shader.hlsl", 0, 0, "SkyVS", "vs_4_0", 0, 0, 0, &VS, &error, 0);
	D3DX11CompileFromFile("sky_shader.hlsl", 0, 0, "SkyPS", "ps_4_0", 0, 0, 0, &PS, &error, 0);

	//Create shader objects
	m_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_pVShader);
	m_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_pPShader);

	//Set the shader objects as active
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);

	//Create and set the input layout object
	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"COLOR", 0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &m_pInputLayout);

	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	return S_OK;
}

HRESULT Skybox::AddTexture(char* filename)
{
	// Load texture
	D3DX11CreateShaderResourceViewFromFile(m_pD3DDevice, filename, NULL, NULL, &m_pTexture0, NULL);

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	m_pD3DDevice->CreateSamplerState(&sampler_desc, &m_pSampler0);

	return S_OK;
}

void Skybox::Draw(XMMATRIX view, XMMATRIX projection, float camX, float camY, float camZ)
{
	world = XMMatrixScaling(skyboxScaling, skyboxScaling, skyboxScaling);
	world *= XMMatrixRotationX(XMConvertToRadians(0.0f));
	world *= XMMatrixRotationY(XMConvertToRadians(0.0f));
	world *= XMMatrixRotationZ(XMConvertToRadians(0.0f));
	world *= XMMatrixTranslation(camX, camY, camZ);

	skybox_cb_values.WorldViewProjection = world * view * projection;

	// Update constant buffer
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &skybox_cb_values, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	UINT stride = sizeof(POS_COL_TEX_NORM_VERTEX);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Stuff
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);
	m_pImmediateContext->IASetInputLayout(m_pInputLayout); // dont know if needing this

	// Use texture
	m_pImmediateContext->PSSetSamplers(0, 1, &m_pSampler0);
	m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTexture0);

	// Draw skybox
	m_pImmediateContext->OMSetDepthStencilState(m_pDepthWriteSkyBox, 0);
	m_pImmediateContext->RSSetState(m_pRasterSkyBox);
	m_pImmediateContext->Draw(sizeof(verticesCube) / sizeof(verticesCube[0]), 0);
	m_pImmediateContext->RSSetState(m_pRasterSolid);
	m_pImmediateContext->OMSetDepthStencilState(m_pDepthWriteSolid, 0);
}