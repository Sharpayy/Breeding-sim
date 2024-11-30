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
	MovementManager(std::filesystem::path path) {
		loadCollisionData(path);
	}

	bool createEntityPath(Astar::point s, Astar::point e, Entity& entity) { //Entity& entity) {
		auto path = movement.findPath(s, e);
		//IMPL
	}

	bool createSquadPath(Astar::point s, Astar::point e, Squad& squad) { //Entity& entity) {
		auto path = movement.findPath(s, e);
		squadsMovementData[squad.getSquadID()] = SquadMovementInfo{ &squad, path };
	}
	
	void update() {
		moveSquads();
		SDL_Delay(300);
	}

private:
	void moveSquads() {
		Astar::point p;
		for (auto& squad : squadsMovementData) {
			p = squad.second.path.at(0);
			squad.second.squad->setSquadPosition(glm::vec2{ p.x, p.y });
			squad.second.path.erase(squad.second.path.begin());
		}
	}

	void loadCollisionData(std::filesystem::path path) {
		std::ifstream file;
		file.open(path.string());
		if (!file) {
			std::cout << "collision.txt File not found\n";
			return;
		};

		int x, y, collumns;
		file >> collumns;

		for (int i = 0; i < collumns; i++) {
			file >> x >> y;
			movement.addBlockade(Astar::point{ x / 16, y / 16 });
		}
	}

	struct SquadMovementInfo {
		Squad* squad;
		std::vector<Astar::point> path;
	};

	int tileSize = 16;
	Astar movement;
	std::unordered_map<uint64_t, SquadMovementInfo> squadsMovementData;
};