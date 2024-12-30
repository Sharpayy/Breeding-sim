#pragma once
#include "Faction.h"
#include "MovementManager.h";
#include <array>
#include "inputHandler.h"
#include "BuildingManager.h"
#include "FactionManager.h"
#include <glm/vec2.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <random>
#include "timer.h"
#include "TextureLoader.h"
#include "EntityBattleManager.h"

#define THRESHOLD 10.0f
#define MAP_WIDTH 1400.0f
#define MAP_HEIGHT 800.0f

#define GAMETYPE_BIGMAP		1
#define GAMETYPE_FIGHT		2

class gameManager {
public:
	Inventory inv;

	gameManager(rasticore::RastiCoreRender* r_, rasticore::ModelCreationDetails rect_mcd) : instance(InputHandler::getInstance()) {
		this->rect_mcd = rect_mcd;
		this->r = r_;
		std::filesystem::path path = std::filesystem::current_path();
		std::filesystem::path collisionPath = path, buildingPath = path;
		collisionPath.append("Data\\collision.txt");
		buildingPath.append("Data\\buildings.txt");
		movementManager = SquadMovementManager{ collisionPath, 4096, 16, r_, rect_mcd };
		buildingManager = BuildingManager{ buildingPath };
		factionManager = FactionManager{r_, rect_mcd, 16};
		battleManager = EntityBattleManager{ r, rect_mcd };
		cameraOffset = CameraOffset{ 0, 0, 500.0f };
		initGame(path);
		game_type = GAMETYPE_BIGMAP;

		std::random_device rd;
		gen = std::mt19937{ rd() };
	}
	
	void update() {
		inputHandler();
		//auto pos = getMousePosition();
		//Astar::point p;
		//for (auto& squad : squads) {
		//	SetSquadPosition(squad->getSquadPosition(), squad);
		//}

		if (game_type == GAMETYPE_BIGMAP)
		{
			movementManager.update();
			handleSquadLogic();
			r->RenderSelectedModel(MODEL_PLAYER);
			r->RenderSelectedModel(MODEL_ORKS);
			r->RenderSelectedModel(MODEL_HUMANS);
			r->RenderSelectedModel(MODEL_NOMADS);
			r->RenderSelectedModel(MODEL_GOBLINS);
			r->RenderSelectedModel(MODEL_EVIL_HUMANS);
			//_r.RenderSelectedModel(MODEL_BANDITS);
			//_r.RenderSelectedModel(MODEL_ANIMALS);
		}

		if (game_type == GAMETYPE_FIGHT)
		{
			battleManager.update();
		}
		//Test sln

	}

	uint32_t getGameType()
	{
		return game_type;
	}

	void setGameType(uint32_t g)
	{
		game_type = g;
	}

