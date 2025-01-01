#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <string.h>
#include <cassert>
#include "Entity.h"

class ItemLoader {
public:
	ItemLoader() = default;

	void loadItem(Item& item) {
		switch (item.getObjectType()) {
		case ARMOR:
			itemMap[item.getItemName()] = new ArmorItem{ item.getItemName(), item.getItemTexture(), item.getObjectType(), (ArmorItem::ObjectStatistic*)item.getObjectStatistic(), item.getItemPrice() };
			break;
		case WEAPON:
			itemMap[item.getItemName()] = new WeaponItem{ item.getItemName(), item.getItemTexture(), item.getObjectType(), (WeaponItem::ObjectStatistic*)item.getObjectStatistic(), item.getItemPrice() };
			break;
		default:
			itemMap[item.getItemName()] = new Item{ item.getItemName(), item.getItemTexture(), item.getObjectType(), item.getItemPrice() };
			break;
		}
	}

	template <typename T = Item>
	T* getItem(std::string itemName) {
		auto it = itemMap.find(itemName);
		if (it != itemMap.end()) {
			uint8_t objectType = it->second->getObjectType();
			if (objectType & WEAPON) {
				if constexpr (std::is_same_v<T, WeaponItem>) {
					return (T*)it->second;
				}
				else return nullptr;
			}
			if (objectType & ARMOR) {
				if constexpr (std::is_same_v<T, ArmorItem>) {
					return (T*)it->second;
				}
				return nullptr;
			}
			if (objectType & EVERY_ITEM) {
				if constexpr (std::is_same_v<T, Item>) {
					return (T*)it->second;
				}
				return nullptr;
			}
		}
		return nullptr;
	}

private:
	std::unordered_map<std::string, Item*> itemMap;
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
	Slot(Item* object, glm::vec2 position, int width, int height, uint8_t type = EVERY_ITEM) {
		if(object) assert(object->getObjectType() & type);
		this->object = object;
		this->slotDim = { position, width, height };
		this->type = type;
	}

	Item* getItem() {
		return object;
	}

	uint8_t getSlotType() {
		return type;
	}

	ObjectDim getDim() {
		return slotDim;
	}

	//DO NOT USE THAT
	void changePosition(glm::vec2 position) {
		this->slotDim.position = position;
	}

	bool changeItem(Item* object) {
		if (object == nullptr)
		{
			((GComponentImage*)item_comp)->texture = 0;
			this->object = object;
			return false;
		}

		uint64_t tx = (uint64_t)object->getItemTexture();

		if (tx == -1)
			((GComponentImage*)item_comp)->texture = 0;
		else
			((GComponentImage*)item_comp)->texture = tx;
		
		return true;
	}

	bool pointInRect(glm::vec2 point) {
		return (point.x >= slotDim.position.x && point.x <= slotDim.position.x + slotDim.width
			&& point.y >= slotDim.position.y && point.y <= slotDim.position.y + slotDim.height);
	}

	GComponentImage* slot_comp;
	GComponentImage* item_comp;
	GWindow* parent_win;

private:
	Item* object;
	ObjectDim slotDim;
	uint8_t type;
};

class Inventory {
public:
	Inventory() {
		windowSlots[0] = {};
		windowSlots[1] = {};

		cursor_comp = new GComponentImage(glm::vec2(50.0f), glm::vec3(0.0f, 0.0f, 50.0f), 0);
	}

	struct Window {
		std::string name;
		ObjectDim dim;
		uint8_t height;
		std::vector<Slot*> slots;
		GWindow* win;
	};

	void RenderCursor(glm::mat4 pm)
	{
		cursor_comp->Render(pm);
	}

	void SetCursorPosition(glm::vec2 pos)
	{
		cursor_comp->pos.x += pos.x;
		cursor_comp->pos.y += pos.y;
	}

	void SetCursorItemHold(Item* item)
	{
		cursor_hold = item;
		if (item != nullptr)
			cursor_comp->texture = (uint64_t)item->getItemTexture();
		else
			cursor_comp->texture = 0;
	}

	Item* GetCursorItemHold()
	{
		return cursor_hold;
	}

	void RenderText(glm::mat4 pm)
	{
		float base_depth = 2.0f;
		auto wins = getActiveWindows();

		for (auto& i : wins)
		{
			i->win->UpdateDepth(base_depth);
			i->win->UpdateZComp();
			i->win->RenderText(pm);
			base_depth += 2.0f;
		}
	}

	void Render(glm::mat4 pm)
	{
		float base_depth = 2.0f;
		auto wins = getActiveWindows();

		auto svec = std::vector<Window*>(wins.rbegin(), wins.rend());
		for (auto& i : svec)
		{
			i->win->UpdateDepth(base_depth);
			i->win->UpdateZComp();
			i->win->Render(pm);
			base_depth += 2.0f;
		}
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
		offsetX = win->dim.position.x + x;
		offsetY = win->dim.position.y + y;
		win->dim.position.x = x;
		win->dim.position.y = y;
		auto slots = getAllSlotsFromWindow(windowName);
		for (auto& slot : slots) {
			slot->changePosition(glm::vec2{ offsetX,offsetY });	
		}
		win->win->ChangeComponentPosition(x, y);
	}

