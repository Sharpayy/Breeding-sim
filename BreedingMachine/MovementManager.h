#pragma once
#include "Astar.h"
#include <vector>
#include <filesystem>
#include <fstream>
#include "Astar.h"
#include "Squad.h"
#include <unordered_map>

#define STANDARD_SPEED 2

class MovementManager {
public:
	MovementManager() = default;
	MovementManager(std::filesystem::path path, uint32_t mapSize, uint8_t tileSize) {
		border = Astar::border{ -((int)mapSize / 2), ((int)mapSize / 2), (int)mapSize, (int)mapSize, tileSize };
		this->movement = Astar{ &border };
		loadCollisionData(path);

		this->mapSize = mapSize;
		this->tileSize = tileSize;
	}

	bool createEntityPath(Astar::point e, Entity* entity) { //Entity& entity) {
		//auto path = movement.findPath(, e);
		//IMPL
	}

	void createSquadPath(Astar::point e, Squad* squad) { //Entity& entity) {
		auto position = squad->getSquadPosition();
		position.x = (int)(position.x / tileSize) * tileSize;
		position.y = (int)(position.y / tileSize) * tileSize;
		e.x = (int)(e.x / tileSize) * tileSize;
		e.y = (int)(e.y / tileSize) * tileSize;
		auto path = movement.findPath(Astar::point{(int)position.x, (int)position.y}, e, tileSize);
		squadsMovementData[squad->getSquadID()] = SquadMovementInfo{ squad, path };
	}
	
	void update() {
		moveSquads();
		SDL_Delay(10);
	}

	uint8_t getMapTileSize() {
		return tileSize;
	}

	uint64_t getMapSize() {
		return mapSize;
	}

private:
	void moveSquads() {
		Astar::point prevAP, nextAP;
		glm::vec2 prevPosition, nextPosition, currentPosition;
		float offset = tileSize / 2.0f;
		std::vector<uint64_t> squadsPathToRemove;
		for (auto& squadData : squadsMovementData) {
			if (squadData.second.path.size() >= 2) {
				prevAP = squadData.second.path.at(0);
				nextAP = squadData.second.path.at(1);
				prevPosition = glm::vec2{ prevAP.x, prevAP.y } + offset;
				nextPosition = glm::vec2{ nextAP.x, nextAP.y } + offset;
				float speed = calculateSquadMovementSpeed(*squadData.second.squad);
				currentPosition = lerp(prevPosition, nextPosition, squadData.second.dt);
				if (currentPosition == nextPosition) {
					squadData.second.path.erase(squadData.second.path.begin());
					squadData.second.dt = 0;
					continue;
				}
				squadData.second.squad->setSquadPosition(currentPosition);
			}
			else {
				if (squadData.second.path.size()) {
					squadData.second.path.clear();
					squadsPathToRemove.push_back(squadData.first);
;				}
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

	glm::vec2 lerp(const glm::vec2& actualPos, const glm::vec2& destination, float dt) {
		dt = glm::clamp(dt, 0.0f, 1.0f);
		return actualPos + (destination - actualPos) * dt;
	}

	glm::vec2 smoothstep(const glm::vec2& actualPos, const glm::vec2& destination, float dt) {
		dt = glm::clamp(dt, 0.0f, 1.0f);
		float smoothT = glm::smoothstep(0.0f, 1.0f, dt);
		return glm::mix(actualPos, destination, smoothT);
	}

	glm::vec2 easeOutCubic(const glm::vec2& actualPos, const glm::vec2& destination, float dt) {
		float clampedDt = glm::clamp(dt, 0.0f, 1.0f);
		float easeValue = 1.0f - std::pow(1.0f - clampedDt, 3.0f);
		return actualPos + (destination - actualPos) * easeValue;
	}

	//time = current time;
	float calculateSquadMovementSpeed(Squad& squad, float t = 1.0f) {
		float armySizeFactor = 1.0f - ((float)squad.getArmySize() / (float)SQUAD_MAX_SIZE);
		float totalSpeed = t * armySizeFactor / ((float)tileSize / (float)STANDARD_SPEED);
		SquadMovementInfo& data = squadsMovementData[squad.getSquadID()];
		data.dt += totalSpeed;
		return data.dt;
	}

	struct SquadMovementInfo {
		Squad* squad;
		std::vector<Astar::point> path;
		float dt;
	};
	
	uint8_t tileSize;
	uint32_t mapSize;
	Astar::border border;
	Astar movement;
	std::unordered_map<uint64_t, SquadMovementInfo> squadsMovementData;
};