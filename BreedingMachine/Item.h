#pragma once
#include <iostream>
#include <string>
#include <unordered_map>

//#define UNDEFINE	0
//#define HELMET		(1 << 0)
//#define CHESTPLATE  (1 << 1)
//#define LEGS		(1 << 2)
//#define BOOTS		(1 << 3)
//#define SHIELD		(1 << 4)
//#define ARMOR		(HELMET | CHESTPLATE | LEGS | BOOTS)
//#define MELEE		(1 << 4)
//#define RANGED		(1 << 5)
//#define WEAPON		(MELEE | RANGED)
//#define MISC		(1 << 6)
//#define EVERY_ITEM	(ARMOR | WEAPON | MISC)
//#define ENTITY		(1 << 7)
//
//#define TIER_0		(1 << 0)
//#define TIER_1		(1 << 1)
//#define TIER_2		(1 << 2)
//#define TIER_3		(1 << 3)
//#define TIER_ALL	(TIER_0 | TIER_1 | TIER_2 | TIER_3)
#define UNDEFINE	0
#define HELMET		(1 << 0)
#define CHESTPLATE  (1 << 1)
#define LEGS		(1 << 2)
#define BOOTS		(1 << 3)
#define SHIELD		(1 << 4)
#define ARMOR		(HELMET | CHESTPLATE | LEGS | BOOTS | SHIELD)
#define MELEE		(1 << 5)
#define RANGED		(1 << 6)
#define WEAPON		(MELEE | RANGED)
#define MISC		(1 << 7)
#define EVERY_ITEM	(ARMOR | WEAPON | MISC)
#define ENTITY		(1 << 8)

#define TIER_0		(1 << 0)
#define TIER_1		(1 << 1)
#define TIER_2		(1 << 2)
#define TIER_3		(1 << 3)
#define TIER_ALL	(TIER_0 | TIER_1 | TIER_2 | TIER_3)

class Item {
public:
	struct ObjectStatistic {
		virtual ~ObjectStatistic() = default;
	};
public:
	Item(std::string itemName = "UNDEFINE", void* texture = nullptr, uint32_t objeType = UNDEFINE, ObjectStatistic* objStats = {}, uint32_t price = 0, uint8_t tier = TIER_0) {
		this->itemName = itemName;
		this->object = texture;
		this->objType = objeType;
		this->objStat = objStats;
		this->price = price;
		this->tier = tier;
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

	uint32_t getObjectType() {
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

	uint8_t getItemTier() {
		return tier;
	}

protected:
	std::string itemName;
	void* object;
	uint32_t objType;
	ObjectStatistic* objStat;
	uint32_t price;
	uint8_t tier;
};

class ArmorItem : public Item {
public:
	struct ObjectStatistic : public Item::ObjectStatistic {
		float armor;
		ObjectStatistic(float armor = 0) { this->armor = armor; };
	};
public:
	//ArmorItem() = default;
	ArmorItem(std::string itemtName = "UNDEFINE", void* newItem = nullptr, uint32_t objType = ARMOR, ObjectStatistic* objStats = {}, uint32_t price = 0, uint8_t tier = TIER_0) {
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

private:
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
	WeaponItem(std::string itemtName = "UNDEFINE", void* newItem = nullptr, uint32_t objType = WEAPON, ObjectStatistic* objStat = {}, uint32_t price = 0, uint8_t tier = TIER_0) {
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

private:
	ObjectStatistic* objStat;
};

class ItemLoader {
public:
	ItemLoader() {
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
		while(!it) {
			idx = rand() % tierItemMap.at(tier).size();
			if constexpr (std::is_same_v<decltype(it),T>) {
				it = tierItemMap.at(tier).at(idx);
			}
		}
		return it;
	}

private:
	std::unordered_map<std::string, Item*> itemMap;
	std::unordered_map<uint8_t, std::vector<Item*>> tierItemMap;
};