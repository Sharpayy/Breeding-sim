#pragma once

struct Stats {
public:
	uint8_t melee;
	uint8_t defense;
	uint8_t ranged;
	uint8_t bravery;
	uint8_t stamina;
	uint8_t hp;
};

Stats defaultStats = {
	0,
	0,
	0,
	0,
	0,
	0
};