	bool ActivateWindow(std::string windowName) {
		Window* win = windowExist(windowName, 0);
		if (!win) return false;
		win->height = MAXINT64;
		windowSlots[1].push_back(win);
		auto& vectorAt1 = windowSlots[0];
		auto it = std::find(vectorAt1.begin(), vectorAt1.end(), win);
		if (it != vectorAt1.end()) {
			vectorAt1.erase(it);
		}
		win->height = 0xff;
		sortVec(1);
	}

	bool DisableWindow(std::string windowName) {
		Window* win = windowExist(windowName, 1);
		if (!win) return false;
		win->height = 0;
		windowSlots[0].push_back(win);
		auto& vectorAt1 = windowSlots[1];
		auto it = std::find(vectorAt1.begin(), vectorAt1.end(), win);
		if (it != vectorAt1.end()) {
			vectorAt1.erase(it);
		}
		sortVec(0);
	}

	void AddWindow(std::string windowName, ObjectDim dim, uint8_t height, uint64_t tex) {
		Window* win = new Window{ windowName, dim, height, {}, new GWindow{ dim.position, glm::vec2{ dim.width, dim.height }, tex } };
		windowSlots[0].push_back(win);
		sortVec(0);
	}

	GWindow* getGWindow(std::string windowName) {
		Window* win = windowExist(windowName, 0);
		if (!win) win = windowExist(windowName, 1);
		if (!win) return nullptr;
		return win->win;
	}

	std::vector<Window*> getActiveWindows() {
		return windowSlots.at(1);
	}

	bool isWindowActive(std::string windowName) {
		Window* win = windowExist(windowName, 1);
		if (win) return true;
		else return false;
	}

	std::vector<Slot*> getAllSlotsFromWindow(std::string windowName) {
		Window* win = windowExist(windowName, 0);
		if (!win) win = windowExist(windowName, 1);
		if (!win) return {};
		return win->slots;
	}

	Slot* AddSlotToWindow(std::string windowName, Slot slot, uint64_t tex) {
		Window* win = windowExist(windowName, 0);
		if (!win) win = windowExist(windowName, 1);
		if (!win) return nullptr;
		ObjectDim slotDim = slot.getDim();
		if (!win->dim.isRectInRect(slotDim)) return nullptr;
		Slot* nslot = new Slot{ slot.getItem(), slotDim.position, slotDim.width, slotDim.height, slot.getSlotType() };
		nslot->item_comp = new GComponentImage(glm::vec2(slot.getDim().width, slot.getDim().height), glm::vec3(slot.getDim().position.x, slot.getDim().position.y, 0.1f), 0);
		nslot->slot_comp = new GComponentImage(glm::vec2(slot.getDim().width, slot.getDim().height), glm::vec3(slot.getDim().position.x, slot.getDim().position.y, 0.2f), tex);
		win->slots.push_back(nslot);
		nslot->parent_win = win->win;
		win->win->AddComponent(nslot->item_comp);
		win->win->AddComponent(nslot->slot_comp);
		return nslot;
	}

	bool swapItems(Slot* slot1, Slot* slot2) {
		if (!(slot1->getSlotType() & slot2->getSlotType())) return false;
		Item* itemTemp = slot1->getItem();
		slot1->changeItem(slot2->getItem());
		slot2->changeItem(itemTemp);
		return true;
	}

	void setPressedWindowOnTop(glm::vec2 position) {
		bool windowExist = false;
		for (auto& window : windowSlots.at(true)) {
			if (pointInRect(position, window->dim)) {
				window->height = INT_MAX;
				windowExist = true;
			}
		}
		if(windowExist) sortVec(1);
	}

	Slot* getSlot(glm::vec2 position) {
		uint8_t windowsAmount = windowSlots.at(true).size();
		uint8_t height = 0;
		Slot* potentialSlot = nullptr;
		int i = 0;
		for (auto& window : windowSlots.at(true)) {
			if (potentialSlot) break;
			i++;
			if (pointInRect(position, window->dim)) {
				for (auto& slot : window->slots) {
					if (pointInRect(position, slot->getDim())) {
						height = i - 1;
						potentialSlot = slot;
						break;
					}
				}
			}
		}
		for (int x = 0; x < i - 1; x++) {
			if (pointInRect(position, windowSlots.at(true).at(x)->dim)) { potentialSlot = nullptr; break; };
		}
		return potentialSlot;
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
		int height = 0, windowsAmount = windowSlots.at(active).size();
		for (auto& win : windowSlots.at(active)) {
			if (win->height != height) win->height = windowsAmount - height;
			height++;
		}
	}

	std::unordered_map<bool, std::vector<Window*>> windowSlots;

	GComponentImage* cursor_comp;
	Item* cursor_hold;
};