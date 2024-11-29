#pragma once
#include "Astar.h"
#include "Entity.h"
#include <vector>

class MovementManager {
public:
	bool moveEntity(Entity& entity) {

	}

private:
	enum MovementState {
		IDLE,
		RUNNING,
	};

	Astar* movement;
};