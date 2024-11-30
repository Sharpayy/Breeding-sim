#pragma once
#include "Entity.h"

#define SQUAD_MAX_SIZE 20

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
	Squad(uint64_t squadID, glm::vec2 position) {
		squadComp.entities[0] = Entity{};
	}

	void setSquadPosition(glm::vec2 position) {
		this->position = position;
	}

	uint64_t getSquadID() {
		return squadID;
	}

private:
	glm::vec2 position;

	struct SquadComp {
		Entity entities[SQUAD_MAX_SIZE];
		uint8_t size = 1;
	} squadComp;

	uint64_t squadID;
};