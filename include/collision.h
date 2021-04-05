#ifndef COLLISION_H
#define COLLISION_H

#include "objects.h"

bool checkPlayerFloorCollision(Collider *p, Floor *f)
{
    if (p->x < f->x + f->width &&
        p->x + p->width > f->x  &&
        p->y < f->y + f->height  &&
        p->y + p->height > f->y)
        return true;
    else
        return false;
}

bool checkPlayerEnemyCollision(Collider *p, Enemy *e)
{
    if (p->x < e->x + e->width &&
        p->x + p->width > e->x &&
        p->y < e->y + e->height &&
        p->y + p->height > e->y)
        return true;
    else
        return false;
}

bool checkPlayerCollision(Collider *p, Collider *e)
{
    if (p->x < e->x + e->width &&
        p->x + p->width > e->x &&
        p->y < e->y + e->height &&
        p->y + p->height > e->y)
        return true;
    else
        return false;
}

bool checkCollision(Collider *p, Floor *f)
{
    if (p->x < f->x + f->width &&
        p->x + p->width > f->x &&
        p->y < f->y + f->height &&
        p->y + p->height > f->y)
        return true;
    else
        return false;
}

#endif