#pragma once
#include <iostream>
#include "glm/vec2.hpp"
#include "EntityStatistics.h"
#include "Item.h"
#include <filesystem>

struct BattleMap {
	uint64_t texture;
	std::filesystem::path path;
	float mapSize;
	float tileSize;
	float tileAmount;
};

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

#define SQUAD_MAX_SIZE 15

class Entity {
public:

	class IEntityState
	{
	public:
		Entity* self;
		virtual int MoveEntity(void* battleContext) = 0;
		virtual int NextState() = 0;
		virtual int CanMoveEntity() = 0;
		//virtual int AttackEntity(void* battleContext) = 0;
	};

	uint64_t id;
	struct EquipedItems {
		ArmorItem* helmet = nullptr;
		ArmorItem* Chestplate = nullptr;
		ArmorItem* Legs = nullptr;
		ArmorItem* Boots = nullptr;
		WeaponItem* weapon_primary = nullptr;
		WeaponItem* weapon_secondary = nullptr;

	};
public:
	Entity(std::string name = "", uint64_t texture = 0, Stats stats = {}, EquipedItems * items = {}) {
		this->name = name;
		this->texture = texture;
		this->stats = stats;
		this->items = items;

		hp = stats.hp;
		state = 0;
	}

	std::string getName() {
		return name;
	}

	void SetHp(float hp)
	{
		this->hp = hp;
	}

	void changeEntityState(Entity::IEntityState* new_)
	{
		Entity::IEntityState* old = state;

		if (old != 0)
			delete old;

		state = new_;
	}

	void EntityClearMove()
	{
		canMoveEnt = true;
	}

	void EntitySetMove()
	{
		canMoveEnt = false;
	}

	bool canMove()
	{
		return canMoveEnt;
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

	Stats* getStats()
	{
		return &stats;
	}

	float getHp()
	{
		return hp;
	}

	EquipedItems* getEquipedItems() {
		return items;
	}

	IEntityState* state;
	glm::vec2 travel;
	//IMPL
private:
	std::string name;
	glm::vec2 position;
	Stats stats;
	uint64_t texture;
	EquipedItems* items;
	bool canMoveEnt;

	float hp;
};

class Squad {
public:
	struct SquadComp {
		Entity* entities[SQUAD_MAX_SIZE];
		uint8_t size;
	};
public:
	Squad() {
		std::cout << "CHUJOWY SQUAD error \n";
	};
	Squad(uint64_t squadID, uint8_t factionID, glm::vec2 position) {
		squadComp = new SquadComp{};
		//squadComp->entities[0] = new Entity{};
		//squadComp->size = 1;
		squadComp->size = rand() % SQUAD_MAX_SIZE;
		for (int i = 0; i < squadComp->size; i++) {
			squadComp->entities[i] = new Entity( "Some chujstwo", 0, {}, new Entity::EquipedItems{});
		}
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
		return squadComp->size;
	}

	float force = 10.0f;

	void setSquadState(SquadState squadState) {
		this->squadState = squadState;
	}

	SquadState getSquadState() {
		return squadState;
	}

	SquadComp* getSquadComp() {
		return squadComp;
	}

	

private:
	glm::vec2 position;

	uint64_t squadID;
	uint8_t factionID;

	SquadState squadState;
	SquadComp* squadComp;
};

typedef struct _BattleData {
	Squad* s1;
	Squad* s2;
}BattleData;

class EntityCombatCloseRange : public Entity::IEntityState
{
public:
	EntityCombatCloseRange(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
};

class EntityCombatLongRange : public Entity::IEntityState
{
public:
	EntityCombatLongRange(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
};

class EntityCombatDead : public Entity::IEntityState
{
public:
	EntityCombatDead(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
};

class EntityCombatEscape : public Entity::IEntityState
{
public:
	EntityCombatEscape(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
};

class EntityCombatStand : public Entity::IEntityState
{
public:
	EntityCombatStand(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
};

class EntityItem : public Item {
public:
	EntityItem(Entity* entityItem = nullptr, uint32_t price = 0) {
		this->itemName = "SOME ENTITY";
		this->object = (void*)entityItem->getTexture();
		this->entityItem = entityItem;
		this->objType = ENTITY;
		this->price = price;
	}

	Entity* getEntity() {
		return entityItem;
	}

	void* getItemTexture() {
		return object;
	}
private:
	Entity* entityItem;
};