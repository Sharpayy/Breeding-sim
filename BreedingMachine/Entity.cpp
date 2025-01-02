#include "Entity.h"
#include "glm/gtc/matrix_transform.hpp"

using namespace glm;

EntityCombatCloseRange::EntityCombatCloseRange(Entity* self)
{
    this->self = self;
}

int EntityCombatCloseRange::MoveEntity(void* battleContext)
{
    float tileSize = 64.0f;
    Entity* enemy_close = 0;
    float dist_close = INFINITY;

    vec2 tilePos = floor(self->getPosition() / vec2(tileSize));

    Squad::SquadComp* bc = ((BattleData*)battleContext)->s1->getSquadComp();
    for (int i = 0; i < bc->size; i++)
    {
        Entity* e = bc->entities[i];
        vec2 enemyPos = floor(e->getPosition() / vec2(tileSize));
        if (dist_close > distance(tilePos, enemyPos))
        {
            enemy_close = e;
            dist_close = distance(tilePos, enemyPos);
        }
    }

    self->travel = enemy_close->getPosition();
    return true;
}

int EntityCombatCloseRange::NextState()
{
    if (self->getHp() <= 0.0f)
    {
        self->changeEntityState(new EntityCombatDead(self));
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

EntityCombatLongRange::EntityCombatLongRange(Entity* self)
{
    this->self = self;
}

int EntityCombatLongRange::MoveEntity(void* battleContext)
{
    float tileSize = 64.0f;
    Entity* enemy_close = 0;
    float dist_close = INFINITY;

    vec2 tilePos = floor(self->getPosition() / vec2(tileSize));

    Squad::SquadComp* bc = ((BattleData*)battleContext)->s1->getSquadComp();
    for (int i = 0; i < bc->size; i++)
    {
        Entity* e = bc->entities[i];
        vec2 enemyPos = floor(e->getPosition() / vec2(tileSize));
        if (dist_close > distance(tilePos, enemyPos))
        {
            enemy_close = e;
            dist_close = distance(tilePos, enemyPos);
        }
    }

    self->travel = enemy_close->getPosition();
    return true;
}

int EntityCombatLongRange::NextState()
{
    if (self->getHp() <= 0.0f)
    {
        self->changeEntityState(new EntityCombatDead(self));
        return 1;
    }

    return 0;
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

EntityCombatEscape::EntityCombatEscape(Entity* self)
{
    this->self = self;
}

int EntityCombatEscape::MoveEntity(void* battleContext)
{
    float tileSize = 64.0f;
    Entity* enemy_close = 0;
    float dist_close = INFINITY;

    vec2 tilePos = floor(self->getPosition() / vec2(tileSize));

    Squad::SquadComp* bc = ((BattleData*)battleContext)->s2->getSquadComp();
    for (int i = 0; i < bc->size; i++)
    {
        Entity* e = bc->entities[i];
        vec2 enemyPos = floor(e->getPosition() / vec2(tileSize));
        if (dist_close > distance(tilePos, enemyPos))
        {
            enemy_close = e;
            dist_close = distance(tilePos, enemyPos);
        }
    }

    return true;
}

int EntityCombatEscape::NextState()
{
    return 0;
}

EntityCombatStand::EntityCombatStand(Entity* self)
{
    this->self = self;
}

int EntityCombatStand::MoveEntity(void* battleContext)
{
    float tileSize = 64.0f;
    Entity* enemy_close = 0;
    float dist_close = INFINITY;

    vec2 tilePos = floor(self->getPosition() / vec2(tileSize));

    Squad::SquadComp* bc = ((BattleData*)battleContext)->s2->getSquadComp();
    for (int i = 0; i < bc->size; i++)
    {
        Entity* e = bc->entities[i];
        vec2 enemyPos = floor(e->getPosition() / vec2(tileSize));
        if (dist_close > distance(tilePos, enemyPos))
        {
            enemy_close = e;
            dist_close = distance(tilePos, enemyPos);
        }
    }

    return true;
}

int EntityCombatStand::NextState()
{
    return 0;
}

