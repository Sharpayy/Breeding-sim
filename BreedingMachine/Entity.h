#pragma once
#include <iostream>
#include "glm/vec2.hpp"
#include "EntityStatistics.h"
#include "Item.h"
#include <filesystem>
#include "textures.h"
#include "EntityTextures.h"
#include "gui.h"
#include "generator.h"
#include "EntityNames.h"

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

extern std::unordered_map<uint8_t, std::vector<std::string>> factionNames;

std::string getRandomFactionName(uint8_t factionID);
bool loadNames(const char* path, uint8_t factionID);

class MeleeAnimationController
{
public:
	float t;
	uint64_t tex;
	glm::vec2 position;
	uint32_t lOpacity;

	rasticore::RastiCoreRender* r;

	MeleeAnimationController() {};
	MeleeAnimationController(rasticore::RastiCoreRender* r, rasticore::ModelCreationDetails* mcd);

	void Update(float dt);
	void SetAnimation(uint64_t texture, glm::vec2 p);
	void Render();

};

glm::vec2 lerp2f(glm::vec2 a, glm::vec2 b, float t);

class RangedAnimationController
{
public:
	float t;
	uint64_t tex;
	glm::vec2 pos0;
	glm::vec2 pos1;

	rasticore::RastiCoreRender* r;

	RangedAnimationController() {};
	RangedAnimationController(rasticore::RastiCoreRender* r, rasticore::ModelCreationDetails* mcd);

	void SetAnimation(uint64_t texture, glm::vec2 p0, glm::vec2 p1);
	void Update(float dt);
	void Render();
};

