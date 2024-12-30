#pragma once
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

class Item {
public:
	struct ObjectStatistic {
		virtual ~ObjectStatistic() = default;
	};
public:
	Item(std::string itemtName = "UNDEFINE", void* newItem = nullptr, uint8_t objeType = UNDEFINE, ObjectStatistic* objStats = {}, uint32_t price = 0) {
		this->itemName = itemtName;
		this->object = newItem;
		this->objType = objeType;
		this->objStat = objStats;
		this->price = price;
	}

	void setAsset(void* asset) {
		this->object = asset;
	}

	void* getItemTexture() {
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

	void setItemStats(ObjectStatistic* objStats) {};

	virtual ObjectStatistic* getObjectStatistic() {
		return objStat;
	};

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
		uint8_t armor;
		ObjectStatistic(uint8_t armor = 0) { this->armor = armor; };
	};
public:
	ArmorItem(std::string itemtName = "UNDEFINE", void* newItem = nullptr, uint8_t objType = ARMOR, ObjectStatistic* objStats = {}, uint32_t price = 0) {
		assert(objType & ARMOR);
		this->itemName = itemtName;
		this->object = newItem;
		this->objType = objType;
		this->objStat = objStats;
		this->price = price;
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
		uint8_t damage;
		ObjectStatistic(uint8_t damage = 0) { this->damage = damage; };
	};
public:
	WeaponItem(std::string itemtName = "UNDEFINE", void* newItem = nullptr, uint8_t objType = WEAPON, ObjectStatistic* objStat = {}, uint32_t price = 0) {
		assert(objType & WEAPON);
		this->itemName = itemtName;
		this->object = newItem;
		this->objType = objType;
		this->objStat = objStat;
		this->price = price;
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