#pragma once
#include <Windows.h>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

namespace rasticore
{
	class Image
	{
	public:
		uint8_t* data;
		uint32_t x_, y_;
		uint32_t channels;

		Image(const char* filename, int comps);
		~Image();

		void WritePixel(uint32_t x, uint32_t y, uint32_t d);
		uint32_t ReadPixel(uint32_t x, uint32_t y);

		void CopyImageBlock(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t* d);
	};

	class Ray
	{
	public:
		glm::vec3 d;
		glm::vec3 o;

		Ray(glm::vec3 d, glm::vec3 o);
	};

}

typedef struct RS_MAPPEDFILE
{
	HANDLE file;
	HANDLE map;
	LPVOID data;

} MAPPEDFILE;

namespace rasticore
{
	MAPPEDFILE MapFile(const char* filename);

	void UnmapFile(MAPPEDFILE mf);
}