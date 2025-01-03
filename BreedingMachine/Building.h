#pragma once
#include <iostream>
#include <glm/vec2.hpp>
#include "inventory.h"

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
	uint8_t type;
	glm::vec2 position;
	std::vector<Item*> items_rotation;
};