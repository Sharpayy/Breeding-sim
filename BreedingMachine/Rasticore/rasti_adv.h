#pragma once
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <string>

namespace rasticore
{

	class ShaderStorageBufferAdv2
	{
	public:
		uint32_t buffers[2];
		char buffer_active_write;

	};

	class ShaderStorageBufferOrphan
	{
	public:
		uint32_t id;
		GLenum flg;
		uint32_t size;

		ShaderStorageBufferOrphan(GLenum buffer_flags, uint32_t buffer_size);
		ShaderStorageBufferOrphan() = default;

		void data(void* data, uint32_t s = 0);
		void bind();
		void bindBase(uint32_t index);
		void subdata(size_t offset, size_t size, void* data);
	private:
		void StartModify();
	};
}