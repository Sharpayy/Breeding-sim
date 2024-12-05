#include "rasti_alloc.h"

rasticore::RsMatrixAllocator::RsMatrixAllocator(DWORD s_alloc)
{
	actPagePool = std::list<LPVOID>();
	//fulPagePool = std::list<LPVOID>();

	pgAllocSize = 2;
	AllocNewMatrixPage(s_alloc);

	cPage = (MATRIXPAGE*)actPagePool.front();
}

void inline rasticore::RsMatrixAllocator::InitMatrixPage(MATRIXPAGE* mp)
{
	mp->d_0 = 0x7FFFFFFFFFFFFFFF;
}

LPVOID rasticore::RsMatrixAllocator::AllocMatrix()
{
	unsigned long index;
	if (!_BitScanForward64(&index, cPage->d_0))
	{
		//fulPagePool.push_back(cPage);
		actPagePool.pop_front();

		if (actPagePool.size() == 0)
			AllocNewMatrixPage(pgAllocSize);
		cPage = (MATRIXPAGE*)actPagePool.front();
	}

	cPage->d_0 = cPage->d_0 ^ (1 << index);
	return cPage->data + index * 64;
}

void rasticore::RsMatrixAllocator::FreeMatrix(LPVOID mat)
{
	MATRIXPAGE* pg = (MATRIXPAGE*)((DWORD64)mat & 0xffffffffffff0000);
	int64_t index = (((char*)mat - (char*)pg->data) / 64) + 1;


	if ((pg->d_0 & index) == index)
		return;

	if (IS_MATRIXPAGE_FULL(pg))
		actPagePool.push_back(pg);
	pg->d_0 = pg->d_0 ^ index;
}

void rasticore::RsMatrixAllocator::AllocNewMatrixPage(DWORD cnt)
{
	LPVOID mem = VirtualAlloc(NULL, 4096 * cnt, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	for (int i = 0; i < cnt; i++)
	{
		InitMatrixPage((MATRIXPAGE*)((uint8_t*)mem + 4096 * i));
		actPagePool.push_back((uint8_t*)mem + 4096 * i);
	}
}

LPVOID rasticore::RsAlloc(SIZE_T size)
{
	return VirtualAlloc(NULL, min(4096, size), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}
