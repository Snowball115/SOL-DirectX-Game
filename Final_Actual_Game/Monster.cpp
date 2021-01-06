#include "Monster.h"

Monster::Monster(ID3D11Device* device, ID3D11DeviceContext* context, BattleSystem* battleSystem, vector<Model*> colliders) : Model(device, context)
{
	// Standard values
	m_pBattleSystem = battleSystem;
	m_state = AIState::Wander;
	m_isAlive = true;
	m_isInBattle = false;
	m_health = 10.0f;
	m_attack = 1.0f;
	m_defense = 0.0f;
	m_wanderSpeed = 0.005f;
	m_chaseDistance = 16.0f;
	m_chaseSpeed = 0.01f;
	m_fleeDistance = 25.0f;

	m_staticModels = colliders;

	// "Monster gets hit" effect
	m_pHitEffect = new ParticleGenerator(device, context);
	m_pHitEffect->AddParticles(10);
	m_pHitEffect->SetParticleEffect(ParticleGenerator::Effects::HitImpact);
	m_pHitEffect->SetGeneratorActive(true);
}

Monster::Monster(ID3D11Device* device, ID3D11DeviceContext* context, BattleSystem* battleSystem) : Model(device, context)
{
	// Standard values
	m_pBattleSystem = battleSystem;
	m_state = AIState::Wander;
	m_isAlive = true;
	m_isInBattle = false;
	m_health = 10.0f;
	m_attack = 1.0f;
	m_defense = 0.0f;
	m_wanderSpeed = 0.005f;
	m_chaseDistance = 16.0f;
	m_chaseSpeed = 0.01f;
	m_fleeDistance = 25.0f;

	// "Monster gets hit" effect
	m_pHitEffect = new ParticleGenerator(device, context);
	m_pHitEffect->AddParticles(10);
	m_pHitEffect->SetParticleEffect(ParticleGenerator::Effects::HitImpact);
	m_pHitEffect->SetGeneratorActive(true);
}

Monster::~Monster()
{
	if (m_pHitEffect) delete m_pHitEffect;
	if (m_pPlayer) delete m_pPlayer;
}

void Monster::SetStats(float health, float attack, float defense)
{
	m_health = health;
	m_attack = attack;
	m_defense = defense;
}

bool Monster::IsAlive() { return m_isAlive; }
bool Monster::IsInBattle() { return m_isInBattle; }
float Monster::GetHealth() { return m_health; }
float Monster::GetAttackValue() { return m_attack; }
float Monster::GetDefenseValue() { return m_defense; }

// Get Player in scene
void Monster::SetPlayer(Player* player)
{
	m_pPlayer = player;
}

// ******************************
//		BATTLE ACTIONS
// ******************************
void Monster::Attack(Monster* enemy)
{
	enemy->GetHurt(m_attack);
}

void Monster::GetHurt(float damage)
{
	particleTimer = 0;
	m_pHitEffect->SetPosition(GetPosition());

	m_health -= damage - (m_defense / 2);

	if (m_health <= 0) Die();
}

void Monster::Die()
{
	// Move NPC under the map so it cant interact with anything
	m_y = -20.0f;

	m_isAlive = false;

	SetState(AIState::Dead);
}

void Monster::Heal(float amount) 
{
	m_health += amount;
}

bool Monster::FleeFromBattle()
{
	if (Utilities::RandValue(0.0f, 1.0f) < 0.3f) 
	{
		Utilities::DebugLog("SUCCESFFULL");
		return true;
	}

	Utilities::DebugLog("NOT SUCCESSFUL");
	return false;
}

void Monster::DrawIfAlive(XMMATRIX* view, XMMATRIX* projection, XMFLOAT3 camPos)
{
	particleTimer += 0.1f;
	m_pHitEffect->Draw(view, projection, camPos);
	//if (particleTimer > 1.0f) m_pHitEffect->SetPosition(0.0f, -100.0f, 0.0f);

	if (m_isAlive) Draw(view, projection);
}

// ******************************
//		AI STATE METHODS
// ******************************
void Monster::Wander() 
{
	if (Utilities::CheckDistance(GetPosition(), m_pPlayer->GetPosition(), m_chaseDistance)) SetState(AIState::Chasing);

	moveTimer += 0.001f;

	if (moveTimer >= moveInterval)
	{
		xPos = Utilities::RandValue(-m_wanderSpeed, m_wanderSpeed);
		zPos = Utilities::RandValue(-m_wanderSpeed, m_wanderSpeed);
		moveTimer = 0.0f;

		/*for (UINT i = 0; m_staticModels.size(); i++) 
		{
			if (CheckCollision(m_staticModels[i]))
			{
				xPos -= xPos * 2.0f;
				zPos -= zPos * 2.0f;
			}
		}*/
	}

	IncPos(xPos, 0.0f, zPos);
	LookAt_XZ(GetPosX() + xPos, GetPosZ() + zPos);

	// Freeze NPC if not in battle but battle system active
	if (m_pBattleSystem->IsActive()) SetState(AIState::Freeze);
}

void Monster::Chase(XMFLOAT3 target)
{
	// Player gets out of sight
	if (!Utilities::CheckDistance(GetPosition(), m_pPlayer->GetPosition(), m_chaseDistance))
	{
		SetState(AIState::Wander);
	}

	// Collide with player and start battle
	if (CheckCollision(m_pPlayer))
	{
		SetState(AIState::InBattle);
	}

	LookAt_XZ(target.x, target.z);
	MoveForward(m_chaseSpeed);

	// Freeze NPC if not in battle but battle system active
	if (m_pBattleSystem->IsActive()) SetState(AIState::Freeze);
}

void Monster::InitBattlePhase() 
{
	// Move monster away if player escapes battle
	if (!m_pBattleSystem->IsActive()) 
	{
		m_isInBattle = false;
		SetState(AIState::Flee);
	}

	// Don't repeat code if Battle System is already running
	if (m_pBattleSystem->IsActive()) return;

	m_isInBattle = true;
	m_pBattleSystem->SetEnemyActor(this);
	m_pBattleSystem->StartStateMachine();
}

void Monster::Flee(XMFLOAT3 target)
{
	// Player gets out of sight
	if (!Utilities::CheckDistance(GetPosition(), m_pPlayer->GetPosition(), m_fleeDistance))
	{
		SetState(AIState::Wander);
	}

	LookAt_XZ_Reversed(target.x, target.z);
	MoveForward(m_chaseSpeed);
}

void Monster::WaitForBattleEnd()
{
	// Monsters outside of ballte wait for battle to end
	if (m_pBattleSystem->IsActive()) return;

	// Stop AI if actor died
	if (!m_isAlive)
	{
		SetState(AIState::Dead);
		return;
	}

	// Back to normal state if battle system is inactive but move away from player
	SetState(AIState::Flee);
}

// Switch state
void Monster::SetState(AIState newState) 
{
	m_state = newState;
}

// Run state machine
void Monster::RunAI()
{
	// Do nothing if NPC is in battle
	if (m_isInBattle) return;

	switch (m_state)
	{
		case AIState::Wander:
			Wander();
			break;

		case AIState::Chasing:
			Chase(m_pPlayer->GetPosition());
			break;

		case AIState::InBattle:
			InitBattlePhase();
			break;

		case AIState::Flee:
			Flee(m_pPlayer->GetPosition());
			break;

		case AIState::Freeze:
			WaitForBattleEnd();
			break;

		case AIState::Dead:
			break;

		default:
			break;
	}
}