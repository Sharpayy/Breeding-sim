#include "rasti_main.h"

void ApplyTextureStdFilters(uint32_t id)
{
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void ApplyTextureStdFilters(rasticore::Texture2D tex)
{
	tex.bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void ApplyBindedTextureStdFilters()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void BindExternalTexture2d(uint32_t ld, uint32_t tx)
{
	glActiveTexture(ld);
	glBindTexture(GL_TEXTURE_2D, tx);
}

void BindSampler(const char* name, int32_t value, uint32_t prgm)
{
	glUniform1i(glGetUniformLocation(prgm, name), value);
}

uint32_t CalcFps(rasticore::CPUPerformanceTimer cpt, rasticore::LoopStatistics* ls)
{
	uint32_t t = ls->ticks;

	ls->tick_time_left -= cpt.GetElapsed();
	if (ls->tick_time_left <= 0)
	{
		ls->tick_time_left += CLOCKS_PER_SEC;
		ls->ticks = 1;
		return t;
	}
	ls->ticks++;
	return 0;
}

rasticore::PerformanceTimer GetEmptyTimer()
{
	return rasticore::PerformanceTimer(0);
}

void rasticore::InitEmptyTimer(rasticore::PerformanceTimer* tm)
{
	glGenQueries(1, &tm->id);
}

rasticore::Ray rasticore::RsCalcMouseRay(rasticore::InShaderMVP_DATA* mvp, glm::vec2 mouse, glm::vec2 screen)
{
	glm::vec4 nds;
	nds.x = (2.0f * mouse.x) / screen.x - 1.0f;
	nds.y = (2.0f * mouse.y) / screen.y - 1.0f;
	nds.z = -1.0f;
	nds.w = 1.0f;

	glm::mat4 inv_projection = glm::inverse(mvp->matProj);

	nds = inv_projection * nds;
	nds = glm::vec4(nds.x, nds.y, -1.0f, 0.0f);

	glm::mat4 inv_view = glm::inverse(mvp->matCamera);

	nds = inv_view * nds;

	rasticore::Ray ray = rasticore::Ray(glm::vec3(nds.x, nds.y, nds.z), glm::vec3(inv_view[3][0], inv_view[3][1], inv_view[3][2]));
	glm::normalize(ray.d);
	return ray;
}

uint8_t rasticore::RsCalcRayPlaneIntersection(rasticore::Ray ray, glm::vec3 n_plane, float d_plane, glm::vec3* r)
{
	float t = -((glm::dot(ray.o, n_plane) + d_plane) / glm::dot(ray.d, n_plane));

	if (t >= 0)
	{
		*r = ray.o + ray.d * t;
		return 1;
	}

	return 0;
}

void BoolSwap(bool* a, bool* b)
{
	bool t = *b;
	*b = *a;
	*a = t;
}

rasticore::VertexBuffer::VertexBuffer()
{
	glGenVertexArrays(1, &id);
	glBindVertexArray(id);
}

void rasticore::VertexBuffer::bind()
{
	glBindVertexArray(id);
}

void rasticore::VertexBuffer::addAttrib(GLenum type, uint32_t index, size_t size, uint32_t stride, uint32_t first)
{
	glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)first);
}

void rasticore::VertexBuffer::enableAttrib(uint32_t index)
{
	glEnableVertexAttribArray(index);
}

void rasticore::VertexBuffer::disableAttrib(uint32_t index)
{
	glDisableVertexAttribArray(index);
}

rasticore::UniformBufferObject::UniformBufferObject()
{
	glGenBuffers(1, &id);
}

void rasticore::UniformBufferObject::data(size_t size, void* data, GLenum usage)
{
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
}

void rasticore::UniformBufferObject::bind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, id);
}

void rasticore::UniformBufferObject::subdata(size_t offset, size_t size, void* data)
{
	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

void rasticore::UniformBufferObject::bindBase(uint32_t index)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, index, id);
}

rasticore::Program::Program()
{
	id = glCreateProgram();
}

void rasticore::Program::programCompile()
{
	glLinkProgram(id);
}

void rasticore::Program::programAddShader(uint32_t shdr)
{
	glAttachShader(id, shdr);
}