typedef struct _AnimationControllers
{
	MeleeAnimationController mac;
	RangedAnimationController rac;
}AnimationControllers;

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
		virtual int AttackEntity(void* battleContext, AnimationControllers* ac) = 0;
		virtual int EntityCanBattle() = 0;
	};

	class EntityCombatStand : public Entity::IEntityState
	{
	public:
		EntityCombatStand(Entity* self);
		virtual int MoveEntity(void* battleContext);
		virtual int NextState();
		virtual int CanMoveEntity();
		virtual int AttackEntity(void* battleContext, AnimationControllers* ac);
		virtual int EntityCanBattle();
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
		state = new Entity::EntityCombatStand(this);
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

class ItemLoader {
public:
	ItemLoader() :
		generatorInstance(Generator::getInstance())
	{
		tierItemMap[TIER_0] = {};
		tierItemMap[TIER_1] = {};
		tierItemMap[TIER_2] = {};
		tierItemMap[TIER_3] = {};
	}

	void loadItem(Item& item) {
		uint8_t type = item.getObjectType();
		assert(!(item.getItemTier() == TIER_ALL));
		if (type & ARMOR) {
			auto armor = new ArmorItem{ item.getItemName(), item.getItemTexture(), item.getObjectType(), (ArmorItem::ObjectStatistic*)item.getObjectStatistic(), item.getItemPrice(), item.getItemTier() };
			itemMap[item.getItemName()] = armor;
			tierItemMap.at(item.getItemTier()).push_back(armor);
		}
		if (type & WEAPON) {
			auto weapon = new WeaponItem{ item.getItemName(), item.getItemTexture(), item.getObjectType(), (WeaponItem::ObjectStatistic*)item.getObjectStatistic(), item.getItemPrice(), item.getItemTier() };
			itemMap[item.getItemName()] = weapon;
			tierItemMap.at(item.getItemTier()).push_back(weapon);
		}
		else {
			auto nitem = new Item{ item.getItemName(), item.getItemTexture(), item.getObjectType(), item.getObjectStatistic(), item.getItemPrice(), item.getItemTier() };
			itemMap[item.getItemName()] = nitem;
			tierItemMap.at(item.getItemTier()).push_back(nitem);
		}
	}
	void loadItem(Item* item) {
		assert(!(item->getItemTier() == TIER_ALL));
		itemMap[item->getItemName()] = item;
		tierItemMap.at(item->getItemTier()).push_back(item);
	}

	template <typename T = Item>
	T* getItem(std::string itemName) {
		auto it = itemMap.find(itemName);
		if (it != itemMap.end()) {
			uint8_t objectType = it->second->getObjectType();
			if (objectType & WEAPON) {
				if constexpr (std::is_same_v<T, WeaponItem>) {
					return (T*)it->second;
				}
				else return nullptr;
			}
			if (objectType & ARMOR) {
				if constexpr (std::is_same_v<T, ArmorItem>) {
					return (T*)it->second;
				}
				return nullptr;
			}
			if (objectType & EVERY_ITEM) {
				if constexpr (std::is_same_v<T, Item>) {
					return (T*)it->second;
				}
				return nullptr;
			}
		}
		return nullptr;
	}

	Item* getRandomItem(uint8_t tier = TIER_ALL) {
		int idx = 0;
		if (tier == TIER_ALL) {
			uint8_t availableTiers[] = { TIER_1, TIER_2, TIER_3 };
			tier = availableTiers[rand() % 3];
		}
		idx = rand() % tierItemMap.at(tier).size();
		auto it = tierItemMap.at(tier).at(idx);

		return it;
	}

	template<typename T>
	Item* getRandomSpecificItem(uint8_t tier = TIER_ALL) {
		int idx = 0;
		if (tier == TIER_ALL) {
			uint8_t availableTiers[] = { TIER_1, TIER_2, TIER_3 };
			tier = availableTiers[rand() % 3];
		}
		Item* it = nullptr;
		while (!it) {
			idx = rand() % tierItemMap.at(tier).size();
			if constexpr (std::is_same_v<decltype(it), T>) {
				it = tierItemMap.at(tier).at(idx);
			}
		}
		return it;
	}

	void loadSet(Entity::EquipedItems set) {
		sets.push_back(set);
	}

	//void loadSet(std::string setName, Entity::EquipedItems& set) {
	//	sets[setName] = set;
	//}

	Entity::EquipedItems getRandomSet() {
		int size = sets.size();
		if (!size) return Entity::EquipedItems{};
		return sets.at(rand() % size);
	}


	//FOR NOW
	float calculateEntityPrice(Entity* entity) {
		if (!entity) return 0;
		float sum = BASE_ENTITY_VALUE;
		auto items = entity->getEquipedItems();
		if (items.helmet) sum += items.helmet->getItemPrice();
		if (items.Chestplate) sum += items.Chestplate->getItemPrice();
		if (items.Legs) sum += items.Legs->getItemPrice();
		if (items.Boots) sum += items.Boots->getItemPrice();
		if (items.shield) sum += items.shield->getItemPrice();
		if (items.weapon) sum += items.weapon->getItemPrice();

		auto stats = entity->getStats();
		sum += MAX_MELEE_STAT_PRICE * getPercentage(20, 5, stats->melee)
			+ MAX_RANGED_STAT_PRICE * getPercentage(20, 5, stats->ranged)
			+ MAX_BRAVERY_STAT_PRICE * getPercentage(200, 100, stats->bravery)
			+ MAX_DEFENCE_STAT_PRICE * getPercentage(10, 2, stats->defense)
			+ MAX_HP_STAT_PRICE * getPercentage(30, 10, stats->hp);
		return sum;
	}

	//FOR NOW
	Entity* generateRandomEntity(uint8_t factionID) {
		Stats entityStats = {
			generatorInstance.getRandomNumber(5,20),
			generatorInstance.getRandomNumber(2,10),
			generatorInstance.getRandomNumber(5,20),
			generatorInstance.getRandomNumber(100,200),
			4.1f + generatorInstance.getRandomNumber(0,2),
			generatorInstance.getRandomNumber(10,30) };
		Entity::EquipedItems items = getRandomSet();
		Entity* entity = new Entity(getRandomFactionName(factionID), 0, entityStats, items);
		uint32_t index = GetEntityRandomTextureIndex(factionID);
		entity->SetEntityTextureIndex(GetEntityTextureFromIndex(index, factionID), index);
		return entity;
	}

private:
	//FOR NOW
	float getPercentage(float min, float max, float value) {
		return ((value - min) / (max - min));
	}

	std::unordered_map<std::string, Item*> itemMap;
	std::unordered_map<uint8_t, std::vector<Item*>> tierItemMap;
	//std::unordered_map<std::string, Entity::EquipedItems> sets;
	std::vector<Entity::EquipedItems> sets;
	Generator& generatorInstance;

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
	Squad(uint64_t squadID, uint8_t factionID, glm::vec2 position, ItemLoader* il);

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
	virtual int AttackEntity(void* battleContext, AnimationControllers* ac);
	virtual int EntityCanBattle();
};

class EntityCombatLongRange : public Entity::IEntityState
{
public:
	EntityCombatLongRange(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
	virtual int AttackEntity(void* battleContext, AnimationControllers* ac);
	virtual int EntityCanBattle();
};

class EntityCombatDead : public Entity::IEntityState
{
public:
	EntityCombatDead(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
	virtual int AttackEntity(void* battleContext, AnimationControllers* ac);
	virtual int EntityCanBattle();
};

class EntityCombatEscape : public Entity::IEntityState
{
public:
	EntityCombatEscape(Entity* self);
	virtual int MoveEntity(void* battleContext);
	virtual int NextState();
	virtual int CanMoveEntity();
	virtual int AttackEntity(void* battleContext, AnimationControllers* ac);
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