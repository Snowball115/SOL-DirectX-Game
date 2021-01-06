#include "MovableObject.h"

MovableObject::MovableObject(ID3D11Device* device, ID3D11DeviceContext* context) : Model(device, context){}

MovableObject::~MovableObject(){}

// Move object by another actor
void MovableObject::Move(Camera* pusher)
{
	m_x += pusher->GetDX();
	m_z += pusher->GetDZ();
}