int rasticore::Program::programGetDebugInfo(char* bf, size_t bf_size)
{
	int v;
	glGetProgramiv(id, GL_LINK_STATUS, &v);

	if (v == 1)
		return v;

	int wr;
	glGetProgramInfoLog(id, bf_size, &wr, bf);
	return v;
}

void rasticore::Program::use()
{
	glUseProgram(id);
}


rasticore::ShaderStorageBufferObject::ShaderStorageBufferObject()
{
	glGenBuffers(1, &id);
}

void rasticore::ShaderStorageBufferObject::data(size_t size, void* data)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void rasticore::ShaderStorageBufferObject::bind()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
}

void rasticore::ShaderStorageBufferObject::bindBase(uint32_t index)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, id);
}

void rasticore::ShaderStorageBufferObject::subdata(size_t offset, size_t size, void* data)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
}

rasticore::RastiCoreRender::RastiCoreRender(uint32_t model_amount)
{
	models = DynamicList<RenderModel*>(model_amount);
	idTranslator = DynamicList<uint32_t>(model_amount);
	MatrixAlloc = rasticore::RsMatrixAllocator(10);

	md = nullptr;
	mdi = -1;

	MVP = InShaderMVP_DATA{ glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f) };

	uMVP = UniformBufferObject();
	uMVP.data(64 * 3, &MVP, GL_DYNAMIC_COPY);

	uMVP.bindBase(RENDERER_SHADER_MPV_DATA_LOCATION);
	/*
	mat4 matProj;
	mat4 matCamera;
	mat4 matProjCamera;
	*/

}

void rasticore::RastiCoreRender::setCameraMatrix(glm::mat4 cam)
{
	MVP.matCamera = cam;
	MVP.matProjCamera = MVP.matProj * MVP.matCamera;
}

void rasticore::RastiCoreRender::setProjectionMatrix(glm::mat4 proj)
{
	MVP.matProj = proj;
	MVP.matProjCamera = MVP.matProj * MVP.matCamera;
}

void rasticore::RastiCoreRender::UpdateShaderDataProjection()
{
	uMVP.subdata(0, SIZEOF_MAT4, &MVP.matProj);
	uMVP.subdata(SIZEOF_MAT4 * 2, SIZEOF_MAT4, &MVP.matProjCamera);
}

void rasticore::RastiCoreRender::UpdateShaderData()
{
	uMVP.subdata(0, SIZEOF_MAT4 * 3, &MVP);
}

void rasticore::RastiCoreRender::UpdateShaderDataCamera()
{
	uMVP.subdata(SIZEOF_MAT4, SIZEOF_MAT4, &MVP.matCamera);
	uMVP.subdata(SIZEOF_MAT4 * 2, SIZEOF_MAT4, &MVP.matProjCamera);
}

glm::mat4* rasticore::RastiCoreRender::GetShaderCameraMatrixLocation()
{
	return &MVP.matCamera;
}

rasticore::RenderModel* rasticore::RastiCoreRender::getModel(uint32_t id_model)
{
	return models.at(idTranslator.at(id_model));
}

rasticore::RenderModelDetails* rasticore::RastiCoreRender::getModelDetails(uint32_t m_id, RENDER_OBJECT_ID o_id)
{
	RenderModel* md_ = models.at(idTranslator.at(m_id));
	return md_->objects.at(md_->dlIndexSpace.at(md_->objectsId.at(o_id)));
}

void rasticore::RastiCoreRender::newModel(uint32_t id_model, rasticore::VertexBuffer vao, rasticore::Program prgm, uint32_t vertices, GLenum rtype, rasticore::Texture2DBindless texture, uint32_t o_amount)
{
	RenderModel* md = new RenderModel;
	*(idTranslator.base_ptr + id_model) = models.c_size;

	md->vao = vao;
	md->std_prgm = prgm;
	md->vertices = vertices;
	md->renedrPrimitive = rtype;
	md->std_texture2d = texture;
	md->std_normalMap = Texture2D();

	md->matrixSSBO = ShaderStorageBufferObject();
	md->matrixSSBO.data(o_amount * 64, NULL);
	md->matrixSSBO.bindBase(RENDERER_SHADER_MATRIX_SPACE_LOCATION);
	md->objectsId = DynamicList<uint32_t>(o_amount);
	md->stFreeIdSpace = STACK<rasticore::DoubleInt32>(o_amount);

	md->idSpaceSSBO = ShaderStorageBufferObject();
	md->idSpaceSSBO.data(o_amount * 4, NULL);
	//md->idSpaceSSBO = rasticore::ShaderStorageBufferOrphan(GL_DYNAMIC_DRAW, o_amount * 4);
	//md->idSpaceSSBO.bind();
	md->matrixSSBO.bindBase(RENDERER_SHADER_IDX_SPACE_LOCATION);
	md->dlIndexSpace = DynamicList<uint32_t>(o_amount);

	md->objAmount = 0;
	md->activeObjects = 0;
	md->objects = DynamicList<RenderModelDetails*>(o_amount);

	models.push_back(md);
}