	void inputHandler() {
		instance.handleKeys();
		if (instance.KeyPressed(SDL_SCANCODE_W)) {
			cameraOffset.y += 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_S)) {
			cameraOffset.y -= 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_A)) {
			cameraOffset.x -= 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_D)) {
			cameraOffset.x += 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_Q)) {
			cameraOffset.z -= 20.0f;
		}
		if (instance.KeyPressed(SDL_SCANCODE_E)) {
			cameraOffset.z += 20.0f;
		}
		if (instance.KeyPressed(SDL_SCANCODE_R))
		{
			game_type = (!(game_type - 1) + 1);
			SDL_Delay(100);
		}
		if (instance.KeyPressedOnce(SDL_SCANCODE_LEFT)) {
			
			auto mp = getMousePosition();
			Slot* slot = inv.getSlot(mp);

			ArmorItem item = ArmorItem();
			item.setAsset((void*)LoadTextureFromFile("Data\\EquipmentIconsC2.png"));
			
			if (slot != nullptr)
			{
				printf("%p\n", slot->getItem());
				if (slot->getItem() == nullptr)
				{
					slot->changeItem(&item);
				}
				else
				{
					slot->changeItem(nullptr);
				}
			}

			auto pos = getCorrectedMousePosition();
			movementManager.createSquadPath(Astar::point{ (int)pos.x, (int)pos.y }, player);
		}
		if (instance.KeyPressedOnce(SDL_SCANCODE_RIGHT)) {
			auto pos = getCorrectedMousePosition();
			pos.x = (int)(pos.x / 16) * 16;
			pos.y = (int)(pos.y / 16) * 16;
			player->setSquadPosition(pos);
			uint64_t id = player->getSquadID();
			r->BindActiveModel(LONG_GET_MODEL(id));
			r->SetObjectMatrix(LONG_GET_OBJECT(id), glm::translate(glm::mat4{ 1.0f }, glm::vec3{ player->getSquadPosition(), 1.1f}), true);

		}
	}

	glm::vec2 getMousePosition() {
		int x, y;
		SDL_GetMouseState(&x, &y);
		return glm::vec2{ x,y };
	}

	void StartBattle(Squad* s0, Squad* s1)
	{

	}

	glm::vec2 getCorrectedMousePosition() {
		glm::vec2 mousePos = getMousePosition();
		glm::vec2 screen = glm::vec2(MAP_WIDTH,MAP_HEIGHT);

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

	struct CameraOffset {
		float x = 0, y = 0, z = 0;
	};

	CameraOffset getCameraOffset() {
		return cameraOffset;
	}

private:
	void initItems() {
		//Weaponry
		WeaponItem bastard_sword = {"bastard sword", (void*) LoadTextureFromFile("","EquipmentIconsC2"), MELEE, new WeaponItem::ObjectStatistic{0}, 0};
		WeaponItem spear = { "spear", (void*)LoadTextureFromFile("","EquipmentIconsC61"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		WeaponItem hatchet = { "hatchet", (void*)LoadTextureFromFile("","EquipmentIconsC57"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		WeaponItem short_bow = { "short bow", (void*)LoadTextureFromFile("","EquipmentIconsC103"), RANGED, new WeaponItem::ObjectStatistic{0}, 0 };
		WeaponItem crossbow = { "crossbow", (void*)LoadTextureFromFile("","EquipmentIconsC121"), RANGED, new WeaponItem::ObjectStatistic{0}, 0 };
		WeaponItem morningstar = { "morningstar", (void*)LoadTextureFromFile("","EquipmentIconsC29"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		WeaponItem sickle_blade = { "sickle_blade", (void*)LoadTextureFromFile("","EquipmentIconsC15"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		WeaponItem berserker_blade = { "berserker blade", (void*)LoadTextureFromFile("","EquipmentIconsC13"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		WeaponItem battlehammer = { "battlehammer", (void*)LoadTextureFromFile("","EquipmentIconsC31"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		WeaponItem trident = { "trident", (void*)LoadTextureFromFile("","EquipmentIconsC65"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		WeaponItem battleaxe = { "battleaxe", (void*)LoadTextureFromFile("","EquipmentIconsC49"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		//Armory
		ArmorItem iron_chestplate = { "iron chestplate", (void*)LoadTextureFromFile("","EquipmentIconsC193"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem iron_greaves = { "iron greaves", (void*)LoadTextureFromFile("","EquipmentIconsC217"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem iron_cap = { "iron cap", (void*)LoadTextureFromFile("","EquipmentIconsC166"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };

		ArmorItem guardian_helmet = { "guardian helmet", (void*)LoadTextureFromFile("","EquipmentIconsC179"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem guardian_chestplate = { "guardian chestplate", (void*)LoadTextureFromFile("","EquipmentIconsC199"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem guardian_greaves = { "guardian greaves", (void*)LoadTextureFromFile("","EquipmentIconsC216"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };

		ArmorItem copper_helmet = { "copper helmet", (void*)LoadTextureFromFile("","EquipmentIconsC163"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem copper_chestplate = { "copper chestplate", (void*)LoadTextureFromFile("","EquipmentIconsC183"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem copper_greaves = { "copper greaves", (void*)LoadTextureFromFile("","EquipmentIconsC204"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };

		ArmorItem cap = { "cap", (void*)LoadTextureFromFile("","EquipmentIconsC161"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem clothes = { "clothes", (void*)LoadTextureFromFile("","EquipmentIconsC181"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem rags = { "rags", (void*)LoadTextureFromFile("","EquipmentIconsC202"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };

		ArmorItem darkwraith_helmet = { "darkwraith helmet", (void*)LoadTextureFromFile("","EquipmentIconsC176"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem darkwraith_chestplate = { "darkwraith chestplate", (void*)LoadTextureFromFile("","EquipmentIconsC191"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem darkwraith_greaves = { "darkwraith greaves", (void*)LoadTextureFromFile("","EquipmentIconsC207"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };

		ArmorItem copper_cap = { "copper cap", (void*)LoadTextureFromFile("","EquipmentIconsC162"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem copper_vest = { "copper vest", (void*)LoadTextureFromFile("","EquipmentIconsC182"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		ArmorItem leather_greaves = { "leather greaves", (void*)LoadTextureFromFile("","EquipmentIconsC203"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };
	}

	void initPrimaryInv(int width, int height, uint64_t texItemFrame) {
		inv.AddWindow("inventory", ObjectDim{ {0, 0}, width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		auto gwin = inv.getGWindow("inventory");
		
		//przyciski do zmiany na kolejny panel ekwipunku
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(0, 5, 2.5f), nullptr, LoadTextureFromFile("Data\\red.png")));
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(20 + 10, 5, 2.5f), nullptr, LoadTextureFromFile("Data\\red.png")));
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(40 + 20, 5, 2.5f), nullptr, LoadTextureFromFile("Data\\red.png")));
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(60 + 30, 5, 2.5f), nullptr, LoadTextureFromFile("Data\\red.png")));
		//labelka z napisem inventory + które okienko
		gwin->AddComponent(new GComponentButton(glm::vec2(60, 20), glm::vec3(140, 5, 2.5f), nullptr, LoadTextureFromFile("Data\\red.png")));
		//przycisk do zamkniêcia okienka
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(300, 5, 2.5f), nullptr, LoadTextureFromFile("Data\\red.png")));
		for (int i = 35; i < height - 75; i += 35) {
			for (int j = 5; j < width - 15; j += 35) {
				inv.AddSlotToWindow("inventory", Slot(nullptr, glm::vec2(j, i), 30.0f, 30.0f, EVERY_ITEM), texItemFrame);
			}
		}
		//labelka z szmeklami
		gwin->AddComponent(new GComponentSlider(glm::vec2(60, 20), glm::vec3(140, 300, 2.5f), nullptr, LoadTextureFromFile("Data\\red.png"), LoadTextureFromFile("Data\\angy.png")));

		inv.ActivateWindow("inventory");
	}
	void initCharInv(int width, int height, uint64_t texItemFrame) {
		inv.AddWindow("char_inv", ObjectDim{ {0,0} , width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		auto gwin = inv.getGWindow("char_inv");
		//nazwa ch³opa
		gwin->AddComponent(new GComponentButton(glm::vec2(40, 20), glm::vec3(55, 0, 2.5f), nullptr, LoadTextureFromFile("Data\\red.png")));
		//wyjœcie
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(130, 0, 2.5f), nullptr, LoadTextureFromFile("Data\\red.png")));
		//sloty
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(60, 30), 30.0f, 30.0f, HELMET), texItemFrame);
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(60, 60), 30.0f, 30.0f, CHESTPLATE), texItemFrame);
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(30, 60), 30.0f, 30.0f, WEAPON), texItemFrame);
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(90, 60), 30.0f, 30.0f, WEAPON), texItemFrame);
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(60, 90), 30.0f, 30.0f, LEGS), texItemFrame);
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(60, 120), 30.0f, 30.0f, BOOTS), texItemFrame);

		inv.ActivateWindow("char_inv");
	}

	void initGame(std::filesystem::path path) {
		//DO TOTALNEJ ZMIANY
		path = path.append("Data\\buildings.txt");
		initItems();

		factionManager.CreateNewFaction(MODEL_ORKS, "Data\\ork.png", "Orks", buildingManager.getRaceBuildings(MODEL_ORKS));
		factionManager.CreateNewFaction(MODEL_HUMANS, "Data\\human.png", "Humans", buildingManager.getRaceBuildings(MODEL_HUMANS));
		factionManager.CreateNewFaction(MODEL_NOMADS, "Data\\mongo.png", "Nomads", buildingManager.getRaceBuildings(MODEL_NOMADS));
		factionManager.CreateNewFaction(MODEL_EVIL_HUMANS, "Data\\evil_human.png", "EvilHumans", buildingManager.getRaceBuildings(MODEL_EVIL_HUMANS));
		factionManager.CreateNewFaction(MODEL_GOBLINS, "Data\\goblin.png", "Goblin", buildingManager.getRaceBuildings(MODEL_GOBLINS));
		factionManager.CreateNewFaction(MODEL_PLAYER, "Data\\player.png", "Player", buildingManager.getRaceBuildings(MODEL_PLAYER));
		factionManager.CreateNewFaction(MODEL_BANDITS, "Data\\bandit.png", "Bandit", buildingManager.getRaceBuildings(MODEL_BANDITS));
		factionManager.CreateNewFaction(MODEL_ANIMALS, "Data\\animal.png", "Furry", buildingManager.getRaceBuildings(MODEL_ANIMALS));
		
		inv = Inventory();
		auto texItemFrame = LoadTextureFromFile("Data\\item_frame.png");
		//initPrimaryInv(400, 400, texItemFrame);
		initCharInv(400, 400, texItemFrame);
		//inv.AddWindow("main_player_eq", ObjectDim{ {100.0f, 100.0f}, 600, 600 }, 2, LoadTextureFromFile("Data\\gui.png"));
		//inv.ActivateWindow("main_player_eq");
		//Slot* s0 = inv.AddSlotToWindow("main_player_eq", Slot(nullptr, glm::vec2(400.0f, 400.0f), 50, 50), r->getModel(0)->std_texture2d.handle);

		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_HUMANS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_EVIL_HUMANS, ALLY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_NOMADS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_ORKS, ALLY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_PLAYER, ENEMY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_BANDITS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_ANIMALS, ENEMY);


		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_HUMANS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_PLAYER, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_ANIMALS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_EVIL_HUMANS, ALLY);
		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_BANDITS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_NOMADS, ENEMY);


		factionManager.setFactionsRelationships(MODEL_EVIL_HUMANS, MODEL_PLAYER, NEUTRAL);
		factionManager.setFactionsRelationships(MODEL_EVIL_HUMANS, MODEL_ANIMALS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_EVIL_HUMANS, MODEL_HUMANS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_EVIL_HUMANS, MODEL_BANDITS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_EVIL_HUMANS, MODEL_NOMADS, NEUTRAL);


		factionManager.setFactionsRelationships(MODEL_ANIMALS, MODEL_PLAYER, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ANIMALS, MODEL_HUMANS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ANIMALS, MODEL_NOMADS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ANIMALS, MODEL_BANDITS, ENEMY);


		factionManager.setFactionsRelationships(MODEL_PLAYER, MODEL_NOMADS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_PLAYER, MODEL_HUMANS, NEUTRAL);
		factionManager.setFactionsRelationships(MODEL_PLAYER, MODEL_BANDITS, ENEMY);


		factionManager.setFactionsRelationships(MODEL_NOMADS, MODEL_BANDITS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_NOMADS, MODEL_HUMANS, ENEMY);


		factionManager.setFactionsRelationships(MODEL_HUMANS, MODEL_BANDITS, ENEMY);

		player = factionManager.CreateNewSquad(MODEL_PLAYER, glm::vec2(-1000.0f));
		player->force = 1.0f;

		Squad* squad;
		srand(time(NULL));
		player->setSquadState(STAND);
		int amount = 0;
		for (int i = 0; i < 32; i++) {
			auto buildings = buildingManager.getRaceBuildings(i % 8);
			if (i == MODEL_PLAYER || !buildings.size()) continue;
			squad = factionManager.CreateNewSquad(i % 8, buildings.at(rand() % buildings.size()).getBuildingPosition());
			if (squad) amount++;
			timer.startMeasure(squad->getSquadID(), 0);
			squad->force = getRandomNumber(10, 100);
		}
		std::cout << "Amount of squads: " << amount << "\n";
	}

	float calculateSquadViewDistance(Squad* squad) {
		//TO DO
		return 16.0f * 16;
	}

	void handleSquadLogic() {
		float distance = 0;
		float threashold = 20.0f;
		uint64_t id;
		float dist;
		for (auto& squadF : factionManager.getAllSquads()) {
			if (squadF == player) continue;
			id = squadF->getSquadID();
			for (auto& squadS : factionManager.getAllSquads()) {
				if (glm::distance(squadF->getSquadPosition(), player->getSquadPosition()) <= 4.0f) {
					game_type = GAMETYPE_FIGHT;
					EntityBattleManager::BattleData battleData = {
						squadF,
						player,
					};
					battleManager.startBattle(battleData);
					return;
				}
				else if (squadF != squadS) {
					handleSquadState(squadF, squadS);
					handleSquadStateLogic(squadF);
				}
			}
		}
	}

	void SquadRetreat(Squad* s1, glm::vec2 end) {
		glm::vec2 position1, position2;
		position1 = getCorrectedSquadPosition(s1->getSquadPosition());
		position2 = getCorrectedSquadPosition(end);

		glm::vec2 dirVec = glm::normalize(position2 - position1) * 16.0f * 6.0f; // 22.6274166f
		glm::vec2 endPoint = position1 - dirVec;
		int pathFound = movementManager.createSquadPath(Astar::point{ (int)endPoint.x, (int)endPoint.y }, s1);
		int angle = 30;
		glm::mat2 rotationMatrix;
		while (!pathFound && angle <= 180) {
			//if (angle >= 180) return;
			rotationMatrix = glm::mat2(
				glm::cos(360 - angle), -glm::sin(360 - angle),
				glm::sin(360 - angle), glm::cos(360 - angle)
			);
			dirVec = (dirVec * rotationMatrix);
			endPoint = position1 + dirVec;
			pathFound = movementManager.createSquadPath(Astar::point{ (int)endPoint.x, (int)endPoint.y }, s1);
			
			if (pathFound) return;

			rotationMatrix = glm::mat2(
				glm::cos(angle), -glm::sin(angle),
				glm::sin(angle), glm::cos(angle)
			);
			dirVec = (dirVec * rotationMatrix);
			endPoint = position1 + dirVec;
			pathFound = movementManager.createSquadPath(Astar::point{ (int)endPoint.x, (int)endPoint.y }, s1);

			angle += 20;
		}
	}

	void SquadChase(Squad* s1, glm::vec2 end) {
		movementManager.createSquadPath(Astar::point{ (int)end.x, (int)end.y }, s1);
	}

	void SquadPatrol(Squad* squad) {
		auto buildings = buildingManager.getRaceBuildings(squad->getSquadFactionID());
		int buildingAmount = buildings.size();
		if (buildingAmount) {
			if (!movementManager.SquadHasPath(squad)) { 
				glm::vec2 randomBuildingPosition = buildings.at(getRandomNumber(0, buildingAmount - 1)).getBuildingPosition();
				movementManager.createSquadPath(Astar::point{ (int)randomBuildingPosition.x, (int)randomBuildingPosition.y }, squad);
			}
		}
		//IF BUILDINGS NOT FOUND ACT LIKE NORMAL WANDER STATE
		else {
			squad->setSquadState(WANDER);
			timer.startMeasure(squad->getSquadID(), getRandomNumber(20, 50));
		}
	}

	void SquadStand(Squad* squad) {

	}

	void SquadWander(Squad* squad, int distance) {
		while (!movementManager.SquadHasPath(squad)) {
			glm::vec2 squadPosition, newPosition;
			squadPosition = squad->getSquadPosition();
			int angle = getRandomNumber(0, 360);
			glm::mat2 rotationMatrix = glm::mat2(
				glm::cos(angle), -glm::sin(angle),
				glm::sin(angle), glm::cos(angle)
			);
			newPosition = squadPosition + (rotationMatrix * glm::vec2(getRandomNumber(32, distance)));
			movementManager.createSquadPath(Astar::point{ (int)newPosition.x, (int)newPosition.y }, squad);
		}
	}

	void handleSquadState(Squad* squadF, Squad* squadS) {
		float distance = calculateSquadViewDistance(squadF);
		if (factionManager.getFactionsRelationships(squadS->getSquadFactionID(), squadF->getSquadFactionID()) == ENEMY) {
			if (glm::distance(squadF->getSquadPosition(), squadS->getSquadPosition()) <= distance) {
				//if (abs(squadF->force - squadS->force) >= THRESHOLD) {
				if(squadF->force >= squadS->force) {
					//if(calculateChance(80)) 
					squadF->setSquadState(CHASE);
					stateH[squadF->getSquadID()] = squadS->getSquadPosition();
				}
				else {
					squadF->setSquadState(RETREAT);
					stateH[squadF->getSquadID()] = squadS->getSquadPosition();
				}
				//if (stateH.find(squadF->getSquadID()) != stateH.end()) stateH.erase(squadF->getSquadID());
				timer.resetTimer(squadF->getSquadID());
				return;
			}
		}
		if (!timer.hasTimeElapsed(squadS->getSquadID())) return;
			
		//Default options
		int state = getRandomNumber(0,2);
		switch (state)
		{
		case STAND:
			squadF->setSquadState(STAND);
			timer.startMeasure(squadF->getSquadID(), getRandomNumber(5, 10));
			break;
		case PATROL:
			squadF->setSquadState(PATROL);
			timer.startMeasure(squadF->getSquadID(), getRandomNumber(30, 60));
			break;
		case WANDER:
			squadF->setSquadState(WANDER);
			timer.startMeasure(squadF->getSquadID(), getRandomNumber(20, 50));
			break;
		default:
			break;
		}
		
	}
	
	glm::vec2 getCorrectedSquadPosition(glm::vec2 position) {
		float offset, tileSize = movementManager.getMapTileSize() / 2.0f;
		offset = tileSize / 2.0f;
		position.x = ((int)(position.x - offset) / tileSize) * tileSize;
		position.y = ((int)(position.y - offset) / tileSize) * tileSize;
		return position;
	}

	void handleSquadStateLogic(Squad* squadF) {
		switch (squadF->getSquadState()) {
		case PATROL:
			SquadPatrol(squadF);
			break;
		case STAND:
			SquadStand(squadF);
			break;
		case WANDER:
			SquadWander(squadF, 16 * 32);
			break;
		case RETREAT:
			SquadRetreat(squadF, stateH[squadF->getSquadID()]);
			break;
		case CHASE:
			SquadChase(squadF, stateH[squadF->getSquadID()]);
			break;
		}
	}

	bool calculateChance(int chance) {
		chance = glm::clamp(chance, 0, 100);
		return (std::uniform_int_distribution(0, chance)(gen) == 0);
	}

	int getSquadDrawnState(int amountOfStates) {
		auto val = std::uniform_int_distribution(0, amountOfStates - 1)(gen);
		return val;
	}

	int getRandomNumber(int min, int max) {
		auto val = std::uniform_int_distribution(min, max)(gen);
		return val;
	}

private:
	rasticore::RastiCoreRender* r;
	rasticore::ModelCreationDetails rect_mcd;

	Squad* player;
	SquadMovementManager movementManager;
	BuildingManager buildingManager;
	FactionManager factionManager;
	EntityBattleManager battleManager;
	//std::vector<Squad*> squads;
	CameraOffset cameraOffset;
	InputHandler& instance;

	std::unordered_map<uint64_t, glm::vec2> stateH;
	//
	Timer timer;
	std::mt19937 gen;

	uint32_t game_type;
};