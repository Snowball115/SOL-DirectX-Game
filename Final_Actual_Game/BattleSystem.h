#pragma once

#include "Camera.h"

class Monster;
class Player;
class BattleSystem
{
private:

	enum class BattleState 
	{
		Start,
		PlayerTurn,
		EnemyTurn,
		End
	};

	BattleState m_state;

	bool m_isActive;
	bool m_isPlayerTurn;
	float m_inputTimer;
	int m_playerAction;

	Camera* m_pPlayerCamera;
	Player* m_pPlayer;
	Monster* m_pPlayerMonster;
	Monster* m_pEnemyMonster;
	XMFLOAT3 m_playerPos;
	XMFLOAT3 m_enemyPos;

	void SetState(BattleState newState);
	void StartBattle();
	void EndBattle();
	void CalcPlayerAction();
	void CalcEnemyAction();

public:

	BattleSystem(Camera* camera, Player* player);
	~BattleSystem();

	bool IsActive();
	bool IsPlayerTurn();
	void SetPlayerActor(Monster* player);
	void SetEnemyActor(Monster* enemy);
	Monster* GetEnemyActor();
	void GetPlayerInput(int i);
	void StartStateMachine();
	void StopStateMachine();
	void Run();
};