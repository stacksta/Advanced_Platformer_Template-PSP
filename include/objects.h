#ifndef OBJECTS_H
#define OBJECTS_H

typedef struct Collider
{
    float x, y;
    float width, height;
} Collider;

typedef struct Player
{
    float x, y;
    float width, height;
    float speed;
    float jumpHeight;
    int health;
    int maxHealth;
    bool onFloor;
    bool isMoving;
    bool isJump;
    bool isRight;
    bool isAttack;
} Player;

typedef struct Enemy
{
    float x, y;
    float width, height;
    float speed;
    bool onFloor;
    bool isMoving;
    bool isRight;
    bool isAttack;
    bool isDead;

    Collider colliderLeft;
    Collider colliderRight;
    Collider feet;
} Enemy;

typedef struct Floor
{
    float x, y;
    float width, height;
} Floor;

#endif