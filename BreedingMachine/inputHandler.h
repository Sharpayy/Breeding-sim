#pragma once
#include <SDL.h>

class InputHandler {
public:
    static InputHandler& getInstance() {
        static InputHandler inputHandlerInstance;
        return inputHandlerInstance;
    }

    InputHandler(const InputHandler&) = delete;
    InputHandler& operator=(const InputHandler&) = delete;

    bool KeyPressed(SDL_Scancode keycode) const {
        return inputData.keysPressed[keycode];
    }

    bool KeyPressedOnce(SDL_Scancode key) {
        if (inputData.keysPressed[key]) {
            if (inputData.wasPressed[key]) return false;
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
            if (events->type == SDL_QUIT)
                exit(0);
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
    InputHandler() {
        events = new SDL_Event;
    }

    ~InputHandler() {
        delete events;
    }

    SDL_Event* events = nullptr;
    struct InputData {
        bool keysPressed[SDL_NUM_SCANCODES] = { false };
        bool wasPressed[SDL_NUM_SCANCODES] = { false };
    } inputData;
};
