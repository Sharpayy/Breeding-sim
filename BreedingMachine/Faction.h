#pragma once
#include <unordered_map>
#include "Squad.h"
#include "Building.h"

class Faction {
public:
	Faction() {

	}
	Faction(std::string factionName, uint8_t factionID, std::vector<Building*> buildings = {}) {
		this->factionName = factionName;
		this->factionID_ = factionID;
		this->factionBuildings = buildings;
	}

	void setFactionRelationship(uint8_t factionID, uint8_t state) {
		if(factionID_ != factionID)
			data[factionID] = state;
	}

	uint8_t getFactionRelationship(uint8_t factionID) {
		if (factionID_ != factionID) 
			return data[factionID];
	}


	std::vector<Building*> getFactionBuildings() const {
		return factionBuildings;
	}

private:

	std::string factionName;
	uint8_t factionID_;
	std::unordered_map<uint8_t, uint8_t> data;
	std::vector<Building*> factionBuildings;
};