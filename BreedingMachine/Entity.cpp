#include "Entity.h"
#include "glm/gtc/matrix_transform.hpp"

using namespace glm;

#define MAPCLAMP(A) clamp(A, vec2(-512.0f), vec2(512.0f-74.0f))

std::unordered_map<uint8_t, std::vector<std::string>> factionNames;

EntityCombatCloseRange::EntityCombatCloseRange(Entity* self)
{
    this->self = self;
}

int EntityCombatCloseRange::MoveEntity(void* battleContext)
{
    float tileSize = 64.0f;
    Entity* enemy_close = 0;
    float advance_factor = -INFINITY;

    vec2 tilePos = floor(self->getPosition() / vec2(tileSize));

    Squad::SquadComp* bc = ((BattleData*)battleContext)->s1->getSquadComp();
    for (int i = 0; i < bc->size; i++)
    {
        Entity* e = bc->entities[i];
        vec2 enemyPos = floor(e->getPosition() / vec2(tileSize));
        if (advance_factor < AiGetEntityAdvanceFactor(e))
        {
            enemy_close = e;
            advance_factor = AiGetEntityAdvanceFactor(e);
        }
    }

    if (distance(self->getPosition(), enemy_close->getPosition()) > self->getStats()->stamina * 60.0f)
    {
        vec2 enemy_direction = normalize(self->getPosition() - enemy_close->getPosition());
        vec2 move_position = self->getPosition() - (enemy_direction * self->getStats()->stamina * 64.0f);
        self->travel = MAPCLAMP(move_position);
    }
    else
    {
        vec2 move_position = enemy_close->getPosition();
        self->travel = MAPCLAMP(move_position);
    }
    return true;
}

int EntityCombatCloseRange::NextState()
{
    if (self->getHp() <= 0.0f)
    {
        self->changeEntityState(new EntityCombatDead(self));
        return 1;
    }

    if (self->GetBravery() <= 0.0f)
    {
        self->changeEntityState(new EntityCombatEscape(self));
        return 1;
    }

    Stats* s = self->getStats();
    if (self->getHp() / s->hp <= 0.2f)
    {
        if (s->ranged != 0.0f)
        {
            self->changeEntityState(new EntityCombatLongRange(self));
            return 1;
        }
    }

    return 0;
}

int EntityCombatCloseRange::CanMoveEntity()
{
    return 1;
}

int EntityCombatCloseRange::AttackEntity(void* battleContext, AnimationControllers* ac)
{
    float tileSize = 64.0f;
    Entity* enemy_close = 0;
    float attack_factor = -INFINITY;

    vec2 tilePos = floor(self->getPosition() / vec2(tileSize));

    Squad::SquadComp* bc = ((BattleData*)battleContext)->s1->getSquadComp();
    for (int i = 0; i < bc->size; i++)
    {
        Entity* e = bc->entities[i];
        vec2 enemyPos = floor(e->getPosition() / vec2(tileSize));
        if (attack_factor < AiGetEntityAttackFactor(e) && distance(tilePos, enemyPos) * 64.0f <= 64.0f * sqrt(2.0f))
        {
            enemy_close = e;
            attack_factor = AiGetEntityAttackFactor(e);
        }
    }

    if (enemy_close == 0)
        return false;

    float new_hp = min(0.0f, enemy_close->getHp() - AiGetAttackAfterArmor(enemy_close, AiGetUnitAttack(self)));
    enemy_close->SetHp(new_hp);
    ac->mac.SetAnimation(LoadTextureFromFile("Data\\purple.png"), enemy_close->getPosition());

    return true;
}

int EntityCombatCloseRange::EntityCanBattle()
{
    return true;
}

