#pragma once
#include "Building.h"
#include "filesystem"
#include <unordered_map>
#include <fstream>
#include "generator.h"
#include "timer.h"
#include "Rasticore/rasti_main.h"
//enum {
//	ORKS = 0,
//	HUMANS = 1,
//	NOMADS = 2,
//	EVIL_HUMANS = 3,
//	GOBLINS = 4,
//};

class BuildingManager {
public:
	BuildingManager(std::filesystem::path path = "", rasticore::RastiCoreRender* r = nullptr) :
	generatorInstance(Generator::getInstance()),
	timerInstance(Timer::getInstance())
	{
		int id, race, size, buildingType;
		glm::vec2 position;
		std::ifstream file;
		file.open(path.string());
		if (!file) {
			std::cout << "buildings.txt File not found\n";
			return;
		};
		r->newModel(MODEL_BUILDING, {}, {}, {}, {}, {}, 50);
		file >> size;
		for (int i = 0; i < size; i++) {
			file >> position.x >> position.y >> id >> race >> buildingType;
			uint64_t id;
			r->newObject(MODEL_BUILDING, {}, &id);
			Building* building = new Building{ id, position, (uint8_t)buildingType};
			timerInstance.startMeasure(id, 0);
			fBuildings[race].push_back(building);
			allBuildings.push_back(building);
		}
	}

	void updateBuildingItemsRotation(ItemLoader* itm) {
		for (auto& building : allBuildings) {
			if (timerInstance.hasTimeElapsed(building->getID())) {
				building->clearItems();
				building->clearEntityItems();
				setRandomRotation(building, itm);
				building->setNewRotationState(true);
				timerInstance.startMeasure(building->getID(), (float)SHOP_ROTATION_TIMER + generatorInstance.getRandomNumber(0, 10));
			}
		}
	}

	std::vector<Building*> getRaceBuildings(uint8_t raceID) {
		for (auto& building : fBuildings) {
			if (building.first == raceID) return building.second;
		}
		std::cout << "Buildings not found\n";
		return std::vector<Building*>{};
	}

	std::vector<Building*> getAllBuildings() {
		return allBuildings;
	}


	uint8_t getBuildingRace(Building* building) {
		uint8_t res = 0;
		for (auto& fbuilding : fBuildings) {
			for (auto& b : fbuilding.second) {
				if (b == building) return fbuilding.first;
			}
		}
		return res;
	}
private:
	void setRandomRotation(Building* building, ItemLoader* itm) {
		int itemAmount = 0;
		int entitiesAmount = 0;
		uint8_t type = 0;
		EntityItem* entityItem;
		switch (building->getBuildingType()) {
		case BUILDING_TYPE_VILLAGE:
			type = TIER_1;
			itemAmount = generatorInstance.getRandomNumber(5, 11);
			entitiesAmount = generatorInstance.getRandomNumber(1, 3);
			break;
		case BUILDING_TYPE_CASTLE:
			type = TIER_2;
			itemAmount = generatorInstance.getRandomNumber(10, 20);
			entitiesAmount = generatorInstance.getRandomNumber(3, 6);
			break;
		case BUILDING_TYPE_CITY:
			type = TIER_ALL;
			itemAmount = generatorInstance.getRandomNumber(18, 30);
			entitiesAmount = generatorInstance.getRandomNumber(7, 11);
			break;
		default:
			type = TIER_ALL;
			break;
		}
		for (int i = 0; i < itemAmount; i++) building->addSingleItemToRotation(itm->getRandomItem(type), itemAmount);

		for (int i = 0; i < entitiesAmount; i++) {
			Entity* entity = itm->generateRandomEntity(getBuildingRace(building));
			entityItem = new EntityItem(entity, itm->calculateEntityPrice(entity));
			building->addSingleEntityItemToRotation(entityItem, entitiesAmount);
		}
	}

	Generator& generatorInstance;
	Timer& timerInstance;

	std::unordered_map<uint8_t, std::vector<Building*>> fBuildings;
	std::vector<Building*> allBuildings;
};