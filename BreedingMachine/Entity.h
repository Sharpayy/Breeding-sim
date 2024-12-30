#pragma once
#include <iostream>
#include "glm/vec2.hpp"
#include "EntityStatistics.h"
#include "Item.h"

class Entity {
public:
	uint32_t id;
	struct EquipedItems {
		ArmorItem* helmet = nullptr;
		ArmorItem* Chestplate = nullptr;
		ArmorItem* Legs = nullptr;
		ArmorItem* Boots = nullptr;
		WeaponItem* weapon_primary = nullptr;
		WeaponItem* weapon_secondary = nullptr;

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
	Stats stats;
	uint64_t texture;
	EquipedItems items;
};

class EntityItem : public Item {
public:
	EntityItem(Entity* entity = nullptr) {
		this->entity = entity;
		objType = ENTITY;
	}

	Entity* getEntity() {
		return entity;
	}
private:
	Entity* entity;
};