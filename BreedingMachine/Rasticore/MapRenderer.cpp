#include "rasti_main.h"
#include "rasti_alloc.h"
#include "rasti_utils.h"
#include <glm/gtc/matrix_transform.hpp>

class MapChunk
{
public:
	rasticore::Texture2DBindless txb;
	rasticore::Texture2D tx;

	MapChunk()
	{
		tx = rasticore::Texture2D();
		txb = rasticore::Texture2DBindless(tx);
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
	}
};

class GameMap
{
public:
	uint32_t pMapSizeX, pMapSizeY;
	uint32_t pChunkSizeX, pChunkSizeY;

	uint32_t blk;

	rasticore::VertexBuffer rChunkVao;
	rasticore::Buffer<GL_ARRAY_BUFFER> rChunkVbo;

	MapChunk* aChunkArray;

	
	GameMap(rasticore::Image* mapImg, uint32_t blocks)
	{

		pMapSizeX = mapImg->x_;
		pMapSizeY = mapImg->y_;

		pChunkSizeX = pMapSizeX / blocks;
		pChunkSizeY = pMapSizeY / blocks;

		blk = blocks;

		//float plane_vtx[] = {
		//
		//		-0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		//		-0.5f, 0.5f, 1.0f, 0.0f, 1.0f,
		//		0.5, -0.5f, 1.0f, 1.0f, 0.0f,

		//		-0.5f, 0.5f, 1.0f, 0.0f, 1.0f,
		//		0.5, -0.5f, 1.0f, 1.0f, 0.0f,
		//		0.5, 0.5f, 1.0f, 1.0f, 1.0f
		//};

		float plane_vtx[] = {

			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f,

			0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 1.0f
		};

		rChunkVbo = rasticore::Buffer<GL_ARRAY_BUFFER>(sizeof(plane_vtx), plane_vtx, GL_STATIC_DRAW);
		rChunkVao = rasticore::VertexBuffer();
		rChunkVao.bind();

		rChunkVao.bind();
		rChunkVao.addAttrib(GL_FLOAT, 0, 3, 20, 0);
		rChunkVao.addAttrib(GL_FLOAT, 1, 2, 20, 12);
		rChunkVao.enableAttrib(0);
		rChunkVao.enableAttrib(1);

		aChunkArray = new MapChunk[blk * blk];
		uint8_t* img_buffer = new uint8_t[pChunkSizeX * pChunkSizeY * 4];


		for (int y = 0; y < blk; y++)
		{
			for (int x = 0; x < blk; x++)
			{
				rasticore::Texture2D _tx;
				mapImg->CopyImageBlock(x * pChunkSizeX, y * pChunkSizeY, pChunkSizeX, pChunkSizeY, img_buffer);
				_tx = rasticore::Texture2D(img_buffer, pChunkSizeX, pChunkSizeY, GL_RGBA, GL_RGBA8);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				aChunkArray[blk * y + x] = MapChunk(_tx);
				aChunkArray[blk * y + x].txb.MakeResident();
			}
		}

		delete[] img_buffer;

	}

	MapChunk* GetMapChunk(int x, int y)
	{
		return aChunkArray+(blk * y + x);
	}
};

class GameMapProxy
{
public:
	GameMapProxy(GameMap* gm)
	{
		map = gm;
	}

	MapChunk* GetMapChunk(int x, int y, float vVisMapL, float vVisMapR, float vVisMapU, float vVisMapD)
	{
		float cX = x * map->pChunkSizeX;
		float cY = y * map->pChunkSizeY;

		vVisMapL = 0;
		vVisMapU = 0;
		vVisMapR *= 2.0f;
		vVisMapD *= 2.0f;

		if ((cX >= vVisMapL && cX + map->pChunkSizeX <= vVisMapR) && (cY >= vVisMapU && cY + map->pChunkSizeY <= vVisMapD))
			{
				auto c = map->GetMapChunk(x, y);
				int64_t packed = x;
				packed = packed << 32;
				packed += y;
				if (cache.find(packed) == cache.end())
				{
					c->txb.MakeResident();
					cache.insert({ packed, c });
				}
				return c;
			}

		auto c = map->GetMapChunk(x, y);

		int64_t packed = x;
		packed = packed << 32;
		packed += y;
		if (cache.find(packed) != cache.end())
		{
			c->txb.MakeNonResident();
			cache.erase(packed);
		}
		return 0;
	}


private:
	GameMap* map;
	std::unordered_map<int64_t, MapChunk*> cache;
};