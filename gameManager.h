#pragma once
#include "Faction.h"
#include "MovementManager.h";
#include <array>
#include "inputHandler.h"

class gameManager {
public:
	gameManager() {
		std::filesystem::path path = std::filesystem::current_path().append("Data\\collision.txt");
		movementManager = MovementManager{ path, 4096, 16 };
	}
	
	void update() {

	}

private:
	void initGame() {
		//DO TOTALNEJ ZMIANY
		Faction humans;
		Faction orks;
		Faction nomands;
		Faction elfs;
		Faction dwarfs;

		//Orks
		int i, chuj = 0;
		for (i = 0; i < 3; i++) {
			//Squad{ , }
			squads.push_back(Squad{chuj++, (rand() % 4096) - 2048,  (rand() % 4096) - 2048 })
		}
	}

	Squad* player;
	MovementManager movementManager;
	std::array<Faction, 5> factions;
	std::vector<Squad> squads;
	InputHandler inputManager;
};