RENDER_OBJECT_ID rasticore::RastiCoreRender::newObject(uint32_t id_model, glm::mat4 s_mat)
{
	if (mdi != id_model)
		BindActiveModel(id_model);

	rasticore::RenderModelDetails* rmd = (RenderModelDetails*)MatrixAlloc.AllocMatrix();
	uint32_t _object_id = -1;

	rasticore::DoubleInt32 spaceIdIdx = GetFreeMdIdSpaceIndex();


	rmd->model = s_mat;

	//md->objects.push_back(rmd);
	*md->objects.atp(spaceIdIdx.a) = rmd;
	*(md->objectsId.base_ptr + spaceIdIdx.a) = md->objAmount;
	md->objAmount++;
	md->activeObjects++;

	SetIdxSpaceValue(md->objAmount - 1, spaceIdIdx.b);
	*(md->dlIndexSpace.base_ptr + md->objAmount - 1) = spaceIdIdx.b;
	SetObjectMatrixMem(spaceIdIdx.b, s_mat);

	_object_id = spaceIdIdx.a;

	if (md->activeObjects != md->objAmount)
		EnableObjectInternal(_object_id);

	return _object_id;
}

RENDER_OBJECT_ID rasticore::RastiCoreRender::newObject(uint32_t id_model, glm::mat4 s_mat, uint64_t* longId)
{
	if (mdi != id_model)
		BindActiveModel(id_model);

	RenderModelDetails* rmd = (RenderModelDetails*)MatrixAlloc.AllocMatrix();
	uint32_t _object_id = -1;

	rasticore::DoubleInt32 spaceIdIdx = GetFreeMdIdSpaceIndex();

	rmd->model = s_mat;

	//md->objects.push_back(rmd);
	*md->objects.atp(spaceIdIdx.a) = rmd;
	*(md->objectsId.base_ptr + spaceIdIdx.a) = md->objAmount;
	md->objAmount++;
	md->activeObjects++;

	SetIdxSpaceValue(md->objAmount - 1, spaceIdIdx.b);
	*(md->dlIndexSpace.base_ptr + md->objAmount - 1) = spaceIdIdx.b;
	SetObjectMatrixMem(spaceIdIdx.b, s_mat);

	_object_id = spaceIdIdx.a;

	if (md->activeObjects != md->objAmount)
		EnableObjectInternal(_object_id);

	((RENDER_LONG_ID*)longId)->ModelId = mdi;
	((RENDER_LONG_ID*)longId)->ObjectId = _object_id;
	return _object_id;
}

void rasticore::RastiCoreRender::RenderSelectedModel(uint32_t id_model)
{
	RenderModel* md = models.at(idTranslator.at(id_model));

	if (md->activeObjects == 0)
		return;

	md->std_prgm.use();
	md->vao.bind();
	md->matrixSSBO.bindBase(RENDERER_SHADER_MATRIX_SPACE_LOCATION);
	md->idSpaceSSBO.bindBase(RENDERER_SHADER_IDX_SPACE_LOCATION);
	//md->std_texture2d.bind();
	glUniformHandleui64ARB(1, md->std_texture2d.handle);
	

	glDrawElementsInstanced(md->renedrPrimitive, md->vertices, GL_UNSIGNED_INT, NULL, md->activeObjects);

}

void rasticore::RastiCoreRender::RenderAll() // *
{
	for (int i = 0; i < models.c_size; i++)
	{
		RenderSelectedModel(i);
	}
}

