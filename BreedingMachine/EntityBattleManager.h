#pragma once
#include "Squad.h"
#include "MovementManager.h"

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
	EntityBattleManager(rasticore::RastiCoreRender* r_ = {}, rasticore::ModelCreationDetails rect_mcd = {}) {
		this->r_ = r_;
		this->rect_mcd = rect_mcd;
	}

	void startBattle(BattleData& battleData) {
		data = battleData;
		//std::filesystem::path path, uint32_t mapSize, uint8_t tileSize, rasticore::RastiCoreRender* r_, rasticore::ModelCreationDetails rect_mcd
		entityMovementManager = EntityMovementManager{ battleData.battleMap.path, battleData.battleMap.mapSize, battleData.battleMap.tileSize, r_, rect_mcd };
	}

	bool moveEntity(glm::vec2 e, Entity* entity) {
		if(!entityMovementManager.pathExist())
			entityMovementManager.createEntityPath(Astar::point{ (int)e.x, (int)e.y }, entity);
	}

	void update() {

	}

private:
	EntityMovementManager entityMovementManager;
	//
	BattleData data;
	//
	rasticore::RastiCoreRender* r_;
	rasticore::ModelCreationDetails rect_mcd;
};