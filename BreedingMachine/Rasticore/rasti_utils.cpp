#include "rasti_utils.h"

MAPPEDFILE rasticore::MapFile(const char* filename)
{
		MAPPEDFILE o;
		o.file = CreateFileA(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		o.map = CreateFileMappingA(o.file, 0, PAGE_READONLY, 0, 0, 0);
		o.data = MapViewOfFile(o.map, FILE_MAP_READ, 0, 0, 0);
		return o;
}

void rasticore::UnmapFile(MAPPEDFILE mf)
{
	UnmapViewOfFile(mf.data);
	CloseHandle(mf.map);
	CloseHandle(mf.file);
}

rasticore::Image::Image(const char* filename, int comps)
{
	data = stbi_load(filename, (int*)&x_, (int*)&y_, (int*)&channels, comps);
	if (comps != 0)
		channels = comps;
}

rasticore::Image::~Image()
{
	stbi_image_free(data);
}

void rasticore::Image::WritePixel(uint32_t x, uint32_t y, uint32_t d)
{
		
}

uint32_t rasticore::Image::ReadPixel(uint32_t x, uint32_t y)
{
	return 0;
}

void rasticore::Image::CopyImageBlock(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t* d)
{
	uint32_t of = (x_ * y + x) * channels;//(x_ * channels * y) + (channels * x);
	uint32_t off = 0;
	for (int i = 0; i < h; i++)
	{
		memcpy(d + off, data + of, channels * w);
		of += channels * x_;
		off += channels * w;
	}
}

rasticore::Ray::Ray(glm::vec3 d, glm::vec3 o)
{
	this->d = d;
	this->o = o;
}