void rasticore::RastiCoreRender::deleteObject(uint32_t m_id, RENDER_OBJECT_ID o_id)
{
	if (mdi != m_id)
		BindActiveModel(m_id);

	uint32_t lastObjIdx = md->objAmount - 1;
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	RenderModelDetails* obj = GetMdObject(o_id);


	//NotifyFreeIdx(o_id, obj->matrixId);
	NotifyFreeIdx(o_id, cObjectIdx);
	md->objAmount--;
	MatrixAlloc.FreeMatrix((LPVOID)GetMdObject(cObjectIdx));
	//md->objects.del_last();
	//md->objAmount--;
	//poolAllocator.freeAlignedMemory(obj);

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrder(cObjectIdx, lastObjIdx);
}

void rasticore::RastiCoreRender::SyncObjectMatrix(RENDER_OBJECT_ID o_id)
{
	uint32_t obj_idx = MapObjectToSpaceIdx(MapToObjectIdx(o_id));
	RenderModelDetails* obj = GetMdObject(o_id);
	md->matrixSSBO.bind();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, SIZEOF_MAT4 * obj_idx, SIZEOF_MAT4, &obj->model[0].x);
}

void rasticore::RastiCoreRender::SetObjectMatrix(RENDER_OBJECT_ID o_id, glm::mat4 mat, bool just_in_vram)
{
	uint32_t obj_idx = MapToObjectIdx(o_id);

	if (just_in_vram == 0)
		GetMdObject(obj_idx)->model = mat;

	if (IsObjectActive(o_id) == true)
	{
		md->matrixSSBO.bind();
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, MapObjectToSpaceIdx(obj_idx) * SIZEOF_MAT4, SIZEOF_MAT4, &mat[0].x);
	}
}

glm::mat4 rasticore::RastiCoreRender::GetObjectMatrix(RENDER_OBJECT_ID o_id)
{
	return GetMdObject(MapToObjectIdx(o_id))->model;
}

void rasticore::RastiCoreRender::EnableObject(uint32_t m_id, RENDER_OBJECT_ID o_id)
{
	if (mdi != m_id)
		BindActiveModel(m_id);
	EnableObject(o_id);
}

void rasticore::RastiCoreRender::EnableObject(RENDER_OBJECT_ID o_id)
{
	uint32_t lastObjIdx = (GetActiveObjectsInMd() + 1);
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	if ((cObjectIdx < md->activeObjects) == true)
		return;

	md->activeObjects++;

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrder(cObjectIdx, lastObjIdx);
}

void rasticore::RastiCoreRender::EnableObjectInternal(RENDER_OBJECT_ID o_id)
{
	uint32_t lastObjIdx = (GetActiveObjectsInMd());
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrder(cObjectIdx, lastObjIdx);
}

void rasticore::RastiCoreRender::DisableObject(uint32_t m_id, RENDER_OBJECT_ID o_id)
{
	if (mdi != m_id)
		BindActiveModel(m_id);
	DisableObject(o_id);
}

void rasticore::RastiCoreRender::DisableObject(RENDER_OBJECT_ID o_id)
{
	uint32_t lastObjIdx = (GetIdFirstMdInactiveObject() - 1);
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	if ((cObjectIdx < md->activeObjects) == false)
		return;

	md->activeObjects--;

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrder(cObjectIdx, lastObjIdx);
}

void rasticore::RastiCoreRender::EnableObjectNoSync(RENDER_OBJECT_ID o_id)
{
	uint32_t lastObjIdx = (GetActiveObjectsInMd() + 1);
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	if ((cObjectIdx < md->activeObjects) == true)
		return;

	md->activeObjects++;

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrderNoSync(cObjectIdx, lastObjIdx);
}

void rasticore::RastiCoreRender::DisableObjectNoSync(RENDER_OBJECT_ID o_id)
{
	uint32_t lastObjIdx = (GetIdFirstMdInactiveObject() - 1);
	uint32_t cObjectIdx = MapToObjectIdx(o_id);
	uint32_t sObjectIdx = MapObjectToSpaceIdx(cObjectIdx);

	if ((cObjectIdx < md->activeObjects) == false)
		return;

	md->activeObjects--;

	if (lastObjIdx == cObjectIdx)
		return;

	SwapObjectIdxOrder(sObjectIdx, MapObjectToSpaceIdx(lastObjIdx));
	SwapInBufferIdxOrderNoSync(cObjectIdx, lastObjIdx);
}

