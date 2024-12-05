#pragma once
#include <unordered_map>
#include "Squad.h"
#include "Building.h"

#define NEUTRAL 0
#define ENEMY 1
#define ALLY 2

class Faction {
public:
	Faction() {

	}
	Faction(std::string factionName, std::vector<Building> buildings = {}) {
		this->factionName = factionName;
		this->factionBuildings = buildings;
	}

	void setAlly(uint64_t squadID) {
		removeSquadByID(squadID);
		data.push_back(squadData{ squadID, ALLY });
	}

	void setEnemy(uint64_t squadID) {
		removeSquadByID(squadID);
		data.push_back(squadData{ squadID, ENEMY });
	}

	void setNeutral(uint64_t squadID) {
		removeSquadByID(squadID);
	}

	uint8_t getSquadAttitue(uint64_t squadId) {
		for (auto squad : data) {
			if (squad.id == squadId) return squad.type;
		}
		return NEUTRAL;
	}

	std::vector<Building> getFactionBuildings() const {
		return factionBuildings;
	}

private:
	bool removeSquadByID(uint64_t squadID) {
		for (auto squad : data) {
			if (squad.id == squadID) return true;
		}
		return false;
	}

	std::string factionName;
	struct squadData {
		uint64_t id;
		uint8_t type;
	};
	std::vector<squadData> data;
	std::vector<Building> factionBuildings;
};