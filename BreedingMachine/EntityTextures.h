#pragma once
#include "textures.h"
#include <ctime>

#define INCLUDE_ENTITY_TEXTURE_MAP	extern const char* LDR_MODELS LDR_FACTION_TEXTURE_MAP[]

#define MODEL_ORKS 0
#define MODEL_HUMANS 1
#define MODEL_NOMADS 2
#define MODEL_EVIL_HUMANS 3
#define MODEL_GOBLINS 4
#define MODEL_PLAYER 5
#define MODEL_BANDITS 6
#define MODEL_ANIMALS 7

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
