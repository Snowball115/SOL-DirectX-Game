#pragma once
#include <d3d11.h>
#include <xnamath.h>

class DirectionalLight
{
private:
	XMVECTOR m_directional_light_shines_from;
	XMVECTOR m_directional_light_colour;
	XMVECTOR m_ambient_light_colour;

	XMFLOAT4 m_light_vector;
	XMFLOAT4 m_light_colour;
	XMFLOAT4 m_ambient_color;

	float m_directional_light_xPos;
	float m_directional_light_yPos;
	float m_directional_light_zPos;
	float m_directional_light_rCol;
	float m_directional_light_gCol;
	float m_directional_light_bCol;

public:
	DirectionalLight(float xPos, float yPos, float zPos, float r, float g, float b);
	~DirectionalLight();

	void SetAmbientLight(float r, float g, float b);
	XMFLOAT4 GetLightVector();
	XMFLOAT4 GetLightColor();
	XMFLOAT4 GetAmbientLightColor();

	void IncRot(float x, float y, float z);
	void SetRot(float x, float y, float z);
	void IncColor(float r, float g, float b);
	void SetColor(float r, float g, float b);
	void Draw();
};