EntityCombatLongRange::EntityCombatLongRange(Entity* self)
{
    this->self = self;
}
//scary factor = hp * ArmorReduction(atk) * stamina * is_mele
int EntityCombatLongRange::MoveEntity(void* battleContext)
{
    float ai_range = 5.0f;
    float tileSize = 64.0f;
    Entity* enemy_close = 0;
    float advance_factor = -INFINITY;

    vec2 tilePos = floor(self->getPosition() / vec2(tileSize));

    std::vector<Entity*> nearEntity = std::vector<Entity*>();

    Squad::SquadComp* bc = ((BattleData*)battleContext)->s1->getSquadComp();
    for (int i = 0; i < bc->size; i++)
    {
        Entity* e = bc->entities[i];
        vec2 enemyPos = floor(e->getPosition() / vec2(tileSize));
        if (advance_factor < AiGetEntityAdvanceFactor(e))
        {
            enemy_close = e;
            advance_factor = AiGetEntityAdvanceFactor(e);
        }

        if (distance(tilePos, enemyPos) * 64.0f <= 64.0f * sqrt(2.0f))
            nearEntity.push_back(e);
    }

    if (nearEntity.size() != 0)
    {
        Entity* escape = 0;
        float scary_factor = -1;

        for (auto& i : nearEntity)
        {
            if (AiGetEntityScaryFactor(i) > scary_factor)
            {
                escape = i;
                scary_factor = AiGetEntityScaryFactor(i);
            }
        }

        float escape_distance = min(ai_range, self->getStats()->stamina);

        vec2 enemy_direction = normalize(self->getPosition() - escape->getPosition());
        vec2 escape_position = MAPCLAMP(self->getPosition() + (enemy_direction * escape_distance * 64.0f));
        if (distance(escape->getPosition(), self->getPosition()) < 0.8f * escape_distance)
        {
            escape_position = MAPCLAMP(self->getPosition() - (enemy_direction * escape_distance * 64.0f));
        }
        self->travel = escape_position;

        return true;
    }

    if (distance(self->getPosition(), enemy_close->getPosition()) > self->getStats()->stamina * 64.0f)
    {
        vec2 enemy_direction = normalize(self->getPosition() - enemy_close->getPosition());
        vec2 move_position = MAPCLAMP(self->getPosition() - (enemy_direction * min(ai_range, self->getStats()->stamina) * 64.0f));
        self->travel = MAPCLAMP(move_position);
        return true;
    }

    return false;
}

int EntityCombatLongRange::NextState()
{
    if (self->getHp() <= 0.0f)
    {
        self->changeEntityState(new EntityCombatDead(self));
        return 1;
    }

    if (self->GetBravery() <= 0.0f)
    {
        self->changeEntityState(new EntityCombatEscape(self));
        return 1;
    }

    return 0;
}

int EntityCombatLongRange::CanMoveEntity()
{
    return 1;
}

int EntityCombatLongRange::AttackEntity(void* battleContext, AnimationControllers* ac)
{
    float ai_range = 5.0f;
    float tileSize = 64.0f;
    Entity* enemy_close = 0;
    float attack_factor = -INFINITY;

    vec2 tilePos = floor(self->getPosition() / vec2(tileSize));

    Squad::SquadComp* bc = ((BattleData*)battleContext)->s1->getSquadComp();
    for (int i = 0; i < bc->size; i++)
    {
        Entity* e = bc->entities[i];
        vec2 enemyPos = floor(e->getPosition() / vec2(tileSize));
        if (attack_factor < AiGetEntityAttackFactor(e) && distance(tilePos, enemyPos) * 64.0f <= 64.0f * ai_range)
        {
            enemy_close = e;
            attack_factor = AiGetEntityAttackFactor(e);
        }
    }

    if (enemy_close == 0)
        return false;

    float new_hp = max(0.0f, enemy_close->getHp() - AiGetAttackAfterArmor(enemy_close, AiGetUnitAttack(self)));
    enemy_close->SetHp(new_hp);
    ac->rac.SetAnimation(LoadTextureFromFile("Data\\purple.png"), self->getPosition(), enemy_close->getPosition());
    return true;
}

int EntityCombatLongRange::EntityCanBattle()
{
    return true;
}

EntityCombatDead::EntityCombatDead(Entity* self)
{
    this->self = self;
}

int EntityCombatDead::MoveEntity(void* battleContext)
{
    return false;
}

int EntityCombatDead::NextState()
{
    return 0;
}

int EntityCombatDead::CanMoveEntity()
{
    return 0;
}

int EntityCombatDead::AttackEntity(void* battleContext, AnimationControllers* ac)
{
    return 0;
}

int EntityCombatDead::EntityCanBattle()
{
    return 0;
}

