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
	};
public:
	EntityBattleManager(rasticore::RastiCoreRender* r, rasticore::ModelCreationDetails rect_mcd ) {
		this->r = r;
		this->rect_mcd = rect_mcd;

		LoadTextureFromFile("Data\\mongo.png", "pilgrim");
		r->newModel(DUPA_CYCE_WADOWICE, rect_mcd.vb, rect_mcd.p, rect_mcd.v_cnt, rect_mcd.rm, GetTextureFullInfo("pilgrim")->txb, 50);
	}

	EntityBattleManager()
	{

	}

	void createBattleMap(std::string battleMapName, uint64_t texture, std::filesystem::path collisionPath, float mapSize, float tileSize) {
		battleMaps[battleMapName] = BattleMap{ texture, collisionPath, mapSize, tileSize };
	}

	void startBattle(BattleData& battleData) {
		data = battleData;
		int idx = (int)(rand() % battleMaps.size());
		auto it = battleMaps.begin();
		std::advance(it, idx);
		currentMap = it->second;
		entityMovementManager = EntityMovementManager{ currentMap.path, currentMap.mapSize, currentMap.tileSize, r, rect_mcd };

		Squad::SquadComp* units = data.s1->getSquadComp();
		
		for (int i = 0; i < units->size; i++)
		{
			Entity* e = units->entities[i];
			e->setEntityPosition(glm::vec2{ 0,0 });
			r->newObject(DUPA_CYCE_WADOWICE, glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 2.0f)));
		}

		units = data.s2->getSquadComp();

		for (int i = 0; i < units->size; i++)
		{
			Entity* e = units->entities[i];
			e->setEntityPosition(glm::vec2{ 0,0 });
			r->newObject(DUPA_CYCE_WADOWICE, glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 2.0f)));
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
	std::unordered_map<std::string, BattleMap> battleMaps;
	BattleData data;
	BattleMap currentMap;
	//std::vector<>
	//
	rasticore::RastiCoreRender* r;
	rasticore::ModelCreationDetails rect_mcd;
};