#pragma once
#include "Squad.h"
#include "MovementManager.h"
#include "inputHandler.h"
#include "Define.h"
#include "inventory.h"

#define MODEL_ORKS 0
#define MODEL_HUMANS 1
#define MODEL_NOMADS 2
#define MODEL_EVIL_HUMANS 3
#define MODEL_GOBLINS 4
#define MODEL_PLAYER 5
#define MODEL_BANDITS 6
#define MODEL_ANIMALS 7

#define ENTITY_ALLY 1
#define ENTITY_ENEMY 2
#define ENTITY_WHATEVER 3

typedef struct _LDR_MODELS
{
	const char** models;
	uint32_t amount;
} LDR_MODELS;

const char* LDR_ORC_TEXTURE[] = {
	"Data\\bt_orc_0.png"
};
const char* LDR_GOBLIN_TEXTURE[] = {
	"Data\\bt_goblin_0.png"
};
const char* LDR_NOMAD_TEXTURE[] = {
	"Data\\bt_nomad_0.png"
};
const char* LDR_HUMAN_TEXTURE[] = {
	"Data\\bt_human_0.png",
	"Data\\bt_human_1.png",
	"Data\\bt_human_2.png"
};
const char* LDR_EVILHUMAN_TEXTURE[] = {
	"Data\\bt_evilhuman_0.png",
	"Data\\bt_evilhuman_1.png"
};
LDR_MODELS LDR_FACTION_TEXTURE_MAP[] = {
	LDR_MODELS{LDR_ORC_TEXTURE, sizeof(LDR_ORC_TEXTURE) / sizeof(LDR_ORC_TEXTURE[0])},
	LDR_MODELS{LDR_HUMAN_TEXTURE, sizeof(LDR_HUMAN_TEXTURE) / sizeof(LDR_HUMAN_TEXTURE[0])},
	LDR_MODELS{LDR_NOMAD_TEXTURE, sizeof(LDR_NOMAD_TEXTURE) / sizeof(LDR_NOMAD_TEXTURE[0])},
	LDR_MODELS{LDR_EVILHUMAN_TEXTURE, sizeof(LDR_EVILHUMAN_TEXTURE) / sizeof(LDR_EVILHUMAN_TEXTURE[0])},
	LDR_MODELS{LDR_GOBLIN_TEXTURE, sizeof(LDR_GOBLIN_TEXTURE) / sizeof(LDR_GOBLIN_TEXTURE[0])},
	LDR_MODELS{LDR_HUMAN_TEXTURE, sizeof(LDR_HUMAN_TEXTURE) / sizeof(LDR_HUMAN_TEXTURE[0])}
};

