#include "NPCTrainer.h"

class Monster;
NPCTrainer::NPCTrainer(ID3D11Device* device, ID3D11DeviceContext* context, BattleSystem* system, Player* player, Monster* trainerMonster) : Player(device, context)
{
	m_pBattleSystem = system;
	m_pPlayer = player;
	m_pEnemyTrainerMonster = trainerMonster;
	m_state = TrainerAIState::Scouting;

	m_startPos = GetPosition();
	m_isChasingPlayer = false;
	m_chaseDistance = 15.0f;
	m_chaseSpeed = 0.01f;
	SetPos(Utilities::RandValue(60.0f, 90.0f), 1.8f, Utilities::RandValue(60.0f, 90.0f));
}

NPCTrainer::~NPCTrainer(){}

Monster* NPCTrainer::GetMonster()
{
	return m_pEnemyTrainerMonster;
}

bool NPCTrainer::GetChaseState()
{
	return m_isChasingPlayer;
}

void NPCTrainer::ScoutForPlayer()
{
	m_isChasingPlayer = false;

	currentPos = GetPosition();

	//if (currentPos.x != m_startPos.x || currentPos.z != m_startPos.z) 
	//{
	//	while (moveProgress <= 1.0f) 
	//	{
	//		moveProgress += m_chaseSpeed * 0.5f;
	//		// NOT WORKING
	//		SetPosX(Utilities::LerpValue(currentPos.x, m_startPos.x, moveProgress));
	//		SetPosZ(Utilities::LerpValue(currentPos.z, m_startPos.z, moveProgress));
	//	}
	//}
	//else 
	//{
	//	moveProgress = 0.0f;
	//}

	// Player in sight
	if (Utilities::CheckDistance(GetPosition(), m_pPlayer->GetPosition(), m_chaseDistance))
	{
		SetState(TrainerAIState::Chasing);
	}
}

void NPCTrainer::Chase(XMFLOAT3 target)
{
	m_isChasingPlayer = true;

	// Player gets out of sight
	if (!Utilities::CheckDistance(GetPosition(), m_pPlayer->GetPosition(), m_chaseDistance))
	{
		SetState(TrainerAIState::Scouting);
	}

	// Collide with player and start battle
	if (CheckCollision(m_pPlayer))
	{
		SetState(TrainerAIState::InitBattle);
	}

	LookAt_XZ(target.x, target.z);
	MoveForward(m_chaseSpeed);

	// Freeze NPC if not in battle but battle system active
	if (m_pBattleSystem->IsActive()) SetState(TrainerAIState::Freeze);
}

void NPCTrainer::InitBattlePhase()
{
	m_isChasingPlayer = false;

	//if (m_pEnemyTrainerMonster->IsAlive()) SetState();

	// Don't repeat code if Battle System is already running
	if (m_pBattleSystem->IsActive()) return;

	m_pBattleSystem->SetEnemyActor(m_pEnemyTrainerMonster);
	m_pBattleSystem->StartStateMachine();
}

void NPCTrainer::WaitForBattleEnd() 
{
	// If battle system is not active go back scouting
	if (!m_pBattleSystem->IsActive()) SetState(TrainerAIState::Scouting);
}

void NPCTrainer::SetState(TrainerAIState newState)
{
	m_state = newState;
}

void NPCTrainer::RunTrainerAI()
{
	switch (m_state)
	{
		case TrainerAIState::Scouting:
			ScoutForPlayer();
			break;

		case TrainerAIState::Chasing:
			Chase(m_pPlayer->GetPosition());
			break;

		case TrainerAIState::InitBattle:
			InitBattlePhase();
			break;

		case TrainerAIState::Freeze:
			WaitForBattleEnd();
			break;

		case TrainerAIState::Dead:
			break;
	}
}
