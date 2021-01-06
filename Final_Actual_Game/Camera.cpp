#include "camera.h"

Camera::Camera(float x, float y, float z, float camera_rot) 
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_camera_rotation = camera_rot;
	m_camera_pitch = 0;

	m_dx = (float)sin(camera_rot * (XM_PI / 180.0));
	m_dz = (float)cos(camera_rot * (XM_PI / 180.0));
}

Camera::~Camera() {}

void Camera::Rotate(float dx, float dy)
{
	m_camera_rotation += dx;
	m_camera_pitch += dy;

	//m_camera_pitch = std::clamp(m_camera_pitch + m_dy, 0.995f * -XM_PI / 2.0f, 0.995f * XM_PI / 2.0f));
	if (m_camera_pitch < -89.0f || m_camera_pitch > 89.0f) m_camera_pitch -= dy;
}

// Rotate camera around two objects
void Camera::RotateAroundTwoObjects(XMFLOAT3 objA, XMFLOAT3 objB, float speed)
{
	XMFLOAT3 newPos = Utilities::LerpVectors(objA, objB, 0.5f);
	LookAt_XZ(newPos.x, newPos.z);
	Strafe(speed);
}

// Move camera forward/backwards
void Camera::Forward(float distance) 
{
	m_x += m_dx * distance;
	m_z += m_dz * distance;
}

// Strafe camera left/right
void Camera::Strafe(float distance) 
{
	XMVECTOR forward;	// Forward vector
	XMVECTOR direction;	// Right vector

	forward = XMVector3Normalize(m_lookat - m_position);

	// Cross product from forward and up vector here
	direction = XMVector3Cross(forward, m_up);

	m_x += direction.x * distance;
	m_z += direction.z * distance;
}

// Move camera up/down
void Camera::Up(float distance)
{
	m_y += distance;
}

XMMATRIX Camera::GetViewMatrix() 
{
	m_dx = (float)sin(m_camera_rotation * (XM_PI / 180.0));
	m_dz = (float)cos(m_camera_rotation * (XM_PI / 180.0));
	m_dy = (float)tan(m_camera_pitch * (XM_PI / 180.0));

	m_position = XMVectorSet(m_x, m_y, m_z, 0.0f);
	m_lookat = XMVectorSet(m_x + m_dx, m_y + m_dy, m_z + m_dz, 0.0f);
	m_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	return XMMatrixLookAtLH(m_position, m_lookat, m_up);
}

void Camera::LookAt_XZ(float objX, float objZ)
{
	m_dx = objX - m_x;
	m_dz = objZ - m_z;
	m_camera_rotation = XMConvertToDegrees((float)atan2(m_dx, m_dz));
}

void Camera::SetPitch(float y)
{
	m_camera_pitch = y;
}

float Camera::GetDX()
{
	return m_dx;
}

float Camera::GetDZ()
{
	return m_dz;
}

void Camera::SetPosition(XMFLOAT3 newPos)
{
	m_x = newPos.x;
	m_y = newPos.y;
	m_z = newPos.z;
}

XMFLOAT3 Camera::GetPosition() 
{
	return XMFLOAT3(m_x, m_y, m_z);
}

void Camera::SetPosX(float x) { m_x = x; }
void Camera::SetPosY(float y) { m_y = y; }
void Camera::SetPosZ(float z) { m_z = z; }
float Camera::GetPosX() { return m_x; }
float Camera::GetPosY() { return m_y; }
float Camera::GetPosZ() { return m_z; }
