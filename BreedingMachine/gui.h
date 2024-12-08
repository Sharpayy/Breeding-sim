#pragma once
#include <vector>
#include <string>
#include <cstdlib>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

#include "Rasticore/rasti_main.h"
#include "Rasticore/rasti_utils.h"

#define GLT_IMPLEMENTATION
#include "gltext.h"

#define RESOURCE_STORE_BIND_LOCATION 11
#define GUI_ELEMENT_BIND_LOCATION	 12

#define GUI_ELEMRNT_UNDEFINIED		0
#define GUI_ELEMENT_SLIDER			1

#define GUI_SHADER_DEBUG_INFO

extern rasticore::Program* gui_main_program;
extern rasticore::VertexBuffer* gui_square;
extern uint32_t gui_program_mat_loc;

extern rasticore::UniformBufferObject* gui_prog_ubo;

void gui_init();

typedef uint32_t GTEXTUREID;

typedef struct _GTEXTURECONTAINER
{
	rasticore::Texture2D tx;
	rasticore::Texture2DBindless txb;

} GTEXTURECONTAINER;

class GResourceStore
{
public:
	std::vector<GTEXTURECONTAINER> textures;
	rasticore::ShaderStorageBufferObject textures_ssbo;

	GResourceStore(uint32_t start_size);
	GTEXTUREID AddNewTexture(const char* filename);

};

typedef struct _GSHADERRENDERDATA
{
	glm::vec2 scale_;
	glm::vec2 pos_;
	int gui_element;

	float val;

	float Reserved0;
	float Reserved1;

} GSHADERRENDERDATA;

typedef struct _GSHADERRENDERDATA_SLIDER
{
	GSHADERRENDERDATA d;
	uint32_t textures[2];

} GSHADERRENDERDATA_SLIDER;

extern GResourceStore* gui_resources;

class GComponentSlider
{
public:

	GLTtext* text;
	float scale_x, scale_y;
	float pos_x, pos_y;

	GTEXTUREID base_tex_id;
	GTEXTUREID fill_tex_id;

	float value;

	GComponentSlider(glm::vec2 scale, glm::vec2 pos, const char* text_, GTEXTUREID base, GTEXTUREID fill);
	void SetText(const char* text_);
	const char* GetText();

	void SetPosition(glm::vec2 new_pos);

	void Render(glm::mat4 pm);

};

class GWindow
{
public:
	
};

