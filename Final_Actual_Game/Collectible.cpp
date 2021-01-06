#include "Collectible.h"
#include "Player.h"

Collectible::Collectible(ID3D11Device* device, ID3D11DeviceContext* context) : Model(device, context)
{
	m_rotateSpeed = 0.2f;
	m_isPickedUp = false;

	SetScale(0.2f, 0.2f, 0.2f);
}

Collectible::~Collectible(){}

// Pick up object by player
void Collectible::PickUp(Player* player) 
{
	if (player->CheckIfInventoryFull()) return;
	m_isPickedUp = true;
	SetPosY(-20.0f);
}

// Draw collectible until it gets picked up
void Collectible::DrawIfNotPickedUp(XMMATRIX* view, XMMATRIX* projection)
{
	if (!m_isPickedUp)
	{
		m_yAngle += m_rotateSpeed;

		Draw(view, projection);
	}
}
