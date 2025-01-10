#pragma once
#include "textures.h"
#include <ctime>
#include "Define.h"

#define INCLUDE_ENTITY_TEXTURE_MAP	extern const char* LDR_MODELS LDR_FACTION_TEXTURE_MAP[]


typedef struct _LDR_MODELS
{
	const char** models;
	uint32_t amount;
} LDR_MODELS;

extern const char* LDR_ORC_TEXTURE[];
extern const char* LDR_HUMAN_TEXTURE[];

extern LDR_MODELS LDR_FACTION_TEXTURE_MAP[8];

#define MODEL_ENEMY_FACTION_BASE		20
#define MODEL_PLAYER_FACTION_BASE		40
#define LDR_MAX_FACTION_MODELS		    16

uint64_t GetEntityTextureFromIndex(uint64_t Index, uint8_t factionId);

uint64_t GetEntityRandomTextureIndex(uint8_t factionId);
