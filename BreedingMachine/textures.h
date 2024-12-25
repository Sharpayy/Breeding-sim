#pragma once

#include <unordered_map>
#include "Rasticore/rasti_main.h"

typedef struct _TEXTURE_CONTAINER
{
	rasticore::Texture2D tex;
	rasticore::Texture2DBindless txb;
} TEXTURE_CONTAINER;

extern std::unordered_map<const char*, TEXTURE_CONTAINER> texture_store;

void texture_store_init();

uint64_t LoadTextureFromFile(const char* file, const char* tag = 0);