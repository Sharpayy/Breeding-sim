#include "EntityTextures.h"

const char* LDR_ORC_TEXTURE[] = {
	"Data\\bt_orc_0.png",
	"Data\\bt_orc_1.png"
};

const char* LDR_HUMAN_TEXTURE[] = {
	"Data\\bt_human_0.png",
	"Data\\bt_human_1.png",
	"Data\\bt_human_2.png"
};

LDR_MODELS LDR_FACTION_TEXTURE_MAP[] = {
	LDR_MODELS{ LDR_ORC_TEXTURE, sizeof(LDR_ORC_TEXTURE) / sizeof(LDR_ORC_TEXTURE[0]) },
	LDR_MODELS{ LDR_HUMAN_TEXTURE, sizeof(LDR_HUMAN_TEXTURE) / sizeof(LDR_HUMAN_TEXTURE[0]) },
	LDR_MODELS{ nullptr, 0 },
	LDR_MODELS{ nullptr, 0 },
	LDR_MODELS{ nullptr, 0 },
	LDR_MODELS{ LDR_HUMAN_TEXTURE, sizeof(LDR_HUMAN_TEXTURE) / sizeof(LDR_HUMAN_TEXTURE[0]) },
	LDR_MODELS{ nullptr, 0 },
	LDR_MODELS{ nullptr, 0 },
};

uint64_t GetEntityTextureFromIndex(uint64_t Index, uint8_t factionId)
{
	LDR_MODELS* model = LDR_FACTION_TEXTURE_MAP + factionId;

	if (Index > model->amount)
		return 0;

	if (model->models == 0)
		return 0;

	return LoadTextureFromFile(model->models[Index]);
}

uint64_t GetEntityRandomTextureIndex(uint8_t factionId)
{
	LDR_MODELS* model = LDR_FACTION_TEXTURE_MAP + factionId;

	if (model->amount == 0)
		return 0;

	return rand() % model->amount;
}
