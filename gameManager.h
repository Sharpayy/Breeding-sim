#pragma once
#include "Faction.h"
#include "MovementManager.h";
#include <array>
#include "inputHandler.h"
#include "fstream"

class gameManager {
public:
	gameManager() {
		std::filesystem::path path = std::filesystem::current_path();
		std::filesystem::path collisionPath = path;
		collisionPath.append("Data\\collision.txt");
		movementManager = MovementManager{ collisionPath, 4096, 16 };
		initGame(path);
	}
	
	void update() {
		movementManager.update();
	}

private:
	void initGame(std::filesystem::path path) {
		//DO TOTALNEJ ZMIANY
		path = path.append("Data\\buildings.txt");
		
		//enum {
		//	ORKS = 0,
		//	HUMANS = 1,
		//	NOMADS = 2,
		//	EVIL_HUMANS = 3,
		//	GOBLINS = 4,
		//};

		std::unordered_map<uint8_t, std::vector<Building>> fBuildings;
		int id, race, size;
		glm::vec2 position;
		std::ifstream file;
		file.open(path.string());
		if (!file) {
			std::cout << "buildings.txt File not found\n";
			return;
		};
		file >> size;
		for (int i = 0; i < size; i++) {
			file >> position.x >> position.y >> id >> race;
			fBuildings[race].push_back(Building{ position });
		}

		Faction orks = { "Orks",  fBuildings[0] };
		Faction humans = { "Humans", fBuildings[1] };
		Faction nomands = { "Nomads", fBuildings[2] };
		Faction evilHumans = { "EvilHumans", fBuildings[3] };
		Faction goblins = { "Dwards", fBuildings[4] };
		Faction animals = { "Animals" };
		Faction bandits = { "Bandits" };

		//Orks
		int i, chuj = 0;
		for (i = 0; i < 3; i++) {
			//Squad{ , }
			//squads.push_back(Squad{ chuj++, glm::vec2{(rand() % 4096) - 2048,  (rand() % 4096) - 2048 } });
		}
	}

	Squad* player;
	MovementManager movementManager;
	std::array<Faction, 5> factions;
	std::vector<Squad> squads;
	InputHandler inputManager;
};