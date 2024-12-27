#pragma once
#include "textures.h"
#include "filesystem"
#include <string>

void loadMultipleTextures() {
	std::string path;
	char* pathcstr;
	for (auto file : std::filesystem::directory_iterator("Data\\Equipment Icons")) {
		auto path = file.path();
		LoadTextureFromFile(path.string().c_str(), path.filename().string().c_str());
	}
}