#pragma once

#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT

#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <list>
#include "Utilities.h"

class ParticleGenerator
{
private:

	struct Particle 
	{
		float age;
		float gravity;
		XMFLOAT3 position;
		XMFLOAT3 velocity;
		XMFLOAT4 color;
	};

	struct PARTICLE_CONSTANT_BUFFER 
	{
		XMMATRIX WorldViewProjection;
		XMFLOAT4 Color;
	};

	PARTICLE_CONSTANT_BUFFER particle_cb_values;

	ID3D11Device* m_pD3DDevice;
	ID3D11DeviceContext* m_pImmediateContext;

	ID3D11RasterizerState* m_pRasterSolid = 0;
	ID3D11RasterizerState* m_pRasterParticle = 0;

	ID3D11Buffer* m_pConstantBuffer;
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11VertexShader* m_pVShader;
	ID3D11PixelShader* m_pPShader;
	ID3D11InputLayout* m_pInputLayout;

	XMMATRIX world;
	float m_x, m_y, m_z;
	float m_dx, m_dy, m_dz;
	float m_yAngle;

	bool m_isActive;
	float m_age;
	float m_timePrevious;
	float m_untilParticle;
	float RandomZeroToOne();
	float RandomNegOneToPosOne();
	int InitParticleGenerator();
	void LookAt_XZ(float objectX, float objectZ);

	std::list<Particle*> m_free;
	std::list<Particle*> m_active;

	// iteration list for pointing to the correct particle in the list
	std::list<Particle*>::iterator it; 

public:

	enum class Effects
	{
		RainbowFountain,
		HitImpact,
		WaterFall
	};

	Effects m_particleEffect;

	ParticleGenerator(ID3D11Device* device, ID3D11DeviceContext* context);
	~ParticleGenerator();

	void AddParticles(int count);
	void SetPosition(XMFLOAT3 newPos);
	void SetPosition(float x, float y, float z);
	void IncPosition(float x, float y, float z);
	void InitialiseParticleEffect();
	void SetGeneratorActive(bool b);
	void SetParticleEffect(Effects effect);
	void Draw(XMMATRIX* view, XMMATRIX* projection, XMFLOAT3 camPos);
};

