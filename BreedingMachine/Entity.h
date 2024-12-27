#pragma once
#include <iostream>
#include "glm/vec2.hpp"
#include "EntityStatistics.h"
#include "inventory.h"

class Entity {
public:
	struct EquipedItems {
		Armor* helmet = nullptr;
		Armor* Chestplate = nullptr;
		Armor* Legs = nullptr;
		Armor* Boots = nullptr;
		Weapon* weapon_primary = nullptr;
		Weapon* weapon_secondary = nullptr;

	};
public:
	Entity(uint64_t texture = 0, Stats stats = defaultStats, EquipedItems items = {}) {
		this->texture = texture;
		this->stats = stats;
		this->items = items;
	}

	glm::vec2 getPosition() {
		return position;
	}

	uint64_t getTexture() {
		return texture;
	}

	void setEntityPosition(glm::vec2 position) {
		this->position = position;
	}

	EquipedItems getEquipedItems() {
		return items;
	}
	//IMPL
private:
	glm::vec2 position;
	uint32_t id;
	Stats stats;
	uint64_t texture;
	EquipedItems items;
};