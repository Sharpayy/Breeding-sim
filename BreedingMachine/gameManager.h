#pragma once
#include "Faction.h"
#include "MovementManager.h";
#include <array>
#include "inputHandler.h"
#include "BuildingManager.h"
#include "FactionManager.h"
#include <glm/vec2.hpp>
#include <glm/gtc/quaternion.hpp> 
#include "generator.h"
#include "timer.h"
#include "TextureLoader.h"
#include "EntityBattleManager.h"
#include <functional>
#include "EntityNames.h"
#include "timer.h"

#define THRESHOLD 10.0f

#define GAMETYPE_BIGMAP		1
#define GAMETYPE_FIGHT		2

class gameManager {
public:
	Inventory inv;

	gameManager(rasticore::RastiCoreRender* r_, rasticore::ModelCreationDetails rect_mcd, rasticore::VertexBuffer mapVao, rasticore::Program mapPrg) :
		itemLoader(ItemLoader()),
		inputHandlerInstance(InputHandler::getInstance()),
		generatorInstance(Generator::getInstance()),
		battleManager(r_, rect_mcd, mapPrg, mapVao, &inv, &draggedObj, &cameraOffset),
		buildingManager(std::filesystem::current_path() / "Data/buildings.txt", r_),
		timerInstance(Timer::getInstance())
	{
		this->rect_mcd = rect_mcd;
		this->r = r_;
		std::filesystem::path path = std::filesystem::current_path();
		std::filesystem::path collisionPath = path, buildingPath = path;
		collisionPath.append("Data\\collision.txt");
		buildingPath.append("Data\\buildings.txt");
		movementManager = SquadMovementManager{ collisionPath, 4096, 16, r_, rect_mcd };
		factionManager = FactionManager{r_, rect_mcd, 16};
		cameraOffset = CameraOffset{ 0, 0, 1.0f };
		initGame(path);
		game_type = GAMETYPE_BIGMAP;
	}
	
