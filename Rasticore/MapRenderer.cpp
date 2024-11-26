#include "rasti_main.h"
#include "rasti_alloc.h"
#include <glm/gtc/matrix_transform.hpp>

class MapChunk
{
public:
	rasticore::Texture2DBindless chunkTexture;
	STACK<::rasticore::RENDER_LONG_ID> units;

	MapChunk(rasticore::Texture2DBindless tex)
	{
		chunkTexture = tex;

		units = STACK<rasticore::RENDER_LONG_ID>(8);
	}
};

class GameMap
{
public:
	uint32_t pMapSizeX, pMapSizeY;
	uint32_t pChunkSizeX, pChunkSizeY;

	uint32_t chunkCntX, chunkCntY;

	rasticore::VertexBuffer rChunkVbo;

	MapChunk* aChunkArray;

	
	

	

};