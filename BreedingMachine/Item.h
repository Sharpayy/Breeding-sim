#pragma once
#include <iostream>
#include <string>
#include <unordered_map>

#define UNDEFINE	0
#define HELMET		(1 << 0)
#define CHESTPLATE  (1 << 1)
#define LEGS		(1 << 2)
#define BOOTS		(1 << 3)
#define ARMOR		(HELMET | CHESTPLATE | LEGS | BOOTS)
#define MELEE		(1 << 4)
#define RANGED		(1 << 5)
#define WEAPON		(MELEE | RANGED)
#define MISC		(1 << 6)
#define EVERY_ITEM	(ARMOR | WEAPON | MISC)
#define ENTITY		(1 << 7)

#define TIER_0 0
#define TIER_1 1
#define TIER_2 2
#define TIER_3 3

class Item {
public:
	struct ObjectStatistic {
		virtual ~ObjectStatistic() = default;
	};
public:
	Item(std::string itemName = "UNDEFINE", void* texture = nullptr, uint8_t objeType = UNDEFINE, ObjectStatistic* objStats = {}, uint32_t price = 0) {
		this->itemName = itemName;
		this->object = texture;
		this->objType = objeType;
		this->price = price;
		this->objStat = objStats;
	}

	void setAsset(void* asset) {
		this->object = asset;
	}

	virtual void* getItemTexture() {
		return object;
	}

	const std::string getItemName() {
		return itemName;
	}

	void getItemName(std::string itemName) {
		this->itemName = itemName;
	}

	uint8_t getObjectType() {
		return objType;
	}

	virtual ObjectStatistic* getObjectStatistic() {
		return objStat;
	}

	void setItemPrice(uint32_t price) {
		this->price = price;
	}

	uint32_t getItemPrice() {
		return price;
	}

protected:
	std::string itemName;
	void* object;
	uint32_t price;
	uint8_t objType;
	ObjectStatistic* objStat;
};

class ArmorItem : public Item {
public:
	struct ObjectStatistic : public Item::ObjectStatistic {
		float armor;
		ObjectStatistic(float armor = 0) { this->armor = armor; };
	};
public:
	//ArmorItem() = default;
	ArmorItem(std::string itemtName = "UNDEFINE", void* newItem = nullptr, uint8_t objType = ARMOR, ObjectStatistic* objStats = {}, uint32_t price = 0, uint8_t tier = TIER_0) {
		assert(objType & ARMOR);
		this->itemName = itemtName;
		this->object = newItem;
		this->objType = objType;
		this->objStat = objStats;
		this->price = price;
		this->tier = tier;
	}

	void setItemStats(ObjectStatistic* objStats) {
		this->objStat = objStats;
	}

	ObjectStatistic* getObjectStatistic() override {
		return objStat;
	}

	uint8_t getItemTier() {
		return tier;
	}

private:
	uint8_t tier;
	ObjectStatistic* objStat;
};

class WeaponItem : public Item {
public:
	struct ObjectStatistic : public Item::ObjectStatistic {
		float damage;
		ObjectStatistic(float damage = 0) { this->damage = damage; };
	};
public:
	WeaponItem() = default;
	WeaponItem(std::string itemtName = "UNDEFINE", void* newItem = nullptr, uint8_t objType = WEAPON, ObjectStatistic* objStat = {}, uint32_t price = 0, uint8_t tier = TIER_0) {
		assert(objType & WEAPON);
		this->itemName = itemtName;
		this->object = newItem;
		this->objType = objType;
		this->objStat = objStat;
		this->price = price;
		this->tier = tier;
	}

	void setItemStats(ObjectStatistic* objStat) {
		this->objStat = objStat;
	}

	ObjectStatistic* getObjectStatistic() override {
		return objStat;
	}

	uint8_t getItemTier() {
		return tier;
	}

private:
	uint8_t tier;
	ObjectStatistic* objStat;
};

class ItemLoader {
public:
	ItemLoader() = default;

	void loadItem(Item& item) {
		uint8_t type = item.getObjectType();
		if (type & ARMOR) 
			itemMap[item.getItemName()] = new ArmorItem{ item.getItemName(), item.getItemTexture(), item.getObjectType(), (ArmorItem::ObjectStatistic*)item.getObjectStatistic(), item.getItemPrice() };
		if (type & WEAPON) 
			itemMap[item.getItemName()] = new WeaponItem{ item.getItemName(), item.getItemTexture(), item.getObjectType(), (WeaponItem::ObjectStatistic*)item.getObjectStatistic(), item.getItemPrice() };		
		else 
			itemMap[item.getItemName()] = new Item{ item.getItemName(), item.getItemTexture(), item.getObjectType(), item.getObjectStatistic(), item.getItemPrice()};
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

	Item* getRandomItem() {
		int idx = rand() % itemMap.size();
		auto it = itemMap.begin();
		std::advance(it, idx);
		return it->second;
	}

private:
	std::unordered_map<std::string, Item*> itemMap;
};