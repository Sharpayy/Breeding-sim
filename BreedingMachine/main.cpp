#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <GL/glew.h>
#include <cstring>
#include <cstdlib>
#include <SDL.h>
#include <Windows.h>
#include <glm/gtc/matrix_transform.hpp>
#include <ctime>
#include <cmath>

#include "Rasticore/rasti_window.h"
#include "Rasticore/rasti_utils.h"
#include "Rasticore/rasti_alloc.h"
#include "Rasticore/rasti_main.h"
#include "Rasticore/rasti_models.h"
#include "Rasticore/objload.h"

#include "gui.h"
#include "inventory.h"
#include "textures.h"

#include "Rasticore/MapRenderer.cpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define GLT_MANUAL_VIEWPORT

#undef main

// Test models ids
#define MODEL_ORKS 0
#define MODEL_HUMANS 1
#define MODEL_NOMADS 2
#define MODEL_EVIL_HUMANS 3
#define MODEL_GOBLINS 4
#define MODEL_PLAYER 5
#define MODEL_BANDITS 6
#define MODEL_ANIMALS 7

//Tests includes
#include "MovementManager.h"
#include "inputHandler.h"
#include "gameManager.h"

using namespace glm;

class Camera
{
public:
	Camera(vec3 pos)
	{
		yaw = 90.0f; pitch = 0.0f; roll = 0.0f;
		this->pos = pos;

		f = normalize(vec3(
			cos(yaw) * cos(pitch),
			sin(pitch),
			cos(pitch) * sin(yaw)
		));
		r = normalize(cross(f, vec3(0.0f, 1.0f, 0.0f)));
		u = normalize(cross(f, r));
	}
	mat4 getMatrix()
	{
		f = normalize(vec3(
			cos(yaw) * cos(pitch),
			sin(pitch),
			cos(pitch) * sin(yaw)
		));

		mat4 roll_mat = glm::rotate(mat4(1.0f), roll, f);

		r = normalize(cross(f, vec3(0.0f, 1.0f, 0.0f)));
		u = mat3(roll_mat) * normalize(cross(f, r));
		mat4 M = lookAt(pos, f + pos, u);
		return M;
	}
	vec3 getPos()
	{
		return pos;
	}
	vec3* getPPos()
	{
		return &pos;
	}
	void addYaw(float x)
	{
		yaw += x;
	}
	void addPitch(float x)
	{
		pitch += x;
	}
	void addRoll(float x)
	{
		roll += x;
	}
	void setMatrix(mat4* matrix)
	{
		f = normalize(vec3(
			cos(yaw) * cos(pitch),
			sin(pitch),
			cos(pitch) * sin(yaw)
		));

		mat4 roll_mat = glm::rotate(mat4(1.0f), roll, f);

		r = normalize(cross(f, vec3(0.0f, 1.0f, 0.0f)));
		u = mat3(roll_mat) * normalize(cross(f, r));

		*matrix = lookAt(pos, f + pos, u);
	}
	void setYaw(float yaw)
	{
		this->yaw = yaw;
	}
	void setPitch(float pitch)
	{
		this->pitch = pitch;
	}
	void setRoll(float roll)
	{
		this->roll = roll;
	}
	vec3 getRight()
	{
		return r;
	}
	void setPos(vec3 pos)
	{
		this->pos = pos;
	}
	void addPos(vec3 pos)
	{
		this->pos += pos;
	}
	void posAddFront(float s)
	{
		pos += f * s;
	}
	void posAddRight(float s)
	{
		pos += r * s;
	}

	float yaw, pitch, roll;
	vec3 pos;
	vec3 r, u, f;
};

void* LoadImageData(const char* filename, bool flip, int* channels, int* x, int* y)
{
	return stbi_load(filename, x, y, channels, STBI_rgb_alpha);
}

