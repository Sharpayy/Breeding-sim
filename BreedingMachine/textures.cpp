#include "textures.h"

std::unordered_map<const char*, TEXTURE_CONTAINER> texture_store;

void texture_store_init()
{
	texture_store = std::unordered_map<const char*, TEXTURE_CONTAINER>();
}

TEXTURE_CONTAINER* GetTextureFullInfo(const char* name)
{
	auto f = texture_store.find(name);
	if (f != texture_store.end())
		return &f->second;
	return nullptr;
}

uint64_t LoadTextureFromFile(const char* file, const char* tag)
{
	if (tag != 0)
	{
		auto f = texture_store.find(tag);
		if (f != texture_store.end())
			return f->second.txb.handle;
	}

	auto f = texture_store.find(file);
	if (f != texture_store.end())
		return f->second.txb.handle;

	rasticore::Image img = rasticore::Image(file, 4);

	if (img.data == NULL)
		return -1;

	TEXTURE_CONTAINER txc;

	txc.tex = rasticore::Texture2D( img.data, (int)img.x_, (int)img.y_, GL_RGBA, GL_RGBA8 );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	txc.tex.genMipmap();
	txc.txb = rasticore::Texture2DBindless(txc.tex);
	txc.txb.MakeResident();

	if (tag == 0)
		texture_store.insert({ file, txc });
	else
		texture_store.insert({ tag, txc });

	return txc.txb.handle;
}
