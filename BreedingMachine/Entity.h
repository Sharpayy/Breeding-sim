#pragma once
#include <iostream>
#include "glm/vec2.hpp"
#include "EntityStatistics.h"

class Entity {
public:

	//IMPL
private:
	glm::vec2 position;
	uint32_t id;
	Stats stats;
};