	void update() {
		//Update pressed keys
		inputHandlerInstance.handleKeys();

		if (game_type == GAMETYPE_BIGMAP)
		{
			//Key handler
			inputHandler();

			//Handling Squad Movement
			movementManager.update();
			
			//Handling Squad Logic
			handleSquadLogic();
			
			//Handle buildings items rotation and item shop window
			buildingManager.updateBuildingItemsRotation(&itemLoader);
			auto recruitShopActive = inv.isWindowActive(gui_windows.recruitShop);
			auto itemShopActive = inv.isWindowActive(gui_windows.itemShop);
			if (selectedItems.building &&  (recruitShopActive || itemShopActive) ) {
				if (selectedItems.building->newRotationOccured()) {
					if (draggedObj.draggedItem.item) {
						draggedObj.draggedItem.previousSlot->changeItem(draggedObj.draggedItem.item);
						draggedObj.draggedItem.item = nullptr;
						inv.SetCursorItemHold(nullptr);
					}
					selectedItems.building->setNewRotationState(false);
					if (itemShopActive) setShopItemsRotation(nullptr, nullptr, &selectedItems.building, &inv, gui_windows.itemShop);
					if (recruitShopActive) {
						inv.DisableWindow(gui_windows.characterWindow);
						selectedItems.entityItem = nullptr;
						setShopEntityRotation(nullptr, nullptr, &selectedItems.building, &inv, gui_windows.recruitShop);
					}
				}
			}

			//Render
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
		if (inputHandlerInstance.KeyPressed(SDL_SCANCODE_W)) {
			cameraOffset.y += 20;
		}
		if (inputHandlerInstance.KeyPressed(SDL_SCANCODE_S)) {
			cameraOffset.y -= 20;
		}
		if (inputHandlerInstance.KeyPressed(SDL_SCANCODE_A)) {
			cameraOffset.x -= 20;
		}
		if (inputHandlerInstance.KeyPressed(SDL_SCANCODE_D)) {
			cameraOffset.x += 20;
		}
		if (inputHandlerInstance.KeyPressed(SDL_SCANCODE_Q)) {
			cameraOffset.z *= 0.9f;
		}
		if (inputHandlerInstance.KeyPressed(SDL_SCANCODE_E)) {
			cameraOffset.z *= 1.1f;
		}
		if (inputHandlerInstance.KeyPressedOnce(SDL_SCANCODE_I))
		{
			if (!inv.isWindowActive(gui_windows.inventory)) {
				setInventory(nullptr, nullptr, &playerData.money, &inv, gui_windows.inventory);
				inv.ActivateWindow(gui_windows.inventory);
			}
			else inv.DisableWindow(gui_windows.inventory);
		}
		if (inputHandlerInstance.KeyPressedOnce(SDL_SCANCODE_V))
		{
			if (!inv.isWindowActive(gui_windows.partyView)) {
				setParty(nullptr, nullptr, &playerData.player, &inv, gui_windows.partyView);
				//inv.ActivateWindow(gui_windows.partyView);
			}
			else inv.DisableWindow(gui_windows.partyView);
		}

		if (inputHandlerInstance.KeyPressed(SDL_SCANCODE_R))
		{
			game_type = (!(game_type - 1) + 1);
			SDL_Delay(100);
		}
		if (inputHandlerInstance.KeyPressed(SDL_SCANCODE_LEFT)) {
			auto mp = getMousePosition();
			if (draggedObj.draggedWindow.win && !draggedObj.draggedWindow.wasPressed) {
				draggedObj.draggedWindow.wasPressed = true;
				auto dim = draggedObj.draggedWindow.win->getDim();
				int offsetX = abs(dim.position.x - mp.x);
				int offsetY = abs(dim.position.y - mp.y);
				draggedObj.draggedWindow.offset.x = offsetX;
				draggedObj.draggedWindow.offset.y = offsetY;
			}
			else if (draggedObj.draggedWindow.wasPressed) {
				//std::cout << offset << "\n";
				draggedObj.draggedWindow.win->changeWindowPosition(mp.x - draggedObj.draggedWindow.offset.x, mp.y - draggedObj.draggedWindow.offset.y);
			}
			else if (draggedObj.draggedItem.item && !draggedObj.draggedItem.wasPressed) {
				draggedObj.draggedItem.wasPressed = true;
				inv.SetCursorItemHold(draggedObj.draggedItem.item);
			}
			//else if (draggedObj.draggedItem.wasPressed) {
			//	inv.getSlot
			//	//std::cout << offset << "\n";
			//	//draggedObj.draggedWindow.win->changeWindowPosition(mp.x - draggedObj.draggedWindow.offset.x, mp.y - draggedObj.draggedWindow.offset.y);
			//}
		}
		else {
			auto mp = getMousePosition();
			if (draggedObj.draggedItem.item) {
				Inventory::Window* win = new Inventory::Window();
				Slot* slot = inv.getSlot(mp, win);
				draggedObj.draggedItem.previousSlot->changeItem(draggedObj.draggedItem.item);
				if (slot) {
					//if (win == gui_windows.characterWindow && draggedObj.draggedItem.win == gui_windows.inventory) {
					if (win == gui_windows.characterWindow && draggedObj.draggedItem.win == gui_windows.itemShop) {
						//if (inv.swapItems(slot, draggedObj.draggedItem.previousSlot)) {
						if(buyItem(slot)) equip_deequipItem(slot);
					}
					else if (win == gui_windows.itemShop && draggedObj.draggedItem.win == gui_windows.characterWindow) {
						if (sellItem(slot)) equip_deequipItem(slot);
					}
					else if (win == gui_windows.characterWindow && draggedObj.draggedItem.win == gui_windows.inventory) {
						if(inv.swapItems(slot, draggedObj.draggedItem.previousSlot)) equip_deequipItem(slot);
					}
					else if (win == gui_windows.inventory && draggedObj.draggedItem.win == gui_windows.characterWindow) {
						if (inv.swapItems(slot, draggedObj.draggedItem.previousSlot)) equip_deequipItem(draggedObj.draggedItem.previousSlot);
					}
					else if (win == gui_windows.characterWindow && draggedObj.draggedItem.win == gui_windows.characterWindow) {
						if (inv.swapItems(slot, draggedObj.draggedItem.previousSlot)) {
							equip_deequipItem(draggedObj.draggedItem.previousSlot);
							equip_deequipItem(slot);
						}
					}
					else if (draggedObj.draggedItem.win == gui_windows.itemShop && win == gui_windows.itemShop) {
						//SWAP ITEMS IN ITEM SHOP
						if (!inv.swapItems(slot, draggedObj.draggedItem.previousSlot)) return;
						int idx0, idx1, buildingItemsAmount;
						buildingItemsAmount = selectedItems.building->getItemsRotation().size();
						idx0 = gui_windows.itemShop->getSlotIndex(slot);
						idx1 = gui_windows.itemShop->getSlotIndex(draggedObj.draggedItem.previousSlot);
						if (!(idx0 >= buildingItemsAmount || idx1 >= buildingItemsAmount))
							selectedItems.building->swapByValue(idx0, idx1);
						setShopItemsRotation(nullptr, nullptr, &selectedItems.building, &inv, gui_windows.itemShop);
					}
					else if (draggedObj.draggedItem.win == gui_windows.itemShop && win == gui_windows.inventory) {
						buyItem(slot);
					}
					else if (draggedObj.draggedItem.win == gui_windows.inventory && win == gui_windows.itemShop) {
						sellItem(slot);
					}
					else if (draggedObj.draggedItem.win == gui_windows.inventory && win == gui_windows.inventory) {
						inv.swapItems(slot, draggedObj.draggedItem.previousSlot);
					}
				}
			}
			draggedObj.draggedWindow = {};
			draggedObj.draggedItem = {};
			inv.SetCursorItemHold(nullptr);
		}
		if (inputHandlerInstance.KeyPressedOnce(SDL_SCANCODE_LEFT)) {
			auto mp = getMousePosition();

			//GUI
			if (inv.isGuiClicked(mp)) {
				Inventory::Window* win = inv.setPressedWindowOnTop(mp);

				Slot* slot = inv.getSlot(mp, draggedObj.draggedItem.win);
				
				if (slot) {
					//if (slot->getItem()) {
					//	if (slot->getItem()->getObjectType() & ENTITY) {
					//		selectedItems.entityItem = (EntityItem*)slot->getItem();
					//	}
					//}
					draggedObj.draggedItem.item = slot->getItem();
					slot->changeItem(nullptr);
					draggedObj.draggedItem.previousSlot = slot;
					if (win != gui_windows.recruitShop && slot->getSlotType() != ENTITY) win->getGWindow()->CollisionCheck(mp.x, mp.y);
				}
				else win->getGWindow()->CollisionCheck(mp.x, mp.y);
			}
			else {
				auto mousePos = getCorrectedMousePosition();
				if (inputHandlerInstance.KeyPressed(SDL_SCANCODE_LALT)) {
					for (auto& building : buildingManager.getAllBuildings()) {
						auto buildingPos = building->getPosition();
						if (glm::distance(mousePos, buildingPos) < 64.0f) {
							inv.ActivateWindow(gui_windows.interaction);
							buildingPos = getOnScreenPosition(buildingPos);
							selectedItems.building = building;
							gui_windows.interaction->changeWindowPosition(buildingPos.x + 128, buildingPos.y - 64);
						}
					}
					
				}
				else {

					movementManager.createSquadPath(Astar::point{ (int)mousePos.x, (int)mousePos.y }, playerData.player);
				}
			}
		}
		if (inputHandlerInstance.KeyPressedOnce(SDL_SCANCODE_RIGHT)) {
			auto mp = getMousePosition();
			if (inv.isGuiClicked(mp)) {
				Inventory::Window* win;
				Slot* slot = inv.getSlot(mp, win);
				if (slot) {
					if (slot->getSlotType() == ENTITY) {
						selectedItems.entityItem = (EntityItem*)slot->getItem();
						win->getGWindow()->CollisionCheck(mp.x, mp.y);
					}
					
				}
			}
			else {
				auto pos = getCorrectedMousePosition();
				pos.x = (int)(pos.x / 16) * 16;
				pos.y = (int)(pos.y / 16) * 16;
				playerData.player->setSquadPosition(pos);
				uint64_t id = playerData.player->getSquadID();
				r->BindActiveModel(LONG_GET_MODEL(id));
				r->SetObjectMatrix(LONG_GET_OBJECT(id), glm::translate(glm::mat4{ 1.0f }, glm::vec3{ playerData.player->getSquadPosition(), 1.1f }), true);
			}
		}
	}

	bool buyItem(Slot* slot) {
		int draggedItemPrice, possibleItemPrice = 0, finallPrice;
		draggedItemPrice = draggedObj.draggedItem.item->getItemPrice();
		finallPrice = draggedItemPrice;
		if (!slot->getItem()) {
			if (playerData.money >= finallPrice) {
				if (!inv.swapItems(slot, draggedObj.draggedItem.previousSlot)) return false;
				selectedItems.building->eraseItemFromRotation(slot->getItem());
				setShopItemsRotation(nullptr, nullptr, &selectedItems.building, &inv, gui_windows.itemShop);
				playerData.money -= finallPrice;
				setInventory(nullptr, nullptr, &playerData.money, &inv, gui_windows.inventory);
				return true;
			}
		}
		else {
			possibleItemPrice = slot->getItem()->getItemPrice();
			finallPrice = draggedItemPrice - possibleItemPrice;
			if (playerData.money >= finallPrice) {
				if (!inv.swapItems(slot, draggedObj.draggedItem.previousSlot)) return false;
				int idx0 = gui_windows.itemShop->getSlotIndex(draggedObj.draggedItem.previousSlot);
				selectedItems.building->addSingleItemToRotation(draggedObj.draggedItem.previousSlot->getItem(), idx0);
				selectedItems.building->eraseItemFromRotation(slot->getItem());

				setShopItemsRotation(nullptr, nullptr, &selectedItems.building, &inv, gui_windows.itemShop);
				playerData.money -= finallPrice;
				setInventory(nullptr, nullptr, &playerData.money, &inv, gui_windows.inventory);
				return true;
			}
		}
		return false;
	}

	bool sellItem(Slot* slot) {
		int draggedItemPrice, possibleItemPrice = 0, finallPrice;
		draggedItemPrice = draggedObj.draggedItem.item->getItemPrice();
		finallPrice = -draggedItemPrice;
		if (!slot->getItem()) {
			if (playerData.money >= finallPrice) {
				if (!inv.swapItems(slot, draggedObj.draggedItem.previousSlot)) return false;
				selectedItems.building->addSingleItemToRotation(slot->getItem(), -1);

				setShopItemsRotation(nullptr, nullptr, &selectedItems.building, &inv, gui_windows.itemShop);
				playerData.money -= finallPrice;
				setInventory(nullptr, nullptr, &playerData.money, &inv, gui_windows.inventory);
				return true;
			}
		}
		else {
			possibleItemPrice = slot->getItem()->getItemPrice();
			finallPrice = -draggedItemPrice + possibleItemPrice;
			if (playerData.money >= finallPrice) {
				if (!inv.swapItems(slot, draggedObj.draggedItem.previousSlot)) return false;
				int idx0 = gui_windows.itemShop->getSlotIndex(slot);
				selectedItems.building->addSingleItemToRotation(slot->getItem(), idx0);

				selectedItems.building->eraseItemFromRotation(draggedObj.draggedItem.previousSlot->getItem());

				setShopItemsRotation(nullptr, nullptr, &selectedItems.building, &inv, gui_windows.itemShop);
				playerData.money -= finallPrice;
				setInventory(nullptr, nullptr, &playerData.money, &inv, gui_windows.inventory);
				return true;
			}
		}
	}

	void equip_deequipItem(Slot* slot) {
		int idx;
		auto items = selectedItems.entityItem->getEntity()->getEquipedItems();
		switch (slot->getSlotType())
		{
		case HELMET:
			items.helmet = (ArmorItem*)slot->getItem();
			break;
		case CHESTPLATE:
			items.Chestplate = (ArmorItem*)slot->getItem();
			break;
		case LEGS:
			items.Legs = (ArmorItem*)slot->getItem();
			break;
		case BOOTS:
			items.Boots = (ArmorItem*)slot->getItem();
			break;
		case SHIELD:
			items.shield = (ArmorItem*)slot->getItem();
		case WEAPON:
			items.weapon = (WeaponItem*)slot->getItem();
			break;
		default:
			break;
		}
		//selectedItems.building->getItemsRotation();
	}

	glm::vec2 getOnScreenPosition(glm::vec2 p)
	{
		glm::vec4 res = r->MVP.matProjCamera * glm::vec4(p.x, p.y, 0.0f, 1.0f);

		return glm::vec2((res.x * MAP_WIDTH) / (2.0 * res.w) + MAP_WIDTH / 2.0f, (-res.y * MAP_HEIGHT) / (2.0 * res.w) + MAP_HEIGHT / 2.0f);
	}

	glm::vec2 getMousePosition() {
		int x, y;
		SDL_GetMouseState(&x, &y);
		return glm::vec2{ x,y };
	}

	glm::vec2 getCorrectedMousePosition(glm::vec2 v = glm::vec2((float)0xbadc0ffe)) {
		glm::vec2 mousePos;
		if (v == glm::vec2((float)0xbadc0ffe))
			mousePos = getMousePosition();
		else
			mousePos = v;
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

	CameraOffset getCameraOffset() {
		return cameraOffset;
	}

private:
	void initItems() {
		//Weaponry
		stbi_set_flip_vertically_on_load(false);
		WeaponItem* bastard_sword = new WeaponItem{"bastard sword", (void*) LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC2.png","EquipmentIconsC2.png"), MELEE, new WeaponItem::ObjectStatistic{4}, 83, TIER_2};
		itemLoader.loadItem(bastard_sword);
		WeaponItem* spear = new WeaponItem{ "spear", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC61.png","EquipmentIconsC61"), MELEE, new WeaponItem::ObjectStatistic{3}, 80, TIER_1 };
		itemLoader.loadItem(spear);
		WeaponItem* hatchet = new WeaponItem{ "hatchet", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC57.png","EquipmentIconsC57"), MELEE, new WeaponItem::ObjectStatistic{2}, 60, TIER_1 };
		itemLoader.loadItem(hatchet);
		WeaponItem* twin_daggers = new WeaponItem{ "twin daggers", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC57.png","EquipmentIconsC57"), MELEE, new WeaponItem::ObjectStatistic{4.5f}, 102, TIER_2 };
		itemLoader.loadItem(twin_daggers);
		WeaponItem* short_bow = new WeaponItem{ "short bow", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC103.png","EquipmentIconsC103"), RANGED, new WeaponItem::ObjectStatistic{3}, 78, TIER_1};
		itemLoader.loadItem(short_bow);
		WeaponItem* recurve_bow = new WeaponItem{ "recurve bow", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC118.png","EquipmentIconsC118"), RANGED, new WeaponItem::ObjectStatistic{4}, 88, TIER_2 };
		itemLoader.loadItem(recurve_bow);
		WeaponItem* crossbow = new WeaponItem{ "crossbow", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC121.png","EquipmentIconsC121"), RANGED, new WeaponItem::ObjectStatistic{6}, 160, TIER_3};
		itemLoader.loadItem(crossbow);
		WeaponItem* morningstar = new WeaponItem{ "morningstar", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC29.png","EquipmentIconsC29"), MELEE, new WeaponItem::ObjectStatistic{5}, 124, TIER_2 };
		itemLoader.loadItem(morningstar);
		WeaponItem* flail = new WeaponItem{ "flail", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC15.png","EquipmentIconsC15"), MELEE, new WeaponItem::ObjectStatistic{3.5f}, 62, TIER_1 };
		itemLoader.loadItem(flail);
		WeaponItem *sickle_blade = new WeaponItem{ "sickle_blade", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC15.png","EquipmentIconsC15"), MELEE, new WeaponItem::ObjectStatistic{4}, 70, TIER_1 };
		itemLoader.loadItem(sickle_blade);
		WeaponItem *berserker_blade = new WeaponItem{ "berserker blade", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC13.png","EquipmentIconsC13"), MELEE, new WeaponItem::ObjectStatistic{8}, 203, TIER_3 };
		itemLoader.loadItem(berserker_blade);
		WeaponItem *battlehammer = new WeaponItem{ "battlehammer", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC31.png","EquipmentIconsC31"), MELEE, new WeaponItem::ObjectStatistic{6}, 148, TIER_3};
		itemLoader.loadItem(battlehammer);
		WeaponItem *trident = new WeaponItem{ "trident", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC65.png","EquipmentIconsC65"), MELEE, new WeaponItem::ObjectStatistic{5}, 132, TIER_3 };
		itemLoader.loadItem(trident);
		WeaponItem *battleaxe = new WeaponItem{ "battleaxe", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC49.png","EquipmentIconsC49"), MELEE, new WeaponItem::ObjectStatistic{4}, 112,  TIER_2};
		itemLoader.loadItem(battleaxe);
		WeaponItem *excalibur = new WeaponItem{ "excalibur", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC19.png","EquipmentIconsC19"), MELEE, new WeaponItem::ObjectStatistic{10}, 300,  TIER_3 };
		itemLoader.loadItem(excalibur);
		//Armory
		ArmorItem *iron_chestplate = new ArmorItem{ "iron chestplate", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC193.png","EquipmentIconsC193"), CHESTPLATE, new ArmorItem::ObjectStatistic{5}, 132, TIER_2 };
		itemLoader.loadItem(iron_chestplate);
		ArmorItem *iron_greaves = new ArmorItem{ "iron greaves", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC217.png","EquipmentIconsC217"), LEGS, new ArmorItem::ObjectStatistic{3}, 90, TIER_2 };
		itemLoader.loadItem(iron_greaves);
		ArmorItem *iron_cap = new ArmorItem{ "iron cap", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC166.png","EquipmentIconsC166"), HELMET, new ArmorItem::ObjectStatistic{3}, 90, TIER_2 };
		itemLoader.loadItem(iron_cap);
		ArmorItem *iron_boots = new ArmorItem{ "iron boots", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC237.png","EquipmentIconsC237"), BOOTS, new ArmorItem::ObjectStatistic{2}, 52, TIER_2 };
		itemLoader.loadItem(iron_boots);
		ArmorItem *tower_shield = new ArmorItem{ "tower shield", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC152.png","EquipmentIconsC152"), SHIELD, new ArmorItem::ObjectStatistic{4}, 67, TIER_2 };
		itemLoader.loadItem(tower_shield);

		ArmorItem *guardian_helmet = new ArmorItem{ "guardian helmet", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC179.png","EquipmentIconsC179"), HELMET, new ArmorItem::ObjectStatistic{5}, 129, TIER_3 };
		itemLoader.loadItem(guardian_helmet);
		ArmorItem *guardian_chestplate = new ArmorItem{ "guardian chestplate", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC199.png","EquipmentIconsC199"), CHESTPLATE, new ArmorItem::ObjectStatistic{7}, 164, TIER_3 };
		itemLoader.loadItem(guardian_chestplate);
		ArmorItem *guardian_greaves = new ArmorItem{ "guardian greaves", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC216.png","EquipmentIconsC216"), LEGS, new ArmorItem::ObjectStatistic{4}, 102, TIER_3 };
		itemLoader.loadItem(guardian_greaves);
		ArmorItem *guardian_boots = new ArmorItem{ "guardian boots", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC231.png","EquipmentIconsC231"), BOOTS, new ArmorItem::ObjectStatistic{3}, 73, TIER_3 };
		itemLoader.loadItem(guardian_boots);


		ArmorItem *copper_helmet = new ArmorItem{ "copper helmet", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC163.png","EquipmentIconsC163"), HELMET, new ArmorItem::ObjectStatistic{3}, 64, TIER_2 };
		itemLoader.loadItem(copper_helmet);
		ArmorItem *copper_chestplate = new ArmorItem{ "copper chestplate", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC183.png","EquipmentIconsC183"), CHESTPLATE, new ArmorItem::ObjectStatistic{4}, 89, TIER_2 };
		itemLoader.loadItem(copper_chestplate);
		ArmorItem *copper_greaves = new ArmorItem{ "copper greaves", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC204.png","EquipmentIconsC204"), LEGS, new ArmorItem::ObjectStatistic{2}, 56, TIER_2 };
		itemLoader.loadItem(copper_greaves);
		ArmorItem *copper_boots = new ArmorItem{ "copper boots", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC222.png","EquipmentIconsC222"), BOOTS, new ArmorItem::ObjectStatistic{1.5f}, 31, TIER_2 };
		itemLoader.loadItem(copper_boots);
		ArmorItem *buckler = new ArmorItem{ "buckler", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC152.png","EquipmentIconsC152"), SHIELD, new ArmorItem::ObjectStatistic{1.5f}, 20, TIER_1 };
		itemLoader.loadItem(buckler);

		ArmorItem *cap = new ArmorItem{ "cap", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC161.png","EquipmentIconsC161"), HELMET, new ArmorItem::ObjectStatistic{0.3f}, 14, TIER_1};
		itemLoader.loadItem(cap);
		ArmorItem *clothes = new ArmorItem{ "clothes", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC181.png","EquipmentIconsC181"), CHESTPLATE, new ArmorItem::ObjectStatistic{0.5f}, 21, TIER_1 };
		itemLoader.loadItem(clothes);
		ArmorItem *rags = new ArmorItem{ "rags", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC202.png","EquipmentIconsC202"), LEGS, new ArmorItem::ObjectStatistic{0.2f}, 8, TIER_1 };
		itemLoader.loadItem(rags);
		ArmorItem *shoes = new ArmorItem{ "shoes", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC223.png","EquipmentIconsC223"), BOOTS, new ArmorItem::ObjectStatistic{0.1f}, 4, TIER_1 };
		itemLoader.loadItem(shoes);
		ArmorItem *wooden_shield = new ArmorItem{ "wooden shield", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC152.png","EquipmentIconsC152"), SHIELD, new ArmorItem::ObjectStatistic{1}, 15, TIER_1 };
		itemLoader.loadItem(wooden_shield);


		ArmorItem *darkwraith_helmet = new ArmorItem{ "darkwraith helmet", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC176.png","EquipmentIconsC176"), HELMET, new ArmorItem::ObjectStatistic{5}, 118, TIER_3 };
		itemLoader.loadItem(darkwraith_helmet);
		ArmorItem *darkwraith_chestplate = new ArmorItem{ "darkwraith chestplate", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC191.png","EquipmentIconsC191"), CHESTPLATE, new ArmorItem::ObjectStatistic{7}, 171, TIER_3 };
		itemLoader.loadItem(darkwraith_chestplate);
		ArmorItem *darkwraith_greaves = new ArmorItem{ "darkwraith greaves", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC207.png","EquipmentIconsC207"), LEGS, new ArmorItem::ObjectStatistic{4}, 104, TIER_3 };
		itemLoader.loadItem(darkwraith_greaves);
		ArmorItem *darkwraith_boots = new ArmorItem{ "darkwraith boots", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC227.png","EquipmentIconsC227"), BOOTS, new ArmorItem::ObjectStatistic{3}, 74, TIER_3 };
		itemLoader.loadItem(darkwraith_boots);
		ArmorItem *darkwraith_shield = new ArmorItem{ "darkwraith shield", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC150.png","EquipmentIconsC150"), SHIELD, new ArmorItem::ObjectStatistic{5}, 95, TIER_3 };
		itemLoader.loadItem(darkwraith_shield);

		ArmorItem *copper_cap = new ArmorItem{ "copper cap", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC162.png","EquipmentIconsC162"), HELMET, new ArmorItem::ObjectStatistic{2}, 31, TIER_1 };
		itemLoader.loadItem(copper_cap);
		ArmorItem *copper_vest = new ArmorItem{ "copper vest", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC182.png","EquipmentIconsC182"), CHESTPLATE, new ArmorItem::ObjectStatistic{3}, 54, TIER_1 };
		itemLoader.loadItem(copper_vest);
		ArmorItem *leather_greaves = new ArmorItem{ "leather greaves", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC203.png","EquipmentIconsC203"), LEGS, new ArmorItem::ObjectStatistic{2}, 36, TIER_1 };
		itemLoader.loadItem(leather_greaves);
		ArmorItem *leather_boots = new ArmorItem{ "leather boots", (void*)LoadTextureFromFile("Data\\Equipment Icons\\EquipmentIconsC221.png","EquipmentIconsC221"), BOOTS, new ArmorItem::ObjectStatistic{1}, 21, TIER_1 };
		itemLoader.loadItem(leather_boots);

		//ArmorItem* helmet = nullptr;
		//ArmorItem* Chestplate = nullptr;
		//ArmorItem* Legs = nullptr;
		//ArmorItem* Boots = nullptr;
		//ArmorItem* shield = nullptr;
		//WeaponItem* weapon = nullptr;


		//weak sets
		Entity::EquipedItems setWeakAxe = {
			copper_cap,
			copper_vest,
			leather_greaves,
			leather_boots,
			nullptr,
			hatchet

		};
		itemLoader.loadSet(setWeakAxe);
		Entity::EquipedItems setWeakSpear = {
			cap,
			clothes,
			rags,
			shoes,
			wooden_shield,
			spear

		};
		itemLoader.loadSet(setWeakSpear);
		Entity::EquipedItems setWeakArcher = {
			cap,
			copper_vest,
			rags,
			shoes,
			nullptr,
			short_bow
		};
		itemLoader.loadSet(setWeakArcher);
		Entity::EquipedItems setSickleGuy = {
			copper_cap,
			copper_vest,
			rags,
			shoes,
			nullptr,
			sickle_blade

		};
		itemLoader.loadSet(setWeakArcher);
		//Mid sets
		Entity::EquipedItems setIronTank = {
			iron_cap,
			iron_chestplate,
			iron_greaves,
			iron_boots,
			tower_shield,
			bastard_sword

		};
		itemLoader.loadSet(setIronTank);
		Entity::EquipedItems setCopperDPS = {
			copper_helmet,
			copper_chestplate,
			copper_greaves,
			copper_boots,
			wooden_shield,
			battleaxe

		};
		itemLoader.loadSet(setCopperDPS);
		Entity::EquipedItems setIronCrossbow = {
			iron_cap,
			iron_chestplate,
			iron_greaves,
			iron_boots,
			nullptr,
			crossbow
		};
		itemLoader.loadSet(setIronCrossbow);
		//stronk
		Entity::EquipedItems setDarkwraith = {
			darkwraith_helmet,
			darkwraith_chestplate,
			darkwraith_greaves,
			darkwraith_boots,
			nullptr,
			berserker_blade

		};
		itemLoader.loadSet(setDarkwraith);
		Entity::EquipedItems setGuardian = {
			guardian_helmet,
			guardian_chestplate,
			guardian_greaves,
			guardian_boots,
			tower_shield,
			battlehammer
		};
		itemLoader.loadSet(setIronCrossbow);
		//random sets
		Entity::EquipedItems setBerserk = {
			darkwraith_helmet,
			nullptr,
			darkwraith_greaves,
			darkwraith_boots,
			nullptr,
			berserker_blade
		};
		itemLoader.loadSet(setBerserk);
		Entity::EquipedItems setCheckOutMyBoots = {
			nullptr,
			nullptr,
			rags,
			guardian_boots,
			nullptr,
			hatchet
		};
		itemLoader.loadSet(setCheckOutMyBoots);
		Entity::EquipedItems setHeavyCrossbow = {
			nullptr,
			guardian_chestplate,
			guardian_greaves,
			guardian_boots,
			tower_shield,
			crossbow
		};
		itemLoader.loadSet(setHeavyCrossbow);
		Entity::EquipedItems setTridentGuy = {
			cap,
			iron_chestplate,
			copper_greaves,
			copper_boots,
			wooden_shield,
			trident
		};
		itemLoader.loadSet(setTridentGuy);
		Entity::EquipedItems setRecurvedArcher = {
			cap,
			clothes,
			rags,
			copper_boots,
			wooden_shield,
			recurve_bow
		};
		itemLoader.loadSet(setRecurvedArcher);
		Entity::EquipedItems setFlailGuy = {
			iron_cap,
			copper_vest,
			rags,
			copper_boots,
			buckler,
			flail
		};
		itemLoader.loadSet(setFlailGuy);
		Entity::EquipedItems setChosenOne = {
			nullptr,
			nullptr,
			nullptr,
			shoes,
			nullptr,
			excalibur
		};
		itemLoader.loadSet(setChosenOne);
		Entity::EquipedItems setTwinDaggers = {
			cap,
			darkwraith_chestplate,
			rags,
			iron_boots,
			nullptr,
			twin_daggers
		};
		itemLoader.loadSet(setTwinDaggers);


		stbi_set_flip_vertically_on_load(true);
	}

	void initOverworldHud() {
		uint64_t texButton = 0;
		int y = MAP_HEIGHT - 40;
		int x = (MAP_WIDTH) / 4;
		int buttonWidth = 100;
		int offset = 10;
		Inventory::Window* win = inv.AddWindow("overworld_hud", ObjectDim{ {0, y},  int(MAP_WIDTH), 40 }, 2, LoadTextureFromFile("Data\\gui.png"), 1);
		auto gwin = win->getGWindow();
		GComponent* c;
		//labelka z iloœci¹ ch³opa
		gwin->AddComponent(new GComponentLabel(glm::vec2(100, 20), glm::vec3(x, y, 1.0f), "Squad count:"));
		c = new GComponentLabel(glm::vec2(10, 20), glm::vec3(x + 105, y, 1.0f), "0/16");
		AddNamedComponent(c, "squad_count");
		gwin->AddComponent(c);
		x += 150 + 50;
		gwin->AddComponent(new GComponentButton(glm::vec2(buttonWidth, 20), glm::vec3(x, y, 0.1f), "Party", texButton));
		x += buttonWidth + offset;
		gwin->AddComponent(new GComponentButton(glm::vec2(buttonWidth, 20), glm::vec3(x, y, 0.1f), "Inventory", texButton));
		x += buttonWidth + offset;
		gwin->AddComponent(new GComponentButton(glm::vec2(buttonWidth, 20), glm::vec3(x, y, 0.1f), "Settings", texButton));
		x += buttonWidth + offset;
		gwin->AddComponent(new GComponentButton(glm::vec2(buttonWidth, 20), glm::vec3(x, y, 0.1f), "Exit", texButton));
		
		inv.ActivateWindow(win);
		gui_windows.overworldHud = win;
	}
	void initBattleHud() {
		uint64_t texButton = LoadTextureFromFile("Data\\red.png");
		int y = MAP_HEIGHT - 40;
		int x = (MAP_WIDTH) / 6;
		int buttonWidth = 100;
		int labelWidth = 115;
		int offsetButton = 5;
		int offsetLabel = 80;
		Inventory::Window* win = inv.AddWindow("battle_hud", ObjectDim{ {0, y},  int(MAP_WIDTH), 40 }, 2, LoadTextureFromFile("Data\\gui.png"), 1);
		auto gwin = win->getGWindow();
		GComponent* c;
		gwin->AddComponent(new GComponentLabel(glm::vec2(labelWidth, 20), glm::vec3(x, y, 1.0f), "Current turn: "));
		c = new GComponentLabel(glm::vec2(50, 20), glm::vec3(x + labelWidth, y, 0.01f), "Player");
		AddNamedComponent(c, "current_turn_label");
		gwin->AddComponent(c);
		x += labelWidth + offsetLabel;
		gwin->AddComponent(new GComponentLabel(glm::vec2(labelWidth, 20), glm::vec3(x, y, 0.1f), "Turns passed: "));
		c = new GComponentLabel(glm::vec2(50, 20), glm::vec3(x + labelWidth, y, 0.01f), "0");
		AddNamedComponent(c, "turns_passed_label");
		gwin->AddComponent(c);
		x += labelWidth + offsetLabel - 40;
		gwin->AddComponent(new GComponentLabel(glm::vec2(labelWidth, 20), glm::vec3(x, y, 0.1f), "Enemy units: "));
		c = new GComponentLabel(glm::vec2(50, 20), glm::vec3(x + labelWidth, y, 0.01f), "0");
		AddNamedComponent(c, "enemy_units_label");
		gwin->AddComponent(c);
		x += labelWidth + offsetLabel - 40;
		gwin->AddComponent(new GComponentLabel(glm::vec2(labelWidth, 20), glm::vec3(x, y, 0.1f), "Player units: "));
		c = new GComponentLabel(glm::vec2(50, 20), glm::vec3(x + labelWidth, y, 0.01f), "0");
		AddNamedComponent(c, "player_units_label");
		gwin->AddComponent(c);
		x += labelWidth + offsetButton + 30;
		gwin->AddComponent(new GComponentButton(glm::vec2(buttonWidth, 20), glm::vec3(x, y, 0.1f), "Settings", texButton));
		x += buttonWidth + offsetButton;
		gwin->AddComponent(new GComponentButton(glm::vec2(buttonWidth, 20), glm::vec3(x, y, 0.1f), "Exit", texButton));

		//inv.ActivateWindow(win);
		gui_windows.battleHud = win;
	}
	void initShopItems(int width, int height, uint64_t texItemFrame) {
		Inventory::Window* win = inv.AddWindow("shop_items", ObjectDim{ {0, 0}, width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		
		auto gwin = win->getGWindow();
		GComponentButton* drag = new GComponentButton(glm::vec2(width - 21, 20), glm::vec3(0, 0, 0.1f), "Shop", LoadTextureFromFile("Data\\red.png"));
		drag->callback = std::bind(SetDraggedWindow, std::placeholders::_1, std::placeholders::_2, &draggedObj.draggedWindow, win);
		gwin->AddComponent(drag);
		GComponentButton* exit = new GComponentButton(glm::vec2(20, 20), glm::vec3(width - 20, 0, 0.1f), "X", LoadTextureFromFile("Data\\red.png"));
		exit->callback = std::bind(DisableWindow, std::placeholders::_1, std::placeholders::_2, &inv, win);
		gwin->AddComponent(exit);
		GComponent* c;
		int counter = 0;
		std::string labelName;
		for (int i = 35; i < height - 35; i += 50) {
			for (int j = 5; j < width - 15; j += 35) {
				win->AddSlotToWindow(Slot(nullptr, glm::vec2(j, i), 30.0f, 30.0f, EVERY_ITEM), texItemFrame);
				c = new GComponentLabel(glm::vec2(30, 10), glm::vec3(j, i + 35, 1.0f), "0");
				labelName = std::string("shopItem") + std::to_string(counter++);
				AddNamedComponent(c, labelName.c_str());
				gwin->AddComponent(c);
			}
		}

		gui_windows.itemShop = win;
		//inv.ActivateWindow(win);
	}
	
	void initInteractionViewer(int width, int height) {
		Inventory::Window* win = inv.AddWindow("interaction_viewer", ObjectDim{ {0,0} , width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		auto gwin = win->getGWindow();
		GComponentButton* drag = new GComponentButton(glm::vec2(width - 21, 20), glm::vec3(0, 0, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png"));
		drag->callback = std::bind(SetDraggedWindow, std::placeholders::_1, std::placeholders::_2, &draggedObj.draggedWindow, win);
		gwin->AddComponent(drag);
		GComponentButton* exit = new GComponentButton(glm::vec2(20, 20), glm::vec3(width - 20, 0, 0.1f), "X", LoadTextureFromFile("Data\\red.png"));
		exit->callback = std::bind(DisableWindow, std::placeholders::_1, std::placeholders::_2, &inv, win);
		gwin->AddComponent(exit);
		GComponentButton* tradeItems = new GComponentButton(glm::vec2(50, 30), glm::vec3(width / 2 - 30, 40, 0.1f), "Trade", LoadTextureFromFile("Data\\red.png"));
		tradeItems->callback = std::bind(setShopItemsRotation, std::placeholders::_1, std::placeholders::_2, &selectedItems.building, &inv, gui_windows.itemShop);
		gwin->AddComponent(tradeItems);
		GComponentButton* tradeEntities = new GComponentButton(glm::vec2(50, 30), glm::vec3(width / 2 - 30, 90, 0.1f), "Recruit", LoadTextureFromFile("Data\\red.png"));
		tradeEntities->callback = std::bind(setShopEntityRotation, std::placeholders::_1, std::placeholders::_2, &selectedItems.building, &inv, gui_windows.recruitShop);
		gwin->AddComponent(tradeEntities);
		gwin->AddComponent(new GComponentButton(glm::vec2(50, 30), glm::vec3(width / 2 - 30, 140, 0.1f), "Wait", LoadTextureFromFile("Data\\red.png")));
		//inv.ActivateWindow(win);
		gui_windows.interaction = win;

	}
	void initPrimaryInv(int width, int height, uint64_t texItemFrame) {
		Inventory::Window* win = inv.AddWindow("inventory", ObjectDim{ {0, 0}, width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		auto gwin = win->getGWindow();
		GComponentButton* drag = new GComponentButton(glm::vec2(width - 21, 20), glm::vec3(0, 0, 0.1f), "Inventory 1", LoadTextureFromFile("Data\\red.png"));
		drag->callback = std::bind(SetDraggedWindow, std::placeholders::_1, std::placeholders::_2, &draggedObj.draggedWindow, win);
		gwin->AddComponent(drag);
		//przyciski do zmiany na kolejny panel ekwipunku
		//gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(0, 5, 0.5f), nullptr, LoadTextureFromFile("Data\\red.png")));
		//gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(20 + 10, 5, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png")));
		//gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(40 + 20, 5, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png")));
		//gwin->AddComponent(new GComponentButton(glm::vec2(20, 20), glm::vec3(60 + 30, 5, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png")));
		//labelka z napisem inventory + które okienko
		//gwin->AddComponent(new GComponentButton(glm::vec2(1, 1), glm::vec3(140, 20, 0.1f), "Inventory 1", 0));
		//przycisk do zamkniêcia okienka
		GComponentButton* exit = new GComponentButton(glm::vec2(20, 20), glm::vec3(width - 20, 0, 0.1f), "X", LoadTextureFromFile("Data\\red.png"));
		exit->callback = std::bind(DisableWindow, std::placeholders::_1, std::placeholders::_2, &inv, win);
		gwin->AddComponent(exit);
		for (int i = 35; i < height - 75; i += 35) {
			for (int j = 5; j < width - 15; j += 35) {
				win->AddSlotToWindow(Slot(nullptr, glm::vec2(j, i), 30.0f, 30.0f, EVERY_ITEM), texItemFrame);
			}
		}
		//labelka z szmeklamis
		GComponent* c;
		gwin->AddComponent(new GComponentLabel(glm::vec2(70, 20), glm::vec3(width / 2 - 70, height - 35, 0.5f), "Denars: "));
		c = new GComponentLabel(glm::vec2(1, 20), glm::vec3(width / 2, height - 35, 0.5f), "0", false);
		AddNamedComponent(c, "playerMoney");
		gwin->AddComponent(c);
		gui_windows.inventory = win;
		//inv.ActivateWindow(win);

	}
	void initCharInv(int width, int height, uint64_t texItemFrame) {
		GComponent* c;
		Inventory::Window* win = inv.AddWindow("char_inv", ObjectDim{ {0,0} , width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		auto gwin = win->getGWindow();
		GComponentButton* drag = new GComponentButton(glm::vec2(width - 21, 20), glm::vec3(0, 0, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png"));
		drag->callback = std::bind(SetDraggedWindow, std::placeholders::_1, std::placeholders::_2, &draggedObj.draggedWindow, win);
		gwin->AddComponent(drag);
		//nazwa ch³opa
		c = new GComponentLabel(glm::vec2(20, 1), glm::vec3(width/2 - 20, 10, 0.1f), "null", true);
		AddNamedComponent(c, "Vname");
		gwin->AddComponent(c);
		//wyjœcie
		GComponentButton* exit = new GComponentButton(glm::vec2(20, 20), glm::vec3(width - 20, 0, 0.1f), "X", LoadTextureFromFile("Data\\red.png"));
		exit->callback = std::bind(DisableWindow, std::placeholders::_1, std::placeholders::_2, &inv, win);
		gwin->AddComponent(exit);
		int y = 30;
		//sloty
		win->AddSlotToWindow(Slot(nullptr, glm::vec2(width / 2 - 30, y), 30.0f, 30.0f, HELMET), texItemFrame);
		y += 30;
		win->AddSlotToWindow(Slot(nullptr, glm::vec2(width / 2 - 30, y), 30.0f, 30.0f, CHESTPLATE), texItemFrame);
		win->AddSlotToWindow(Slot(nullptr, glm::vec2(width / 2 - 30 - 30, y), 30.0f, 30.0f, SHIELD), texItemFrame);
		win->AddSlotToWindow(Slot(nullptr, glm::vec2(width / 2 - 30 + 30, y), 30.0f, 30.0f, WEAPON), texItemFrame);
		y += 30;
		win->AddSlotToWindow(Slot(nullptr, glm::vec2(width / 2 - 30, y), 30.0f, 30.0f, LEGS), texItemFrame);
		y += 30;
		win->AddSlotToWindow(Slot(nullptr, glm::vec2(width / 2 - 30, y), 30.0f, 30.0f, BOOTS), texItemFrame);
		//staty
		auto texBase = LoadTextureFromFile("Data\\blue.png");
		auto texFill = LoadTextureFromFile("Data\\purple.png");
		y += 40;
		c = new GComponentSlider(glm::vec2(80, 30), glm::vec3(10, y, 0.5f), "hp", texBase, texFill, true);
		AddNamedComponent(c, "Vhp");
		gwin->AddComponent(c);
		c = new GComponentSlider(glm::vec2(80, 30), glm::vec3(10 + 90, y, 0.5f), "stamina", texBase, texFill, true);
		AddNamedComponent(c, "Vstamina");
		gwin->AddComponent(c);
		c = new GComponentSlider(glm::vec2(80, 30), glm::vec3(100 + 90, y, 0.5f), "bravery", texBase, texFill, true);
		AddNamedComponent(c, "Vbravery");
		gwin->AddComponent(c);
		y += 40;
		c = new GComponentSlider(glm::vec2(80, 30), glm::vec3(10, y, 0.5f), "melee", texBase, texFill, true);
		AddNamedComponent(c, "Vmelee");
		gwin->AddComponent(c);
		c = new GComponentSlider(glm::vec2(80, 30), glm::vec3(10 + 90, y, 0.5f), "ranged", texBase, texFill, true);
		AddNamedComponent(c, "Vranged");
		gwin->AddComponent(c);
		c = new GComponentSlider(glm::vec2(80, 30), glm::vec3(100 + 90, y, 0.5f), "defense", texBase, texFill, true);
		AddNamedComponent(c, "Vdefense");
		gwin->AddComponent(c);
		//inv.ActivateWindow(win);
		gui_windows.characterWindow = win;
	}

	void initShopRecruits(int width, int height, uint64_t texItemFrame) {
		Inventory::Window* win = inv.AddWindow("shop_recruits", ObjectDim{ {0,0} , width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		auto gwin = win->getGWindow();

		//Drag
		GComponentButton* drag = new GComponentButton(glm::vec2(width - 21, 20), glm::vec3(0, 0, 0.1f), nullptr, LoadTextureFromFile("Data\\red.png"));
		drag->callback = std::bind(SetDraggedWindow, std::placeholders::_1, std::placeholders::_2, &draggedObj.draggedWindow, win);
		gwin->AddComponent(drag);
		//wyjœcie
		GComponentButton* exit = new GComponentButton(glm::vec2(20, 20), glm::vec3(width - 20, 0, 0.1f), "X", LoadTextureFromFile("Data\\red.png"));
		exit->callback = std::bind(DisableWindow, std::placeholders::_1, std::placeholders::_2, &inv, win);
		gwin->AddComponent(exit);

		GComponent* c;
		int counter = 0;
		std::string labelName;
		for (int i = 30; i < height - 90; i += 90) {
			for (int j = 10; j < width - 60; j += 60) {
				win->AddSlotToWindow(Slot(nullptr, glm::vec2(j, i), 60.0f, 60.0f, ENTITY), texItemFrame);
				c = new GComponentLabel(glm::vec2(20, 20), glm::vec3(j, i + 60, 0.1f), "0");
				labelName = std::string("shopEntityItem") + std::to_string(counter++);
				AddNamedComponent(c, labelName.c_str());
				gwin->AddComponent(c);

				//
				GComponentButton* button = new GComponentButton(glm::vec2(60.0f, 60.0f), glm::vec3(j, i, 0.0f), "", 0);
				button->callback = std::bind(getCharacterInventory_EI, std::placeholders::_1, std::placeholders::_2, &selectedItems.entityItem, &inv, gui_windows.characterWindow);
				gwin->AddComponent(button);
			}
		}

		gui_windows.recruitShop = win;
		//inv.ActivateWindow(win);
	}

	void initSquadViewer(int width, int height, uint64_t texItemFrame) {
		Inventory::Window* win = inv.AddWindow("party_view", ObjectDim{ {0,0} , width, height }, 2, LoadTextureFromFile("Data\\gui.png"));
		auto gwin = win->getGWindow();
		GComponentButton* drag = new GComponentButton(glm::vec2(width - 21, 20), glm::vec3(0, 0, 0.1f), nullptr, 0);
		drag->callback = std::bind(SetDraggedWindow, std::placeholders::_1, std::placeholders::_2, &draggedObj.draggedWindow, win);
		gwin->AddComponent(drag);
		//wyjœcie
		GComponentButton* exit = new GComponentButton(glm::vec2(20, 20), glm::vec3(width - 20, 0, 0.1f), "X", 0);
		exit->callback = std::bind(DisableWindow, std::placeholders::_1, std::placeholders::_2, &inv, win);
		gwin->AddComponent(exit);
		for (int i = 30; i < height - 90; i += 90) {
			for (int j = 10; j < width - 60; j += 60) {
				GComponentButton* button = new GComponentButton(glm::vec2(60.0f, 60.0f), glm::vec3(j, i, 0.0f), "", 0);
				gwin->AddComponent(button);
				button->callback = std::bind(getCharacterInventory_EI, std::placeholders::_1, std::placeholders::_2, &selectedItems.entityItem, &inv, gui_windows.characterWindow);
				win->AddSlotToWindow(Slot(nullptr, glm::vec2(j, i), 60.0f, 60.0f, ENTITY), texItemFrame);
			}
		}
		gui_windows.partyView = win;
		//inv.ActivateWindow(win);
	}

	void initGame(std::filesystem::path path) {
		//DO TOTALNEJ ZMIANY
		path = path.append("Data\\buildings.txt");
		initItems();
		loadNames("Data\\entityNames\\evilhumans.txt", MODEL_EVIL_HUMANS);
		loadNames("Data\\entityNames\\gobbos.txt", MODEL_GOBLINS);
		loadNames("Data\\entityNames\\humans.txt", MODEL_HUMANS);
		loadNames("Data\\entityNames\\nomads.txt", MODEL_NOMADS);
		loadNames("Data\\entityNames\\orcs.txt", MODEL_ORKS);

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
		initOverworldHud();
		initBattleHud();
		initShopItems(300, 300, texItemFrame);
		initCharInv(300, 250, texItemFrame);
		initShopRecruits(300, 400, texItemFrame);
		initInteractionViewer(200, 200);
		initSquadViewer(300, 400, texItemFrame);
		initPrimaryInv(300, 400, texItemFrame);

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


		factionManager.setFactionsRelationships(MODEL_EVIL_HUMANS, MODEL_PLAYER, ENEMY);
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
		
		playerData.player = factionManager.CreateNewSquad(MODEL_PLAYER, glm::vec2(-1000.0f));
		playerData.player->force = 1.0f;

		Squad* squad;
		srand(time(NULL));
		playerData.player->setSquadState(STAND);
		int amount = 0;
		for (int i = 0; i < 32; i++) {
			auto buildings = buildingManager.getRaceBuildings(i % 8);
			if (i == MODEL_PLAYER || !buildings.size()) continue;
			squad = factionManager.CreateNewSquad(i % 8, buildings.at(rand() % buildings.size())->getPosition());
			if (squad) amount++;
			timerInstance.startMeasure(squad->getSquadID(), 0);
			squad->force = generatorInstance.getRandomNumber(10, 100);
		}
		std::cout << "Amount of squads: " << amount << "\n";

		//GUI INIT
		setSquadCompSize(nullptr, nullptr, &playerData.player->getSquadComp()->size);
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
			if (squadF == playerData.player) continue;
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
				glm::vec2 randomBuildingPosition = buildings.at(generatorInstance.getRandomNumber(0, buildingAmount - 1))->getPosition();
				movementManager.createSquadPath(Astar::point{ (int)randomBuildingPosition.x, (int)randomBuildingPosition.y }, squad);
			}
		}
		//IF BUILDINGS NOT FOUND ACT LIKE NORMAL WANDER STATE
		else {
			squad->setSquadState(WANDER);
			timerInstance.startMeasure(squad->getSquadID(), generatorInstance.getRandomNumber(20, 50));
		}
	}

	void SquadStand(Squad* squad) {

	}

	void SquadWander(Squad* squad, int distance) {
		while (!movementManager.SquadHasPath(squad)) {
			glm::vec2 squadPosition, newPosition;
			squadPosition = squad->getSquadPosition();
			int angle = generatorInstance.getRandomNumber(0, 360);
			glm::mat2 rotationMatrix = glm::mat2(
				glm::cos(angle), -glm::sin(angle),
				glm::sin(angle), glm::cos(angle)
			);
			newPosition = squadPosition + (rotationMatrix * glm::vec2(generatorInstance.getRandomNumber(32, distance)));
			movementManager.createSquadPath(Astar::point{ (int)newPosition.x, (int)newPosition.y }, squad);
		}
	}

	void handleSquadState(Squad* squadF, Squad* squadS) {
		float distance = calculateSquadViewDistance(squadF);
		if (factionManager.getFactionsRelationships(squadS->getSquadFactionID(), squadF->getSquadFactionID()) == ENEMY) {
			if (glm::distance(squadF->getSquadPosition(), squadS->getSquadPosition()) <= distance) {
				if (glm::distance(squadF->getSquadPosition(), playerData.player->getSquadPosition()) <= 4.0f) {
					game_type = GAMETYPE_FIGHT;
					EntityBattleManager::BattleData battleData = {
						playerData.player,
						squadF
					};
					r->setCameraMatrix(glm::lookAt(glm::vec3(0.0f, 0.0f, 1000.0f), (glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f)));
					r->UpdateShaderData();
					cameraOffset.x = 0.0f;
					cameraOffset.y = 0.0f;

					inv.DisableWindow(gui_windows.characterWindow);
					inv.DisableWindow(gui_windows.interaction);
					inv.DisableWindow(gui_windows.inventory);
					inv.DisableWindow(gui_windows.itemShop);
					inv.DisableWindow(gui_windows.recruitShop);
					inv.DisableWindow(gui_windows.partyView);
					inv.DisableWindow(gui_windows.overworldHud);

					inv.ActivateWindow(gui_windows.battleHud);
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
				timerInstance.resetTimer(squadF->getSquadID());
				return;
			}
		}
		if (!timerInstance.hasTimeElapsed(squadS->getSquadID())) return;
			
		//Default options
		int state = generatorInstance.getRandomNumber(0,2);
		switch (state)
		{
		case STAND:
			squadF->setSquadState(STAND);
			timerInstance.startMeasure(squadF->getSquadID(), generatorInstance.getRandomNumber(5, 10));
			break;
		case PATROL:
			squadF->setSquadState(PATROL);
			timerInstance.startMeasure(squadF->getSquadID(), generatorInstance.getRandomNumber(30, 60));
			break;
		case WANDER:
			squadF->setSquadState(WANDER);
			timerInstance.startMeasure(squadF->getSquadID(), generatorInstance.getRandomNumber(20, 50));
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

	//EntityItem generateRandomEntityItem(uint8_t factionID) {
	//	Stats entityStats = {
	//		getRandomNumber(5,20),
	//		getRandomNumber(2,10),
	//		getRandomNumber(5,20),
	//		getRandomNumber(100,200),
	//		5.1f,
	//		getRandomNumber(10,30) };
	//	Entity::EquipedItems* items = nullptr;
	//	Entity* entity = new Entity(getRandomFactionName(factionID), 0, entityStats, items);
	//	EntityItem entityItem{ entity, 0 };
	//	return entityItem;
	//}

private:
	rasticore::RastiCoreRender* r;
	rasticore::ModelCreationDetails rect_mcd;
	ItemLoader itemLoader;
	SquadMovementManager movementManager;
	BuildingManager buildingManager;
	FactionManager factionManager;
	EntityBattleManager battleManager;
	//std::vector<Squad*> squads;
	CameraOffset cameraOffset;
	InputHandler& inputHandlerInstance;

	std::unordered_map<uint64_t, glm::vec2> stateH;
	//
	Timer& timerInstance;
	Generator& generatorInstance;

	struct PlayerData {
		Squad* player;
		int money = 300;
	} playerData;

	struct GUI_Windows {
		Inventory::Window* partyView;
		Inventory::Window* inventory;
		Inventory::Window* interaction;
		Inventory::Window* itemShop;
		Inventory::Window* characterWindow;
		Inventory::Window* recruitShop;
		Inventory::Window* battleHud;
		Inventory::Window* overworldHud;
	} gui_windows;

	struct SelectedItems {
		Building* building;
		EntityItem* entityItem;
	} selectedItems;

	 DraggedObj draggedObj;

	uint32_t game_type;
};