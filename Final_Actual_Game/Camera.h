#pragma once

#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT

#include <d3dx11.h>
#include <xnamath.h>
#include <math.h>
#include "Utilities.h"

class Camera
{
private:
	float m_x, m_y, m_z;
	float m_dx, m_dy, m_dz;
	float m_camera_rotation;
	float m_camera_pitch;
	XMVECTOR m_position;
	XMVECTOR m_lookat;
	XMVECTOR m_up;

public:
	Camera(float x, float y, float z, float camera_rot);
	~Camera();

	void Rotate(float dx, float dy);
	void RotateAroundTwoObjects(XMFLOAT3 objA, XMFLOAT3 objB, float speed);
	void Forward(float distance);
	void Strafe(float distance);
	void Up(float distance);
	XMMATRIX GetViewMatrix();

	void LookAt_XZ(float objX, float objZ);
	void SetPitch(float y);
	float GetDX();
	float GetDZ();
	void SetPosition(XMFLOAT3 newPos);
	XMFLOAT3 GetPosition();
	void SetPosX(float x);
	void SetPosY(float y);
	void SetPosZ(float z);
	float GetPosX();
	float GetPosY();
	float GetPosZ();
};

