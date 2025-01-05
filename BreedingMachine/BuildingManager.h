#pragma once
#include "Building.h"
#include "filesystem"
#include <unordered_map>
#include <fstream>

//enum {
//	ORKS = 0,
//	HUMANS = 1,
//	NOMADS = 2,
//	EVIL_HUMANS = 3,
//	GOBLINS = 4,
//};

class BuildingManager {
public:
	BuildingManager() {
		std::cout << "chujowyBuioldingManager\n";
	}
	BuildingManager(std::filesystem::path path) {
		int id, race, size, buildingType;
		glm::vec2 position;
		std::ifstream file;
		file.open(path.string());
		if (!file) {
			std::cout << "buildings.txt File not found\n";
			return;
		};
		file >> size;
		for (int i = 0; i < size; i++) {
			file >> position.x >> position.y >> id >> race >> buildingType;
			Building* building = new Building{ position, (uint8_t)buildingType };
			fBuildings[race].push_back(building);
			allBuildings.push_back(building);
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

private:
	std::unordered_map<uint8_t, std::vector<Building*>> fBuildings;
	std::vector<Building*> allBuildings;
};