EntityCombatEscape::EntityCombatEscape(Entity* self)
{
    this->self = self;
}

int EntityCombatEscape::MoveEntity(void* battleContext)
{
    std::vector<vec2> corners = { vec2(-512.0f, -512.0f), vec2(512.0f, -512.0f), vec2(-512.0f, 512.0f), vec2(512.0f, 512.0f)};

    vec2 close_corner = vec2(-512.0f, -512.0f);
    float d = INFINITY;

    for (int i = 0; i < corners.size(); i++)
    {
        if (distance(self->getPosition(), corners.at(i)) < d)
        {
            d = distance(self->getPosition(), corners.at(i));
            close_corner = corners.at(i);
        }
    }

    if (d < 64.0f)
    {
        // pufff i nie ma
        return false;
    }

    self->travel = MAPCLAMP((self->getPosition() + (normalize(self->getPosition() - close_corner) * self->getStats()->stamina * 64.0f)));
    return true;                                                                                 
}

int EntityCombatEscape::NextState()
{
    return 0;
}

int EntityCombatEscape::CanMoveEntity()
{
    return 1;
}

int EntityCombatEscape::AttackEntity(void* battleContext, AnimationControllers* ac)
{
    return 0;
}

int EntityCombatEscape::EntityCanBattle()
{
    return 0;
}

EntityCombatStand::EntityCombatStand(Entity* self)
{
    this->self = self;
}

int EntityCombatStand::MoveEntity(void* battleContext)
{
    return false;
}

int EntityCombatStand::NextState()
{
    return 0;
}

int EntityCombatStand::CanMoveEntity()
{
    return 0;
}

int EntityCombatStand::AttackEntity(void* battleContext, AnimationControllers* ac)
{
    return 0;
}

int EntityCombatStand::EntityCanBattle()
{
    return 0;
}

Squad::Squad(uint64_t squadID, uint8_t factionID, glm::vec2 position, ItemLoader* il)
{
    squadComp = new SquadComp{};
    //squadComp->entities[0] = new Entity{};
    //squadComp->size = 1;
    squadComp->size = (rand() % (SQUAD_MAX_SIZE - 1)) + 1;
    for (int i = 0; i < squadComp->size; i++) {
        squadComp->entities[i] = il->generateRandomEntity(factionID);
        uint64_t index = GetEntityRandomTextureIndex(factionID);
        squadComp->entities[i]->SetEntityTextureIndex(GetEntityTextureFromIndex(index, factionID), index);
    }
    this->squadID = squadID;
    this->position = position;
    this->factionID = factionID;
    this->squadState = STAND;
}

std::string getRandomFactionName(uint8_t factionID)
{
    int size = factionNames[factionID].size();
    std::string name;
    if (!size) return "MAMBAJAMBA FLORCZYK KARAMBA";
    return factionNames[factionID].at(rand() % size);
}

bool loadNames(const char* path, uint8_t factionID)
{
    std::fstream file;

    file.open(path);

    if (!file) return false;

    int size;
    file >> size;

    std::string name;
    int idx;
    for (idx = 1; idx < size + 1; idx++) {
        file >> name;
        factionNames[factionID].push_back(name);
    }
    return true;
}

glm::vec2 lerp2f(glm::vec2 a, glm::vec2 b, float t)
{
    return a + (b - a) * t;
}

float AiGetEntityScaryFactor(Entity* e)
{
    Stats* s = e->getStats();

    float mele_mul = 1.0f;
    float atk = s->ranged;
    if (s->ranged <= s->melee)
    {
        mele_mul = 1.2f;
        atk = s->melee;
    }

    return sqrt(e->getHp()) * atk * s->stamina * mele_mul;
}

float AiGetEntityAdvanceFactor(Entity* e)
{
    float atk = max(e->getStats()->melee, e->getStats()->ranged);
    return pow((1.0 - e->getHp() / e->getStats()->hp), 1.5f) * atk;
}

float AiGetEntityAttackFactor(Entity* e)
{
    float atk = max(e->getStats()->melee, e->getStats()->ranged);
    return pow((1.0 - e->getHp() / e->getStats()->hp), 1.8f) * atk;
}

