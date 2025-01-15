#pragma once
#include "random"
#include "glm/glm.hpp"

class Generator {
public:
	static Generator& getInstance() {
		static Generator generatorInstance;
		return generatorInstance;
	}

	Generator(const Generator&) = delete;
	Generator& operator=(const Generator&) = delete;

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
	Generator() {
		std::random_device rd;
		gen = std::mt19937{ rd() };
	}
	std::mt19937 gen;
};
