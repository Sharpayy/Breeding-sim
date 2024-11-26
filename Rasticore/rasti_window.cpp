#include "rasti_window.h"

rasticore::RastiCoreWindow::RastiCoreWindow(const char* title, int x, int y)
{
	win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, x, y, SDL_WINDOW_OPENGL);
	//r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	customEventDispatch = windowEventMinDispatch;
	window_x = x;
	window_y = y;
}

rasticore::RastiCoreWindow::RastiCoreWindow(const char* title, int x, int y, Uint32 flg)
{
	win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, x, y, SDL_WINDOW_OPENGL | flg);
	//r = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	customEventDispatch = windowEventMinDispatch;
	window_x = x;
	window_y = y;
}

glm::mat4 rasticore::RastiCoreWindow::GetProjectionMatrix(float fov)
{
	return glm::perspective(fov, (float)window_x / (float)window_y, 0.1f, 1000.0f);
}

void rasticore::RastiCoreWindow::glCreateContext()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	//SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	glContext = SDL_GL_CreateContext(win);
	SDL_GL_SetSwapInterval(0);
}

void rasticore::RastiCoreWindow::handleEvents()
{
	while (SDL_PollEvent(&e))
		customEventDispatch(&e);
}

void rasticore::RastiCoreWindow::swap()
{
	SDL_GL_SwapWindow(win);

}

void rasticore::windowEventMinDispatch(SDL_Event* e)
{
	while (SDL_PollEvent(e))
	{
		if (e->type == SDL_QUIT)
			exit(0);
	}
}
