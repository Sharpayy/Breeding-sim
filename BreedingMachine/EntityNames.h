#pragma once
#include <iostream>
#include "Define.h"
#include <unordered_map>
#include <string.h>
#include <vector>
#include <fstream>

std::unordered_map<uint8_t, std::vector<std::string>> factionNames;

std::string getRandomFactionName(uint8_t factionID) {
	int size = factionNames[factionID].size();
	std::string name;
	if (!size) return "MAMBAJAMBA FLORCZYK KARAMBA";
	return factionNames[factionID].at(rand() % size);
}

bool loadNames(const char* path, uint8_t factionID) {
	std::fstream file;

	file.open(path);

	if (!file) return false;

	int size;
	file >> size;

	std::string name;
	int idx;
	for (idx = 1; idx < size + 1; idx++) {
		file >> name;
		factionNames[factionID].push_back(name);
	}
	return true;
}