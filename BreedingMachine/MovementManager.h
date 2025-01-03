#pragma once
#include "Astar.h"
#include <vector>
#include <filesystem>
#include <fstream>
#include "Astar.h"
#include "Squad.h"
#include <unordered_map>
#include "Rasticore/rasti_main.h"
#include <algorithm>

#define STANDARD_SPEED 2

class SquadMovementManager {
public:
	SquadMovementManager() = default;
	SquadMovementManager(std::filesystem::path path, uint32_t mapSize, uint8_t tileSize, rasticore::RastiCoreRender* r_, rasticore::ModelCreationDetails rect_mcd) {
		this->r = r_;
		this->rect_mcd = rect_mcd;

		this->mapSize = mapSize;
		this->tileSize = tileSize;
		
		this->movement = Astar{ new Astar::border{ 0, 0, (int)mapSize, (int)mapSize} };
		loadCollisionData(path);

	}

	int createSquadPath(Astar::point e, Squad* squad) { //Entity& entity) {
		auto position = squad->getSquadPosition() + (mapSize / 2.0f);;
		//float offset = (tileSize / 2.0f)
		position.x = ((int)((position.x) / tileSize)) * tileSize;
		position.y = ((int)((position.y) / tileSize)) * tileSize;
		e.x = (((int)((e.x) / tileSize)) * tileSize) + (mapSize / 2.0f);;
		e.y = (((int)((e.y) / tileSize)) * tileSize) + (mapSize / 2.0f);;
		if (glm::vec2{ e.x, e.y } == position) return true;
		if (squadsMovementData[squad->getSquadID()].path.size() >= 2) {
			if (squadsMovementData[squad->getSquadID()].path.back() == e) return true;
			std::vector<Astar::point> merged;
			merged.insert(merged.end(), squadsMovementData[squad->getSquadID()].path.begin(), squadsMovementData[squad->getSquadID()].path.begin() + 2);
			auto path = movement.findPath(Astar::point{ (int)merged.back().x, (int)merged.back().y}, e, tileSize);
			merged.insert(merged.end(), path.begin(), path.end());

			merged.erase(std::unique(merged.begin(), merged.end()), merged.end());
			squadsMovementData[squad->getSquadID()].path = merged;
		}
		else {
			auto path = movement.findPath(Astar::point{ (int)position.x, (int)position.y }, e, tileSize);
			squadsMovementData[squad->getSquadID()] = SquadMovementInfo{ squad, path, 0 };
		}
		return squadsMovementData[squad->getSquadID()].path.size();
	}

