#pragma once
#include "Astar.h"
#include "Entity.h"
#include <vector>
#include <filesystem>
#include <fstream>

class MovementManager {
public:
	MovementManager() = default;
	MovementManager(std::filesystem::path path) {
		loadCollisionData(path);
	}

	bool moveEntity(Entity& entity) {

	}

private:
	void loadCollisionData(std::filesystem::path path) {
		std::ifstream file;
		file.open("collision.txt");
		if (!file) {
			std::cout << "collision.txt File not found\n";
			return;
		};

		int x, y, collumns;
		file >> collumns;

		for (int i = 0; i < collumns; i++) {
			file >> x >> y;
			movement->addBlockade(Astar::point{ x / 16, y / 16 });
		}
	}

	enum MovementState {
		IDLE,
		RUNNING,
	};

	int tileSize = 16;
	Astar* movement;
};