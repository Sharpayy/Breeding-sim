#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include "gui.h"

class Item {
public:
	int i = 0;
};

struct ObjectDim {
	glm::vec2 position;
	int width;
	int height;

	bool isRectInRect(const ObjectDim& outer) {
		return ((outer.position.x + outer.width) <= (position.x + width)
			&& (outer.position.x) >= (position.x)
			&& (outer.position.y) >= (position.y)
			&& (outer.position.y + outer.height) <= (position.y + height)
			);
	}
};

class Slot {
public:
	Slot() = default;
	Slot(Item* item, glm::vec2 position, int width, int height) {
		this->item = item;
		this->slotDim = { position, width, height };
	}

	Item* getItem() {
		return item;
	}

	ObjectDim getDim() {
		return slotDim;
	}

	//DO NOT USE THAT
	void changePosition(glm::vec2 position) {
		this->slotDim.position -= position;
	}

	void changeItem(Item* item) {
		this->item = item;
	}

	bool pointInRect(glm::vec2 point) {
		return (point.x >= slotDim.position.x && point.x <= slotDim.position.x + slotDim.width
			&& point.y >= slotDim.position.y && point.y <= slotDim.position.y + slotDim.height);
	}

private:
	Item* item;
	ObjectDim slotDim;

	GComponentImage* comp;
};

struct Window {
	std::string name;
	ObjectDim dim;
	uint8_t height;
	std::vector<Slot*> slots;
	GWindow* win;
};

class Inventory {
public:
	Inventory() {
		windowSlots[0] = {};
		windowSlots[1] = {};
	}

	void Render(glm::mat4 pm)
	{
		float base_h = 2.0f;
		auto wins = getActiveWindows();

		for (auto& i : wins)
		{
			i->win->Render(pm);
		}
	}

	std::vector<Window*> getActiveWindows() {
		return windowSlots.at(1);
	}

	bool setWindowHeight(std::string windowName, uint8_t height) {
		bool active = false;
		Window* win = windowExist(windowName, active);
		if (!win)
		{
			active = true;
			win = windowExist(windowName, active);
		}
		else if (!win) return false;
		win->height = height;
		sortVec(active);
	}

	bool changeWindowPosition(std::string windowName, int x, int y) {
		bool active = false;
		Window* win = windowExist(windowName, active);
		if (!win) {
			active = true;
			win = windowExist(windowName, active);
		}
		if (!win) return false;
		int offsetX, offsetY;
		offsetX = win->dim.position.x - x;
		offsetY = win->dim.position.y - y;
		win->dim.position.x = x;
		win->dim.position.y = y;
		auto slots = getAllSlotsFromWindow(windowName);
		for (auto& slot : slots) {
			slot->changePosition(glm::vec2{ offsetX,offsetY });
		}
	}

	bool ActivateWindow(std::string windowName) {
		Window* win = windowExist(windowName, 0);
		if (!win) return false;
		windowSlots[1].push_back(win);
		auto& vectorAt1 = windowSlots[0];
		auto it = std::find(vectorAt1.begin(), vectorAt1.end(), win);
		if (it != vectorAt1.end()) {
			vectorAt1.erase(it);
		}
		sortVec(1);
	}

	bool DisableWindow(std::string windowName) {
		Window* win = windowExist(windowName, 0);
		if (!win) return false;
		windowSlots[0].push_back(win);
		auto& vectorAt1 = windowSlots[1];
		auto it = std::find(vectorAt1.begin(), vectorAt1.end(), win);
		if (it != vectorAt1.end()) {
			vectorAt1.erase(it);
		}
		sortVec(0);
	}

	void AddWindow(std::string windowName, ObjectDim dim, uint8_t height, uint64_t tex) {
		Window* win = new Window{ windowName, dim, height, {}, new GWindow(dim.position, glm::vec2(dim.width, dim.height), tex)};
		windowSlots[0].push_back(win);
		sortVec(0);
	}

	std::vector<Slot*> getAllSlotsFromWindow(std::string windowName) {
		Window* win = windowExist(windowName, 0);
		if (!win) win = windowExist(windowName, 1);
		if (!win) return {};
		return win->slots;
	}

	Slot* AddSlotToWindow(std::string windowName, Slot slot, uint64_t tex = 0) {
		Window* win = windowExist(windowName, 0);
		if (!win) win = windowExist(windowName, 1);
		if (!win) return nullptr;
		ObjectDim slotDim = slot.getDim();
		if (!win->dim.isRectInRect(slotDim)) return nullptr;
		Slot* nslot = new Slot{ slot.getItem(), slotDim.position, slotDim.width, slotDim.height };
		win->slots.push_back(nslot);
		win->win->AddComponent(new GComponentImage(glm::vec2(slot.getDim().width, slot.getDim().height), glm::vec3(slot.getDim().position.x, slot.getDim().position.y, win->win->z + 0.1f), tex));
		return nslot;
	}

	void swapItems(Slot* slot1, Slot* slot2) {
		Item* itemTemp = slot1->getItem();
		slot1->changeItem(slot2->getItem());
		slot2->changeItem(itemTemp);
	}

	Slot* getSlot(glm::vec2 position) {
		for (auto& window : windowSlots.at(true)) {
			if (pointInRect(position, window->dim)) {
				for (auto& slot : window->slots) {
					if (pointInRect(position, slot->getDim())) return slot;
				}
			}
		}
		return nullptr;
	}

private:

	bool pointInRect(glm::vec2 position, ObjectDim dim) {
		return position.x >= dim.position.x && position.x <= dim.position.x + dim.width
			&& position.y >= dim.position.y && position.y <= dim.position.y + dim.height;
	}

	Window* windowExist(std::string windowName, bool active) {
		auto& windows = windowSlots.at(active);
		for (auto& window : windows) {
			if (window->name == windowName) {
				return window;
			}
		}
		return nullptr;
	}

	void sortVec(uint8_t active) {
		auto& windows = windowSlots.at(active);
		std::sort(windows.begin(), windows.end(), [](Window* a, Window* b) {
			return a->height > b->height;
			});
	}

	std::unordered_map<bool, std::vector<Window*>> windowSlots;
};
