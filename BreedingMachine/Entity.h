#pragma once
#include <iostream>
#include "glm/vec2.hpp"
#include "EntityStatistics.h"
#include "Item.h"
#include <filesystem>
#include "textures.h"
#include "EntityTextures.h"
#include "gui.h"

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

typedef struct _EntityApperance
{
	float attackTextureOpacity;
	float blockadeOpacity;
	
	float Reserved[2];
} EntityApperance;

typedef struct _EntityData
{
	glm::vec2 pos;
	float visn;
	float Reserevd;
} EntityData;

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
		virtual int AttackEntity(void* battleContext) = 0;
		virtual int EntityCanBattle() = 0;
	};

	uint64_t id;
	struct EquipedItems {
		ArmorItem* helmet = nullptr;
		ArmorItem* Chestplate = nullptr;
		ArmorItem* Legs = nullptr;
		ArmorItem* Boots = nullptr;
		ArmorItem* shield = nullptr;
		WeaponItem* weapon = nullptr;
	};
public:
	Entity(std::string name = "", uint64_t texture = 0, Stats stats = {}, EquipedItems items = {}) {
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

	EquipedItems& getEquipedItems() {
		return items;
	}

	uint64_t GetIndex()
	{
		return index;
	}

	float GetBravery()
	{
		return bravery;
	}

	void SetBravery(float b)
	{
		bravery = b;
	}

	void SetEntityTextureIndex(uint64_t tex, uint64_t ind)
	{
		texture = tex;
		index = ind;
	}

	void SetBaseStats()
	{
		hp = stats.hp;
		bravery = stats.bravery;
	}

	void UpdateHpBar()
	{

	}

	void RenderHpBar(glm::mat4 pm)
	{

	}

	IEntityState* state;
	glm::vec2 travel;
	//IMPL
private:
	std::string name;
	glm::vec2 position;
	Stats stats;
	uint64_t texture;
	uint64_t index;
	EquipedItems items;
	bool canMoveEnt;

	float hp;
	float bravery;

	//GComponentSlider* hp_bar;
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
	Squad(uint64_t squadID, uint8_t factionID, glm::vec2 position);

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

float AiGetEntityScaryFactor(Entity* e);
float AiGetEntityAdvanceFactor(Entity* e);
float AiGetEntityAttackFactor(Entity* e);

float AiGetUnitArmor(Entity* e);
float AiGetAttackAfterArmor(Entity* e, float atk);
float AiGetUnitAttack(Entity* e);

float AiGetUnitBraveryDamage(Entity* e);
void AiGainUnitStdBravery(Entity* e);

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
	virtual int AttackEntity(void* battleContext);
	virtual int EntityCanBattle();
};

class EntityCombatLongRange : public Entity::IEntityState
{
public:
	EntityCombatLongRange(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
	virtual int AttackEntity(void* battleContext);
	virtual int EntityCanBattle();
};

class EntityCombatDead : public Entity::IEntityState
{
public:
	EntityCombatDead(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
	virtual int AttackEntity(void* battleContext);
	virtual int EntityCanBattle();
};

class EntityCombatEscape : public Entity::IEntityState
{
public:
	EntityCombatEscape(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
	virtual int AttackEntity(void* battleContext);
	virtual int EntityCanBattle();
};

class EntityCombatStand : public Entity::IEntityState
{
public:
	EntityCombatStand(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
	virtual int AttackEntity(void* battleContext);
	virtual int EntityCanBattle();
};

class EntityItem : public Item {
public:
	EntityItem(Entity* entity = nullptr, uint32_t price = 0) {
		if (entity) {
			this->itemName = entity->getName();
			this->entity = entity;
			this->object = (void*)entity->getTexture();
		}
		this->objType = ENTITY;
		this->price = price;
	}

	Entity* getEntity() {
		return entity;
	}
private:
	Entity* entity;
};