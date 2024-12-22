#pragma once
#include <array>
#include "Faction.h"
#include "Rasticore/rasti_main.h"
#include <unordered_map>

class FactionManager {
public:
	FactionManager() = default;
	FactionManager(rasticore::RastiCoreRender* r_, rasticore::ModelCreationDetails rect_mcd, int tileSize) {
		this->r = r_;
		this->rect_mcd = rect_mcd;
		this->tileSize = tileSize;
	}

	Squad* CreateNewSquad(uint32_t faction_id, glm::vec2 starting_pos)
	{
		rasticore::RENDER_LONG_ID unique_id;
		starting_pos.x = (int)(starting_pos.x / tileSize) * tileSize + tileSize / 2;
		starting_pos.y = (int)(starting_pos.y / tileSize) * tileSize + tileSize / 2;
		r->newObject(faction_id, glm::translate(glm::mat4{ 1.0f }, glm::vec3{ starting_pos.x, starting_pos.y, 1.1f }), (uint64_t*)&unique_id);

		Squad* squad = new Squad(*(uint64_t*)&unique_id, faction_id, starting_pos);
		factions[faction_id].squads.push_back(squad);
		

		return squad;
	}

	void CreateNewFaction(uint32_t faction_id, const char* filename, const char* faction_name, std::vector<Building> factionBuildings = {})
	{
		rasticore::Image img = rasticore::Image(filename, 4);
		rasticore::Texture2D tx{ img.data, (int)img.x_, (int)img.y_, GL_RGBA, GL_RGBA8 };
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		tx.genMipmap();
		rasticore::Texture2DBindless txb{ tx };
		txb.MakeResident();

		r->newModel(faction_id, rect_mcd.vb, rect_mcd.p, rect_mcd.v_cnt, rect_mcd.rm, txb, 30);

		factions[faction_id] = { Faction(faction_name, faction_id, factionBuildings), {} };
	}

	void setFactionsRelationships(uint8_t factionID_f, uint8_t factionID_s, uint8_t state) {
		factions[factionID_f].faction.setFactionRelationship(factionID_s, state);
		factions[factionID_s].faction.setFactionRelationship(factionID_f, state);
	}

	uint8_t getFactionsRelationships(uint8_t factionID_f, uint8_t factionID_s) {
		if (factionID_f == factionID_s) return ALLY;
		return factions[factionID_f].faction.getFactionRelationship(factionID_s);
	}

	std::vector<Squad*> getAllSquads() {
		std::vector<Squad*> squads = {};
		for (auto& faction : factions) {
			for (auto squad : faction.second.squads) {
				squads.push_back(squad);
			}
		}
		return squads;
	}

private:

	struct FactionData {
		Faction faction;
		std::vector<Squad*> squads;
	};

	rasticore::ModelCreationDetails rect_mcd;
	rasticore::RastiCoreRender* r;
	std::unordered_map<uint8_t, FactionData> factions;
	int tileSize;
	//std::array<Faction, 8> factions;
};