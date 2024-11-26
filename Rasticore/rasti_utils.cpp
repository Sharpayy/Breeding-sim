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