	bool SquadHasPath(Squad* squad) {
		return squadsMovementData[squad->getSquadID()].path.size();
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
		uint64_t id;
		float offset = (tileSize / 2.0f) - (mapSize / 2.0f);
		std::vector<uint64_t> squadsPathToRemove;
		for (auto& squadData : squadsMovementData) {
			if (squadData.second.path.size() >= 2) {
				prevAP = squadData.second.path.at(0);
				nextAP = squadData.second.path.at(1);
				//prevPosition = glm::vec2{ prevAP.x + offset, prevAP.y - offset };
				//nextPosition = glm::vec2{ nextAP.x + offset, nextAP.y - offset };
				prevPosition = glm::vec2{ prevAP.x + offset, prevAP.y + offset };
				nextPosition = glm::vec2{ nextAP.x + offset, nextAP.y + offset };
				float speed = calculateSquadMovementSpeed(*squadData.second.squad, glm::distance(prevPosition,nextPosition));
				currentPosition = lerp(prevPosition, nextPosition, squadData.second.dt);
				squadData.second.squad->setSquadPosition(currentPosition);
				if (currentPosition == nextPosition) {
					squadData.second.path.erase(squadData.second.path.begin());
					squadData.second.dt = 0;
				}
				id = squadData.second.squad->getSquadID();
				r->BindActiveModel(LONG_GET_MODEL(id));
				r->SetObjectMatrix(LONG_GET_OBJECT(id), glm::translate(glm::mat4{ 1.0f }, glm::vec3{ squadData.second.squad->getSquadPosition().x, squadData.second.squad->getSquadPosition().y, 1.1f }), true);
				//std::cout << currentPosition.x << " " << currentPosition.y << "\n";
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
			movement.addBlockade(Astar::point{ x + (int)(mapSize / 2.0f), y + (int)(mapSize / 2.0f) });
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
	float calculateSquadMovementSpeed(Squad& squad, float dist, float t = 1.0f) {
		float armySizeFactor = 1.0f - (std::log2(1 + squad.getArmySize()) / (float)SQUAD_MAX_SIZE);
		float totalSpeed = t * armySizeFactor / (dist / (float)STANDARD_SPEED);
		SquadMovementInfo& data = squadsMovementData[squad.getSquadID()];
		data.dt = glm::clamp(data.dt + totalSpeed, 0.0f, 1.0f);
		return data.dt;
	}

	struct SquadMovementInfo {
		Squad* squad;
		std::vector<Astar::point> path;
		float dt;
	};
	
	uint8_t tileSize;
	uint32_t mapSize;
	Astar movement;
	std::unordered_map<uint64_t, SquadMovementInfo> squadsMovementData;

	rasticore::RastiCoreRender* r;
	rasticore::ModelCreationDetails rect_mcd;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class EntityMovementManager {
public:
	EntityMovementManager() = default;
	EntityMovementManager(std::filesystem::path path, float mapSize, float tileSize, rasticore::RastiCoreRender* r_, rasticore::ModelCreationDetails rect_mcd) {
		this->r = r_;
		this->rect_mcd = rect_mcd;
		this->movement = Astar{ new Astar::border{ 0, 0, (int)mapSize, (int)mapSize} };
		loadCollisionData(path);

		this->mapSize = mapSize;
		this->tileSize = tileSize;
	}

	int createEntityPath(Astar::point e, Entity* entity) {
		auto position = entity->getPosition() + (mapSize / 2.0f);
		float offset = tileSize / 2.0f;
		position.x = ((int)((position.x) / tileSize)) * tileSize;
		position.y = ((int)((position.y) / tileSize)) * tileSize;
		e.x = (((int)((e.x) / tileSize)) * tileSize);
		e.y = (((int)((e.y) / tileSize)) * tileSize);
		if (glm::vec2{ e.x, e.y } == position) {
			return true;
		}
		entityMovementData.path = movement.findPath(Astar::point{ (int)position.x, (int)position.y }, e, tileSize);
		entityMovementData.dt = 0;
		entityMovementData.entity = entity;
	}

	bool pass(glm::vec2 p)
	{
		return movement.canPass(Astar::point{ (int)p.x, (int)p.y });
	}

	bool pathExist() {
		return entityMovementData.path.size();
	}

	void update() {
		moveEntity();
		//SDL_Delay(10);
	}

	uint8_t getMapTileSize() {
		return tileSize;
	}

	uint64_t getMapSize() {
		return mapSize;
	}

	void AddCollision(glm::vec2 v)
	{
		movement.addBlockade(Astar::point{ (int)v.x, (int)v.y });
	}

	void DelCollision(glm::vec2 v)
	{
		movement.delBlockade(Astar::point{ (int)v.x, (int)v.y });
	}

private:
	void moveEntity() {
		if (!entityMovementData.entity) return;

		Astar::point prevAP, nextAP;
		glm::vec2 prevPosition, nextPosition, currentPosition;
;		float offset = (tileSize / 2.0f) - (mapSize / 2.0f);

		if (entityMovementData.path.size() >= 2) {
			prevAP = entityMovementData.path.at(0);
			nextAP = entityMovementData.path.at(1);

			prevPosition = glm::vec2{ prevAP.x + offset, prevAP.y + offset};
			nextPosition = glm::vec2{ nextAP.x + offset, nextAP.y + offset };
			float speed = 0.05f;
			entityMovementData.dt = glm::clamp(0.0f, 1.0f, speed + entityMovementData.dt);
			currentPosition = lerp(prevPosition, nextPosition, entityMovementData.dt);
			entityMovementData.entity->setEntityPosition(currentPosition);
			if (currentPosition == nextPosition) {
				entityMovementData.path.erase(entityMovementData.path.begin());
				entityMovementData.dt = 0;
			}
		}
		else {
			entityMovementData = {};
		}
	}

	void loadCollisionData(std::filesystem::path path) {
		std::ifstream file;
		file.open(path.string());
		if (!file) {
			std::cout << path.filename() <<  " File not found\n";
			return;
		};

		int x, y, tileSize, collumns;
		file >> collumns;
		file >> tileSize;

		for (int i = 0; i < collumns; i++) {
			file >> x >> y;
			movement.addBlockade(Astar::point{ x , y });
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
	struct EntityMovementInfo {
		Entity* entity = nullptr;
		std::vector<Astar::point> path;
		float dt;
	};

	uint8_t tileSize;
	uint32_t mapSize;
	Astar movement;
	EntityMovementInfo entityMovementData;

	rasticore::RastiCoreRender* r;
	rasticore::ModelCreationDetails rect_mcd;
};