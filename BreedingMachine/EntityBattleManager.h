#pragma once
#include "Squad.h"
#include "MovementManager.h"

#define DUPA_CYCE_WADOWICE	15

class EntityBattleManager {
public:
	struct BattleMap {
		uint64_t texture;
		std::filesystem::path path;
		float mapSize;
		float tileSize;
		float tileAmount;
	};
	struct BattleData {
		Squad* s1;
		Squad* s2;
		BattleMap battleMap;
	};
public:
	EntityBattleManager(rasticore::RastiCoreRender* r, rasticore::ModelCreationDetails rect_mcd ) {
		this->r = r;
		this->rect_mcd = rect_mcd;

		r->newModel(DUPA_CYCE_WADOWICE, rect_mcd.vb, rect_mcd.p, rect_mcd.v_cnt, rect_mcd.rm, rect_mcd.txb, 50);
	}

	EntityBattleManager()
	{

	}

	void startBattle(BattleData& battleData) {
		data = battleData;
		//std::filesystem::path path, uint32_t mapSize, uint8_t tileSize, rasticore::RastiCoreRender* r_, rasticore::ModelCreationDetails rect_mcd
		entityMovementManager = EntityMovementManager{ battleData.battleMap.path, battleData.battleMap.mapSize, battleData.battleMap.tileSize, r, rect_mcd };

		Squad::SquadComp* units = data.s1->getSquadComp();
		
		for (int i = 0; i < units->size; i++)
		{
			Entity* e = units->entities[i];
			r->newObject(DUPA_CYCE_WADOWICE, glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 1.5f)));
		}

		units = data.s2->getSquadComp();

		for (int i = 0; i < units->size; i++)
		{
			Entity* e = units->entities[i];
			r->newObject(DUPA_CYCE_WADOWICE, glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 1.5f)));
		}

	}

	bool moveEntity(glm::vec2 e, Entity* entity) {
		if(!entityMovementManager.pathExist())
			entityMovementManager.createEntityPath(Astar::point{ (int)e.x, (int)e.y }, entity);
	}

	void update() {
		r->RenderSelectedModel(DUPA_CYCE_WADOWICE);
	}

private:
	EntityMovementManager entityMovementManager;
	//
	BattleData data;
	//
	rasticore::RastiCoreRender* r;
	rasticore::ModelCreationDetails rect_mcd;
};