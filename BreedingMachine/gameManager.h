#pragma once
#include "Faction.h"
#include "MovementManager.h";
#include <array>
#include "inputHandler.h"
#include "BuildingManager.h"
#include "Rasticore/rasti_main.h"

class gameManager {
public:
	gameManager(rasticore::RastiCoreRender* r_) {
		std::filesystem::path path = std::filesystem::current_path();
		std::filesystem::path collisionPath = path, buildingPath = path;
		collisionPath.append("Data\\collision.txt");
		buildingPath.append("Data\\buildings.txt");
		movementManager = MovementManager{ collisionPath, 4096, 16 };
		buildingManager = BuildingManager{ buildingPath };

		this->r = r_;
		initGame(path);
	}
	
	void update() {
		movementManager.update();
	}

	void CreateNewFaction(uint32_t faction_id, const char* filename, rasticore::ModelCreationDetails* mcd, const char* faction_name)
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

		r->newModel(faction_id, mcd->vb, mcd->p, mcd->v_cnt, mcd->rm, txb, 30);

		factions[faction_id] = Faction(faction_name, buildingManager.getRaceBuildings(faction_id));
	}

	uint64_t CreateNewSquad(uint32_t faction_id, glm::vec2 starting_pos)
	{
		rasticore::RENDER_LONG_ID unique_id;
		r->newObject(faction_id, glm::translate(glm::mat4{ 1.0f }, glm::vec3{ starting_pos.x, starting_pos.y, 1.1f }), (uint64_t*)&unique_id);

		squads.push_back(Squad(*(uint64_t*)&unique_id, starting_pos));

		return *(uint64_t*)&unique_id;
	}

	void SetSquadPosition(uint64_t id, glm::vec2 pos)
	{
		r->BindActiveModel(LONG_GET_MODEL(id));
		r->SetObjectMatrix(LONG_GET_OBJECT(id), glm::translate(glm::mat4{ 1.0f }, glm::vec3{ pos.x, pos.y, 1.1f }), true);
	}

private:
	void initGame(std::filesystem::path path) {
		//DO TOTALNEJ ZMIANY
		path = path.append("Data\\buildings.txt");

		Faction orks = { "Orks",  buildingManager.getRaceBuildings(0) };
		Faction humans = { "Humans", buildingManager.getRaceBuildings(1) };
		Faction nomands = { "Nomads", buildingManager.getRaceBuildings(2) };
		Faction evilHumans = { "EvilHumans",buildingManager.getRaceBuildings(3) };
		Faction goblins = { "Dwards", buildingManager.getRaceBuildings(4) };
		Faction animals = { "Animals" };
		Faction bandits = { "Bandits" };

		//Orks
		int i, chuj = 0;
		for (i = 0; i < 3; i++) {
			//Squad{ , }
			//squads.push_back(Squad{ chuj++, glm::vec2{(rand() % 4096) - 2048,  (rand() % 4096) - 2048 } });
		}
	}

	rasticore::RastiCoreRender* r;


	Squad* player;
	MovementManager movementManager;
	BuildingManager buildingManager;
	std::array<Faction, 5> factions;
	std::vector<Squad> squads;
	InputHandler inputManager;
};