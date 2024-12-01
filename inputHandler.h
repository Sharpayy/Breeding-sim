#pragma once
#include <SDL.h>

class InputHandler {
public:
	InputHandler() {
		events = new SDL_Event;
	}
	InputHandler(SDL_Event* events) {
		this->events = events;
	}

	bool KeyPressed(SDL_Scancode keycode) {
		return inputData.keysPressed[keycode];
	}

	bool KeyPressedOnce(SDL_Scancode key) {
		if (inputData.keysPressed[key]) {
			if (inputData.wasPressed[key] == true) return false;
			inputData.wasPressed[key] = true;
			return true;
		}
		else {
			inputData.wasPressed[key] = false;
		}
		return false;
	}

	void handleKeys() {
		while (SDL_PollEvent(events)) {
			switch (events->type) {
			case SDL_KEYDOWN:
				inputData.keysPressed[events->key.keysym.scancode] = true;
				break;
			case SDL_KEYUP:
				inputData.keysPressed[events->key.keysym.scancode] = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (events->button.button == SDL_BUTTON_LEFT) {
					inputData.keysPressed[SDL_SCANCODE_LEFT] = true;
				}
				else if (events->button.button == SDL_BUTTON_RIGHT) {
					inputData.keysPressed[SDL_SCANCODE_RIGHT] = true;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (events->button.button == SDL_BUTTON_LEFT) {
					inputData.keysPressed[SDL_SCANCODE_LEFT] = false;
				}
				else if (events->button.button == SDL_BUTTON_RIGHT) {
					inputData.keysPressed[SDL_SCANCODE_RIGHT] = false;
				}
				break;
			case SDL_QUIT:
				break;


			default:
				break;
			}
		}
	}
private:
	SDL_Event* events;
	struct InputData {
		bool keysPressed[SDL_NUM_SCANCODES] = { false };
		bool wasPressed[SDL_NUM_SCANCODES] = { false };
	} inputData;
};