uint32_t rasticore::RastiCoreRender::MapObjectToSpaceIdx(uint32_t obj)
{
	return md->dlIndexSpace.at(obj);
}

bool rasticore::RastiCoreRender::IsObjectActive(RENDER_OBJECT_ID o_id)
{
	uint32_t ObjIdx = MapToObjectIdx(o_id);
	return (ObjIdx < md->activeObjects);
}

void rasticore::RastiCoreRender::MakeModelRef(uint32_t new_m_id, uint32_t ref_m_id)
{
	RenderModel* copy_model = GetRenderModelFromIndex(ref_m_id);

	*(idTranslator.base_ptr + new_m_id) = models.c_size;
	models.push_back(copy_model);
}

void rasticore::RastiCoreRender::SetModelShader(Program shdr)
{
	md->std_prgm = shdr;
}

void rasticore::RastiCoreRender::SetModelVertexArray(VertexBuffer vtx, uint32_t vtx_cnt)
{
	md->vao = vtx;
	md->vertices = vtx_cnt;
}

void rasticore::RastiCoreRender::SetModelTexture(Texture2D tex)
{
	md->std_texture2d = tex;
}

rasticore::VertexBuffer rasticore::RastiCoreRender::GetModelVertexArray()
{
	return md->vao;
}

uint32_t rasticore::RastiCoreRender::GetModelObjectCapacity()
{
	return md->objects.reserved;
}

void rasticore::RastiCoreRender::UpdateShaderIdSpace(uint32_t m_id)
{
	if (mdi != m_id)
		BindActiveModel(m_id);

	md->idSpaceSSBO.bind();
	md->idSpaceSSBO.subdata(0, md->objAmount * 4, md->dlIndexSpace.base_ptr);
	//md->idSpaceSSBO.data(md->dlIndexSpace.base_ptr, md->objAmount * 4);
}

void rasticore::RastiCoreRender::DisableObjectL(uint64_t longId)
{
	RENDER_LONG_ID ld = *((RENDER_LONG_ID*)&longId);
	if (mdi != ld.ModelId)
		BindActiveModel(ld.ModelId);

	DisableObject(ld.ObjectId);
}

void rasticore::RastiCoreRender::EnableObjectL(uint64_t longId)
{
	RENDER_LONG_ID ld = *((RENDER_LONG_ID*)&longId);
	if (mdi != ld.ModelId)
		BindActiveModel(ld.ModelId);

	EnableObject(ld.ObjectId);
}

rasticore::DoubleInt32 rasticore::RastiCoreRender::GetFreeIdSpaceIndex(uint32_t m_id)
{
	RenderModel* md = GetRenderModelFromIndex(GetTranslatedModelIndex(m_id));

	if (md->stFreeIdSpace.c_size == 0)
	{
		md->objectsId.push_back(-1);
		return rasticore::DoubleInt32{ md->objectsId.c_size - 1, md->objectsId.c_size - 1 };
	}

	rasticore::DoubleInt32 value_ = md->stFreeIdSpace.fget();
	md->stFreeIdSpace.pop();
	return value_;
}

uint32_t rasticore::RastiCoreRender::GetTranslatedModelIndex(uint32_t m_id)
{
	return idTranslator.at(m_id);
}

rasticore::RenderModel* rasticore::RastiCoreRender::GetRenderModelFromIndex(uint32_t m_idx)
{
	return models.at(m_idx);
}

uint32_t rasticore::RastiCoreRender::GetActiveObjectsInModel(uint32_t m_id)
{
	return GetRenderModelFromIndex(GetTranslatedModelIndex(m_id))->activeObjects;
}

void rasticore::RastiCoreRender::BindActiveModel(uint32_t m_id)
{
	md = GetRenderModelFromIndex(GetTranslatedModelIndex(m_id));
	mdi = m_id;
}

uint32_t rasticore::RastiCoreRender::GetActiveObjectsInMd()
{
	return md->activeObjects - 1;
}

