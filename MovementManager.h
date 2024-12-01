#pragma once
#include "Astar.h"
#include <vector>
#include <filesystem>
#include <fstream>
#include "Astar.h"
#include "Squad.h"
#include <unordered_map>


class MovementManager {
public:
	MovementManager() = default;
	MovementManager(std::filesystem::path path, uint32_t mapSize, uint8_t tileSize) {
		loadCollisionData(path);
		this->mapSize = mapSize;
		this->tileSize = tileSize;
	}

	bool createEntityPath(Astar::point e, Entity& entity) { //Entity& entity) {
		//auto path = movement.findPath(, e);
		//IMPL
	}

	void createSquadPath(Astar::point e, Squad& squad) { //Entity& entity) {
		auto position = squad.getSquadPosition();
		position.x = (int)(position.x / tileSize) * tileSize;
		position.t = (int)(position.y / tileSize) * tileSize;
		e.x = (int)(e.y / tileSize)* tileSize;
		e.y = (int)(e.y / tileSize) * tileSize;
		auto path = movement.findPath(Astar::point{(int)position.x, (int)position.y}, e);
		squadsMovementData[squad.getSquadID()] = SquadMovementInfo{ &squad, path };
	}
	
	void update() {
		moveSquads();
		SDL_Delay(10);
	}

private:
	void moveSquads() {
		Astar::point p;
		std::vector<uint64_t> squadsPathToRemove;
		for (auto& squad : squadsMovementData) {
			if (squad.second.path.size()) {
				p = squad.second.path.at(0);
				squad.second.squad->setSquadPosition(glm::vec2{ p.x, p.y });
				squad.second.path.erase(squad.second.path.begin());
			}
		}

		for (uint64_t id : squadsPathToRemove) {
			squadsMovementData.erase(id);
		}
	}

	void loadCollisionData(std::filesystem::path path) {
		std::ifstream file;
		file.open(path.string());
		if (!file) {
			std::cout << "collision.txt File not found\n";
			return;
		};

		int x, y, tileSize, collumns;
		file >> collumns;
		file >> tileSize;

		for (int i = 0; i < collumns; i++) {
			file >> x >> y;
			movement.addBlockade(Astar::point{ x , y  });
		}
	}

	struct SquadMovementInfo {
		Squad* squad;
		std::vector<Astar::point> path;
	};

	uint8_t tileSize;
	uint32_t mapSize;
	Astar movement;
	std::unordered_map<uint64_t, SquadMovementInfo> squadsMovementData;
};