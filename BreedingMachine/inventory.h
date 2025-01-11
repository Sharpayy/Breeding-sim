#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <string.h>
#include <cassert>
#include "Entity.h"
#include "gui.h"
#include "Define.h"
#include "Building.h"
#include "generator.h"
#include "EntityNames.h"

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
	Slot(Item* object, glm::vec2 position, int width, int height, uint32_t type = EVERY_ITEM) {
		if(object) assert(object->getObjectType() & type);
		this->object = object;
		this->slotDim = { position, width, height };
		this->type = type;
	}

	Item* getItem() {
		return object;
	}

	uint32_t getSlotType() {
		return type;
	}

	ObjectDim getDim() {
		return slotDim;
	}

	//DO NOT USE THAT
	void changePosition(glm::vec2 position) {
		this->slotDim.position = position;
	}

	glm::vec2 getPosition() {
		return slotDim.position;
	}

	bool changeItem(Item* object) {
		this->object = object;
		if (object == nullptr)
		{
			((GComponentImage*)item_comp)->texture = 0;
			return false;
		}
		else if (type & object->getObjectType()) {

			uint64_t tx = (uint64_t)object->getItemTexture();

			if (tx == -1)
				((GComponentImage*)item_comp)->texture = 0;
			else
				((GComponentImage*)item_comp)->texture = tx;

			return true;
		}
		return false;
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
	uint32_t type;
};

class Inventory {
public:
	Inventory() {
		windows = {};
		active_windows = {};

		cursor_comp = new GComponentImage(glm::vec2(50.0f), glm::vec3(0.0f, 0.0f, 50.0f), 0);
	}

	class Window {
	public:
		Window() = default;
		Window(std::string name, ObjectDim dim, uint8_t height, std::vector<Slot*> slots, GWindow* gwin, uint8_t flag = 0) {
			this->name = name;
			this->dim = dim;
			this->height = height;
			this->slots = slots;
			this->gwin = gwin;
			this->alwaysOnTop = flag == 1 ? true : false;
		}

		Slot* AddSlotToWindow(Slot slot, uint64_t tex) {
			ObjectDim slotDim = slot.getDim();
			if (!dim.isRectInRect(slotDim)) return nullptr;
			Slot* nslot = new Slot{ slot.getItem(), slotDim.position, slotDim.width, slotDim.height, slot.getSlotType() };
			nslot->item_comp = new GComponentImage(glm::vec2(slot.getDim().width, slot.getDim().height), glm::vec3(slot.getDim().position.x, slot.getDim().position.y, 0.1f), 0);
			nslot->slot_comp = new GComponentImage(glm::vec2(slot.getDim().width, slot.getDim().height), glm::vec3(slot.getDim().position.x, slot.getDim().position.y, 0.2f), tex);
			slots.push_back(nslot);
			nslot->parent_win = gwin;
			gwin->AddComponent(nslot->item_comp);
			gwin->AddComponent(nslot->slot_comp);
			return nslot;
		}

		std::vector<Slot*> getAllSlots() {
			return slots;
		}

		GWindow* getGWindow() {
			return gwin;
		}

		std::string getWindowName() {
			return name;
		}

		uint8_t getWindowHeight() {
			return height;
		}

		void setWindowHeight(uint8_t height) {
			this->height = height;
		}

		ObjectDim getDim() {
			return dim;
		}

		void changeWindowPosition(int x, int y) {
			int offsetX, offsetY;
			/*offsetX = glm::distance(dim.position.x, (float)x);
			offsetY = glm::distance(dim.position.y, (float)y);*/
			offsetX = -dim.position.x + x;
			offsetY = -dim.position.y + y;
			dim.position.x = x;
			dim.position.y = y;
			for (auto& slot : slots) {
				glm::vec2 slotCurrentPosition = slot->getPosition();
				slot->changePosition(glm::vec2{ offsetX + slotCurrentPosition.x, offsetY + slotCurrentPosition.y });
			}
			gwin->ChangeComponentPosition(x, y);
		}

