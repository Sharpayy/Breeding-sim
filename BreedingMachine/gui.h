#pragma once
#include <vector>
#include <list>
#include <string>
#include <cstdlib>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

#include "Rasticore/rasti_main.h"
#include "Rasticore/rasti_utils.h"
#define ENABLE_GROOMING

#define GLT_IMPLEMENTATION
#include "gltext.h"
#include "Astar.h"

#include <functional>

#define RESOURCE_STORE_BIND_LOCATION 11
#define GUI_ELEMENT_BIND_LOCATION	 12

#define GUI_ELEMRNT_UNDEFINIED		0
#define GUI_ELEMENT_SLIDER			1
#define GUI_ELEMENT_BUTTON			2
#define GUI_ELEMENT_WINDOW			3
#define GUI_ELEMENT_IMAGE			4
#define GUI_ELEMENT_LABEL			5

#define GUI_SHADER_DEBUG_INFO

extern rasticore::Program* gui_main_program;
extern rasticore::VertexBuffer* gui_square;
extern uint32_t gui_program_mat_loc;
extern rasticore::UniformBufferObject* gui_prog_ubo;

#ifdef ENABLE_GROOMING
extern uint32_t gui_tex_0_loc;
extern uint32_t gui_tex_1_loc;
#endif


void gui_init();

typedef struct _GSHADERRENDERDATA
{
	glm::vec2 scale_;
	glm::vec2 pos_;
	int gui_element;

	float val;

	float z;
	float Reserved1;

} GSHADERRENDERDATA;

typedef struct _GSHADERRENDERDATA_SLIDER
{
	GSHADERRENDERDATA d;
	uint64_t textures[4];

} GSHADERRENDERDATA_SLIDER;

typedef struct _GSHADERRENDERDATA_BUTTON
{
	GSHADERRENDERDATA d;
	uint64_t textures[2];

} GSHADERRENDERDATA_BUTTON;

class GComponent
{
public:
	glm::vec3 pos;
	float depth;
	GComponent() {}

	virtual void Render(glm::mat4 pm) = 0;
	virtual int ClickCheck(float x, float y, void* window) = 0;
	virtual int GetType() = 0;

	virtual void SetOffset(glm::vec3 of) = 0;
	virtual void RenderText(glm::mat4 pm) = 0;
};

extern std::unordered_map<const char*, GComponent*> named_comps;

void AddNamedComponent(GComponent* comp, const char* name);
void DelNamedComponent(const char* name);
GComponent* GetNamedComponent(const char* name);

class GComponentSlider : public GComponent
{
public:

	GLTtext* text;
	float scale_x, scale_y;

	uint64_t base_tex_id;
	uint64_t fill_tex_id;

	float value;

	GComponentSlider(glm::vec2 scale, glm::vec3 pos, const char* text_, uint64_t base, uint64_t fill);
	void SetText(const char* text_);
	const char* GetText();

	void SetPosition(glm::vec3 new_pos);

	virtual void RenderText(glm::mat4 pm);
	virtual void Render(glm::mat4 pm);
	virtual int ClickCheck(float x, float y, void* window = NULL);
	virtual int GetType();
	virtual void SetOffset(glm::vec3 of);

};

class GComponentButton : public GComponent
{
public:
	GLTtext* text;

	float scale_x, scale_y;

	float val;

	uint64_t texture;
	std::function<void(void*, void*)> callback;

	GComponentButton(glm::vec2 scale, glm::vec3 pos, const char* text_, uint64_t tex);
	void SetCallback(std::function<void(void*, void*)> func);

	virtual void RenderText(glm::mat4 pm);
	virtual void Render(glm::mat4 pm);
	virtual int ClickCheck(float x, float y, void* window = NULL);
	virtual int GetType();
	virtual void SetOffset(glm::vec3 of);


};

class GComponentImage : public GComponent
{
public:

	float scale_x, scale_y;

	uint64_t texture;

	GComponentImage(glm::vec2 scale, glm::vec3 pos, uint64_t tex);

	virtual void RenderText(glm::mat4 pm);
	virtual void Render(glm::mat4 pm);
	virtual int ClickCheck(float x, float y, void* window = NULL);
	virtual int GetType();
	virtual void SetOffset(glm::vec3 of);
};

class GComponentLabel : public GComponent
{
public:

	float scale_x, scale_y;
	GLTtext* text;

	GComponentLabel(glm::vec2 scale, glm::vec3 pos, const char* text);

	virtual void RenderText(glm::mat4 pm);
	virtual void Render(glm::mat4 pm);
	virtual int ClickCheck(float x, float y, void* window = NULL);
	virtual int GetType();
	virtual void SetOffset(glm::vec3 of);
};

#define GUI_WINDOW_ACTIVE		1
#define GUI_WINDOW_TOP			2
#define GUI_WINDOW_NO_CLICK		4

class GWindow
{
private:
	std::list<GComponent*> component_list;

	uint32_t window_flags;

public:
	glm::vec2 position;
	glm::vec2 scale;
	float z;

	uint64_t background;

	GWindow(glm::vec2 pos, glm::vec2 scale, uint64_t tex);

	void AddComponent(GComponent* comp);
	void Render(glm::mat4 pm);
	void RenderText(glm::mat4 pm);

	int CollisionCheck(float x, float y);
	void UpdateZComp();
	void UpdateDepth(float z);
	void ChangeComponentPosition(int offsetX, int offsetY);

};

class GWindowBuilder
{
private:
	GWindow* win;

public:
	GWindowBuilder(glm::vec2 pos, glm::vec2 scale, uint64_t background);

	void AddSliderComponent(glm::vec2 scale, glm::vec2 pos, const char* text_, uint64_t image0, uint64_t image1);
	void AddButtonComponent(glm::vec2 scale, glm::vec2 pos, const char* text_, uint64_t image);
	void AddImageComponent(glm::vec2 scale, glm::vec2 pos, uint64_t image);
	void AddLabelComponent(glm::vec2 scale, glm::vec2 pos, const char* text_);

	GWindow* BuildWindow();
};

void pfnBasicButtonCallback(void* button, void* window);