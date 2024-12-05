#pragma once
#include <SDL.h>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>

namespace rasticore
{
	class RastiCoreWindow
	{
	public:
		SDL_Window* win;
		//SDL_Renderer* r;

		SDL_GLContext glContext;
		SDL_Event e;
		int window_x, window_y;

		std::function<void(SDL_Event*)> customEventDispatch;

		RastiCoreWindow(const char* title, int x, int y);
		RastiCoreWindow(const char* title, int x, int y, Uint32 flg);

		glm::mat4 GetProjectionMatrix(float fov);

		void glCreateContext();
		void handleEvents();

		void swap();

	};

	void windowEventMinDispatch(SDL_Event* e);
}

