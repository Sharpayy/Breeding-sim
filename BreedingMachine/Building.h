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
		uint8_t type = 0;
		switch (buildingType) {
		case BUILDING_TYPE_VILLAGE:
			type = TIER_1;
			break;
		case BUILDING_TYPE_CASTLE:
			type = TIER_2;
			break;
		case BUILDING_TYPE_CITY:
			type = TIER_ALL;
			break;
		default:
			type = TIER_ALL;
			break;
		}
		for (int i = 0; i < size; i++) {

			items_rotation.push_back(itm->getRandomItem(type));
		}
	}

	std::vector<Item*> getItemsRotation() {
		return items_rotation;
	}

	uint8_t getBuildingType() {
		return buildingType;
	}

private:
	uint8_t buildingType;
	glm::vec2 position;
	std::vector<Item*> items_rotation;
};