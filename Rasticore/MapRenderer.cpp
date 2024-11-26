#include "rasti_main.h"
#include "rasti_alloc.h"
#include "rasti_utils.h"
#include <glm/gtc/matrix_transform.hpp>

class MapChunk
{
public:
	rasticore::Texture2DBindless txb;
	rasticore::Texture2D tx;
	STACK<void*> units;

	MapChunk()
	{
		tx = rasticore::Texture2D();
		txb = rasticore::Texture2DBindless(tx);
		units = STACK<void*>(8);
	}

	void SetTexture(rasticore::Texture2D tex)
	{
		tx = tex;
		txb = rasticore::Texture2DBindless(tx);
	}

	MapChunk(rasticore::Texture2D tex)
	{
		tx = tex;
		txb = rasticore::Texture2DBindless(tx);
		units = STACK<void*>(8);
	}
};

class GameMap
{
public:
	uint32_t pMapSizeX, pMapSizeY;
	uint32_t pChunkSizeX, pChunkSizeY;

	uint32_t blk;

	rasticore::VertexBuffer rChunkVbo;

	MapChunk* aChunkArray;

	
	GameMap(rasticore::Image mapImg, uint32_t blocks)
	{

		pMapSizeX = mapImg.x_;
		pMapSizeY = mapImg.y_;

		pChunkSizeX = pMapSizeX / blocks;
		pChunkSizeY = pMapSizeY / blocks;

		blk = blocks;

		aChunkArray = new MapChunk[blk*blk];

		for (int y = 0; y < blk; y++)
		{
			for (int x = 0; x < blk; x++)
			{
				
			}
		}

	}

	

};