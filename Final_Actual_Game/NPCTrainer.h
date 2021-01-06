#pragma once

#include "Player.h"
#include "BattleSystem.h"

class NPCTrainer : public Player
{
private:

	enum class TrainerAIState 
	{
		Scouting,
		Chasing,
		InitBattle,
		Freeze,
		Dead
	};

	TrainerAIState m_state;

	XMFLOAT3 m_startPos;
	XMFLOAT3 currentPos;
	bool m_isChasingPlayer;
	float m_chaseDistance;
	float m_chaseSpeed;
	float moveProgress;

	BattleSystem* m_pBattleSystem;
	Monster* m_pEnemyTrainerMonster;
	Player* m_pPlayer;

	void ScoutForPlayer();
	void Chase(XMFLOAT3 target);
	void InitBattlePhase();
	void WaitForBattleEnd();
	void SetState(TrainerAIState newState);

public:

	NPCTrainer(ID3D11Device* device, ID3D11DeviceContext* context, BattleSystem* system, Player* player, Monster* trainerMonster);
	~NPCTrainer();

	Monster* GetMonster();
	bool GetChaseState();

	void RunTrainerAI();
};

