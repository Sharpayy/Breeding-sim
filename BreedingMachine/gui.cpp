#include "gui.h"
#include "gltext.h"

rasticore::Program* gui_main_program;
rasticore::VertexBuffer* gui_square;
uint32_t gui_program_mat_loc;
rasticore::UniformBufferObject* gui_prog_ubo;
std::unordered_map<std::string, GComponent*> named_comps;

#ifdef ENABLE_GROOMING
uint32_t gui_tex_0_loc;
uint32_t gui_tex_1_loc;
#endif


GComponentSlider::GComponentSlider(glm::vec2 scale, glm::vec3 pos, const char* text_, uint64_t base, uint64_t fill, bool textCenter)
{
	scale_x = scale.x;
	scale_y = scale.y;

	this->textCenter = textCenter;

	text = gltCreateText();
	gltSetText(text, text_);
	this->pos = pos;
	depth = pos.z;

	base_tex_id = base;
	fill_tex_id = fill;

	value = 0.0f;
}

void GComponentSlider::SetText(const char* text_)
{
	gltSetText(text, text_);
}

void GComponentSlider::RenderText(glm::mat4 pm)
{
	gltBeginDraw();

	glm::mat4 m = glm::mat4(1.0f);

	int center = 0;
	if (textCenter) center = scale_x / 2.0f - gltGetTextWidth(text, 1.0f) / 2.0f;

	m = glm::translate(m, glm::vec3(pos.x + center, pos.y + scale_y / 2.0f - gltGetTextHeight(text, 1.0f) / 2.0f, pos.z + 1.01f));
	//m = glm::scale(m, glm::vec3(1.0f, scale_y, 1.0f));

	m = pm * m;

	gltDrawText(text, (GLfloat*)&m);
	gltEndDraw();
}