		bool onTop() {
			return alwaysOnTop;
		}

		int getSlotIndex(Slot* slot) {
			auto it = std::find(slots.begin(), slots.end(), slot);

			if (it != slots.end()) {
				return std::distance(slots.begin(), it);
			}

			return -1;
		}

	private:
		std::string name;
		ObjectDim dim;
		uint8_t height;
		std::vector<Slot*> slots;
		GWindow* gwin;
		bool alwaysOnTop;
	};

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
		auto wins = active_windows;

		auto svec = std::vector<Window*>(wins.rbegin(), wins.rend());
		for (auto& i : svec)
		{
			i->getGWindow()->UpdateDepth(base_depth);
			i->getGWindow()->UpdateZComp();
			i->getGWindow()->RenderText(pm);
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
			i->getGWindow()->UpdateDepth(base_depth);
			i->getGWindow()->UpdateZComp();
			i->getGWindow()->Render(pm);
			base_depth += 2.0f;
		}

		cursor_comp->pos.z = base_depth + 0.5f;
		cursor_comp->Render(pm);
	}

	void UpdateCursorPosition(glm::vec2 pos)
	{
		cursor_comp->pos.x = pos.x;
		cursor_comp->pos.y = pos.y;
	}

	bool setWindowHeight(Window* window, uint8_t height) {
		Window* win = windowExist(window);
		if (!win)
		{
			win = windowExist(window);
		}
		else if (!win) return false;
		win->setWindowHeight(height);
		sortVec();
	}

	bool ActivateWindow(Window* win) {
		Window* winExist = windowExist(win);
		Window* winIsActive = windowExistActive(win);
		if (winExist) {
			win->setWindowHeight(MAXINT64);
			active_windows.push_back(win);
			sortVec();

			return true;
		}
		return false;
	}

	bool DisableWindow(Window* win) {
		Window* winExist = windowExist(win);
		Window* winIsActive = windowExistActive(win);
		if (winExist && winIsActive) {
			active_windows.erase(std::remove(active_windows.begin(), active_windows.end(), win), active_windows.end());

			return true;
		}
		return false;
	}

	Window* AddWindow(std::string windowName, ObjectDim dim, uint8_t height, uint64_t tex, uint8_t flag = 0) {
		Window* win = new Window{ windowName, dim, height, {}, new GWindow{ dim.position, glm::vec2{ dim.width, dim.height }, tex }, flag };
		windows.push_back(win);
		return win;
	}

	Window* GetWindow(std::string windowName) {
		return windowExist(windowName);
	}

	std::vector<Window*> getActiveWindows() {
		return active_windows;
	}

	bool isWindowActive(Window* win) {
		Window* winExist = windowExistActive(win);
		if (winExist) return true;
		else return false;
	}

	bool swapItems(Slot* slot1, Slot* slot2) {
		if (!(slot1->getSlotType() & slot2->getSlotType())) return false;
		if (slot1->getItem()) {
			if (!(slot1->getItem()->getObjectType() & slot2->getSlotType())) return false;
		}
		if (slot2->getItem()) {
			if (!(slot2->getItem()->getObjectType() & slot1->getSlotType())) return false;
		}
		Item* itemTemp = slot1->getItem();
		slot1->changeItem(slot2->getItem());
		slot2->changeItem(itemTemp);
		return true;
	}

	Window* setPressedWindowOnTop(glm::vec2 position) {
		Window* windowExist = nullptr;
		for (auto& window : active_windows) {
			if (pointInRect(position, window->getDim())) {
				window->setWindowHeight(INT_MAX);
				windowExist = window;
				break;
			}
		}
		if (windowExist) {
			sortVec();
			return windowExist;
		}
		return nullptr;
	}

	Slot* getSlot(glm::vec2 position, Window*& win) {
		uint8_t windowsAmount = getActiveWindows().size();
		uint8_t height = 0;
		Slot* potentialSlot = nullptr;
		int i = 0;
		for (auto& window : getActiveWindows()) {
			if (potentialSlot) break;
			i++;
			if (pointInRect(position, window->getDim())) {
				for (auto& slot : window->getAllSlots()) {
					if (pointInRect(position, slot->getDim())) {
						height = i - 1;
						win = window;
						potentialSlot = slot;
						break;
					}
				}
			}
		}
		for (int x = 0; x < i - 1; x++) {
			if (pointInRect(position, active_windows.at(x)->getDim())) { 
				potentialSlot = nullptr; 
				win = nullptr;
				break; 
			};
		}
		return potentialSlot;
	}

	Slot* getSlot(glm::vec2 position) {
		uint8_t windowsAmount = getActiveWindows().size();
		uint8_t height = 0;
		Slot* potentialSlot = nullptr;
		int i = 0;
		for (auto& window : getActiveWindows()) {
			if (potentialSlot) break;
			i++;
			if (pointInRect(position, window->getDim())) {
				for (auto& slot : window->getAllSlots()) {
					if (pointInRect(position, slot->getDim())) {
						height = i - 1;
						potentialSlot = slot;
						break;
					}
				}
			}
		}
		for (int x = 0; x < i - 1; x++) {
			if (pointInRect(position, active_windows.at(x)->getDim())) {
				potentialSlot = nullptr;
				break;
			};
		}
		return potentialSlot;
	}

	bool isGuiClicked(glm::vec2 position) {
		bool guiClicked = false;
		for (auto& window : getActiveWindows()) {
			if (pointInRect(position, window->getDim())) return true;
		}
		return false;
	}

