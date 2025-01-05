#pragma once
#include <iostream>
#include <glm/vec2.hpp>
#include "inventory.h"

#define BUILDING_TYPE_UNDEFINE 0
#define BUILDING_TYPE_VILLAGE 1
#define BUILDING_TYPE_CASTLE 2
#define BUILDING_TYPE_CITY 3

class Building {
public:
	Building() {
		std::cout << "chujowy budynek\n";
	}
	Building(glm::vec2 position, uint8_t buildingType) {
		this->position = position;
		this->buildingType = buildingType;
	}

	glm::vec2 getPosition() {
		return position;
	}

	void setRandomItemsRotation(ItemLoader* itm, int size) {
		for (int i = 0; i < size; i++) {
			items_rotation.push_back(itm->getRandomItem());
		}
	}

	std::vector<Item*> getItemsRotation() {
		return items_rotation;
	}

private:
	uint8_t buildingType;
	glm::vec2 position;
	std::vector<Item*> items_rotation;
};