#pragma once
#include "Model.h"
#include "Utilities.h"
#include "Player.h"
#include "BattleSystem.h"
#include "ParticleGenerator.h"

class Monster : public Model
{
private:

	enum class AIState
	{
		Wander,
		Chasing,
		InBattle,
		Flee,
		Freeze,
		Dead
	};

	AIState m_state;

	bool m_isAlive;
	bool m_isInBattle;
	float m_health;
	float m_attack;
	float m_defense;
	float m_wanderSpeed;
	float m_chaseDistance;
	float m_chaseSpeed;
	float m_fleeDistance;

	vector<Model*> m_staticModels;
	BattleSystem* m_pBattleSystem;
	Player* m_pPlayer;
	ParticleGenerator* m_pHitEffect;
	float moveInterval = 3.0f;
	float moveTimer = moveInterval;
	float particleTimer;
	float xPos;
	float zPos;

	void SetState(AIState newState);
	void Wander();
	void Chase(XMFLOAT3 target);
	void InitBattlePhase();
	void Flee(XMFLOAT3 target);
	void WaitForBattleEnd();

public:

	Monster(ID3D11Device* device, ID3D11DeviceContext* context, BattleSystem* battleSystem, vector<Model*> colliders);
	Monster(ID3D11Device* device, ID3D11DeviceContext* context, BattleSystem* battleSystem);
	~Monster();

	void SetStats(float health, float attack, float defense);
	bool IsAlive();
	bool IsInBattle();
	float GetHealth();
	float GetAttackValue();
	float GetDefenseValue();
	void SetPlayer(Player* player);

	void Attack(Monster* enemy);
	void GetHurt(float damage);
	void Die();
	void Heal(float amount);
	bool FleeFromBattle();
	void DrawIfAlive(XMMATRIX* view, XMMATRIX* projection, XMFLOAT3 camPos);

	void RunAI();
};