void GComponentSlider::Render(glm::mat4 pm)
{
	gui_main_program->use();

	GSHADERRENDERDATA_SLIDER data{};
	data.d.gui_element = GUI_ELEMENT_SLIDER;
	data.d.pos_ = glm::vec2(pos.x, pos.y);
	data.d.scale_ = glm::vec2(scale_x, scale_y);
	data.d.val = value;
	data.d.z = pos.z;

#ifndef ENABLE_GROOMING
	data.textures[0] = base_tex_id;
	data.textures[2] = fill_tex_id;
#else
	glUniformHandleui64ARB(gui_tex_0_loc, base_tex_id);
	glUniformHandleui64ARB(gui_tex_1_loc, fill_tex_id);
#endif

	gui_prog_ubo->bind();
	gui_prog_ubo->subdata(0, sizeof(GSHADERRENDERDATA_SLIDER), &data);
	gui_prog_ubo->bindBase(GUI_ELEMENT_BIND_LOCATION);

	gui_square->bind();

	glUniformMatrix4fv(gui_program_mat_loc, 1, GL_FALSE, (float*)&pm);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

int GComponentSlider::ClickCheck(float x, float y, void* window)
{
	return 0;
}

int GComponentSlider::GetType()
{
	return GUI_ELEMENT_SLIDER;
}

void GComponentSlider::SetOffset(glm::vec3 of)
{
	pos += of;
}

void gui_init()
{
	gltInit();
	MAPPEDFILE shdr_src = rasticore::MapFile("Rasticore\\gui_shader_vertex.glsl");
	rasticore::Shader<GL_VERTEX_SHADER> _shdr_vt_n = rasticore::Shader<GL_VERTEX_SHADER>((char*)shdr_src.data);
	rasticore::UnmapFile(shdr_src);

	shdr_src = rasticore::MapFile("Rasticore\\gui_shader_fragment.glsl");
	rasticore::Shader<GL_FRAGMENT_SHADER> _shdr_fg_n = rasticore::Shader<GL_FRAGMENT_SHADER>((char*)shdr_src.data);
	rasticore::UnmapFile(shdr_src);


#ifdef GUI_SHADER_DEBUG_INFO
	char debug_log[2048];
	_shdr_fg_n.shaderGetDebugInfo(debug_log, 2048);
	puts(debug_log);

	_shdr_vt_n.shaderGetDebugInfo(debug_log, 2048);
	puts(debug_log);


#endif

	gui_main_program = new rasticore::Program();
	gui_main_program->programAddShader(_shdr_vt_n.id);
	gui_main_program->programAddShader(_shdr_fg_n.id);
	gui_main_program->programCompile();

	//glGetProgramInfoLog(gui_main_program, )

	gui_main_program->use();

	gui_program_mat_loc = glGetUniformLocation(gui_main_program->id, "projection_matrix");

#ifdef ENABLE_GROOMING
	gui_tex_0_loc = glGetUniformLocation(gui_main_program->id, "tex_0");
	gui_tex_1_loc = glGetUniformLocation(gui_main_program->id, "tex_1");
#endif

	//float plane_vtx[] = {
	//	-0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
	//	-0.5f, 0.5f, 1.0f, 0.0f, 1.0f,  0.0f,
	//	0.5, -0.5f, 1.0f, 1.0f, 0.0f,   1.0f,

	//	0.5, 0.5f, 1.0f, 1.0f, 1.0f,	1.0f
	//};

		float plane_vtx[] = {
		0.0f,  0.0f, 1.0f, 0.0f, 1.0f,		0.0f,
		0.0f,  1.0f, 1.0f, 0.0f, 0.0f,		0.0f,
		1.0f,  0.0f, 1.0f, 1.0f, 1.0f,		1.0f,

		1.0f,  1.0f, 1.0f, 1.0f, 0.0f,		1.0f
	};


	uint32_t chuj[] = {
		2,1,0,3,2,1
	};

	rasticore::Buffer<GL_ARRAY_BUFFER> SquareVBO = rasticore::Buffer<GL_ARRAY_BUFFER>(sizeof(plane_vtx), plane_vtx, GL_STATIC_DRAW);
	rasticore::Buffer<GL_ELEMENT_ARRAY_BUFFER> SquareEBO = rasticore::Buffer<GL_ELEMENT_ARRAY_BUFFER>(sizeof(chuj), chuj, GL_STATIC_DRAW);

	gui_square = new rasticore::VertexBuffer();
	gui_square->bind();
	SquareVBO.bind();
	SquareEBO.bind();

	gui_square->addAttrib(GL_FLOAT, 0, 3, sizeof(float) * 6, 0);
	gui_square->addAttrib(GL_FLOAT, 1, 2, sizeof(float) * 6, 12);
	gui_square->addAttrib(GL_FLOAT, 2, 1, sizeof(float) * 6, 20);
	gui_square->enableAttrib(0);
	gui_square->enableAttrib(1);
	gui_square->enableAttrib(2);

	gui_prog_ubo = new rasticore::UniformBufferObject();
	gui_prog_ubo->data(sizeof(GSHADERRENDERDATA) + 64, NULL, GL_DYNAMIC_DRAW);

}

void AddNamedComponent(GComponent* comp, const char* name)
{
	named_comps[name] = comp;//.insert({ name, comp });
}

void DelNamedComponent(const char* name)
{
	named_comps.erase(name);
}

GComponent* GetNamedComponent(const char* name)
{
	auto f = named_comps.find(name);
	if (f == named_comps.end())
		return 0;
	return f->second;
}

void pfnBasicButtonCallback(void* button, void* window)
{
	printf("chij");
}

GComponentButton::GComponentButton(glm::vec2 scale, glm::vec3 pos, const char* text_, uint64_t tex, bool textCenter)
{
	scale_x = scale.x;
	scale_y = scale.y;

	this->textCenter = textCenter;

	text = gltCreateText();
	gltSetText(text, text_);

	this->pos = pos;
	depth = pos.z;

	texture = tex;
	callback = pfnBasicButtonCallback;

	val = 0.0f;
}

void GComponentButton::SetCallback(std::function<void(void*, void*)> func)
{
	callback = func;
}

void GComponentButton::RenderText(glm::mat4 pm)
{
	if (text->_text == 0)
		return;

	gltBeginDraw();

	int center = 0;
	if (textCenter) center = scale_x / 2.0f - gltGetTextWidth(text, 1.0f) / 2.0f;
	
	glm::mat4 m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(pos.x + center, pos.y + scale_y / 2.0f - gltGetTextHeight(text, 1.0f) / 2.0f, pos.z + 1.01f));
	//m = glm::scale(m, glm::vec3(1.0f, scale_y, 1.0f));

	m = pm * m;

	gltDrawText(text, (GLfloat*)&m);
	gltEndDraw();
}