rasticore::DoubleInt32 rasticore::RastiCoreRender::GetFreeMdIdSpaceIndex()
{
	if (md->stFreeIdSpace.c_size == 0)
	{
		md->objectsId.push_back(-1);
		return rasticore::DoubleInt32{ md->objectsId.c_size - 1, md->objectsId.c_size - 1 };
	}

	rasticore::DoubleInt32 value_ = md->stFreeIdSpace.fget();
	md->stFreeIdSpace.pop();
	return value_;
}

uint32_t rasticore::RastiCoreRender::GetIdFirstMdInactiveObject()
{
	return md->activeObjects;
}

void rasticore::RastiCoreRender::SwapObjectOrder(uint32_t idx0, uint32_t idx1)
{
	Swap<RenderModelDetails>(GetMdObject(idx0), GetMdObject(idx1));
}

void inline rasticore::RastiCoreRender::SwapObjectIdxOrder(uint32_t idx0, uint32_t idx1)
{
	Swap<uint32_t>(md->objectsId.atp(idx0), md->objectsId.atp(idx1));
}

void inline rasticore::RastiCoreRender::SwapInBufferIdxOrder(uint32_t idx0, uint32_t idx1)
{
	Swap<uint32_t>(md->dlIndexSpace.atp(idx0), md->dlIndexSpace.atp(idx1));

	//uint32_t value0, value1;
	md->idSpaceSSBO.bind();
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, idx0 * 4, 4, &value0);
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, idx1 * 4, 4, &value1);

	glBufferSubData(GL_SHADER_STORAGE_BUFFER, idx0 * 4, 4, md->dlIndexSpace.atp(idx0));
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, idx1 * 4, 4, md->dlIndexSpace.atp(idx1));
}

void rasticore::RastiCoreRender::SwapInBufferIdxOrderNoSync(uint32_t idx0, uint32_t idx1)
{
	Swap<uint32_t>(md->dlIndexSpace.atp(idx0), md->dlIndexSpace.atp(idx1));
}

rasticore::RenderModelDetails* rasticore::RastiCoreRender::GetMdObject(uint32_t idx)
{
	return md->objects.at(idx);
}

void rasticore::RastiCoreRender::SetObjectMatrixMem(uint32_t idx, glm::mat4 mat)
{
	md->matrixSSBO.bind();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, idx * SIZEOF_MAT4, SIZEOF_MAT4, &mat[0].x);
}

uint32_t rasticore::RastiCoreRender::MapToObjectIdx(uint32_t id)
{
	return md->objectsId.at(id);
	//return md->dlIndexSpace.at(id);
}

uint32_t rasticore::RastiCoreRender::MapToIndexSpace(uint32_t idx)
{
	uint32_t value;
	md->idSpaceSSBO.bind();
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, idx * 4, 4, &value);
	return value;
}

void rasticore::RastiCoreRender::SetIdxSpaceValue(uint32_t idx, uint32_t value)
{
	md->idSpaceSSBO.bind();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, idx * 4, 4, &value);
}

void rasticore::RastiCoreRender::NotifyFreeIdx(uint32_t idx, uint32_t mat_)
{
	md->stFreeIdSpace.put(rasticore::DoubleInt32{ idx, mat_ });
}

rasticore::Texture2D::Texture2D()
{
	id = 0;
	tu = 0;
}

void rasticore::RastiCoreRender::BindMVP()
{
	uMVP.bind();
	uMVP.bindBase(RENDERER_SHADER_MPV_DATA_LOCATION);
}

rasticore::Texture2D::Texture2D(void* data, int x, int y, GLenum data_type, GLenum data_int)
{
	glGenTextures(1, &id);
	tu = GL_TEXTURE0;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, data_int, x, y, 0, data_type, GL_UNSIGNED_BYTE, data);
}

rasticore::Texture2D::Texture2D(void* data, int x, int y, GLenum data_type, GLenum data_int, uint32_t bind_location)
{
	glGenTextures(1, &id);
	tu = bind_location;
	glActiveTexture(bind_location);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, data_int, x, y, 0, data_type, GL_UNSIGNED_BYTE, data);
}

void rasticore::Texture2D::genMipmap()
{
	glGenerateMipmap(GL_TEXTURE_2D);
}

void rasticore::Texture2D::bind()
{
	glActiveTexture(tu);
	glBindTexture(GL_TEXTURE_2D, id);
}