float AiGetUnitArmor(Entity* e)
{
    float armor = e->getStats()->defense;
    Entity::EquipedItems items = e->getEquipedItems();

    if (items.Boots != nullptr)
        armor += items.Boots->getObjectStatistic()->armor;
    if (items.Chestplate != nullptr)
        armor += items.Chestplate->getObjectStatistic()->armor;
    if (items.helmet != nullptr)
        armor += items.helmet->getObjectStatistic()->armor;
    if (items.Legs != nullptr)
        armor += items.Legs->getObjectStatistic()->armor;
    if (items.shield != nullptr)
        armor += items.shield->getObjectStatistic()->armor;

    return armor;
}

float AiGetAttackAfterArmor(Entity* e, float atk)
{
    return 1.0f / pow(AiGetUnitArmor(e), 0.1f) * atk;
}

float AiGetUnitAttack(Entity* e)
{
    float atk = max(e->getStats()->melee, e->getStats()->ranged);
    Entity::EquipedItems items = e->getEquipedItems();

    if (items.weapon != nullptr)
        atk += items.weapon->getObjectStatistic()->damage;

    return atk;
}

float AiGetUnitBraveryDamage(Entity* e)
{
    float batk = AiGetUnitAttack(e) * 0.5f;
    return batk;
}

void AiGainUnitStdBravery(Entity* e)
{
    float pBravGain = 0.08f;
    if (e->state->EntityCanBattle() == true)
        e->SetBravery(e->GetBravery() + e->getStats()->bravery * pBravGain);
}

MeleeAnimationController::MeleeAnimationController(rasticore::RastiCoreRender* r, rasticore::ModelCreationDetails* mcd)
{
    this->r = r;
    t = 1.0f;
    r->newModel(ANIMATION_MELEE_ATACK, mcd->vb, mcd->p, mcd->v_cnt, mcd->rm, rasticore::Texture2DBindless(), 2);
    r->newObject(ANIMATION_MELEE_ATACK, glm::mat4(1.0f));
    rasticore::Program p = r->getModel(ANIMATION_MELEE_ATACK)->std_prgm;
    p.use();

    lOpacity = glGetUniformLocation(p.id, "opacity");
}

void MeleeAnimationController::Update(float dt)
{
    if (t <= 1.0f)
        t += dt;
    else
        t = 1.0f;
}

void MeleeAnimationController::SetAnimation(uint64_t texture, glm::vec2 p)
{
    position = p;
    tex = texture;
    r->getModel(ANIMATION_MELEE_ATACK)->std_texture2d.handle = texture;
    t = 0.0f;
}

void MeleeAnimationController::Render()
{
    if (t >= 1.0f)
        return;

    glUniform1f(lOpacity, 1.0 - t);
    r->BindActiveModel(ANIMATION_MELEE_ATACK);
    r->SetObjectMatrix(0, glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 2.15f)));
    r->RenderSelectedModel(ANIMATION_MELEE_ATACK);
    glUniform1f(lOpacity, 1.0f);
}

RangedAnimationController::RangedAnimationController(rasticore::RastiCoreRender* r, rasticore::ModelCreationDetails* mcd)
{
    this->r = r;
    t = 1.0f;
    r->newModel(ANIMATION_RANGE_ATTACK, mcd->vb, mcd->p, mcd->v_cnt, mcd->rm, rasticore::Texture2DBindless(), 2);
    r->newObject(ANIMATION_RANGE_ATTACK, glm::mat4(1.0f));
}

void RangedAnimationController::SetAnimation(uint64_t texture, glm::vec2 p0, glm::vec2 p1)
{
    pos0 = p0;
    pos1 = p1;
    tex = texture;
    r->getModel(ANIMATION_RANGE_ATTACK)->std_texture2d.handle = texture;
    t = 0.0f;
}

void RangedAnimationController::Update(float dt)
{
    if (t <= 1.0f)
        t += dt;
    else
        t = 1.0f;
}

void RangedAnimationController::Render()
{
    if (t >= 1.0f)
        return;

    r->BindActiveModel(ANIMATION_RANGE_ATTACK);
    r->SetObjectMatrix(0, glm::translate(glm::mat4(1.0f), glm::vec3(lerp2f(pos0, pos1, t), 2.15f)));
    r->RenderSelectedModel(ANIMATION_RANGE_ATTACK);
}
