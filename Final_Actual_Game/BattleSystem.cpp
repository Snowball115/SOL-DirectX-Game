#include "BattleSystem.h"
#include "Monster.h"

BattleSystem::BattleSystem(Camera* camera, Player* player)
{
	m_pPlayerCamera = camera;
	m_pPlayer = player;
	m_isActive = false;
	m_isPlayerTurn = false;
	m_inputTimer = 0;
	m_playerAction = 0;
}

BattleSystem::~BattleSystem(){}

bool BattleSystem::IsActive() { return m_isActive; }
bool BattleSystem::IsPlayerTurn() { return m_isPlayerTurn; }

void BattleSystem::SetPlayerActor(Monster* player)
{
	m_pPlayerMonster = player;
}

void BattleSystem::SetEnemyActor(Monster* enemy) 
{
	m_pEnemyMonster = enemy;
}

Monster* BattleSystem::GetEnemyActor() 
{
	return m_pEnemyMonster;
}

// ******************************
//		BATTLE STATE METHODS
// ******************************
void BattleSystem::StartBattle()
{
	XMFLOAT3 currentCamPos = m_pPlayerCamera->GetPosition();
	XMFLOAT3 currentPlayerPos = m_pPlayerMonster->GetPosition();
	XMFLOAT3 currentEnemyPos = m_pEnemyMonster->GetPosition();

	// WIP Place monsters in correct battle position
	currentCamPos.y -= currentCamPos.y * 0.5f;
	m_playerPos = currentCamPos;
	m_enemyPos = currentCamPos;
	m_enemyPos.x += 10.0f;

	// WIP Place camera in battle view
	m_pPlayerCamera->SetPitch(0.0f);
	//m_pPlayerCamera->SetPosX(currentCamPos.x + 10.0f);
	m_pPlayerCamera->SetPosZ(currentCamPos.z - 10.0f);
	XMFLOAT3 newCamLookAt = Utilities::LerpVectors(currentPlayerPos, currentEnemyPos, 0.5f);
	m_pPlayerCamera->LookAt_XZ(newCamLookAt.x, newCamLookAt.z);

	// Set Positions of monsters and LookAt
	m_pPlayerMonster->SetPos(m_playerPos);
	m_pPlayerMonster->LookAt_XZ(m_pEnemyMonster->GetPosX(), m_pEnemyMonster->GetPosZ());
	m_pEnemyMonster->SetPos(m_enemyPos);
	m_pEnemyMonster->LookAt_XZ(m_pPlayerMonster->GetPosX(), m_pPlayerMonster->GetPosZ());

	// Reset player input
	m_playerAction = 0;

	// Randomly choose first turn
	float turnRatio = Utilities::RandValue(0.0f, 1.0f);
	if (turnRatio < 0.5f) SetState(BattleState::PlayerTurn);
	else SetState(BattleState::EnemyTurn);
}

void BattleSystem::EndBattle()
{
	// Reset player choice
	m_playerAction = 0;

	// Release pointer to enemy monster
	m_pEnemyMonster = NULL;

	// Stop the battle system
	StopStateMachine();
}

// Get player input to choose his action from
void BattleSystem::GetPlayerInput(int i)
{
	m_playerAction = i;
}

// Calculate players action by his input
void BattleSystem::CalcPlayerAction()
{
	m_isPlayerTurn = true;

	// End battle if one of the actors is dead
	if (!m_pPlayerMonster->IsAlive() || !m_pEnemyMonster->IsAlive()) 
	{
		SetState(BattleState::End);
		return;
	}

	switch (m_playerAction)
	{
		// Wait for input
		case 0:
			return;

		// Attack
		case 1:
			m_pPlayerMonster->Attack(m_pEnemyMonster);
			break;

		// Heal
		case 2:
			m_pPlayer->UseItemOnMonster(m_pPlayerMonster);
			break;

		// Flee
		case 3:
			if (m_pPlayerMonster->FleeFromBattle()) SetState(BattleState::End);
			return;
			break;

		default:
			break;
	}

	// Reset choice
	m_playerAction = 0;

	// Switch sides if both are alive
	if (m_pPlayerMonster->IsAlive() && m_pEnemyMonster->IsAlive())
	{
		SetState(BattleState::EnemyTurn);
	}
}

// Calculate enemy action by given conditions
void BattleSystem::CalcEnemyAction() 
{
	m_isPlayerTurn = false;

	// End battle if one of the actors is dead
	if (!m_pPlayerMonster->IsAlive() || !m_pEnemyMonster->IsAlive())
	{
		SetState(BattleState::End);
		return;
	}

	// Flee if difference between enemy health and player health is too big
	//if (abs(m_pEnemyMonster->GetHealth() - m_pPlayerMonster->GetHealth()) > 5)
	//{
	//	if (m_pEnemyMonster->FleeFromBattle()) SetState(BattleState::End);
	//	return;
	//}

	// Attack
	m_pEnemyMonster->Attack(m_pPlayerMonster);

	// Switch sides if both are alive
	if (m_pPlayerMonster->IsAlive() && m_pEnemyMonster->IsAlive())
	{
		SetState(BattleState::PlayerTurn);
	}
}

// Run battle system
void BattleSystem::StartStateMachine()
{
	m_isActive = true;
	SetState(BattleState::Start);
}

// Stop battle system
void BattleSystem::StopStateMachine()
{
	m_isActive = false;
}

// Switch state
void BattleSystem::SetState(BattleState newState)
{
	m_state = newState;
}

// Run state machine
void BattleSystem::Run()
{
	if (m_isActive)
	{
		// Wait some time before next action
		// Otherwise actions would run every frame which is way too fast
		m_inputTimer += 0.005f;
		if (m_inputTimer <= 1.0f) return;
		m_inputTimer = 0.0f;

		switch (m_state) 
		{
			case BattleState::Start:
				StartBattle();
				break;

			case BattleState::PlayerTurn:
				CalcPlayerAction();
				break;

			case BattleState::EnemyTurn:
				CalcEnemyAction();
				break;

			case BattleState::End:
				EndBattle();
				break;
		}
	}
}