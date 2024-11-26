#include "rasti_adv.h"

rasticore::ShaderStorageBufferOrphan::ShaderStorageBufferOrphan(GLenum buffer_flags, uint32_t buffer_size)
{
	glCreateBuffers(1, &id);
	size = buffer_size;
	flg = buffer_flags;
}

void rasticore::ShaderStorageBufferOrphan::StartModify()
{
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, 0, flg);
}

void rasticore::ShaderStorageBufferOrphan::data(void* data, uint32_t s)
{
	if (s == 0)
		s = size;
	StartModify();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, s, data);
}

void rasticore::ShaderStorageBufferOrphan::bind()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
}

void rasticore::ShaderStorageBufferOrphan::bindBase(uint32_t index)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, id);
}

void rasticore::ShaderStorageBufferOrphan::subdata(size_t offset, size_t size, void* data)
{

}