#define MODEL_ENEMY_FACTION_BASE		20
#define MODEL_PLAYER_FACTION_BASE		40
#define LDR_MAX_FACTION_MODELS 16

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
	EntityBattleManager() = default;
	EntityBattleManager(rasticore::RastiCoreRender* r, rasticore::ModelCreationDetails rect_mcd, rasticore::Program fmp, rasticore::VertexBuffer mapVao, Inventory* inv, DraggedObj* dragObj, CameraOffset* cameraOffset) : instance(InputHandler::getInstance()) {
		this->r = r;
		this->rect_mcd = rect_mcd;
		LoadTextureFromFile("Data\\mongo.png", "pilgrim");
	
		for (int i = 0; i < LDR_MAX_FACTION_MODELS; i++)
		{
			r->newModel(MODEL_ENEMY_FACTION_BASE + i, rect_mcd.vb, rect_mcd.p, rect_mcd.v_cnt, rect_mcd.rm, rasticore::Texture2DBindless(), 20);
			r->newModel(MODEL_PLAYER_FACTION_BASE + i, rect_mcd.vb, rect_mcd.p, rect_mcd.v_cnt, rect_mcd.rm, rasticore::Texture2DBindless(), 20);
		}

		cLoadedFactionTexturesEnemy = -1;
		cLoadedFactionTexturesPlayer = -1;

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
		fightMapVisn = glGetUniformLocation(pid, "visn");
		fightFp = glGetUniformLocation(pid, "fp");

		this->mapProgram = fmp;
		this->mapVao = mapVao;

		selectedEntity = nullptr;
		this->inv = inv;

		this->draggedObj = dragObj;
		this->cameraOffset = cameraOffset;
	}

	void createBattleMap(std::string battleMapName, uint64_t texture, std::filesystem::path collisionPath, float mapSize, float tileSize) {
		battleMaps[battleMapName] = BattleMap{ texture, collisionPath, mapSize, tileSize };
	}

	void LoadPlayerFactionTextures(uint8_t id)
	{
		if (cLoadedFactionTexturesPlayer == id)
			return;

		cLoadedFactionTexturesPlayer = id;
		
		LDR_MODELS* models = LDR_FACTION_TEXTURE_MAP + id;
		texturesPlayerAmount = models->amount;

		for (int i = 0; i < models->amount; i++)
		{
			LoadTextureFromFile(models->models[i]);
			r->getModel(MODEL_PLAYER_FACTION_BASE + i)->std_texture2d = GetTextureFullInfo(models->models[i])->txb;
		}

	}

	void LoadEnemyFactionTextures(uint8_t id)
	{
		if (cLoadedFactionTexturesEnemy == id)
			return;

		cLoadedFactionTexturesEnemy = id;

		LDR_MODELS* models = LDR_FACTION_TEXTURE_MAP + id;
		texturesEnemyAmount = models->amount;

		for (int i = 0; i < models->amount; i++)
		{
			LoadTextureFromFile(models->models[i]);
			r->getModel(MODEL_ENEMY_FACTION_BASE + i)->std_texture2d = GetTextureFullInfo(models->models[i])->txb;
		}
	}

	void startBattle(BattleData& battleData) {
		characterWindow = inv->GetWindow("char_inv");

		tour = BT_TOUR_AI;
		printf("Bitka!\n");
		data = battleData;
		int idx = (int)(rand() % battleMaps.size());
		auto it = battleMaps.begin();
		std::advance(it, idx);
		currentMap = it->second;
		entityMovementManager = EntityMovementManager{ currentMap.path, currentMap.mapSize, currentMap.tileSize, r, rect_mcd };
		Squad::SquadComp* units = data.s1->getSquadComp();

		LoadPlayerFactionTextures(data.s1->getSquadFactionID());
		LoadEnemyFactionTextures(data.s2->getSquadFactionID());

		int tilesAmountX, offsetY, offsetX, tileOffset;
		tilesAmountX = currentMap.mapSize / currentMap.tileSize;
		offsetY = (tilesAmountX / 2) - ((tilesAmountX / 2) - (units->size / 2));
		offsetX = -(tilesAmountX / 2);
		tileOffset = currentMap.tileSize / 2.0f;
		Entity* e = nullptr;
		for (int i = 0; i < units->size; i++)
		{
			e = units->entities[i];
			Stats* st = e->getStats();
			st->stamina = i + 0.1f;
			e->setEntityPosition(glm::vec2{ (offsetX + 2) * currentMap.tileSize + tileOffset, (offsetY + 1) * currentMap.tileSize + tileOffset });
			e->EntityClearMove();
			//entityMovementManager.AddCollision(e->getPosition() + 512.0f - 32.0f);
			offsetY -= 1;
			r->newObject(MODEL_PLAYER_FACTION_BASE + rand() % texturesPlayerAmount, glm::translate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 2.0f)), glm::vec3(1.0f / 100.0f * currentMap.tileSize, 1.0f / 100.0f * currentMap.tileSize, 1.0f)), glm::vec3(0.0f, 0.0f, 0.0f)), &e->id);
		}
		units = data.s2->getSquadComp();
		offsetY = (tilesAmountX / 2) - ((tilesAmountX / 2) - (units->size / 2));
		for (int i = 0; i < units->size; i++)
		{
			e = units->entities[i];
			auto s = e->getStats();
			s->hp = 100.0f;
			e->SetHp(90.0f);
			e->setEntityPosition(glm::vec2{ -(offsetX + 1) * currentMap.tileSize + tileOffset, (offsetY + 1) * currentMap.tileSize + tileOffset });
			entityMovementManager.AddCollision(e->getPosition() + 512.0f - 32.0f);
			e->EntityClearMove();
			offsetY -= 1;
			AiDecideEntityInitialState(e);
			//int b = e->state->MoveEntity(&data);
			//if (b == true) moveEntityNear(e->travel, e);
			r->newObject(MODEL_ENEMY_FACTION_BASE + rand() % texturesEnemyAmount, glm::translate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 2.0f)), glm::vec3(1.0f / 100.0f * currentMap.tileSize, 1.0f / 100.0f * currentMap.tileSize, 1.0f)), glm::vec3(0.0f, 0.0f, 0.0f)), &e->id);
		}
	}

	void SetSelectedUnitPosition(glm::vec2 v)
	{
		unitSelectedPos = v;
	}

	void update() {
		inputHandler();
		entityMovementManager.update();
		Squad::SquadComp* units = data.s1->getSquadComp();

		if (AiCanBattle() == true)
		{
			if (tour == BT_TOUR_AI && entityMovementManager.pathExist() == false)
			{
				Entity* ent = AiGetNextMoveableEntity();
				auto b = ent->state->MoveEntity(&data);
				ent->EntitySetMove();
				if (b == true)
					moveEntityNear(ent->travel, ent);
				tour = BT_TOUR_PLAYER;
			}
		}


		Entity* e = nullptr;
		for (int i = 0; i < units->size; i++)
		{
			e = units->entities[i];
			r->BindActiveModel(LONG_GET_MODEL(e->id));
			r->SetObjectMatrix(LONG_GET_OBJECT(e->id), glm::translate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 2.0f)), glm::vec3(1.0f / 100.0f * currentMap.tileSize, 1.0f / 100.0f * currentMap.tileSize, 1.0f)), glm::vec3(0.0f, 0.0f, 0.0f)), true);
		}

		units = data.s2->getSquadComp();
		for (int i = 0; i < units->size; i++)
		{
			e = units->entities[i];
			AiUpdateEntityState(e);
			//e->state->MoveEntity(&data);
			r->BindActiveModel(LONG_GET_MODEL(e->id));
			r->SetObjectMatrix(LONG_GET_OBJECT(e->id), glm::translate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(e->getPosition().x, e->getPosition().y, 2.0f)), glm::vec3(1.0f / 100.0f * currentMap.tileSize, 1.0f / 100.0f * currentMap.tileSize, 1.0f)), glm::vec3(0.0f, 0.0f, 0.0f)), true);
		}

		mapProgram.use();
		mapVao.bind();

		glUniform1f(fightScaleX, currentMap.mapSize);
		glUniform1f(fightScaleY, currentMap.mapSize);

		glUniform2f(fightMapTil, currentMap.mapSize / currentMap.tileSize, currentMap.mapSize / currentMap.tileSize);

		glUniform2f(fightMapDim, currentMap.mapSize, currentMap.mapSize);
		glm::vec2 mp;
		float vision;

		if (selectedEntity != nullptr)
		{
			mp = selectedEntity->getPosition();// - glm::vec2(32.0f, 32.0f);
			vision = selectedEntity->getStats()->stamina;
		}
		else
		{
			mp = glm::vec2(-1000.0f);
			vision = 0.0f;
		}
		glUniform2f(fightMouse, mp.x + currentMap.mapSize / 2.0f, mp.y + currentMap.mapSize / 2.0f);
		glUniform1f(fightMoveX, -currentMap.mapSize / 2.0f);
		glUniform1f(fightMoveY, -currentMap.mapSize / 2.0f);

		glUniform1f(fightMapVisn, vision);

		glUniformHandleui64ARB(1, currentMap.texture);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glUniform1i(fightFp, 0);
		for (int i = 0; i < LDR_MAX_FACTION_MODELS; i++)
		{
			r->RenderSelectedModel(MODEL_PLAYER_FACTION_BASE + i);
		}

		glUniform1i(fightFp, 1);
		for (int i = 0; i < LDR_MAX_FACTION_MODELS; i++)
		{
			r->RenderSelectedModel(MODEL_ENEMY_FACTION_BASE + i);
		}
		glUniform1i(fightFp, 0);
	}

	Entity* AiGetNextMoveableEntity()
	{
		Entity* found = NULL;
		Squad::SquadComp* units = data.s2->getSquadComp();
		for (int i = 0; i < units->size; i++)
		{
			if (units->entities[i]->state->CanMoveEntity() == true && units->entities[i]->canMove() == true)
			{
				found = units->entities[i];
				return found;
			}
		}

		for (int i = 0; i < units->size; i++)
		{
			units->entities[i]->EntityClearMove();
		}
		
		return AiGetNextMoveableEntity();
	}

	bool AiCanBattle()
	{
		return true;
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

	void AiDecideEntityInitialState(Entity* self)
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
	Entity* getEntity(uint8_t flag = ENTITY_ALLY) {
		Entity* entityItem;
		glm::vec2 entityPos, mousePos;
		mousePos = getCorrectedMousePosition();
		
		Squad::SquadComp* squadComp = nullptr;
		if(flag == ENTITY_ALLY) squadComp = data.s1->getSquadComp();
		else if(flag == ENTITY_ENEMY) squadComp = data.s2->getSquadComp();

		int tileOffset = currentMap.tileSize / 2.0f;
		if (flag == ENTITY_ENEMY || flag == ENTITY_ALLY) {
			for (int idx = 0; idx < squadComp->size; idx++) {
				entityItem = squadComp->entities[idx];
				entityPos = entityItem->getPosition();
				if (pointInRect(mousePos, glm::vec4{ entityPos.x - tileOffset, entityPos.y - tileOffset, currentMap.tileSize, currentMap.tileSize })) {
					return entityItem;
				}
			}
		}
		else {
			squadComp = data.s1->getSquadComp();
			for (int idx = 0; idx < squadComp->size; idx++) {
				entityItem = squadComp->entities[idx];
				entityPos = entityItem->getPosition();
				if (pointInRect(mousePos, glm::vec4{ entityPos.x - tileOffset, entityPos.y - tileOffset, currentMap.tileSize, currentMap.tileSize })) {
					return entityItem;
				}
			}
			squadComp = data.s2->getSquadComp();
			for (int idx = 0; idx < squadComp->size; idx++) {
				entityItem = squadComp->entities[idx];
				entityPos = entityItem->getPosition();
				if (pointInRect(mousePos, glm::vec4{ entityPos.x - tileOffset, entityPos.y - tileOffset, currentMap.tileSize, currentMap.tileSize })) {
					return entityItem;
				}
			}
		}
		return nullptr;
	}

	bool pointInRect(glm::vec2 point, glm::vec4 rect) {
		//x,y,z,w
		return point.x >= rect.x && point.x <= rect.x + rect.z &&
			point.y >= rect.y && point.y <= rect.y + rect.w;

	}

	glm::vec2 getMousePosition() {
		int x, y;
		SDL_GetMouseState(&x, &y);
		return glm::vec2{ x,y };
	}

	glm::vec2 getCorrectedMousePosition() {
		glm::vec2 mousePos = getMousePosition();
		glm::vec2 screen = glm::vec2(MAP_WIDTH, MAP_HEIGHT);

		glm::vec4 nds;
		nds.x = (2.0f * (float)mousePos.x) / screen.x - 1.0f;
		nds.y = (2.0f * (float)mousePos.y) / screen.y - 1.0f;
		nds.z = 1.0f;
		nds.w = 1.0f;

		nds.y *= -1.0f;

		glm::mat4 inv_projection = glm::inverse(r->MVP.matProj);

		nds = inv_projection * nds;

		glm::mat4 inv_camera = glm::inverse(r->MVP.matCamera);

		nds = inv_camera * nds;

		return glm::vec2(nds.x, nds.y);
	}

	bool moveEntityNear(glm::vec2 e, Entity* entityItem)
	{
		for (int a = 1; a > -1; a--)
		{
			for (int b = 1; b > -1; b--)
			{
				if (entityMovementManager.pass(e + glm::vec2(64.0f * a, 64.0f * b) + 512.0f) == false)
				{
					moveEntity(e + glm::vec2(64.0f * a, 64.0f * b), entityItem);
					return true;
				}
			}
		}
		return false;
	}

	bool moveEntity(glm::vec2 e, Entity* entityItem) {
		bool r = false;
		if (!entityMovementManager.pathExist())
		{
			entityMovementManager.DelCollision(entityItem->getPosition() + 512.0f - 32.0f);
			r = entityMovementManager.createEntityPath(Astar::point{ (int)e.x + 512, (int)e.y + 512 }, entityItem);
			e.x = ((int)((e.x + 512.0f) / currentMap.tileSize)) * currentMap.tileSize;
			e.y = ((int)((e.y + 512.0f) / currentMap.tileSize)) * currentMap.tileSize;
			entityMovementManager.AddCollision(e);
			return r;
		}
		return r;
	}

	void inputHandler() {
		if (instance.KeyPressed(SDL_SCANCODE_W)) {
			cameraOffset->y += 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_S)) {
			cameraOffset->y -= 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_A)) {
			cameraOffset->x -= 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_D)) {
			cameraOffset->x += 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_Q)) {
			cameraOffset->z *= 0.9f;
		}
		if (instance.KeyPressed(SDL_SCANCODE_E)) {
			cameraOffset->z *= 1.1f;
		}
		if (instance.KeyPressed(SDL_SCANCODE_LEFT)) {
			//GUI
			glm::vec2 mp = getMousePosition();
			if (draggedObj->draggedWindow.win && !draggedObj->draggedWindow.wasPressed) {
				draggedObj->draggedWindow.wasPressed = true;
				auto dim = draggedObj->draggedWindow.win->getDim();
				int offsetX = abs(dim.position.x - mp.x);
				int offsetY = abs(dim.position.y - mp.y);
				draggedObj->draggedWindow.offset.x = offsetX;
				draggedObj->draggedWindow.offset.y = offsetY;
			}
			else if (draggedObj->draggedWindow.wasPressed) {
				draggedObj->draggedWindow.win->changeWindowPosition(mp.x - draggedObj->draggedWindow.offset.x, mp.y - draggedObj->draggedWindow.offset.y);
			}
		}
		else {
			//GUI
			draggedObj->draggedWindow = {};
			draggedObj->draggedItem = {};
		}
		if (instance.KeyPressedOnce(SDL_SCANCODE_LEFT)) {
			glm::vec2 mp = getMousePosition();
			//GUI
			if (inv->isGuiClicked(mp)) {
				Inventory::Window* win = inv->setPressedWindowOnTop(mp);
				win->getGWindow()->CollisionCheck(mp.x, mp.y);
			}
			//GAME LOGIC
			else {
				mp = getCorrectedMousePosition();
				printf("%f %f\n", mp.x, mp.y);
				Entity* se = getEntity();
				if (selectedEntity == nullptr)
				{
					selectedEntity = se;
				}
				else if (selectedEntity != nullptr && se != nullptr)
				{
					selectedEntity = se;
				}
				else
				{
					if (tour == BT_TOUR_PLAYER && selectedEntity->canMove() == true)
					{
						auto pos = getCorrectedMousePosition();
						moveEntity(pos, selectedEntity);
						selectedEntity->EntitySetMove();
						tour = BT_TOUR_AI;
					}
				}
			}
		}
		if (instance.KeyPressedOnce(SDL_SCANCODE_RIGHT)) {
			auto a = getCorrectedMousePosition();
			Entity* se = getEntity(ENTITY_WHATEVER);
			if (se) {
				getCharacterInventory_E(nullptr, nullptr, &se, inv, characterWindow);
			}
		}
	}

	uint8_t cLoadedFactionTexturesPlayer;
	uint32_t texturesPlayerAmount;

	uint8_t cLoadedFactionTexturesEnemy;
	uint32_t texturesEnemyAmount;

	

	uint32_t tour;

	EntityMovementManager entityMovementManager;
	//
	std::unordered_map<std::string, BattleMap> battleMaps;
	BattleData data;
	BattleMap currentMap;

	Entity* selectedEntity;
	Inventory* inv;

	Inventory::Window* characterWindow;

	DraggedObj* draggedObj;

	InputHandler& instance;
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
	uint32_t fightMapVisn;
	uint32_t fightFp;

	CameraOffset* cameraOffset;

	rasticore::VertexBuffer mapVao;
	rasticore::Program mapProgram;
};