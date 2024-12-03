#pragma once
#include <unordered_map>
#include "Squad.h"

#define NEUTRAL 0
#define ENEMY 1
#define ALLY 2

class Faction {
public:
	Faction() {

	}
	Faction(std::string factionName, std::vector<uint64_t> allies, std::vector<uint64_t> enemies) {
		for (auto squad : allies) {
			data.push_back(squadData{ squad, ALLY });
		}
		for (auto squad : enemies) {
			data.push_back(squadData{ squad, ENEMY });
		}
	}

	uint8_t getSquadAttitue(uint64_t squadId) {
		for (auto squad : data) {
			if (squad.id == squadId) return squad.type;
		}
		return NEUTRAL;
	}

private:
	std::string factionName;
	struct squadData {
		uint64_t id;
		uint8_t type;
	};
	std::vector<squadData> data;
};