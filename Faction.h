#pragma once
#include <unordered_map>
#include "Squad.h"

class Faction {
public:

private:
	std::unordered_map<uint64_t, Squad>	squad;
};