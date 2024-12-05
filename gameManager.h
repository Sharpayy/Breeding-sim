#pragma once
#include "Faction.h"
#include "MovementManager.h";
#include <array>
#include "inputHandler.h"
#include "BuildingManager.h"

class gameManager {
public:
	gameManager() {
		std::filesystem::path path = std::filesystem::current_path();
		std::filesystem::path collisionPath = path, buildingPath = path;
		collisionPath.append("Data\\collision.txt");
		buildingPath.append("Data\\buildings.txt");
		movementManager = MovementManager{ collisionPath, 4096, 16 };
		buildingManager = BuildingManager{ buildingPath };
		initGame(path);
	}
	
	void update() {
		movementManager.update();
	}

private:
	void initGame(std::filesystem::path path) {
		//DO TOTALNEJ ZMIANY
		path = path.append("Data\\buildings.txt");

		Faction orks = { "Orks",  buildingManager.getRaceBuildings(0) };
		Faction humans = { "Humans", buildingManager.getRaceBuildings(1) };
		Faction nomands = { "Nomads", buildingManager.getRaceBuildings(2) };
		Faction evilHumans = { "EvilHumans",buildingManager.getRaceBuildings(3) };
		Faction goblins = { "Dwards", buildingManager.getRaceBuildings(4) };
		Faction animals = { "Animals" };
		Faction bandits = { "Bandits" };

		//Orks
		int i, chuj = 0;
		for (i = 0; i < 3; i++) {
			//Squad{ , }
			//squads.push_back(Squad{ chuj++, });
		}
	}

	Squad* player;
	MovementManager movementManager;
	BuildingManager buildingManager;
	std::array<Faction, 5> factions;
	std::vector<Squad> squads;
	InputHandler inputManager;
};