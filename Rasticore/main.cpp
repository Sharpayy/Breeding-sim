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

#include "rasti_window.h"
#include "rasti_utils.h"
#include "rasti_alloc.h"
#include "rasti_main.h"
#include "rasti_models.h"
#include "objload.h"

#include "MapRenderer.cpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#undef main

// Test models ids
#define RASTICORE_MODEL_CUBE			1
#define RASTICORE_MODEL_TRIANGLE		2

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
	MAPPEDFILE a = rasticore::MapFile("rasticore_shdrVertex.glsl");

	rasticore::RsMatrixAllocator MatrixGen = rasticore::RsMatrixAllocator(4);
	
	glm::mat4* mat0 = (glm::mat4*)MatrixGen.AllocMatrix();
	glm::mat4* mat1 = (glm::mat4*)MatrixGen.AllocMatrix();

	MatrixGen.FreeMatrix(mat1);

	rasticore::RastiCoreWindow _win = rasticore::RastiCoreWindow("Rasticore test!", 800, 800, SDL_WINDOW_OPENGL);
	_win.glCreateContext();

	glewInit();




	rasticore::RastiCoreRender _r = rasticore::RastiCoreRender(100);

	objl::Loader ldr = objl::Loader();
	ldr.LoadFile("girl OBJ.obj");

	MAPPEDFILE _t_shader_file = rasticore::MapFile("rasticore_vertex_shader.glsl");
	rasticore::Shader<GL_VERTEX_SHADER> _shdr_vt_n = rasticore::Shader<GL_VERTEX_SHADER>((char*)_t_shader_file.data);
	rasticore::UnmapFile(_t_shader_file);

	_t_shader_file = rasticore::MapFile("rasticore_fragment_shader.glsl");
	rasticore::Shader<GL_FRAGMENT_SHADER> _shdr_fg_n = rasticore::Shader<GL_FRAGMENT_SHADER>((char*)_t_shader_file.data);
	rasticore::UnmapFile(_t_shader_file);

	rasticore::Program _program_n = rasticore::Program();
	_program_n.programAddShader(_shdr_vt_n.id);
	_program_n.programAddShader(_shdr_fg_n.id);
	_program_n.programCompile();

	//_program_n.use();
	//glUniform1i(glGetUniformLocation(_program_n.id, "image0"), 0); // BindSampler

	CreateMeshes(&ldr, &_r, 5, _program_n, 15, 0);

	// TODO: do this better and put this inside one class 
	rasticore::Buffer<GL_ARRAY_BUFFER> _cube_model_vbo = rasticore::Buffer<GL_ARRAY_BUFFER>(sizeof(rasticore::cube::vertex), rasticore::cube::vertex, GL_STATIC_DRAW);
	rasticore::Buffer<GL_ELEMENT_ARRAY_BUFFER> _cube_model_ebo = rasticore::Buffer<GL_ELEMENT_ARRAY_BUFFER>(sizeof(rasticore::cube::indice), rasticore::cube::indice, GL_STATIC_DRAW);

	rasticore::VertexBuffer _cube_vb = rasticore::VertexBuffer();
	_cube_model_ebo.bind();
	_cube_model_vbo.bind();
	
	_cube_vb.addAttrib(GL_FLOAT, 0, 3, 20, 0);	// vertex x y z
	_cube_vb.addAttrib(GL_FLOAT, 1, 2, 20, 12);	// uv x and y

	_cube_vb.enableAttrib(0);
	_cube_vb.enableAttrib(1);
	// TODO: do this better and put this inside one class 

	//Camera _g_camera = Camera(vec3(5.0f, 0.0f, 5.0f));
	mat4 _g_camera = lookAt(vec3(0.5f, 3.0f, 15.0f), (vec3(0.0f, 3.0f, 0.0f)), vec3(0.0f, 1.0f, 0.0f));

	_r.setCameraMatrix(_g_camera);
	_r.setProjectionMatrix(_win.GetProjectionMatrix(radians(90.0f)));
	_r.UpdateShaderData();
	

	_r.newModel(RASTICORE_MODEL_CUBE, _cube_vb, _program_n, sizeof(rasticore::cube::indice) / 4, GL_TRIANGLES, rasticore::Texture2DBindless(), 10);

	for (int i = 0; i < 10; i++)
	{
		for (int ii = 0; ii < 7; ii++)
			_r.newObject(5+ii, scale(translate(mat4(1.0f), vec3(3.0f * i - 15.0f, 0.0f, 0.0f)), vec3(3.0f)));
	}

	rasticore::Image mapImg = rasticore::Image("mm.png", 4);
	GameMap gm = GameMap(mapImg, 4);
	

	//_r.SetObjectMatrix(0, translate(mat4(1.0f), vec3(15.0f, 0.0f, 15.0f)));

	float t = 0.0f;
	int tf = 0;
	bool s = false;

	int cntr = 0;



	RS_ENABLE_FRATURE(GL_DEPTH_TEST);
	RS_BACKGROUND_CLEAR_COLOR(1.0f, 0.0f, 0.0f, 1.0f);
	while (1)
	{
		RS_CLEAR_FRAMEBUFFER(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		tf++;
		if (tf == 60)
		{
			tf = 0;

			if (s == 0)
			{
				for (int i = 0; i < 7; i++)
				{
					_r.BindActiveModel(5 + i);
					_r.DisableObjectNoSync(cntr);
				}
			}
			else
			{
				for (int i = 0; i < 7; i++)
				{
					_r.BindActiveModel(5 + i);
					_r.EnableObjectNoSync(cntr);
				}
			}

			cntr++;

			if (cntr == 9)
			{
				cntr = 0;
				s = !s;
			}

		}
		for (int i = 0; i < 7; i++)
		{
			_r.UpdateShaderIdSpace(5+i);
		}
		_r.SetObjectMatrix(5, translate(mat4(1.0f), vec3(0.0f, 0.0f, (sin(t) + 1.0f) * 10.0f - 5.0f)));

		//_r.RenderSelectedModel(5);

		_r.SetObjectMatrix(1, translate(mat4(1.0f), vec3(0.0f, 5.0f, 0.0f)));

		for (int i = 0; i < 7; i++)
		{
			_r.RenderSelectedModel(5 + i);
		}

			

		_win.swap();
		_win.handleEvents();
		t += 0.003f;
		SDL_Delay(16);
	}


}
