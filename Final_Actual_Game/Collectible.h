#pragma once

#include "Model.h"
#include "Player.h"

class Collectible : public Model
{
private:

	float m_rotateSpeed;
	bool m_isPickedUp;

public:

	Collectible(ID3D11Device* device, ID3D11DeviceContext* context);
	~Collectible();

	void PickUp(Player* player);
	void DrawIfNotPickedUp(XMMATRIX* view, XMMATRIX* projection);
};