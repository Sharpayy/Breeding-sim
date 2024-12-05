#pragma once
#include <GL/glew.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <ctime>
#include "rasti_alloc.h"
#include "rasti_adv.h"

#define RENDERER_SHADER_MPV_DATA_LOCATION 1
#define RENDERER_SHADER_MATRIX_SPACE_LOCATION 5
#define RENDERER_SHADER_IDX_SPACE_LOCATION 6

typedef uint32_t RENDER_OBJECT_ID;
typedef uint32_t RENDER_MODEL_ID;

#define LONG_GET_MODEL(A)	(*(rasticore::RENDER_LONG_ID*)&A).ModelId
#define LONG_GET_OBJECT(A)	(*(rasticore::RENDER_LONG_ID*)&A).ObjectId

#define RENDERER_SHADER_MAIN_TEXTURE_NAME	"TEXTURE_HANDLE"

#define RS_ENABLE_FRATURE(A)						glEnable(A)
#define RS_DISABLE_FRATURE(A)						glDisable(A)
#define RS_CLEAR_FRAMEBUFFER(A)						glClear(A)
#define RS_BACKGROUND_CLEAR_COLOR(R, G, B, A)		glClearColor(R, G, B, A)

#define FRAMEBUFFER_ATTACHMENT_TEX0 0
#define FRAMEBUFFER_ATTACHMENT_TEX1 4

#define OFFSET_SV_MPV_DATA_PROJ 0
#define OFFSET_SV_MPV_DATA_CAMERA 64
#define OFFSET_SV_MPV_DATA_PROJ_CAMERA 128

#define SIZEOF_MAT4 sizeof(glm::mat4)

namespace rasticore
{
	class GBuffer
	{
	public:
		uint16_t h;
		uint16_t w;
	};

	class CompositeModel
	{
	public:
		uint32_t model_base_id;
		uint32_t model_count;
	};


	template<GLenum T>
	class Shader
	{
	public:
		uint32_t id;

		Shader(const char* src)
		{
			id = glCreateShader(T);
			glShaderSource(id, 1, &src, NULL);
			glCompileShader(id);
		}

		int shaderGetDebugInfo(char* bf, size_t bf_size)
		{
			int v;
			glGetShaderiv(id, GL_COMPILE_STATUS, &v);

			if (v == 1)
				return v;

			int wr;
			glGetShaderInfoLog(id, bf_size, &wr, bf);
			return v;
		}

		void shaderDelete()
		{
			glDeleteShader(id);
		}
	};

	template<GLenum T>
	class Buffer
	{
	public:
		uint32_t id;

		Buffer()
		{
			glGenBuffers(1, &id);
		}

		Buffer(size_t size, void* data, GLenum usage)
		{
			glGenBuffers(1, &id);
			glBindBuffer(T, id);
			glBufferData(T, size, data, usage);
		}

		void bind()
		{
			glBindBuffer(T, id);
		}

		void unbind()
		{
			glBindBuffer(T, 0);
		}

		void data(size_t size, void* data, GLenum usage)
		{
			glBufferData(T, size, data, usage);
		}

		uint64_t nvBufferAddress()
		{
			uint64_t p;
			glGetBufferParameterui64vNV(T, GL_BUFFER_GPU_ADDRESS_NV, &p);
			return p;
		}
	};

	template<GLenum T>
	void MakeBufferResident(Buffer<T> b, GLenum ac)
	{
		b.bind();
		glMakeBufferResidentNV(T, ac);
	}

	class Program
	{
	public:
		uint32_t id;

		Program();

		void programCompile();
		void programAddShader(uint32_t shdr);
		int programGetDebugInfo(char* bf, size_t bf_size);
		void use();
	};

	class VertexBuffer
	{
	public:
		uint32_t id;

		VertexBuffer();

		void bind();
		void addAttrib(GLenum type, uint32_t index, size_t size, uint32_t stride, uint32_t first);
		void enableAttrib(uint32_t index);
		void disableAttrib(uint32_t index);

	};

	enum ATOMIC_COUNTER_OPERATION
	{
		ATC_SYNC_SHADER,
		ATC_SYNC_CPU,
		ATC_WRITE_BUFFER,
		ATC_READ_BUFFER
	};

	class AtomicCounter
	{
	public:
		uint32_t bpoint;
		uint32_t id;

		uint32_t value;

		AtomicCounter(uint32_t svalue, uint32_t bp);
		void bind();
		void bindbase();

		void setBindingPoint(uint32_t bp);
		void* map();
		void unmap();
		void sync(ATOMIC_COUNTER_OPERATION op);
		void zero();
		void bindBaseCustom(uint32_t bp);
	};


