#pragma once
#include <iostream>
#include <glm/vec2.hpp>
#include "Item.h"

#define BUILDING_TYPE_UNDEFINE 0
#define BUILDING_TYPE_VILLAGE 1
#define BUILDING_TYPE_CASTLE 2
#define BUILDING_TYPE_CITY 3

class Building {
public:
	Building() {
		std::cout << "chujowy budynek\n";
	}
	Building(uint64_t buildingID, glm::vec2 position, uint8_t buildingType) {
		this->buildingID = buildingID;
		this->position = position;
		this->buildingType = buildingType;
		this->hasNewRotation = false;
	}

	bool newRotationOccured() {
		return hasNewRotation;
	}

	void setNewRotationState(bool state) {
		hasNewRotation = state;
	}

	uint64_t getID() {
		return buildingID;
	}

	glm::vec2 getPosition() {
		return position;
	}

	void addSingleItemToRotation(Item* item, int idx) {
		//idx = idx <= items_rotation.size() ? idx :  
		Item* temp = nullptr;
		if (idx < items_rotation.size() && idx >= 0) {
			temp = items_rotation.at(idx);
			items_rotation.at(idx) = item;
		}
		else items_rotation.push_back(item);
		if (temp) items_rotation.push_back(temp);
	}

	void clearItems() {
		items_rotation.clear();
	}

	std::vector<Item*> getItemsRotation() {
		return items_rotation;
	}

	void eraseItemFromRotation(Item* item) {
		items_rotation.erase(std::remove(items_rotation.begin(), items_rotation.end(), item), items_rotation.end());
	}

	uint8_t getBuildingType() {
		return buildingType;
	}

	void swapByValue(int idx0, int idx1) {
		std::swap(items_rotation[idx0], items_rotation[idx1]);
	}

private:
	uint64_t buildingID;
	uint8_t buildingType;
	glm::vec2 position;
	std::vector<Item*> items_rotation;
	bool hasNewRotation;
};