void CreateMeshes(objl::Loader* ldr, rasticore::RastiCoreRender* r, int of, rasticore::Program pr, int am, rasticore::CompositeModel* cModel)
{
	for (int i = 0; i < ldr->LoadedMeshes.size(); i++)
	{
		objl::Mesh cm = ldr->LoadedMeshes[i];

		rasticore::VertexBuffer myModelVao = rasticore::VertexBuffer();
		myModelVao.bind();

		rasticore::Buffer<GL_ARRAY_BUFFER> myModelTex = rasticore::Buffer<GL_ARRAY_BUFFER>(cm.Vertices.size() * sizeof(objl::Vertex), cm.Vertices.data(), GL_STATIC_DRAW);
		myModelVao.addAttrib(GL_FLOAT, 0, 3, sizeof(objl::Vertex), 0);
		myModelVao.addAttrib(GL_FLOAT, 1, 3, sizeof(objl::Vertex), 12);
		myModelVao.addAttrib(GL_FLOAT, 2, 2, sizeof(objl::Vertex), 24);

		rasticore::Buffer<GL_ELEMENT_ARRAY_BUFFER> myModelEbo = rasticore::Buffer<GL_ELEMENT_ARRAY_BUFFER>(cm.Indices.size() * 4, cm.Indices.data(), GL_STATIC_DRAW);

		myModelVao.enableAttrib(0);
		myModelVao.enableAttrib(1);
		myModelVao.enableAttrib(2);

		int szr_x, szr_y, szr_chn;
		void* szr_data = LoadImageData(cm.MeshMaterial.map_Kd.c_str(), false, &szr_chn, &szr_x, &szr_y);

		rasticore::Texture2D green = rasticore::Texture2D(szr_data, szr_x, szr_y, GL_RGBA, GL_RGBA);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		green.genMipmap();
		
		rasticore::Texture2DBindless bndl = rasticore::Texture2DBindless(green);
		bndl.MakeResident();
		
		r->newModel(of + i, myModelVao, pr, cm.Indices.size(), GL_TRIANGLES, bndl, am);
		r->BindActiveModel(of + i);
		//std_bmp_empty_texture = Texture2D(0, 1, 1, GL_RGBA, GL_RGBA, GL_TEXTURE1);
		//r->md->std_normalMap = std_bmp_empty_texture;

		if (cModel != NULL)
		{
			cModel->model_base_id = of;
			cModel->model_count = ldr->LoadedMeshes.size();
		}

		/*
		if (cm.MeshMaterial.map_bump != "")
		{
			
			int bmp_x, bmp_y, bmp_chn;
			void* bmp_data = LoadImageData(cm.MeshMaterial.map_bump.c_str(), false, &bmp_chn, &bmp_x, &bmp_y);

			rasticore::Texture2D bmp = rasticore::Texture2D(bmp_data, bmp_x, bmp_y, GL_RGBA, GL_RGBA, GL_TEXTURE1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			bmp.genMipmap();
			//r->md->std_normalMap = bmp;
		}
		*/

	}
}