	class Texture2D
	{
	public:
		uint32_t id;
		GLenum tu;
		Texture2D();
		Texture2D(void* data, int x, int y, GLenum data_type, GLenum data_int);
		Texture2D(void* data, int x, int y, GLenum data_type, GLenum data_int, uint32_t bind_location);
		Texture2D(void* data, int x, int y, GLenum data_type, GLenum data_int, uint32_t bind_location, GLenum type);
		void genMipmap();
		void bind();

	};


	class Texture2DBindless
	{
	public:
		uint64_t handle;
		Texture2DBindless();
		Texture2DBindless(Texture2D tex);
		void genMipmap();

		void MakeResident();
		void MakeNonResident();

	};

	void ApplyTextureStdFilters(uint32_t id);
	void ApplyTextureStdFilters(Texture2D tex);
	void ApplyBindedTextureStdFilters();


	class Framebuffer
	{
	public:
		uint32_t id;
		uint32_t texture_id[2];

		Framebuffer();
		void attachTexture(uint16_t plc, Texture2D tex, GLenum attachTo);
		void attachTextureId(uint16_t plc, uint32_t tex, GLenum attachTo);
		void bind();
		uint32_t getAttachment(uint16_t plc);
		void clear(GLenum comp);

	};

	void BindExternalTex2dLd0(uint32_t tex);
	void BindExternalTexture2d(uint32_t ld, uint32_t tx);

	void BindSampler(const char* name, int32_t value, uint32_t prgm);

	class UniformBufferObject
	{
	public:

		uint32_t id;

		UniformBufferObject();
		void data(size_t size, void* data, GLenum usage);
		void bindBase(uint32_t index);
		void bind();
		void subdata(size_t offset, size_t size, void* data);
	};

	class ShaderStorageBufferObject
	{
	public:
		uint32_t id;

		ShaderStorageBufferObject();

		void data(size_t size, void* data);
		void bind();
		void bindBase(uint32_t index);
		void subdata(size_t offset, size_t size, void* data);
	};

	struct RENDER_LONG_ID
	{
		uint32_t ModelId;
		uint32_t ObjectId;
	};

	struct DoubleInt32
	{
		uint32_t a;
		uint32_t b;
	};

	struct RenderModelDetails
	{
		glm::mat4 model;
	};

	struct RenderModel
	{
		VertexBuffer vao;

		uint32_t vertices;
		ShaderStorageBufferObject matrixSSBO;
		ShaderStorageBufferObject idSpaceSSBO;
		//rasticore::ShaderStorageBufferOrphan idSpaceSSBO;

		DynamicList<RenderModelDetails*> objects;
		DynamicList<uint32_t> objectsId;
		STACK<rasticore::DoubleInt32> stFreeIdSpace;

		DynamicList<uint32_t> dlIndexSpace;

		Program std_prgm;
		//Texture2D std_texture2d;
		//Texture2D std_normalMap;
		Texture2DBindless std_texture2d;
		Texture2DBindless std_normalMap;
		GLenum renedrPrimitive;

		uint32_t objAmount;
		uint32_t activeObjects;
	};

	struct InShaderMVP_DATA
	{
		glm::mat4 matProj;
		glm::mat4 matCamera;
		glm::mat4 matProjCamera;
	};

	struct RenderEvent
	{
		uint32_t type;
		uint32_t pv;
		uint32_t cv;
		uint32_t cs;
	};

	class PerformanceTimer
	{
	public:
		uint32_t id;
		int64_t v;

		PerformanceTimer();
		PerformanceTimer(uint32_t qid);
		void TimeStart();
		void TimeEnd();
		void Reset();
		int64_t GetElapsedTime();
	};

	class CPUPerformanceTimer
	{
	public:
		clock_t s_time; clock_t e_time;

		CPUPerformanceTimer();

		void Reset();
		void Start();
		void End();
		clock_t GetElapsed();
	};

	class LoopStatistics
	{
	public:
		LoopStatistics();
		int tick_time_left;
		uint32_t ticks;
	};

	uint32_t CalcFps(CPUPerformanceTimer cpt, LoopStatistics* ls);

	PerformanceTimer GetEmptyTimer();
	void InitEmptyTimer(PerformanceTimer* tm);
	void InitGlobalTimer();


	void BoolSwap(bool* a, bool* b);

	template<typename T>
	void Swap(T* a, T* b)
	{
		T t = *a;
		*a = *b;
		*b = t;
	}

	template<typename T> void swap(T* t1, T* t2) {
		T* temp = std::move(t1);
		t1 = std::move(t2);
		t2 = std::move(temp);
	}

