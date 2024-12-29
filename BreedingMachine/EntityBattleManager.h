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
	EntityBattleManager(rasticore::RastiCoreRender* r, rasticore::ModelCreationDetails rect_mcd, rasticore::Program fmp, rasticore::VertexBuffer mapVao) {
		this->r = r;
		this->rect_mcd = rect_mcd;

		LoadTextureFromFile("Data\\mongo.png", "pilgrim");
		r->newModel(DUPA_CYCE_WADOWICE, rect_mcd.vb, rect_mcd.p, rect_mcd.v_cnt, rect_mcd.rm, GetTextureFullInfo("pilgrim")->txb, 50);

		mapProgram = fmp;
		mapProgram.use();
		uint32_t pid = mapProgram.id;
		fightScaleX = glGetUniformLocation(pid, "gScalex");
		fightScaleY = glGetUniformLocation(pid, "gScaley");
		fightMoveX = glGetUniformLocation(pid, "gMovex");
		fightMoveY = glGetUniformLocation(pid, "gMovey");
		fightMouse = glGetUniformLocation(pid, "MouseCoord");
		fightMapDim = glGetUniformLocation(pid, "MapDimensions");
		fightMapTil = glGetUniformLocation(pid, "MapTiles");

		this->mapProgram = fmp;
		this->mapVao = mapVao;
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

	void SetSelectedUnitPosition(glm::vec2 v)
	{
		unitSelectedPos = v;
	}

	void update() {

		mapProgram.use();
		mapVao.bind();

		glUniform1f(fightScaleX, currentMap.mapSize);
		glUniform1f(fightScaleY, currentMap.mapSize);

		glUniform2f(fightMapTil, currentMap.mapSize / currentMap.tileSize, currentMap.mapSize / currentMap.tileSize);

		glUniform2f(fightMapDim, currentMap.mapSize, currentMap.mapSize);
		glm::vec2 mp = unitSelectedPos;
		glUniform2f(fightMouse, mp.x + currentMap.mapSize / 2.0f, mp.y + currentMap.mapSize / 2.0f);
		glUniform1f(fightMoveX, -currentMap.mapSize / 2.0f);
		glUniform1f(fightMoveY, -currentMap.mapSize / 2.0f);

		glUniformHandleui64ARB(1, currentMap.texture);

		glDrawArrays(GL_TRIANGLES, 0, 6);

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

	glm::vec2 unitSelectedPos;

	uint32_t fightScaleX;
	uint32_t fightScaleY;
	uint32_t fightMoveX;
	uint32_t fightMoveY;
	uint32_t fightMouse;
	uint32_t fightMapDim;
	uint32_t fightMapTil;

	rasticore::VertexBuffer mapVao;
	rasticore::Program mapProgram;
};