#ifndef COLLISION_H
#define COLLISION_H

#include "objects.h"

bool checkPlayerFloorCollision(Collider *p, Floor *f, Camera *c)
{
    if (p->x < f->x + f->width + c->x &&
        p->x + p->width > f->x + c->x &&
        p->y < f->y + f->height + c->y &&
        p->y + p->height > f->y + c->y)
        return true;
    else
        return false;
}

bool checkPlayerEnemyCollision(Collider *p, Enemy *e, Camera *c)
{
    if (p->x < e->x + e->width + c->x &&
        p->x + p->width > e->x + c->x &&
        p->y < e->y + e->height + c->y &&
        p->y + p->height > e->y + c->y)
        return true;
    else
        return false;
}

bool checkCollision(Collider *p, Floor *f, Camera *c)
{
    if (p->x + c->x < f->x + f->width + c->x &&
        p->x + p->width + c->x > f->x + c->x &&
        p->y + c->y < f->y + f->height + c->y &&
        p->y + p->height + c->y > f->y + c->y)
        return true;
    else
        return false;
}

#endif