rasticore::AtomicCounter::AtomicCounter(uint32_t svalue, uint32_t bp)
{
	glGenBuffers(1, &id);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, id);

	value = svalue;
	bpoint = bp;

	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t), &value, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bpoint, id);
}

void rasticore::AtomicCounter::bind()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, id);
}

void rasticore::AtomicCounter::bindbase()
{
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bpoint, id);
}

void rasticore::AtomicCounter::setBindingPoint(uint32_t bp)
{
	bpoint = bp;
}

void* rasticore::AtomicCounter::map()
{
	return glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_WRITE);
}

void rasticore::AtomicCounter::unmap()
{
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}

void rasticore::AtomicCounter::sync(rasticore::ATOMIC_COUNTER_OPERATION op)
{
	if (op == ATC_SYNC_SHADER)
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, id);
		void* mp = map();
		value = *(uint32_t*)mp;
		unmap();
		return;
	}

	if (op == ATC_SYNC_CPU)
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, id);
		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, 4, &value);
		return;
	}
}

void rasticore::AtomicCounter::zero()
{
	uint32_t zero = 1;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, id);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, 4, &zero);
}

void rasticore::AtomicCounter::bindBaseCustom(uint32_t bp)
{
	bind();
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bp, id);
}

rasticore::Framebuffer::Framebuffer()
{
	glGenFramebuffers(1, &id);
}

void rasticore::Framebuffer::attachTextureId(uint16_t plc, uint32_t tex, GLenum attachTo)
{
	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachTo, GL_TEXTURE_2D, tex, 0);
	*(texture_id + plc) = tex;
}

void rasticore::Framebuffer::attachTexture(uint16_t plc, rasticore::Texture2D tex, GLenum attachTo)
{
	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachTo, GL_TEXTURE_2D, tex.id, 0);
	*(texture_id + plc) = tex.id;
}

void rasticore::Framebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

uint32_t rasticore::Framebuffer::getAttachment(uint16_t plc)
{
	return *(texture_id + plc);
}

void rasticore::Framebuffer::clear(GLenum comp)
{
	bind();
	glClear(comp);
}

rasticore::PerformanceTimer::PerformanceTimer()
{
	glGenQueries(1, &id);
	v = -1;
}

rasticore::PerformanceTimer::PerformanceTimer(uint32_t qid)
{
	id = qid;
	v = -1;
}

void rasticore::PerformanceTimer::TimeStart()
{
	glFinish();
	glBeginQuery(GL_TIME_ELAPSED, id);
}

void rasticore::PerformanceTimer::TimeEnd()
{
	glFinish();
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectui64v(id, GL_QUERY_RESULT, (GLuint64*)&v);
}

void rasticore::PerformanceTimer::Reset()
{
	v = -1;
}

int64_t rasticore::PerformanceTimer::GetElapsedTime()
{
	if (v == -1)
		return 0;
	return v;
}

rasticore::CPUPerformanceTimer::CPUPerformanceTimer()
{
	s_time = 0;
	e_time = 0;
}

void rasticore::CPUPerformanceTimer::Reset()
{
	s_time = 0;
	e_time = 0;
}

void rasticore::CPUPerformanceTimer::Start()
{
	s_time = clock();
}

void rasticore::CPUPerformanceTimer::End()
{
	e_time = clock();
}

clock_t rasticore::CPUPerformanceTimer::GetElapsed()
{
	return e_time - s_time;
}

rasticore::LoopStatistics::LoopStatistics()
{
	tick_time_left = CLOCKS_PER_SEC;
	ticks = 0;
}

rasticore::Texture2DBindless::Texture2DBindless()
{
	handle = 0;
}

rasticore::Texture2DBindless::Texture2DBindless(Texture2D tex)
{
	glBindTexture(GL_TEXTURE_2D, tex.id);
	handle = glGetTextureHandleARB(tex.id);
}

void rasticore::Texture2DBindless::genMipmap()
{
	glGenerateMipmap(GL_TEXTURE_2D);
}

void rasticore::Texture2DBindless::MakeResident()
{
	glMakeTextureHandleResidentARB(handle);
}

void rasticore::Texture2DBindless::MakeNonResident()
{
	glMakeTextureHandleNonResidentARB(handle);
}
