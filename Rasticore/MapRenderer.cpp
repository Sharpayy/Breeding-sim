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

	
	GameMap(rasticore::Image mapImg, uint32_t blocks)
	{

		pMapSizeX = mapImg.x_;
		pMapSizeY = mapImg.y_;

		pChunkSizeX = pMapSizeX / blocks;
		pChunkSizeY = pMapSizeY / blocks;

		blk = blocks;

		aChunkArray = new MapChunk[blk*blk];
		uint8_t* img_buffer = new uint8_t[pChunkSizeX * pChunkSizeY * 4];

		float plane_vtx[] = {
		
				-0.5f, -0.5f, 0.0f,
				-0.5f, 0.5f, 0.0f,
				0.5, -0.5f, 0.0f,
				0.5f, 0.5f, 0.0f
		};

		rChunkVbo = rasticore::Buffer<GL_ARRAY_BUFFER>(sizeof(plane_vtx), plane_vtx, GL_STATIC_DRAW);
		rChunkVao = rasticore::VertexBuffer();
		rChunkVao.bind();

		rChunkVao.bind();
		rChunkVao.addAttrib(GL_FLOAT, 0, 3, 12, 0);
		rChunkVao.enableAttrib(0);
		

		for (int y = 0; y < blk; y++)
		{
			for (int x = 0; x < blk; x++)
			{
				rasticore::Texture2D _tx;
				mapImg.CopyImageBlock(x * pChunkSizeX, y * pChunkSizeY, pChunkSizeX, pChunkSizeY, img_buffer);
				_tx = rasticore::Texture2D(img_buffer, pChunkSizeX, pChunkSizeY, GL_RGBA, GL_RGBA8);
				aChunkArray[blk * y + x] = MapChunk(_tx);
			}
		}

		delete[] img_buffer;

	}

	

};