int main(int argc, char* argv[])
{
	rasticore::RastiCoreWindow _win = rasticore::RastiCoreWindow("Rasticore test!", 800, 800, SDL_WINDOW_OPENGL);
	_win.glCreateContext();

	glewInit();
	gui_init();

	rasticore::RastiCoreRender _r = rasticore::RastiCoreRender(100);

	MAPPEDFILE _t_shader_file = rasticore::MapFile("Rasticore\\rasticore_vertex_shader.glsl");
	rasticore::Shader<GL_VERTEX_SHADER> _shdr_vt_n = rasticore::Shader<GL_VERTEX_SHADER>((char*)_t_shader_file.data);
	rasticore::UnmapFile(_t_shader_file);

	_t_shader_file = rasticore::MapFile("Rasticore\\rasticore_fragment_shader.glsl");
	rasticore::Shader<GL_FRAGMENT_SHADER> _shdr_fg_n = rasticore::Shader<GL_FRAGMENT_SHADER>((char*)_t_shader_file.data);
	rasticore::UnmapFile(_t_shader_file);

	MAPPEDFILE menda = rasticore::MapFile("Rasticore\\menda.glsl");
	rasticore::Shader<GL_VERTEX_SHADER> mendashader = rasticore::Shader<GL_VERTEX_SHADER>((char*)menda.data);
	rasticore::UnmapFile(menda);

	_t_shader_file = rasticore::MapFile("Rasticore\\fight_map_fragment.glsl");
	rasticore::Shader<GL_FRAGMENT_SHADER> fightMapFrag = rasticore::Shader<GL_FRAGMENT_SHADER>((char*)_t_shader_file.data);
	rasticore::UnmapFile(_t_shader_file);

	rasticore::Program fightMapProgram = rasticore::Program();
	fightMapProgram.programAddShader(_shdr_vt_n.id);
	fightMapProgram.programAddShader(fightMapFrag.id);
	fightMapProgram.programCompile();

	fightMapProgram.use();

	rasticore::Program _program_n = rasticore::Program();
	_program_n.programAddShader(_shdr_vt_n.id);
	_program_n.programAddShader(_shdr_fg_n.id);
	_program_n.programCompile();

	_program_n.use();

	rasticore::Program mendaprogram = rasticore::Program();
	mendaprogram.programAddShader(mendashader.id);
	mendaprogram.programAddShader(_shdr_fg_n.id);
	mendaprogram.programCompile();

	mendaprogram.use();

	stbi_set_flip_vertically_on_load(true);

	float plane_vtx[] = {
		-0.5f * 100.0f, -0.5f * 100.0f, 1.0f, 0.0f, 0.0f,
		-0.5f * 100.0f, 0.5f * 100.0f, 1.0f, 0.0f, 1.0f,
		0.5 * 100.0f, -0.5f * 100.0f, 1.0f, 1.0f, 0.0f,

		0.5 * 100.0f, 0.5f * 100.0f, 1.0f, 1.0f, 1.0f
	};

	uint32_t chuj[] = {
		0,1,2,1,2,3
	};

	rasticore::Buffer<GL_ARRAY_BUFFER> SquareVBO = rasticore::Buffer<GL_ARRAY_BUFFER>(sizeof(plane_vtx), plane_vtx, GL_STATIC_DRAW);
	rasticore::Buffer<GL_ELEMENT_ARRAY_BUFFER> SquareEBO = rasticore::Buffer<GL_ELEMENT_ARRAY_BUFFER>(sizeof(chuj), chuj, GL_STATIC_DRAW);

	rasticore::VertexBuffer Square = rasticore::VertexBuffer();
	Square.bind();
	SquareVBO.bind();
	SquareEBO.bind();

	Square.addAttrib(GL_FLOAT, 0, 3, sizeof(float) * 5, 0);
	Square.addAttrib(GL_FLOAT, 1, 2, sizeof(float) * 5, 12);
	Square.enableAttrib(0);
	Square.enableAttrib(1);
	
	rasticore::ModelCreationDetails rect_mcd;
	rect_mcd.p = mendaprogram;
	rect_mcd.rm = GL_TRIANGLES;
	rect_mcd.v_cnt = 6;
	rect_mcd.vb = Square;

	_r.setCameraMatrix(lookAt(vec3(0.0f, 0.0f, 1000.0f), (vec3(0.0f, 0.0f, 1.0f)), vec3(0.0f, 1.0f, 0.0f)));
	_r.setProjectionMatrix(ortho(-500.0f, 500.0f, -500.0f, 500.0f, -1000.0f, 1000.0f));
	_r.UpdateShaderData();
	
	rasticore::Image mapImg = rasticore::Image("Rasticore\\mm.png", 4);
	GameMap gm = GameMap(&mapImg, 4);

	_program_n.use();
	uint32_t lShdrScaleX = glGetUniformLocation(_program_n.id, "gScalex");
	uint32_t lShdrScaleY = glGetUniformLocation(_program_n.id, "gScaley");
	uint32_t lShdrMoveX = glGetUniformLocation(_program_n.id, "gMovex");
	uint32_t lShdrMoveY = glGetUniformLocation(_program_n.id, "gMovey");

	fightMapProgram.use();
	uint32_t fightScaleX = glGetUniformLocation(fightMapProgram.id, "gScalex");
	uint32_t fightScaleY = glGetUniformLocation(fightMapProgram.id, "gScaley");
	uint32_t fightMoveX = glGetUniformLocation(fightMapProgram.id, "gMovex");
	uint32_t fightMoveY = glGetUniformLocation(fightMapProgram.id, "gMovey");
	uint32_t fightMouse = glGetUniformLocation(fightMapProgram.id, "MouseCoord");
	uint32_t fightMapDim = glGetUniformLocation(fightMapProgram.id, "MapDimensions");
	uint32_t fightMapTil = glGetUniformLocation(fightMapProgram.id, "MapTiles");

	RS_ENABLE_FRATURE(GL_DEPTH_TEST);
	RS_BACKGROUND_CLEAR_COLOR(1.0f, 0.0f, 0.0f, 1.0f);

	mat4 gui_projection_matrix = ortho(0.0f, 800.0f, 800.0f, 0.0f, -1000.0f, 1000.0f);

	//TESTS

	RS_ENABLE_FRATURE(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	gameManager gmanager(&_r, rect_mcd, gm.rChunkVao, fightMapProgram);

	gameManager::CameraOffset cameraOffset;
	float tick = 0;
	while (1)
	{
		tick += 1.0f;
		RS_CLEAR_FRAMEBUFFER(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cameraOffset = gmanager.getCameraOffset();
		_r.setCameraMatrix(lookAt(vec3(cameraOffset.x, cameraOffset.y, 100.0f), (vec3(cameraOffset.x, cameraOffset.y, 1.0f)), vec3(0.0f, 1.0f, 0.0f)));
		_r.setProjectionMatrix(ortho(-cameraOffset.z, cameraOffset.z, -cameraOffset.z, cameraOffset.z, -1000.0f, 1000.0f));
		_r.UpdateShaderData();

		
		if (gmanager.getGameType() == GAMETYPE_BIGMAP)
		{
			_program_n.use();
			gm.rChunkVao.bind();
			glUniform1f(lShdrScaleX, gm.pChunkSizeX);
			glUniform1f(lShdrScaleY, gm.pChunkSizeY);
			for (int y = 0; y < gm.blk; y++)
			{
				for (int x = 0; x < gm.blk; x++)
				{
					glUniform1f(lShdrMoveX, (int)gm.pChunkSizeX * x - ((int)gm.pMapSizeX / 2));
					glUniform1f(lShdrMoveY, (int)gm.pChunkSizeY * y - ((int)gm.pMapSizeY / 2));

					glUniformHandleui64ARB(1, gm.aChunkArray[gm.blk * y + x].txb.handle);

					glDrawArrays(GL_TRIANGLES, 0, 6);
				}
			}
		}
		gmanager.update();
		gmanager.inv.Render(gui_projection_matrix);
		gmanager.inv.RenderText(gui_projection_matrix);
		gmanager.inv.RenderCursor(gui_projection_matrix);





		tick += 0.001f;
		_win.swap();
		//_win.handleEvents();
		SDL_Delay(10);
	}


}
