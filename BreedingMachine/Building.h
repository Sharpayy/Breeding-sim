#pragma once
#include <iostream>
#include <glm/vec2.hpp>

#define VILLAGE 0 
#define CASTLE 1
#define CITY 2

class Building {
public:
	Building() {
		std::cout << "chujowy budynek\n";
	}
	Building(glm::vec2 position) {
		this->position = position;
	}

	glm::vec2 getBuildingPosition() {
		return position;
	}

private:
	uint8_t type;
	glm::vec2 position;
};