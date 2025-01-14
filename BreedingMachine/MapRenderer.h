#include "rasticore/rasti_main.h"
#include "rasticore/rasti_alloc.h"
#include "rasticore/rasti_utils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include "Define.h"

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
		return aChunkArray + (blk * y + x);
	}
};

class GameMapProxy
{
public:
	GameMapProxy(GameMap* gm)
	{
		map = gm;
	}

	MapChunk* GetMapChunk(int x, int y, glm::mat4 m)
	{
		glm::vec2 sPoint = glm::vec2(x * map->pChunkSizeX, y * map->pChunkSizeY);
		glm::vec2 ePoint = glm::vec2(x * map->pChunkSizeX + map->pChunkSizeX, y * map->pChunkSizeY + map->pChunkSizeY);
		glm::vec4 res;

		res = m * glm::vec4(sPoint.x, sPoint.y, 0.0f, 1.0f);
		sPoint = glm::vec2(res.x, res.y);
		res = m * glm::vec4(ePoint.x, ePoint.y, 0.0f, 1.0f);
		ePoint = glm::vec2(res.x, res.y);
		glm::vec2 chunkSize = (ePoint - sPoint);

		glm::vec2 mid = sPoint + (chunkSize / 2.0f);

		if (glm::distance(glm::vec2(0.0f), mid) <= (sqrt(2.0f) * 5.0f) + (sqrt(2.0f) * chunkSize.x))
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