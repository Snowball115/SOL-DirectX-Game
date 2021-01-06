#include "Player.h"
#include "Monster.h"

Player::Player(ID3D11Device* device, ID3D11DeviceContext* context) : Model(device, context)
{
	m_inventorySize = 3;
	m_itemsInInventory = 0;
}

Player::~Player(){}

int Player::GetInvSize()
{
	return m_inventorySize;
}

int Player::GetItemsInInvSize()
{
	return m_itemsInInventory;
}

bool Player::CheckIfInventoryFull()
{
	if (m_itemsInInventory >= m_inventorySize) return true;

	return false;
}

void Player::AddItemToInv()
{
	// Check if inventory is full
	if (CheckIfInventoryFull()) return;

	m_itemsInInventory++;
}

void Player::UseItemOnMonster(Monster* monster)
{
	// Check if player has item in inventory
	if (m_itemsInInventory <= 0) return;

	monster->Heal(5);

	m_itemsInInventory--;
}

