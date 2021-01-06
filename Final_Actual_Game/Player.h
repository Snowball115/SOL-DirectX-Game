#pragma once

#include "Model.h"

class Monster;

class Player : public Model
{
private:

	int m_inventorySize;
	int m_itemsInInventory;

public:
	Player(ID3D11Device* device, ID3D11DeviceContext* context);
	~Player();

	int GetInvSize();
	int GetItemsInInvSize();

	bool CheckIfInventoryFull();
	void AddItemToInv();
	void UseItemOnMonster(Monster* monster);
};