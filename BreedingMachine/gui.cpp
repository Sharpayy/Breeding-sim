#include "gui.h"

rasticore::Program* gui_main_program;
rasticore::VertexBuffer* gui_square;
uint32_t gui_program_mat_loc;

rasticore::UniformBufferObject* gui_prog_ubo;
GResourceStore* gui_resources;

GResourceStore::GResourceStore(uint32_t start_size)
{
	textures = std::vector<GTEXTURECONTAINER>();

	textures_ssbo = rasticore::ShaderStorageBufferObject();
	textures_ssbo.bind();
	textures_ssbo.data(start_size * 8, NULL);
	textures_ssbo.bindBase(RESOURCE_STORE_BIND_LOCATION);
}

uint64_t GResourceStore::AddNewTexture(const char* filename)
{
	rasticore::Image img = rasticore::Image(filename, 4);
	rasticore::Texture2D tx{ img.data, (int)img.x_, (int)img.y_, GL_RGBA, GL_RGBA8 };
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	tx.genMipmap();
	rasticore::Texture2DBindless txb{ tx };
	txb.MakeResident();

	GTEXTUREID id = textures.size();
	textures.push_back(GTEXTURECONTAINER{ tx, txb });
	textures_ssbo.bind();
	textures_ssbo.subdata(id * 8, 8, &txb.handle);
	return txb.handle;
}

GComponentSlider::GComponentSlider(glm::vec2 scale, glm::vec2 pos, const char* text_, uint64_t base, uint64_t fill)
{
	scale_x = scale.x;
	scale_y = scale.y;

	text = gltCreateText();
	gltSetText(text, text_);

	pos_x = pos.x;
	pos_y = pos.y;

	base_tex_id = base;
	fill_tex_id = fill;

	value = 0.0f;
}

void GComponentSlider::Render(glm::mat4 pm)
{
	gui_main_program->use();

	GSHADERRENDERDATA_SLIDER data{};
	data.d.gui_element = GUI_ELEMENT_SLIDER;
	data.d.pos_ = glm::vec2(pos_x, pos_y);
	data.d.scale_ = glm::vec2(scale_x, scale_y);
	data.d.val = value;

	data.textures[0] = base_tex_id;
	data.textures[2] = fill_tex_id;

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

void gui_init()
{
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

	float plane_vtx[] = {
		-0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 1.0f, 0.0f, 1.0f,  0.0f,
		0.5, -0.5f, 1.0f, 1.0f, 0.0f,   1.0f,

		0.5, 0.5f, 1.0f, 1.0f, 1.0f,	1.0f
	};

	uint32_t chuj[] = {
		0,1,2,1,2,3
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

	gui_resources = new GResourceStore(500);

	gui_prog_ubo = new rasticore::UniformBufferObject();
	gui_prog_ubo->data(sizeof(GSHADERRENDERDATA) + 64, NULL, GL_DYNAMIC_DRAW);

}

void pfnBasicButtonCallback(GComponentButton* button, GWindow* window)
{
	
}

GComponentButton::GComponentButton(glm::vec2 scale, glm::vec2 pos, const char* text_, uint64_t tex)
{
	scale_x = scale.x;
	scale_y = scale.y;

	text = gltCreateText();
	gltSetText(text, text_);

	pos_x = pos.x;
	pos_y = pos.y;

	texture = tex;
	callback = (GComponentButton_Callback)pfnBasicButtonCallback;

	val = 0.0f;
}

void GComponentButton::SetCallback(GComponentButton_Callback func)
{
	callback = func;
}

void GComponentButton::Render(glm::mat4 pm)
{
	gui_main_program->use();

	GSHADERRENDERDATA_BUTTON data{};
	data.d.gui_element = GUI_ELEMENT_BUTTON;
	data.d.pos_ = glm::vec2(pos_x, pos_y);
	data.d.scale_ = glm::vec2(scale_x, scale_y);
	data.d.val = val;

	data.textures[0] = texture;

	gui_prog_ubo->bind();
	gui_prog_ubo->subdata(0, sizeof(GSHADERRENDERDATA_BUTTON), &data);
	gui_prog_ubo->bindBase(GUI_ELEMENT_BIND_LOCATION);

	gui_square->bind();

	glUniformMatrix4fv(gui_program_mat_loc, 1, GL_FALSE, (float*)&pm);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

int GComponentButton::ClickCheck(float x, float y, void* window)
{
	float _min = -0.5f * scale_x + pos_x;
	float _max =  0.5f * scale_x + pos_x;

	if (x >= _min && x < _max)
	{
		_min = -0.5f * scale_y + pos_y;
		_max =  0.5f * scale_y + pos_y;
		if (y >= _min && y < _max)
		{
			return 1;
			callback(this, window);
		}
	}
	return 0;
}

int GComponentButton::GetType()
{
	return GUI_ELEMENT_BUTTON;
}