void GComponentButton::Render(glm::mat4 pm)
{
	gui_main_program->use();

	GSHADERRENDERDATA_BUTTON data{};
	data.d.gui_element = GUI_ELEMENT_BUTTON;
	data.d.pos_ = glm::vec2(pos.x, pos.y);
	data.d.scale_ = glm::vec2(scale_x, scale_y);
	data.d.val = val;
	data.d.z = pos.z;

#ifndef ENABLE_GROOMING
	data.textures[0] = texture;
#else
	glUniformHandleui64ARB(gui_tex_0_loc, texture);
#endif


	gui_prog_ubo->bind();
	gui_prog_ubo->subdata(0, sizeof(GSHADERRENDERDATA_BUTTON), &data);
	gui_prog_ubo->bindBase(GUI_ELEMENT_BIND_LOCATION);

	gui_square->bind();

	glUniformMatrix4fv(gui_program_mat_loc, 1, GL_FALSE, (float*)&pm);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

int GComponentButton::ClickCheck(float x, float y, void* window)
{
	if (x >= pos.x && x <= pos.x + scale_x
		&& y >= pos.y && y <= pos.y + scale_y) {
		callback(this, window);
		return true;
	}
	return 0;
}

int GComponentButton::GetType()
{
	return GUI_ELEMENT_BUTTON;
}

void GComponentButton::SetOffset(glm::vec3 of)
{
	pos += of;
}

void GComponentButton::SetText(const char* t)
{
	gltSetText(text, t);
}

GWindow::GWindow(glm::vec2 pos, glm::vec2 scale, uint64_t tex)
{
	this->position = glm::vec2(pos.x, pos.y);
	position = pos;
	this->scale = scale;
	background = tex;
	z = 0.0f;

	component_list = std::list<GComponent*>();

	window_flags = 0;
}

void GWindow::AddComponent(GComponent* comp)
{
	//comp->SetOffset(glm::vec3(position.x, position.y, 0.0f));
	component_list.push_back(comp);
}

void GWindow::Render(glm::mat4 pm)
{
	if (window_flags && GUI_WINDOW_ACTIVE != GUI_WINDOW_ACTIVE)
		return;

	gui_main_program->use();

	GSHADERRENDERDATA_BUTTON data{};
	data.d.gui_element = GUI_ELEMENT_WINDOW;
	data.d.pos_ = position;
	data.d.scale_ = scale;
	data.d.z = z;

#ifndef ENABLE_GROOMING
	data.textures[0] = background;
#else
	glUniformHandleui64ARB(gui_tex_0_loc, background);
#endif

	gui_prog_ubo->bind();
	gui_prog_ubo->subdata(0, sizeof(GSHADERRENDERDATA_BUTTON), &data);
	gui_prog_ubo->bindBase(GUI_ELEMENT_BIND_LOCATION);

	gui_square->bind();

	glUniformMatrix4fv(gui_program_mat_loc, 1, GL_FALSE, (float*)&pm);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

	for (auto& i : component_list)
	{
		i->Render(pm);
	}
}

void GWindow::RenderText(glm::mat4 pm)
{
	if (window_flags && GUI_WINDOW_ACTIVE != GUI_WINDOW_ACTIVE)
		return;

	for (auto& i : component_list)
	{
		i->RenderText(pm);
	}
}

int GWindow::CollisionCheck(float x, float y)
{
	if (window_flags && GUI_WINDOW_NO_CLICK == GUI_WINDOW_NO_CLICK)
		return 0;

	int cl = 0;

	for (auto& i : component_list)
	{
		cl += i->ClickCheck(x, y, this);
	}

	return cl;
}

void GWindow::UpdateZComp()
{
	for (auto& i : component_list)
	{
		i->pos.z = z + i->depth;
	}
}

void GWindow::UpdateDepth(float z)
{
	this->z = z;
}

GComponentImage::GComponentImage(glm::vec2 scale, glm::vec3 pos, uint64_t tex)
{
	scale_x = scale.x;
	scale_y = scale.y;

	this->pos = pos;
	depth = pos.z;
	texture = tex;
}

void GComponentImage::RenderText(glm::mat4 pm)
{
	
}

void GComponentImage::Render(glm::mat4 pm)
{
	if (texture == 0)
		return;

	gui_main_program->use();

	GSHADERRENDERDATA_BUTTON data{};
	data.d.gui_element = GUI_ELEMENT_IMAGE;
	data.d.pos_ = glm::vec2(pos.x, pos.y);
	data.d.scale_ = glm::vec2(scale_x, scale_y);
	data.d.z = pos.z;

#ifndef ENABLE_GROOMING
	data.textures[0] = texture;
#else
	glUniformHandleui64ARB(gui_tex_0_loc, texture);
#endif

	gui_prog_ubo->bind();
	gui_prog_ubo->subdata(0, sizeof(GSHADERRENDERDATA_BUTTON), &data);
	gui_prog_ubo->bindBase(GUI_ELEMENT_BIND_LOCATION);

	gui_square->bind();

	glUniformMatrix4fv(gui_program_mat_loc, 1, GL_FALSE, (float*)&pm);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

int GComponentImage::ClickCheck(float x, float y, void* window)
{
	return 0;
}

int GComponentImage::GetType()
{
	return GUI_ELEMENT_IMAGE;
}

void GComponentImage::SetOffset(glm::vec3 of)
{
	pos += of;
}

void GComponentImage::SetText(const char* t)
{

}

GComponentLabel::GComponentLabel(glm::vec2 scale, glm::vec3 pos, const char* text, bool textCenter)
{
	scale_x = scale.x;
	scale_y = scale.y;

	this->textCenter = textCenter;

	this->pos = pos;
	depth = pos.z;
	this->text = gltCreateText();
	gltSetText(this->text, text);
}

void GComponentLabel::RenderText(glm::mat4 pm)
{
	gltBeginDraw();

	int center = 0;
	if (textCenter) center = scale_x / 2.0f - gltGetTextWidth(text, 1.0f) / 2.0f;

	glm::mat4 m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(pos.x + center, pos.y + scale_y / 2.0f - gltGetTextHeight(text, 1.0f) / 2.0f, pos.z + 1.01f));
	//m = glm::scale(m, glm::vec3(1.0f, scale_y, 1.0f));

	m = pm * m;

	gltDrawText(text, (GLfloat*)&m);
	gltEndDraw();
}

void GComponentLabel::Render(glm::mat4 pm)
{
}

int GComponentLabel::ClickCheck(float x, float y, void* window)
{
	return 0;
}

int GComponentLabel::GetType()
{
	return GUI_ELEMENT_LABEL;
}

void GComponentLabel::SetOffset(glm::vec3 of)
{
	pos += of;
}

void GComponentLabel::SetText(const char* t)
{
	gltSetText(text, t);
}

void GWindow::ChangeComponentPosition(int x, int y) {
	int offsetX, offsetY;
	offsetX = x - position.x;
	offsetY = y - position.y;
	position.x = x;
	position.y = y;
	for (auto& component : component_list) {
		component->pos.x += offsetX;
		component->pos.y += offsetY;
	}
}

GWindowBuilder::GWindowBuilder(glm::vec2 pos, glm::vec2 scale, uint64_t background)
{
	win = new GWindow(pos, scale, background);
}

void GWindowBuilder::AddSliderComponent(glm::vec2 scale, glm::vec3 pos, const char* text_, uint64_t image0, uint64_t image1)
{
	win->AddComponent(new GComponentSlider(scale, pos, text_, image0, image1));
}

void GWindowBuilder::AddButtonComponent(glm::vec2 scale, glm::vec3 pos, const char* text_, uint64_t image)
{
	win->AddComponent(new GComponentButton(scale, pos, text_, image));
}

void GWindowBuilder::AddImageComponent(glm::vec2 scale, glm::vec3 pos, uint64_t image)
{
	win->AddComponent(new GComponentImage(scale, pos, image));
}

void GWindowBuilder::AddLabelComponent(glm::vec2 scale, glm::vec3 pos, const char* text_)
{
	win->AddComponent(new GComponentLabel(scale, pos, text_));
}

GWindow* GWindowBuilder::BuildWindow()
{
	return win;
}
