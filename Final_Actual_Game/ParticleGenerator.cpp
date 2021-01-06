#include "ParticleGenerator.h"
#include <string>

ParticleGenerator::ParticleGenerator(ID3D11Device* device, ID3D11DeviceContext* context)
{
	m_pD3DDevice = device;
	m_pImmediateContext = context;

	// Default effect if no other effect is used
	m_particleEffect = Effects::RainbowFountain;
	
	m_timePrevious = float(timeGetTime());
	m_untilParticle = 2.0f;

	AddParticles(100);
	InitParticleGenerator();
}

ParticleGenerator::~ParticleGenerator()
{
	m_active.clear();
	m_free.clear();

	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pPShader) m_pPShader->Release();
	if (m_pVShader) m_pVShader->Release();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
	if (m_pRasterParticle) m_pRasterParticle->Release();
	if (m_pRasterSolid) m_pRasterSolid->Release();
	if (m_pImmediateContext) m_pImmediateContext->Release();
	if (m_pD3DDevice) m_pD3DDevice->Release();
}

// Return random value from 0 to 1
float ParticleGenerator::RandomZeroToOne()
{
	return Utilities::RandValue(0.0f, 1.0f);
}

// Return random value from -1 to 1
float ParticleGenerator::RandomNegOneToPosOne()
{
	return Utilities::RandValue(-1.0f, 1.0f);
}

// Adds particles to our list
void ParticleGenerator::AddParticles(int count) 
{
	m_active.clear();
	m_free.clear();

	for (int i = 0; i <= count; i++)
	{
		m_free.push_back(new Particle);
	}
}

// Turn generator on and off (for showing particles)
void ParticleGenerator::SetGeneratorActive(bool b)
{
	m_isActive = b;
}

// Choose a particle effect the particles should use
void ParticleGenerator::SetParticleEffect(Effects effect)
{
	m_particleEffect = effect;
}

void ParticleGenerator::LookAt_XZ(float objectX, float objectZ)
{
	m_dx = objectX - m_x;
	m_dz = objectZ - m_z;
	m_yAngle = XMConvertToDegrees(atan2(m_dx, m_dz));
}

// Initialise the particle engines DirectX components
int ParticleGenerator::InitParticleGenerator()
{
	HRESULT hr = S_OK;

	XMFLOAT3 vertices[6] =
	{
		XMFLOAT3(-1.0f, -1.0f, 0.0f),
		XMFLOAT3(1.0f, 1.0f, 0.0f),
		XMFLOAT3(-1.0f, 1.0f, 0.0f),
		XMFLOAT3(-1.0f, -1.0f, 0.0f),
		XMFLOAT3(1.0f, -1.0f, 0.0f),
		XMFLOAT3(1.0f, 1.0f, 0.0f)
	};

	D3D11_RASTERIZER_DESC rasterizer_desc;
	ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));
	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	hr = m_pD3DDevice->CreateRasterizerState(&rasterizer_desc, &m_pRasterSolid);
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	hr = m_pD3DDevice->CreateRasterizerState(&rasterizer_desc, &m_pRasterParticle);

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(XMFLOAT3) * 6;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = m_pD3DDevice->CreateBuffer(&bufferDesc, NULL, &m_pVertexBuffer);
	if (FAILED(hr)) return 0;

	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));
	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;					// Can use UpdateSubresource() to update
	constant_buffer_desc.ByteWidth = 80;								// MUST be a multiple of 16, calculate from CB struct
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		// Use a constant buffer
	m_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &m_pConstantBuffer);

	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &particle_cb_values, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	D3D11_MAPPED_SUBRESOURCE ms;

	m_pImmediateContext->Map(m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, vertices, sizeof(vertices));
	m_pImmediateContext->Unmap(m_pVertexBuffer, NULL);

	//Load and compile the pixel and vertex shaders - use vs_5_0 to target DX11 hardware only
	ID3DBlob* VS, * PS, * error;
	D3DX11CompileFromFile("particle_shader.hlsl", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, &error, 0);
	D3DX11CompileFromFile("particle_shader.hlsl", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, &error, 0);

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
	};

	m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &m_pInputLayout);

	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	return 0;
}

void ParticleGenerator::SetPosition(XMFLOAT3 newPos)
{
	m_x = newPos.x;
	m_y = newPos.y;
	m_z = newPos.z;
}

void ParticleGenerator::SetPosition(float x, float y, float z) 
{
	m_x = x;
	m_y = y;
	m_z = z;
}

void ParticleGenerator::IncPosition(float x, float y, float z)
{
	m_x += x;
	m_y += z;
	m_z += x;
}

