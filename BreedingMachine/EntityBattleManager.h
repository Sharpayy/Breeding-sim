#pragma once
#include "Squad.h"
#include "MovementManager.h"
#include "Entity.h"

#define DUPA_CYCE_WADOWICE	15

class EntityBattleManager {
public:

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
		
		int tilesAmountX, offsetY, offsetX;
		tilesAmountX = currentMap.mapSize / currentMap.tileSize;
		offsetY = (tilesAmountX / 2) - ((tilesAmountX / 2) - (units->size / 2));
		offsetX = -(tilesAmountX / 2);
		Entity* e = nullptr;
		for (int i = 0; i < units->size; i++)
		{
			e = units->entities[i];
			e->setEntityPosition(glm::vec2{ (offsetX + 2) * currentMap.tileSize, (offsetY + 1) * currentMap.tileSize });
			units->entities[0]->setEntityPosition(glm::vec2(-512.0f));
			offsetY -= 1;
			e->id = r->newObject(DUPA_CYCE_WADOWICE, glm::translate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 2.0f)), glm::vec3(1.0f / 100.0f * currentMap.tileSize, 1.0f / 100.0f * currentMap.tileSize, 1.0f)), glm::vec3(-50.0f, -50.0f, 0.0f)));
		}
		moveEntity({ 0,0 }, e);

		units = data.s2->getSquadComp();
		offsetY = (tilesAmountX / 2) - ((tilesAmountX / 2) - (units->size / 2));
		for (int i = 0; i < units->size; i++)
		{
			e = units->entities[i];
			offsetY -= 1;
			Stats* s = e->getStats();
			s->hp = 100.0f;
			s->melee = 3.5f;
			s->ranged = 0.0f;
			e->SetHp(100.0f);
			AiDecideEntityInitialState(e, this);
			e->setEntityPosition(glm::vec2{ - (offsetX + 1) * currentMap.tileSize, (offsetY + 1) * currentMap.tileSize });
			e->id = r->newObject(DUPA_CYCE_WADOWICE, glm::translate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 2.0f)), glm::vec3(1.0f / 100.0f * currentMap.tileSize, 1.0f / 100.0f * currentMap.tileSize, 1.0f)), glm::vec3(-50.0f, -50.0f, 0.0f)));
		}
	}

	bool moveEntity(glm::vec2 e, Entity* entity) {
		if (!entityMovementManager.pathExist())
		{
			return entityMovementManager.createEntityPath(Astar::point{ (int)e.x, (int)e.y }, entity);
		}
		return false;
	}

	void SetSelectedUnitPosition(glm::vec2 v)
	{
		unitSelectedPos = v;
	}

	void update() {

		entityMovementManager.update();
		Squad::SquadComp* units = data.s1->getSquadComp();

		Entity* e = nullptr;
		r->BindActiveModel(DUPA_CYCE_WADOWICE);
		for (int i = 0; i < units->size; i++)
		{
			e = units->entities[i];
			r->SetObjectMatrix(e->id, glm::translate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 2.0f)), glm::vec3(1.0f / 100.0f * currentMap.tileSize, 1.0f / 100.0f * currentMap.tileSize, 1.0f)), glm::vec3(50.0f, 50.0f, 0.0f)), true);
		}

		units = data.s2->getSquadComp();
		for (int i = 0; i < units->size; i++)
		{
			e = units->entities[i];
			AiUpdateEntityState(e);
			
			//e->state->MoveEntity(this);
			r->SetObjectMatrix(e->id, glm::translate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 2.0f)), glm::vec3(1.0f / 100.0f * currentMap.tileSize, 1.0f / 100.0f * currentMap.tileSize, 1.0f)), glm::vec3(50.0f, 50.0f, 0.0f)), true);
		}

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

	float AiGetUnitArmor(Entity* e)
	{
		return 1.6f;
	}

	float AiGetAttackAfterArmor(Entity* e, float atk)
	{
		return 1.0f / pow(AiGetUnitArmor(e), 0.1f) * atk;
	}

	void AiUpdateEntityState(Entity* e)
	{
		e->state->NextState();
	}

	void AiDecideEntityInitialState(Entity* self, EntityBattleManager* battle)
	{
		if (self->getHp() <= 0.0f)
		{
			self->changeEntityState(new EntityCombatDead(self));
			return;
		}

		Stats* stats = self->getStats();
		if (stats->melee > stats->ranged)
		{
			self->changeEntityState(new EntityCombatCloseRange(self));
			return;
		}
		else
		{
			self->changeEntityState(new EntityCombatLongRange(self));
			return;
		}

		if (self->getHp() / stats->hp > 0.25f)
		{
			self->changeEntityState(new EntityCombatEscape(self));
			return;
		}
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