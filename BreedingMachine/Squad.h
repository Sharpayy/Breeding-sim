#pragma once
#include "Entity.h"

#define SQUAD_MAX_SIZE 20

enum SquadState {
	STAND = 0,
	WANDER = 1,
	PATROL = 2,
	CHASE = 3,
	RETREAT = 4,
};

enum Race {
	HUMANS = 1,
	ORKS = 2,
	GOBLINS = 3,
	BANDITS = 4,
};

class Squad {
public:
	Squad() {
		std::cout << "CHUJOWY SQUAD error \n";
	};
	Squad(uint64_t squadID, uint8_t factionID, glm::vec2 position) {
		squadComp.entities[0] = Entity{};
		this->squadID = squadID;
		this->position = position;
		this->factionID = factionID;
		this->squadState = STAND;
	}

	void setSquadPosition(glm::vec2 position) {
		this->position = position;
	}

	glm::vec2 getSquadPosition() const {
		return position;
	}

	uint64_t getSquadID() {
		return squadID;
	}

	uint8_t getSquadFactionID() {
		return factionID;
	}

	uint8_t getArmySize() {
		return squadComp.size;
	}

	float force = 10.0f;

	void setSquadState(SquadState squadState) {
		this->squadState = squadState;
	}

	SquadState getSquadState() {
		return squadState;
	}

private:
	glm::vec2 position;
	struct SquadComp {
		Entity entities[SQUAD_MAX_SIZE];
		uint8_t size = 1;
	} squadComp;

	uint64_t squadID;
	uint8_t factionID;

	SquadState squadState;
};