// Load a given particle effect for each particle
void ParticleGenerator::InitialiseParticleEffect()
{
	m_untilParticle = 0.0f;

	if (m_untilParticle <= 0.0f)
	{
		// Check if particle engine is on or off
		if (m_isActive)
		{
			// Point to beginning to the free list
			it = m_free.begin();

			// Safety check
			if (m_free.size() != NULL)
			{
				(*it)->position = XMFLOAT3(m_x, m_y, m_z);

				switch (m_particleEffect)
				{
					case Effects::RainbowFountain:
						m_age = 2.0f;
						m_untilParticle = 0.008f;
						(*it)->color = XMFLOAT4(RandomZeroToOne(), RandomZeroToOne(), RandomZeroToOne(), 1.0f);
						(*it)->gravity = 4.5f;
						(*it)->velocity = XMFLOAT3(RandomNegOneToPosOne(), 3.0f, RandomNegOneToPosOne());
						break;

					case Effects::HitImpact:
						m_age = 0.5f;
						m_untilParticle = 0.008f;
						(*it)->color = XMFLOAT4(0.9f, 0.1f, 0.0f, 1.0f);
						(*it)->gravity = 0.0f;
						(*it)->velocity = XMFLOAT3(RandomNegOneToPosOne() * 10, RandomNegOneToPosOne() * 10, RandomNegOneToPosOne() * 10);
						break;

					case Effects::WaterFall:
						m_age = 1.8f;
						m_untilParticle = 0.008f;
						(*it)->position = XMFLOAT3(m_x + Utilities::RandValue(-5.0f, 5.0f), m_y, m_z);
						(*it)->color = XMFLOAT4(Utilities::RandValue(0.05f, 0.2f), Utilities::RandValue(0.45f, 0.6f), Utilities::RandValue(0.72f, 0.85f), 1.0f);
						(*it)->gravity = 2.0f;
						(*it)->velocity = XMFLOAT3(RandomNegOneToPosOne(), RandomZeroToOne() * -10, RandomNegOneToPosOne());

					default:
						break;
				}

				// Set age to 0, used for knowing when to delete particle
				(*it)->age = 0.0f;

				m_active.push_back(*it);
				m_free.pop_front();
			}
		}

		else m_untilParticle = 0.001f;
	}
}

void ParticleGenerator::Draw(XMMATRIX* view, XMMATRIX* projection, XMFLOAT3 camPos)
{
	float timeNow = float(timeGetTime()) / 1000.0f;
	float deltaTime = timeNow - m_timePrevious;
	m_timePrevious = timeNow;
	m_untilParticle -= deltaTime;

	InitialiseParticleEffect();

	if (m_active.size() != NULL) 
	{
		it = m_active.begin();

		// Move all of the particles
		while (it != m_active.end()) 
		{
			(*it)->age += deltaTime;
			(*it)->velocity.y -= (*it)->gravity * (deltaTime);
			(*it)->position.x += (*it)->velocity.x * (deltaTime);
			(*it)->position.y += (*it)->velocity.y * (deltaTime);
			(*it)->position.z += (*it)->velocity.z * (deltaTime);
			
			world = XMMatrixIdentity();
			LookAt_XZ(camPos.x, camPos.z);

			switch (m_particleEffect)
			{
				case Effects::RainbowFountain:
					world = XMMatrixScaling(0.2f, 0.2f, 0.2f);
					world *= XMMatrixRotationY(XMConvertToRadians(m_yAngle));
					break;

				case Effects::HitImpact:
					world = XMMatrixScaling(0.1f, 0.1f, 0.1f);
					world *= XMMatrixRotationY(XMConvertToRadians(m_yAngle));

				case Effects::WaterFall:
					world = XMMatrixScaling(0.3f, 0.8f, 0.3f);
					world *= XMMatrixRotationY(XMConvertToRadians(m_yAngle));

				default:
					break;
			}

			world *= XMMatrixTranslation((*it)->position.x, (*it)->position.y, (*it)->position.z);

			particle_cb_values.WorldViewProjection = world * (*view) * (*projection);
			particle_cb_values.Color = (*it)->color;

			// Update constant buffer
			m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &particle_cb_values, 0, 0);
			m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

			UINT stride = sizeof(XMFLOAT3);
			UINT offset = 0;
			m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

			// Set shader active
			m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
			m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);
			m_pImmediateContext->IASetInputLayout(m_pInputLayout);

			// dont know if needing this
			m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			m_pImmediateContext->RSSetState(m_pRasterParticle);
			m_pImmediateContext->Draw(6, 0);
			m_pImmediateContext->RSSetState(m_pRasterSolid);

			// Check age of current particle
			if ((*it)->age >= m_age)
			{
				it++;

				m_active.front()->age = m_age;

				m_active.front()->position =
				{
					(RandomNegOneToPosOne() + (*it)->position.x * 10) * (RandomZeroToOne() * 10),
					(*it)->position.y + 5.0f,
					camPos.z + 7.0f
				};

				m_active.front()->velocity =
				{
					0.0f,
					4.5f,
					RandomNegOneToPosOne()
				};

				// Move (now previously) current particle to back of pool
				m_free.push_back(m_active.front());
				// Remove particle
				m_active.pop_front();
			}

			else it++;
		}
	}
}