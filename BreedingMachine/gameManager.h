#pragma once
#include "Faction.h"
#include "MovementManager.h";
#include <array>
#include "inputHandler.h"
#include "BuildingManager.h"
#include "FactionManager.h"
#include <glm/vec2.hpp>
#include <glm/gtx/quaternion.hpp> 

#define THRESHOLD 10.0f

class gameManager {
public:
	gameManager(rasticore::RastiCoreRender* r_, rasticore::ModelCreationDetails rect_mcd) : instance(InputHandler::getInstance()) {
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

		std::random_device rd;
		gen = std::mt19937{ rd() };
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
		instance.handleKeys();
		if (instance.KeyPressed(SDL_SCANCODE_W)) {
			cameraOffset.y += 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_S)) {
			cameraOffset.y -= 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_A)) {
			cameraOffset.x -= 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_D)) {
			cameraOffset.x += 20;
		}
		if (instance.KeyPressed(SDL_SCANCODE_Q)) {
			cameraOffset.z -= 20.0f;
		}
		if (instance.KeyPressed(SDL_SCANCODE_E)) {
			cameraOffset.z += 20.0f;
		}
		if (instance.KeyPressedOnce(SDL_SCANCODE_LEFT)) {
			auto pos = getMousePosition();
			auto start = std::chrono::system_clock::now();

			movementManager.createSquadPath({(int)pos.x, (int)pos.y }, player);

			auto end = std::chrono::system_clock().now();
			auto elapsedMil = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			auto elapsedMic = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

			std::cout << "Time in miliseconds : " << elapsedMil << " Time in microseconds : " << elapsedMic << "\n";
		}
		if (instance.KeyPressedOnce(SDL_SCANCODE_RIGHT)) {
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
		factionManager.CreateNewFaction(MODEL_ANIMALS, "Data\\animal.png", "Furry", buildingManager.getRaceBuildings(MODEL_ANIMALS));

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

		Squad* squad;
		srand(time(NULL));
		player->setSquadState(STAND);
		for (int i = 0; i < 24; i++) {
			auto buildings = buildingManager.getRaceBuildings(i % 8);
			if (i == MODEL_PLAYER || !buildings.size()) continue;
			squad = factionManager.CreateNewSquad(i % 8, buildings.at(rand() % buildings.size()).getBuildingPosition());
			timer.startMeasure(squad->getSquadID(), 0);
			squad->force = getRandomNumber(10, 100);
		}
	}

	float calculateSquadViewDistance(Squad* squad) {
		//TO DO
		return 16.0f * 16;
	}

	void handleSquadLogic() {
		float distance = 0;
		float threashold = 10;
		uint64_t id;
		for (auto& squadF : factionManager.getAllSquads()) {
			if (squadF == player) continue;
			id = squadF->getSquadID();
			for (auto& squadS : factionManager.getAllSquads()) {
				if (squadF != squadS) {
					handleSquadState(squadF, squadS);
					handleSquadStateLogic(squadF);
				}
			}
		}
	}

	void SquadRetreat(Squad* s1, Squad* s2) {
		glm::vec2 position1, position2;
		position1 = getCorrectedSquadPosition(s1->getSquadPosition());
		position2 = getCorrectedSquadPosition(s2->getSquadPosition());

		glm::vec2 dirVec = glm::normalize(position2 - position1) * 16.0f * 6.0f; // 22.6274166f
		glm::vec2 endPoint = position1 - dirVec;
		int pathFound = movementManager.createSquadPath(Astar::point{ (int)endPoint.x, (int)endPoint.y }, s1);
		int angle = 30;
		glm::mat2 rotationMatrix;
		while (!pathFound && angle <= 180) {
			//if (angle >= 180) return;
			rotationMatrix = glm::mat2(
				glm::cos(360 - angle), -glm::sin(360 - angle),
				glm::sin(360 - angle), glm::cos(360 - angle)
			);
			dirVec = (dirVec * rotationMatrix);
			endPoint = position1 + dirVec;
			pathFound = movementManager.createSquadPath(Astar::point{ (int)endPoint.x, (int)endPoint.y }, s1);
			
			if (pathFound) return;

			rotationMatrix = glm::mat2(
				glm::cos(angle), -glm::sin(angle),
				glm::sin(angle), glm::cos(angle)
			);
			dirVec = (dirVec * rotationMatrix);
			endPoint = position1 + dirVec;
			pathFound = movementManager.createSquadPath(Astar::point{ (int)endPoint.x, (int)endPoint.y }, s1);

			angle += 20;
		}
	}

	void SquadChase(Squad* s1, Squad* s2) {
		movementManager.createSquadPath(Astar::point{ (int)s2->getSquadPosition().x, (int)s2->getSquadPosition().y }, s1);
	}

	void SquadPatrol(Squad* squad) {
		auto buildings = buildingManager.getRaceBuildings(squad->getSquadFactionID());
		int buildingAmount = buildings.size();
		if (buildingAmount) {
			if (!movementManager.SquadHasPath(squad)) { 
				glm::vec2 randomBuildingPosition = buildings.at(getRandomNumber(0, buildingAmount - 1)).getBuildingPosition();
				movementManager.createSquadPath(Astar::point{ (int)randomBuildingPosition.x, (int)randomBuildingPosition.y }, squad);
			}
		}
		//IF BUILDINGS NOT FOUND ACT LIKE NORMAL WANDER STATE
		else {
			squad->setSquadState(WANDER);
			timer.startMeasure(squad->getSquadID(), getRandomNumber(20, 50));
		}
	}

	void SquadStand(Squad* squad) {

	}

	void SquadWander(Squad* squad, int distance) {
		while (!movementManager.SquadHasPath(squad)) {
			glm::vec2 squadPosition, newPosition;
			squadPosition = squad->getSquadPosition();
			int angle = getRandomNumber(0, 360);
			glm::mat2 rotationMatrix = glm::mat2(
				glm::cos(angle), -glm::sin(angle),
				glm::sin(angle), glm::cos(angle)
			);
			newPosition = squadPosition + (rotationMatrix * glm::vec2(getRandomNumber(0, distance), getRandomNumber(0, distance)));
			movementManager.createSquadPath(Astar::point{ (int)newPosition.x, (int)newPosition.y }, squad);
		}
	}

	void handleSquadState(Squad* squadF, Squad* squadS) {
		float distance = calculateSquadViewDistance(squadF);
		if (factionManager.getFactionsRelationships(squadS->getSquadFactionID(), squadF->getSquadFactionID()) == ENEMY) {
			if (glm::distance(squadF->getSquadPosition(), squadS->getSquadPosition()) <= distance) {
				if (abs(squadF->force - squadS->force) <= THRESHOLD) {
					//if(calculateChance(80)) 
					squadF->setSquadState(CHASE);
					stateH[squadF->getSquadID()] = squadS;
				}
				else {
					squadF->setSquadState(RETREAT);
					stateH[squadF->getSquadID()] = squadS;
				}
				//if (stateH.find(squadF->getSquadID()) != stateH.end()) stateH.erase(squadF->getSquadID());
				timer.resetTimer(squadF->getSquadID());
				return;
			}
		}
		if (!timer.hasTimeElapsed(squadS->getSquadID())) return;
			
		//Default options
		int state = getRandomNumber(0,2);
		switch (state)
		{
		case STAND:
			squadF->setSquadState(STAND);
			timer.startMeasure(squadF->getSquadID(), getRandomNumber(5, 10));
			break;
		case PATROL:
			squadF->setSquadState(PATROL);
			timer.startMeasure(squadF->getSquadID(), getRandomNumber(30, 60));
			break;
		case WANDER:
			squadF->setSquadState(WANDER);
			timer.startMeasure(squadF->getSquadID(), getRandomNumber(20, 50));
			break;
		default:
			break;
		}
		
	}

	glm::vec2 getCorrectedSquadPosition(glm::vec2 position) {
		float offset, tileSize = movementManager.getMapTileSize() / 2.0f;
		offset = tileSize / 2.0f;
		position.x = ((int)(position.x - offset) / tileSize) * tileSize;
		position.y = ((int)(position.y - offset) / tileSize) * tileSize;
		return position;
	}

	void handleSquadStateLogic(Squad* squadF) {
		switch (squadF->getSquadState()) {
		case PATROL:
			SquadPatrol(squadF);
			break;
		case STAND:
			SquadStand(squadF);
			break;
		case WANDER:
			SquadWander(squadF, 16 * 32);
			break;
		case RETREAT:
			SquadRetreat(squadF, stateH[squadF->getSquadID()]);
			break;
		case CHASE:
			SquadChase(squadF, stateH[squadF->getSquadID()]);
			break;
		}
	}

	bool calculateChance(int chance) {
		chance = glm::clamp(chance, 0, 100);
		return (std::uniform_int_distribution(0, chance)(gen) == 0);
	}

	int getSquadDrawnState(int amountOfStates) {
		auto val = std::uniform_int_distribution(0, amountOfStates - 1)(gen);
		return val;
	}

	int getRandomNumber(int min, int max) {
		auto val = std::uniform_int_distribution(min, max)(gen);
		return val;
	}

private:
	rasticore::RastiCoreRender* r;
	rasticore::ModelCreationDetails rect_mcd;

	Squad* player;
	MovementManager movementManager;
	BuildingManager buildingManager;
	FactionManager factionManager;
	//std::vector<Squad*> squads;
	CameraOffset cameraOffset;
	InputHandler& instance;

	std::unordered_map<uint64_t, Squad*> stateH;
	//
	Timer timer;
	std::mt19937 gen;
};