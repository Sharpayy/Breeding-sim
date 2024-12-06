#pragma once
#include "Faction.h"
#include "MovementManager.h";
#include <array>
#include "inputHandler.h"
#include "BuildingManager.h"
#include "Rasticore/rasti_main.h"
#include <glm/vec2.hpp>

class gameManager {
public:
	gameManager(rasticore::RastiCoreRender* r_, rasticore::ModelCreationDetails rect_mcd) {
		this->rect_mcd = rect_mcd;
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

		Astar::point p;
		for (auto& squad : squads) {
			SetSquadPosition(squad->getSquadPosition(), squad);
		}
		auto pos = getMousePosition();
		std::cout << pos.x << " " << pos.y << "\n";

		r->RenderSelectedModel(MODEL_PLAYER);
		r->RenderSelectedModel(MODEL_ORKS);
		r->RenderSelectedModel(MODEL_HUMANS);
		r->RenderSelectedModel(MODEL_NOMADS);
		r->RenderSelectedModel(MODEL_GOBLINS);
		r->RenderSelectedModel(MODEL_EVIL_HUMANS);
		//_r.RenderSelectedModel(MODEL_BANDITS);
		//_r.RenderSelectedModel(MODEL_ANIMALS);
		//Test sln

	}

	void CreateNewFaction(uint32_t faction_id, const char* filename, const char* faction_name)
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

		factions[faction_id] = Faction(faction_name, buildingManager.getRaceBuildings(faction_id));
	}

	Squad* CreateNewSquad(uint32_t faction_id, glm::vec2 starting_pos)
	{
		rasticore::RENDER_LONG_ID unique_id;
		r->newObject(faction_id, glm::translate(glm::mat4{ 1.0f }, glm::vec3{ starting_pos.x, starting_pos.y, 1.1f }), (uint64_t*)&unique_id);

		Squad* squad = new Squad(*(uint64_t*)&unique_id, starting_pos);
		squads.push_back(squad);

		return squad;
	}

	void SetSquadPosition(glm::vec2 pos, Squad* squad)
	{
		squad->setSquadPosition(pos);
		uint64_t id = squad->getSquadID();
		r->BindActiveModel(LONG_GET_MODEL(id));
		r->SetObjectMatrix(LONG_GET_OBJECT(id), glm::translate(glm::mat4{ 1.0f }, glm::vec3{ pos.x, pos.y, 1.1f }), true);
	}

	void inputHandler() {
		/*if (inputManager.KeyPressed(SDL_SCANCODE_W)) {
			md.y += 20;
		}
		if (inputManager.KeyPressed(SDL_SCANCODE_S)) {
			md.y -= 20;
		}
		if (inputManager.KeyPressed(SDL_SCANCODE_A)) {
			md.x -= 20;
		}
		if (inputManager.KeyPressed(SDL_SCANCODE_D)) {
			md.x += 20;
		}
		if (inputManager.KeyPressed(SDL_SCANCODE_Q)) {
			md.z -= 20.0f;
		}
		if (inputManager.KeyPressed(SDL_SCANCODE_E)) {
			md.z += 20.0f;
		}*/
	}

private:
	void initGame(std::filesystem::path path) {
		//DO TOTALNEJ ZMIANY
		path = path.append("Data\\buildings.txt");

	/*	Faction orks = { "Orks",  buildingManager.getRaceBuildings(0) };
		Faction humans = { "Humans", buildingManager.getRaceBuildings(1) };
		Faction nomands = { "Nomads", buildingManager.getRaceBuildings(2) };
		Faction evilHumans = { "EvilHumans",buildingManager.getRaceBuildings(3) };
		Faction goblins = { "Dwards", buildingManager.getRaceBuildings(4) };
		Faction animals = { "Animals" };
		Faction bandits = { "Bandits" };*/

		CreateNewFaction(0, "Data\\ork.png", "Orks");
		CreateNewFaction(1, "Data\\human.png", "Humans");
		CreateNewFaction(2, "Data\\mongo.png", "Nomads");
		CreateNewFaction(3, "Data\\evil_human.png", "EvilHumans");
		CreateNewFaction(4, "Data\\goblin.png", "Goblin");
		CreateNewFaction(5, "Data\\player.png", "Player");

		//gmanager.CreateNewFaction(5, "Data\\race_furry.png", &rect_mcd, "");


		//Squad* s0 = CreateNewSquad(MODEL_PLAYER, glm::vec2(0.0f));

		Squad* s1 = CreateNewSquad(MODEL_EVIL_HUMANS, glm::vec2(-2000.0f));
		
		//Squad* s2 = CreateNewSquad(MODEL_ORKS, glm::vec2(2000.0f));

		movementManager.createSquadPath(Astar::point{ 1600,1600 }, s1);
	}

	glm::vec2 getMousePosition() {
		int x, y;
		SDL_GetMouseState(&x, &y);

		//float scaleX = settings->ScaleX / (float)screenHalfWidth;
		//float scaleY = settings->ScaleY / (float)screenHalfHeight;

		int screenHalfWidth, screenHalfHeight;
		screenHalfWidth = (int)movementManager.getMapSize() / 2.0f;
		screenHalfHeight = screenHalfWidth;

		return glm::vec2{ x - screenHalfWidth, screenHalfWidth - y };

		//return glm::fvec2{ (x - (float)screenHalfWidth) * scaleX + settings->MoveX, (-1 * (y - (float)screenHalfHeight)) * scaleY + settings->MoveY };
		//return glm::fvec2{ (x - (float)screenHalfWidth) + settings->MoveX, (-1 * (y - (float)screenHalfHeight)) + settings->MoveY };

	}

	rasticore::RastiCoreRender* r;
	rasticore::ModelCreationDetails rect_mcd;

	Squad* player;
	MovementManager movementManager;
	BuildingManager buildingManager;
	std::array<Faction, 8> factions;
	std::vector<Squad*> squads;
	InputHandler inputManager;
};