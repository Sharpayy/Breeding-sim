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

#define GAMETYPE_BIGMAP		1
#define GAMETYPE_FIGHT		2

class gameManager {
public:
	Inventory inv;

	gameManager(rasticore::RastiCoreRender* r_, rasticore::ModelCreationDetails rect_mcd, rasticore::VertexBuffer mapVao, rasticore::Program mapPrg) :
		instance(InputHandler::getInstance()),
		battleManager(r_, rect_mcd, mapPrg, mapVao)
	{
		this->rect_mcd = rect_mcd;
		this->r = r_;
		std::filesystem::path path = std::filesystem::current_path();
		std::filesystem::path collisionPath = path, buildingPath = path;
		collisionPath.append("Data\\collision.txt");
		buildingPath.append("Data\\buildings.txt");
		movementManager = SquadMovementManager{ collisionPath, 4096, 16, r_, rect_mcd };
		buildingManager = BuildingManager{ buildingPath };
		factionManager = FactionManager{r_, rect_mcd, 16};
		itemLoader = ItemLoader();
		cameraOffset = CameraOffset{ 0, 0, 1.0f };
		initGame(path);
		game_type = GAMETYPE_BIGMAP;

		std::random_device rd;
		gen = std::mt19937{ rd() };
	}
	
	void update() {
		instance.handleKeys();
		//auto pos = getMousePosition();
		//Astar::point p;
		//for (auto& squad : squads) {
		//	SetSquadPosition(squad->getSquadPosition(), squad);
		//}

		if (game_type == GAMETYPE_BIGMAP)
		{
			inputHandler();
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
			battleManager.SetSelectedUnitPosition(getCorrectedMousePosition());
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
			cameraOffset.z *= 0.9f;
		}
		if (instance.KeyPressed(SDL_SCANCODE_E)) {
			cameraOffset.z *= 1.1f;
		}
		if (instance.KeyPressedOnce(SDL_SCANCODE_I))
		{
			if (!inv.isWindowActive("party_view")) {
				inv.ActivateWindow("party_view");
				std::vector<Slot*> slots = inv.getAllSlotsFromWindow("party_view");
				auto playerComp = player->getSquadComp();
				for (int i = 0; i < playerComp->size; i++) {
					EntityItem* eitm = new EntityItem(playerComp->entities[i]);
					eitm->setAsset((void*)LoadTextureFromFile("Data\\Goblin.png"));
					slots.at(i)->changeItem((Item*)eitm);
				}
				auto z = 1;
			}
			else inv.DisableWindow("party_view");
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
		itemLoader.loadItem(bastard_sword);
		WeaponItem spear = { "spear", (void*)LoadTextureFromFile("","EquipmentIconsC61"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(spear);
		WeaponItem hatchet = { "hatchet", (void*)LoadTextureFromFile("","EquipmentIconsC57"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(hatchet);
		WeaponItem short_bow = { "short bow", (void*)LoadTextureFromFile("","EquipmentIconsC103"), RANGED, new WeaponItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(short_bow);
		WeaponItem crossbow = { "crossbow", (void*)LoadTextureFromFile("","EquipmentIconsC121"), RANGED, new WeaponItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(crossbow);
		WeaponItem morningstar = { "morningstar", (void*)LoadTextureFromFile("","EquipmentIconsC29"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(morningstar);
		WeaponItem sickle_blade = { "sickle_blade", (void*)LoadTextureFromFile("","EquipmentIconsC15"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(sickle_blade);
		WeaponItem berserker_blade = { "berserker blade", (void*)LoadTextureFromFile("","EquipmentIconsC13"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(berserker_blade);
		WeaponItem battlehammer = { "battlehammer", (void*)LoadTextureFromFile("","EquipmentIconsC31"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(battlehammer);
		WeaponItem trident = { "trident", (void*)LoadTextureFromFile("","EquipmentIconsC65"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(trident);
		WeaponItem battleaxe = { "battleaxe", (void*)LoadTextureFromFile("","EquipmentIconsC49"), MELEE, new WeaponItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(battleaxe);
		//Armory
		ArmorItem iron_chestplate = { "iron chestplate", (void*)LoadTextureFromFile("","EquipmentIconsC193"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(iron_chestplate);
		ArmorItem iron_greaves = { "iron greaves", (void*)LoadTextureFromFile("","EquipmentIconsC217"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(iron_greaves);
		ArmorItem iron_cap = { "iron cap", (void*)LoadTextureFromFile("","EquipmentIconsC166"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(iron_cap);

		ArmorItem guardian_helmet = { "guardian helmet", (void*)LoadTextureFromFile("","EquipmentIconsC179"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(guardian_helmet);
		ArmorItem guardian_chestplate = { "guardian chestplate", (void*)LoadTextureFromFile("","EquipmentIconsC199"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(guardian_chestplate);
		ArmorItem guardian_greaves = { "guardian greaves", (void*)LoadTextureFromFile("","EquipmentIconsC216"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(guardian_greaves);

		ArmorItem copper_helmet = { "copper helmet", (void*)LoadTextureFromFile("","EquipmentIconsC163"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(copper_helmet);
		ArmorItem copper_chestplate = { "copper chestplate", (void*)LoadTextureFromFile("","EquipmentIconsC183"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(copper_chestplate);
		ArmorItem copper_greaves = { "copper greaves", (void*)LoadTextureFromFile("","EquipmentIconsC204"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(copper_greaves);

		ArmorItem cap = { "cap", (void*)LoadTextureFromFile("","EquipmentIconsC161"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(cap);
		ArmorItem clothes = { "clothes", (void*)LoadTextureFromFile("","EquipmentIconsC181"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(clothes);
		ArmorItem rags = { "rags", (void*)LoadTextureFromFile("","EquipmentIconsC202"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(rags);

		ArmorItem darkwraith_helmet = { "darkwraith helmet", (void*)LoadTextureFromFile("","EquipmentIconsC176"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(darkwraith_helmet);
		ArmorItem darkwraith_chestplate = { "darkwraith chestplate", (void*)LoadTextureFromFile("","EquipmentIconsC191"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(darkwraith_chestplate);
		ArmorItem darkwraith_greaves = { "darkwraith greaves", (void*)LoadTextureFromFile("","EquipmentIconsC207"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(darkwraith_greaves);

		ArmorItem copper_cap = { "copper cap", (void*)LoadTextureFromFile("","EquipmentIconsC162"), HELMET, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(copper_cap);
		ArmorItem copper_vest = { "copper vest", (void*)LoadTextureFromFile("","EquipmentIconsC182"), CHESTPLATE, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(copper_vest);
		ArmorItem leather_greaves = { "leather greaves", (void*)LoadTextureFromFile("","EquipmentIconsC203"), LEGS, new ArmorItem::ObjectStatistic{0}, 0 };
		itemLoader.loadItem(leather_greaves);
	}
	void initShop(int width, int height, uint64_t texItemFrame) {
		inv.AddWindow("shop", ObjectDim{ {0, 0}, width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		auto gwin = inv.getGWindow("shop");
		//labelka z napisem inventory + kt�re okienko
		gwin->AddComponent(new GComponentButton(glm::vec2(60, 20), glm::vec3(140, 5, 0.1f), "Shop", 0));
		//przycisk do zamkni�cia okienka
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(width - 20, 0, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png")));
		for (int i = 35; i < height - 35; i += 35) {
			for (int j = 5; j < width - 15; j += 35) {
				inv.AddSlotToWindow("shop", Slot(nullptr, glm::vec2(j, i), 30.0f, 30.0f, EVERY_ITEM), texItemFrame);
			}
		}

		inv.ActivateWindow("shop");
	}
	void initPrimaryInv(int width, int height, uint64_t texItemFrame) {
		inv.AddWindow("inventory", ObjectDim{ {0, 0}, width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		auto gwin = inv.getGWindow("inventory");
		//przyciski do zmiany na kolejny panel ekwipunku
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(0, 5, 0.5f), nullptr, LoadTextureFromFile("Data\\red.png")));
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(20 + 10, 5, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png")));
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(40 + 20, 5, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png")));
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(60 + 30, 5, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png")));
		//labelka z napisem inventory + kt�re okienko
		gwin->AddComponent(new GComponentButton(glm::vec2(60, 20), glm::vec3(140, 5, 0.1f), "Inventory 1", 0));
		//przycisk do zamkni�cia okienka
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(width - 20, 0, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png")));
		for (int i = 35; i < height - 75; i += 35) {
			for (int j = 5; j < width - 15; j += 35) {
				inv.AddSlotToWindow("inventory", Slot(nullptr, glm::vec2(j, i), 30.0f, 30.0f, EVERY_ITEM), texItemFrame);
			}
		}
		//labelka z szmeklami
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(width / 2, height - 35, 0.5f), "szmekle zydowskie: ", LoadTextureFromFile("Data\\red.png")));

		//inv.ActivateWindow("inventory");
	}

	void initCharInv(int width, int height, uint64_t texItemFrame, Entity entity) {
		inv.AddWindow("char_inv", ObjectDim{ {0,0} , width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		auto gwin = inv.getGWindow("char_inv");
		//nazwa ch�opa
		gwin->AddComponent(new GComponentButton(glm::vec2(40, 20), glm::vec3(55, 0, 0.1f), "Ryszard dra�", LoadTextureFromFile("Data\\red.png")));
		//wyj�cie
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(width - 20, 0, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png")));
		//sloty
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(width / 2 - 30, 30), 30.0f, 30.0f, HELMET), texItemFrame);
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(width / 2 - 30, 60), 30.0f, 30.0f, CHESTPLATE), texItemFrame);
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(width / 2 - 30 - 30, 60), 30.0f, 30.0f, WEAPON), texItemFrame);
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(width / 2 - 30 + 30, 60), 30.0f, 30.0f, WEAPON), texItemFrame);
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(width / 2 - 30, 90), 30.0f, 30.0f, LEGS), texItemFrame);
		inv.AddSlotToWindow("char_inv", Slot(nullptr, glm::vec2(width / 2 - 30, 120), 30.0f, 30.0f, BOOTS), texItemFrame);
		//staty
		//inv.ActivateWindow("char_inv");
	}

	void initSquadViewer(int width, int height, uint64_t texItemFrame) {
		inv.AddWindow("party_view", ObjectDim{ {0,0} , width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		auto gwin = inv.getGWindow("party_view");
		//wyj�cie
		gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(width - 20, 0, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png")));
		for (int i = 30; i < height - 90; i += 90) {
			for (int j = 30; j < width - 60; j += 60) {
				inv.AddSlotToWindow("party_view", Slot(nullptr, glm::vec2(j, i), 60.0f, 60.0f, ENTITY), texItemFrame);
				//przycisk do usuni�cia
				gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(j + 20, i + 60, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png")));
			}
		}
		//inv.ActivateWindow("party_view");
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
		
		battleManager.createBattleMap("BattleMap0", LoadTextureFromFile("Data\\BattleMaps\\map.jpg"), std::filesystem::path(), 1024.0f, 64.0f);

		inv = Inventory();
		auto texItemFrame = LoadTextureFromFile("Data\\item_frame.png");
		initShop(500, 500, texItemFrame);
		//initItems();
		//initPrimaryInv();
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
				if (squadF != squadS) {
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
				if (glm::distance(squadF->getSquadPosition(), player->getSquadPosition()) <= 4.0f) {
					game_type = GAMETYPE_FIGHT;
					EntityBattleManager::BattleData battleData = {
						player,
						squadF,
					};
					r->setCameraMatrix(glm::lookAt(glm::vec3(0.0f, 0.0f, 1000.0f), (glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f)));
					r->UpdateShaderData();
					cameraOffset.x = 0.0f;
					cameraOffset.y = 0.0f;
					battleManager.startBattle(battleData);
					return;
				}
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
	ItemLoader itemLoader;
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