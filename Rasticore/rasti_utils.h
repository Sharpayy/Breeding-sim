#pragma once
#include <Windows.h>

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