	class RastiCoreRender
	{
	public:
		RastiCoreRender() = default;

		DynamicList<RenderModel*> models;
		rasticore::RsMatrixAllocator MatrixAlloc;


		DynamicList<uint32_t> idTranslator;
		RenderModel* md;
		uint32_t mdi;

		InShaderMVP_DATA MVP;
		UniformBufferObject uMVP;

		RastiCoreRender(uint32_t model_amount);

		void setCameraMatrix(glm::mat4 cam);
		void setProjectionMatrix(glm::mat4 proj);

		void UpdateShaderDataProjection();
		void UpdateShaderData();
		void UpdateShaderDataCamera();
		glm::mat4* GetShaderCameraMatrixLocation();

		RenderModel* getModel(uint32_t id_model);
		RenderModelDetails* getModelDetails(uint32_t m_id, RENDER_OBJECT_ID o_id);
		void newModel(uint32_t id_model, VertexBuffer vao, Program prgm, uint32_t vertices, GLenum rtype, Texture2DBindless texture, uint32_t o_amount);

		RENDER_OBJECT_ID newObject(uint32_t id_model, glm::mat4 s_mat);
		RENDER_OBJECT_ID newObject(uint32_t id_model, glm::mat4 s_mat, uint64_t* longId);

		void RenderSelectedModel(uint32_t id_model);
		void RenderAll();
		//void renderSelectedObject(uint32_t id_model, uint32_t id_object);

		void deleteObject(uint32_t m_id, RENDER_OBJECT_ID o_id);

		void SyncObjectMatrix(RENDER_MODEL_ID o_id);
		void SetObjectMatrix(RENDER_MODEL_ID o_id, glm::mat4 mat, bool just_in_vram = false);
		glm::mat4 GetObjectMatrix(RENDER_MODEL_ID o_id);

		void EnableObject(uint32_t m_id, RENDER_OBJECT_ID o_id);
		void EnableObject(RENDER_OBJECT_ID o_id);
		void DisableObject(uint32_t m_id, RENDER_OBJECT_ID o_id);
		void DisableObject(RENDER_OBJECT_ID o_id);

		void EnableObjectNoSync(RENDER_OBJECT_ID o_id);
		void DisableObjectNoSync(RENDER_OBJECT_ID o_id);

		void EnableObjectInternal(RENDER_OBJECT_ID o_id);

		uint32_t MapObjectToSpaceIdx(uint32_t obj);
		uint32_t GetObjectIdSpaceIdx(uint32_t obj);

		void PreuseModelShader(uint32_t m_id);
		void MakeModelRef(uint32_t new_m_id, uint32_t ref_m_id);
		void SetModelShader(Program shdr);
		void SetModelVertexArray(VertexBuffer vtx, uint32_t vtx_cnt);
		void SetModelTexture(Texture2D tex);
		VertexBuffer GetModelVertexArray();
		uint32_t GetModelObjectCapacity();

		void UpdateShaderIdSpace(uint32_t m_id);

		void DisableObjectL(uint64_t longId);
		void EnableObjectL(uint64_t longId);


		DoubleInt32 GetFreeIdSpaceIndex(uint32_t m_id);

		uint32_t GetTranslatedModelIndex(uint32_t m_id);
		RenderModel* GetRenderModelFromIndex(uint32_t m_idx);
		uint32_t GetActiveObjectsInModel(uint32_t m_id);
		void BindActiveModel(uint32_t m_id);

		uint32_t GetActiveObjectsInMd();
		DoubleInt32 GetFreeMdIdSpaceIndex();

		uint32_t GetIdFirstMdInactiveObject();

		void SwapObjectOrder(uint32_t idx0, uint32_t idx1);
		void SwapObjectIdxOrder(uint32_t idx0, uint32_t idx1);
		void SwapInBufferIdxOrder(uint32_t idx0, uint32_t idx1);
		void SwapInBufferIdxOrderNoSync(uint32_t idx0, uint32_t idx1);

		uint32_t MapToObjectIdx(uint32_t id);
		uint32_t MapToIndexSpace(uint32_t idx);

		RenderModelDetails* GetMdObject(uint32_t idx);

		void SetObjectMatrixMem(uint32_t idx, glm::mat4 mat);
		void SetIdxSpaceValue(uint32_t idx, uint32_t value);

		bool IsObjectActive(uint32_t o_id);
		void NotifyFreeIdx(uint32_t idx, uint32_t mat_);
		void BindMVP();


	};

	class ModelCreationDetails
	{
	public:
		rasticore::VertexBuffer vb;
		rasticore::Program p;
		rasticore::Texture2DBindless txb;
		GLenum rm;
		uint32_t v_cnt;
	};
}