private:
	bool pointInRect(glm::vec2 position, ObjectDim dim) {
		return position.x >= dim.position.x && position.x <= dim.position.x + dim.width
			&& position.y >= dim.position.y && position.y <= dim.position.y + dim.height;
	}

	Window* windowExist(std::string windowName) {
		for (auto& window : windows) {
			if (window->getWindowName() == windowName) {
				return window;
			}
		}
		return nullptr;
	}

	Window* windowExist(Window* win) {
		for (auto& window : windows) {
			if (window == win) {
				return window;
			}
		}
		return nullptr;
	}

	Window* windowExistActive(Window* win) {
		for (auto& window : active_windows) {
			if (window == win) {
				return window;
			}
		}
		return nullptr;
	}

	void sortVec() {
		auto& windows = active_windows;
		std::sort(windows.begin(), windows.end(), [](Window* a, Window* b) {
			return a->getWindowHeight() > b->getWindowHeight();
			});
		std::sort(windows.begin(), windows.end(), [](Window* a, Window* b) {
			return a->onTop() > b->onTop();
			});
		int height = 0, windowsAmount = active_windows.size();
		for (auto& win : active_windows) {
			if (win->getWindowHeight() != height) win->setWindowHeight(windowsAmount - height);
			height++;
		}
	}

	std::vector<Window*> windows;
	std::vector<Window*> active_windows;
	//std::unordered_map<bool, std::vector<Window*>> windowSlots;

	GComponentImage* cursor_comp;
	Item* cursor_hold;
};

class ItemLoader {
public:
	ItemLoader() :
	generatorInstance(Generator::getInstance())
	{
		tierItemMap[TIER_0] = {};
		tierItemMap[TIER_1] = {};
		tierItemMap[TIER_2] = {};
		tierItemMap[TIER_3] = {};
	}

