#include "DirectionalLight.h"
#include "Utilities.h"

DirectionalLight::DirectionalLight(float xPos, float yPos, float zPos, float r, float g, float b)
{
	m_directional_light_xPos = xPos;
	m_directional_light_yPos = yPos;
	m_directional_light_zPos = zPos;
	m_directional_light_rCol = r;
	m_directional_light_gCol = g;
	m_directional_light_bCol = b;

	m_directional_light_shines_from = XMVectorSet(m_directional_light_xPos, m_directional_light_yPos, m_directional_light_zPos, 0.0f);
	m_directional_light_colour = XMVectorSet(m_directional_light_rCol, m_directional_light_gCol, m_directional_light_bCol, 0.0f);
	m_ambient_light_colour = XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f);
}

DirectionalLight::~DirectionalLight() {}

void DirectionalLight::SetAmbientLight(float r, float g, float b) 
{
	m_ambient_light_colour = XMVectorSet(r, g, b, 0.1f);
}

/*NOTE: Can not transfer XMVECTOR variable types
  First create float4 and then in other class convert back to XMVECTOR*/
XMFLOAT4 DirectionalLight::GetLightVector()
{
	//return m_directional_light_shines_from;
	XMStoreFloat4(&m_light_vector, m_directional_light_shines_from);
	return m_light_vector;
}

XMFLOAT4 DirectionalLight::GetLightColor()
{
	//return m_directional_light_colour;
	XMStoreFloat4(&m_light_colour, m_directional_light_colour);
	return m_light_colour;
}

XMFLOAT4 DirectionalLight::GetAmbientLightColor()
{
	//return m_ambient_light_colour;
	XMStoreFloat4(&m_ambient_color, m_ambient_light_colour);
	return m_ambient_color;
}

void DirectionalLight::IncRot(float x, float y, float z)
{
	m_directional_light_xPos += x;
	m_directional_light_yPos += y;
	m_directional_light_zPos += z;
}

void DirectionalLight::SetRot(float x, float y, float z)
{
	m_directional_light_xPos = x;
	m_directional_light_yPos = y;
	m_directional_light_zPos = z;
}

void DirectionalLight::IncColor(float r, float g, float b)
{
	m_directional_light_rCol += r;
	m_directional_light_gCol += g;
	m_directional_light_bCol += b;
}

void DirectionalLight::SetColor(float r, float g, float b)
{
	m_directional_light_rCol = r;
	m_directional_light_gCol = g;
	m_directional_light_bCol = b;
}

void DirectionalLight::Draw()
{
	m_directional_light_shines_from = XMVectorSet(m_directional_light_xPos, m_directional_light_yPos, m_directional_light_zPos, 0.0f);
	m_directional_light_colour = XMVectorSet(m_directional_light_rCol, m_directional_light_gCol, m_directional_light_bCol, 0.0f);
	m_ambient_light_colour = XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f);
}