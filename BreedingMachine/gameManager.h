#pragma once
#include "Faction.h"
#include "MovementManager.h";
#include <array>
#include "inputHandler.h"
#include "BuildingManager.h"
#include "FactionManager.h"
#include <glm/vec2.hpp>

class gameManager {
public:
	gameManager(rasticore::RastiCoreRender* r_, rasticore::ModelCreationDetails rect_mcd) {
		this->rect_mcd = rect_mcd;
		this->r = r_;
		std::filesystem::path path = std::filesystem::current_path();
		std::filesystem::path collisionPath = path, buildingPath = path;
		collisionPath.append("Data\\collision.txt");
		buildingPath.append("Data\\buildings.txt");
		movementManager = MovementManager{ collisionPath, 4096, 16, r_, rect_mcd };
		buildingManager = BuildingManager{ buildingPath };
		factionManager = FactionManager{r_, rect_mcd, 16};

		cameraOffset = CameraOffset{ 0, 0, 500.0f };
		initGame(path);
	}
	
	void update() {
		inputHandler();
		//auto pos = getMousePosition();
		movementManager.update();
		handleSquadLogic();
		//Astar::point p;
		//for (auto& squad : squads) {
		//	SetSquadPosition(squad->getSquadPosition(), squad);
		//}

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

	void inputHandler() {
		inputManager.handleKeys();
		if (inputManager.KeyPressed(SDL_SCANCODE_W)) {
			cameraOffset.y += 20;
		}
		if (inputManager.KeyPressed(SDL_SCANCODE_S)) {
			cameraOffset.y -= 20;
		}
		if (inputManager.KeyPressed(SDL_SCANCODE_A)) {
			cameraOffset.x -= 20;
		}
		if (inputManager.KeyPressed(SDL_SCANCODE_D)) {
			cameraOffset.x += 20;
		}
		if (inputManager.KeyPressed(SDL_SCANCODE_Q)) {
			cameraOffset.z -= 20.0f;
		}
		if (inputManager.KeyPressed(SDL_SCANCODE_E)) {
			cameraOffset.z += 20.0f;
		}
		if (inputManager.KeyPressedOnce(SDL_SCANCODE_LEFT)) {
			auto pos = getMousePosition();
			movementManager.createSquadPath({(int)pos.x, (int)pos.y }, player);
		}
		if (inputManager.KeyPressedOnce(SDL_SCANCODE_RIGHT)) {
			auto pos = getMousePosition();
			pos.x = (int)(pos.x / 16) * 16;
			pos.y = (int)(pos.y / 16) * 16;
			player->setSquadPosition(pos);
			uint64_t id = player->getSquadID();
			r->BindActiveModel(LONG_GET_MODEL(id));
			r->SetObjectMatrix(LONG_GET_OBJECT(id), glm::translate(glm::mat4{ 1.0f }, glm::vec3{ player->getSquadPosition(), 1.1f}), true);

		}
	}

	struct CameraOffset {
		float x = 0, y = 0, z = 0;
	};

	CameraOffset getCameraOffset() {
		return cameraOffset;
	}

	glm::vec2 getMousePosition() {
		int x, y;
		SDL_GetMouseState(&x, &y);

		glm::vec3 screen = glm::vec3(800.0f);

		glm::vec4 nds;
		nds.x = (2.0f * (float)x) / screen.x - 1.0f;
		nds.y = (2.0f * (float)y) / screen.y - 1.0f;
		nds.z = 1.0f;
		nds.w = 1.0f;

		nds.y *= -1.0f;

		glm::mat4 inv_projection = glm::inverse(r->MVP.matProj);

		nds = inv_projection * nds;

		glm::mat4 inv_camera = glm::inverse(r->MVP.matCamera);

		nds = inv_camera * nds;

		return glm::vec2(nds.x, nds.y);
	}

private:
	void initGame(std::filesystem::path path) {
		//DO TOTALNEJ ZMIANY
		path = path.append("Data\\buildings.txt");

		factionManager.CreateNewFaction(MODEL_ORKS, "Data\\ork.png", "Orks", buildingManager.getRaceBuildings(MODEL_ORKS));
		factionManager.CreateNewFaction(MODEL_HUMANS, "Data\\human.png", "Humans", buildingManager.getRaceBuildings(MODEL_HUMANS));
		factionManager.CreateNewFaction(MODEL_NOMADS, "Data\\mongo.png", "Nomads", buildingManager.getRaceBuildings(MODEL_NOMADS));
		factionManager.CreateNewFaction(MODEL_EVIL_HUMANS, "Data\\evil_human.png", "EvilHumans", buildingManager.getRaceBuildings(MODEL_EVIL_HUMANS));
		factionManager.CreateNewFaction(MODEL_GOBLINS, "Data\\goblin.png", "Goblin", buildingManager.getRaceBuildings(MODEL_GOBLINS));
		factionManager.CreateNewFaction(MODEL_PLAYER, "Data\\player.png", "Player", buildingManager.getRaceBuildings(MODEL_PLAYER));
		factionManager.CreateNewFaction(MODEL_BANDITS, "Data\\bandit.png", "Bandit", buildingManager.getRaceBuildings(MODEL_BANDITS));
		factionManager.CreateNewFaction(MODEL_ANIMALS, "Data\\animal.png", "Animal", buildingManager.getRaceBuildings(MODEL_ANIMALS));

		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_HUMANS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_EVIL_HUMANS, ALLY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_NOMADS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_ORKS, ALLY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_PLAYER, ENEMY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_BANDITS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_GOBLINS, MODEL_ANIMALS, ENEMY);


		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_HUMANS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_PLAYER, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_ANIMALS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_EVIL_HUMANS, ALLY);
		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_BANDITS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ORKS, MODEL_NOMADS, ENEMY);


		factionManager.setFactionsRelationships(MODEL_EVIL_HUMANS, MODEL_PLAYER, NEUTRAL);
		factionManager.setFactionsRelationships(MODEL_EVIL_HUMANS, MODEL_ANIMALS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_EVIL_HUMANS, MODEL_HUMANS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_EVIL_HUMANS, MODEL_BANDITS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_EVIL_HUMANS, MODEL_NOMADS, NEUTRAL);


		factionManager.setFactionsRelationships(MODEL_ANIMALS, MODEL_PLAYER, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ANIMALS, MODEL_HUMANS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ANIMALS, MODEL_NOMADS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_ANIMALS, MODEL_BANDITS, ENEMY);


		factionManager.setFactionsRelationships(MODEL_PLAYER, MODEL_NOMADS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_PLAYER, MODEL_HUMANS, NEUTRAL);
		factionManager.setFactionsRelationships(MODEL_PLAYER, MODEL_BANDITS, ENEMY);


		factionManager.setFactionsRelationships(MODEL_NOMADS, MODEL_BANDITS, ENEMY);
		factionManager.setFactionsRelationships(MODEL_NOMADS, MODEL_HUMANS, ENEMY);


		factionManager.setFactionsRelationships(MODEL_HUMANS, MODEL_BANDITS, ENEMY);

		player = factionManager.CreateNewSquad(MODEL_PLAYER, glm::vec2(-1000.0f));
		player->force = 1.0f;

		srand(time(NULL));
		for (int i = 0; i < 16; i++) {
			auto buildings = buildingManager.getRaceBuildings(i % 8);
			if (i == MODEL_PLAYER || !buildings.size()) continue;
			factionManager.CreateNewSquad(i % 8, buildings.at(rand() % buildings.size()).getBuildingPosition());
		}
	}

	float calculateSquadViewDistance(Squad* squad) {
		//TO DO
		return 16.0f * 16 * 16;
	}

	void handleSquadLogic() {
		float distance = 0;
		uint64_t id;
		for (auto& squad : factionManager.getAllSquads()) {
			id = squad->getSquadID();
			distance = calculateSquadViewDistance(squad);

			if (squad != player) {
				if (factionManager.getFactionsRelationships(squad->getSquadFactionID(), player->getSquadFactionID()) == ENEMY) {
					if (glm::distance(squad->getSquadPosition(), player->getSquadPosition()) <= distance) {
						if (squad->force < player->force) {
							SquadRetreat(squad, player);
						}
						else {
							SquadChase(squad, player);
						}
					}
				}
			}
			//BruteForce TODO
			//factionManager.
		}
	}

	void SquadRetreat(Squad* s1, Squad* s2) {
		glm::vec2 position1, position2;
		position1 = getCorrectedSquadPosition(s1->getSquadPosition());
		position2 = getCorrectedSquadPosition(s2->getSquadPosition());

		glm::vec2 dirVec = glm::normalize(position2 - position1) * 16.0f * 2.0f; // 22.6274166f
		glm::vec2 endPoint = position1 - dirVec;
		movementManager.createSquadPath(Astar::point{ (int)endPoint.x, (int)endPoint.y }, s1);
	}

	glm::vec2 getCorrectedSquadPosition(glm::vec2 position) {
		float offset, tileSize = movementManager.getMapTileSize() / 2.0f;
		offset = tileSize / 2.0f;
		position.x = ((int)(position.x - offset) / tileSize) * tileSize;
		position.y = ((int)(position.y - offset) / tileSize) * tileSize;
		return position;
	}

	void SquadChase(Squad* s1, Squad* s2) {
		auto start = std::chrono::system_clock::now();
		auto pos = getCorrectedSquadPosition(s2->getSquadPosition());
		movementManager.createSquadPath(Astar::point{ (int)s2->getSquadPosition().x, (int)s2->getSquadPosition().y }, s1);

		auto end = std::chrono::system_clock().now();
		auto elapsedMil = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		auto elapsedMic = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		if (elapsedMil > 0) {
			std::cout << "Time in miliseconds : " << elapsedMil << " Time in microseconds : " << elapsedMic << "\n";
		}
	}

	rasticore::RastiCoreRender* r;
	rasticore::ModelCreationDetails rect_mcd;

	Squad* player;
	MovementManager movementManager;
	BuildingManager buildingManager;
	FactionManager factionManager;
	//std::vector<Squad*> squads;
	InputHandler inputManager;

	CameraOffset cameraOffset;
};