	void loadItem(Item& item) {
		uint8_t type = item.getObjectType();
		assert(!(item.getItemTier() == TIER_ALL));
		if (type & ARMOR) {
			auto armor = new ArmorItem{ item.getItemName(), item.getItemTexture(), item.getObjectType(), (ArmorItem::ObjectStatistic*)item.getObjectStatistic(), item.getItemPrice(), item.getItemTier() };
			itemMap[item.getItemName()] = armor;
			tierItemMap.at(item.getItemTier()).push_back(armor);
		}
		if (type & WEAPON) {
			auto weapon = new WeaponItem{ item.getItemName(), item.getItemTexture(), item.getObjectType(), (WeaponItem::ObjectStatistic*)item.getObjectStatistic(), item.getItemPrice(), item.getItemTier() };
			itemMap[item.getItemName()] = weapon;
			tierItemMap.at(item.getItemTier()).push_back(weapon);
		}
		else {
			auto nitem = new Item{ item.getItemName(), item.getItemTexture(), item.getObjectType(), item.getObjectStatistic(), item.getItemPrice(), item.getItemTier() };
			itemMap[item.getItemName()] = nitem;
			tierItemMap.at(item.getItemTier()).push_back(nitem);
		}
	}
	void loadItem(Item* item) {
		assert(!(item->getItemTier() == TIER_ALL));
		itemMap[item->getItemName()] = item;
		tierItemMap.at(item->getItemTier()).push_back(item);
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

	Item* getRandomItem(uint8_t tier = TIER_ALL) {
		int idx = 0;
		if (tier == TIER_ALL) {
			uint8_t availableTiers[] = { TIER_1, TIER_2, TIER_3 };
			tier = availableTiers[rand() % 3];
		}
		idx = rand() % tierItemMap.at(tier).size();
		auto it = tierItemMap.at(tier).at(idx);

		return it;
	}

	template<typename T>
	Item* getRandomSpecificItem(uint8_t tier = TIER_ALL) {
		int idx = 0;
		if (tier == TIER_ALL) {
			uint8_t availableTiers[] = { TIER_1, TIER_2, TIER_3 };
			tier = availableTiers[rand() % 3];
		}
		Item* it = nullptr;
		while (!it) {
			idx = rand() % tierItemMap.at(tier).size();
			if constexpr (std::is_same_v<decltype(it), T>) {
				it = tierItemMap.at(tier).at(idx);
			}
		}
		return it;
	}

	void loadSet(Entity::EquipedItems set) {
		sets.push_back(set);
	}

	//void loadSet(std::string setName, Entity::EquipedItems& set) {
	//	sets[setName] = set;
	//}
	
	Entity::EquipedItems getRandomSet() {
		int size = sets.size();
		if (!size) return Entity::EquipedItems{};
		return sets.at(rand() % size);
	}


	//FOR NOW
	float calculateEntityPrice(Entity* entity) {
		if (!entity) return 0;
		float sum = BASE_ENTITY_VALUE;
		auto items = entity->getEquipedItems();
		if (items.helmet) sum += items.helmet->getItemPrice();
		if (items.Chestplate) sum += items.Chestplate->getItemPrice();
		if (items.Legs) sum += items.Legs->getItemPrice();
		if (items.Boots) sum += items.Boots->getItemPrice();
		if (items.shield) sum += items.shield->getItemPrice();
		if (items.weapon) sum += items.weapon->getItemPrice();

		auto stats = entity->getStats();
		sum += MAX_MELEE_STAT_PRICE * getPercentage(20, 5, stats->melee)
			+ MAX_RANGED_STAT_PRICE * getPercentage(20, 5, stats->ranged)
			+ MAX_BRAVERY_STAT_PRICE * getPercentage(200, 100, stats->bravery)
			+ MAX_DEFENCE_STAT_PRICE * getPercentage(10, 2, stats->defense)
			+ MAX_HP_STAT_PRICE * getPercentage(30, 10, stats->hp);
		return sum;
	}

	//FOR NOW
	Entity* generateRandomEntity(uint8_t factionID) {
		Stats entityStats = {
			generatorInstance.getRandomNumber(5,20),
			generatorInstance.getRandomNumber(2,10),
			generatorInstance.getRandomNumber(5,20),
			generatorInstance.getRandomNumber(100,200),
			4.1f + generatorInstance.getRandomNumber(0,2),
			generatorInstance.getRandomNumber(10,30) };
		Entity::EquipedItems items = getRandomSet();
		Entity* entity = new Entity(getRandomFactionName(factionID), 0, entityStats, items);
		uint32_t index = GetEntityRandomTextureIndex(factionID);
		entity->SetEntityTextureIndex(GetEntityTextureFromIndex(index, factionID), index);
		return entity;
	}

private:
	//FOR NOW
	float getPercentage(float min, float max, float value) {
		return ((value - min) / (max - min));
	}

	std::unordered_map<std::string, Item*> itemMap;
	std::unordered_map<uint8_t, std::vector<Item*>> tierItemMap;
	//std::unordered_map<std::string, Entity::EquipedItems> sets;
	std::vector<Entity::EquipedItems> sets;
	Generator& generatorInstance;

};

struct GUI_DraggedWindow {
	bool wasPressed = false;
	Inventory::Window* win;
	glm::vec2 offset;
};

struct GUI_DraggedItem {
	bool wasPressed = false;
	Item* item;
	Inventory::Window* win;
	Slot* previousSlot;
};

struct DraggedObj {
	GUI_DraggedWindow draggedWindow = {};
	GUI_DraggedItem draggedItem = {};
};

void ActivateWindow(void* v1, void* v2, Inventory* inv, Inventory::Window* win) {
	if (inv) inv->ActivateWindow(win);
}

void DisableWindow(void* v1, void* v2, Inventory* inv, Inventory::Window* win) {
	if (inv) inv->DisableWindow(win);
}

void SetDraggedWindow(void* v1, void* v2, GUI_DraggedWindow* guiDW, Inventory::Window* win) {
	guiDW->win = win;
}

void SetDraggedItem(void* v1, void* v2, GUI_DraggedItem* guiDI, Item* item) {
	guiDI->item = item;
}

void setInventory(void* v1, void* v2, int* money, Inventory* inv, Inventory::Window* win) {
	std::string labelName = "playerMoney";
	auto label = GetNamedComponent(labelName.c_str());
	label->SetText(std::to_string(*money).c_str());
}

void setShopItemsRotation(void* v1, void* v2, Building** building, Inventory* inv, Inventory::Window* win) {
	if (inv && (*building)) {
		if (inv->ActivateWindow(win)) {
			auto items = (*building)->getItemsRotation();
			auto slots = win->getAllSlots();
			int size = items.size() < slots.size() ? items.size() : slots.size();
			std::string labelName;
			for (int idx = 0; idx < slots.size(); idx++) {
				slots.at(idx)->changeItem(nullptr);
				labelName = "shopItem" + std::to_string(idx);
				auto label = GetNamedComponent(labelName.c_str());
				label->SetText(std::string("0").c_str());
			}
			for (int idx = 0; idx < size; idx++) {
				Item* nItem = items.at(idx);
				slots.at(idx)->changeItem(nItem);
				labelName = "shopItem" + std::to_string(idx);
				auto label = GetNamedComponent(labelName.c_str());
				label->SetText(std::to_string(nItem->getItemPrice()).c_str());
			}
		}
	}
}

void setShopEntityRotation(void* v1, void* v2, Building** building, Inventory* inv, Inventory::Window* win) {
	if (inv && (*building)) {
		if (inv->ActivateWindow(win)) {
			auto entities = (*building)->getEntityItemsRotation();
			auto slots = win->getAllSlots();
			std::string labelName;
			for (int idx = entities.size(); idx < slots.size(); idx++) {
				slots.at(idx)->changeItem(nullptr);
				labelName = "shopEntityItem" + std::to_string(idx);
				auto label = GetNamedComponent(labelName.c_str());
				label->SetText(std::string("0").c_str());
			}
			for (int idx = 0; idx < entities.size(); idx++) {
				EntityItem* nItem = (EntityItem*)entities.at(idx);
				slots.at(idx)->changeItem(nItem);
				labelName = "shopEntityItem" + std::to_string(idx);
				auto label = GetNamedComponent(labelName.c_str());
				label->SetText(std::to_string(nItem->getItemPrice()).c_str());
			}
		}
	}
}



//PRAWDOPODOBNIE ZLE DZIALA
void setParty(void* v1, void* v2, Squad** squad, Inventory* inv, Inventory::Window* win) {
	if (inv && (*squad)) {
		if (inv->ActivateWindow(win)) {
			auto squadComp = (*squad)->getSquadComp();
			auto slots = win->getAllSlots();
			for (int idx = 0; idx < slots.size(); idx++) {
				delete slots.at(idx)->getItem();
				slots.at(idx)->changeItem(nullptr);
			}
			for (int idx = 0; idx < squadComp->size; idx++) {
				EntityItem* eitm = new EntityItem(squadComp->entities[idx]);
				slots.at(idx)->changeItem(eitm);
			}
		}
	}
}


void getCharacterInventory_EI(void* v1, void* v2, EntityItem** entityItem, Inventory* inv, Inventory::Window* win) {
	if (inv && (*entityItem)) {
		if (inv->ActivateWindow(win)) {
			auto slots = win->getAllSlots();
			Entity* ent = (*entityItem)->getEntity();
			auto entityItems = ent->getEquipedItems();
			slots.at(0)->changeItem(entityItems.helmet);
			slots.at(1)->changeItem(entityItems.Chestplate);
			slots.at(2)->changeItem(entityItems.shield);
			slots.at(3)->changeItem(entityItems.weapon);
			slots.at(4)->changeItem(entityItems.Legs);
			slots.at(5)->changeItem(entityItems.Boots);

			std::string componentName = "Vname";
			GComponentSlider* component;
			GComponent* cmp;
			cmp = GetNamedComponent(componentName.c_str());
			cmp->SetText(ent->getName().c_str());

			componentName = "Vhp";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));
			component->value = 0.2f;//ent->getStats()->hp / ENTITY_MAX_HEALTH;

			componentName = "Vstamina";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));

			componentName = "Vbravery";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));

			componentName = "Vmelee";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));

			componentName = "Vranged";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));

			componentName = "Vdefense";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));
		}
	}
}

void getCharacterInventory_E(void* v1, void* v2, Entity** entity, Inventory* inv, Inventory::Window* win) {
	if (inv && (*entity)) {
		if (inv->ActivateWindow(win)) {
			auto slots = win->getAllSlots();
			auto entityItems = (*entity)->getEquipedItems();
			slots.at(0)->changeItem(entityItems.helmet);
			slots.at(1)->changeItem(entityItems.Chestplate);
			slots.at(2)->changeItem(entityItems.shield);
			slots.at(3)->changeItem(entityItems.weapon);
			slots.at(4)->changeItem(entityItems.Legs);
			slots.at(5)->changeItem(entityItems.Boots);

			std::string componentName = "Vname";
			GComponentSlider* component;
			GComponent* comp;
			comp = GetNamedComponent(componentName.c_str());
			comp->SetText((*entity)->getName().c_str());

			componentName = "Vhp";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));
			component->value = (*entity)->getHp() / (*entity)->getStats()->hp;

			componentName = "Vstamina";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));

			componentName = "Vbravery";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));

			componentName = "Vmelee";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));

			componentName = "Vranged";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));

			componentName = "Vdefense";
			component = ((GComponentSlider*)GetNamedComponent(componentName.c_str()));
		}
	}
}

void setSquadCompSize(void* v1, void* v2, uint8_t* squadSize) {
	std::string text, componentName = "squad_count";
	GComponent* component = GetNamedComponent(componentName.c_str());
	text = std::to_string((int)(*squadSize)) + "/" + std::to_string((int)SQUAD_MAX_SIZE);
	component->SetText(text.c_str());
}
