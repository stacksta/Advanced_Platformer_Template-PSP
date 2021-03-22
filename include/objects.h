#ifndef OBJECTS_H
#define OBJECTS_H

typedef struct Player
{
    float x, y;
    float width, height;
    float speed;
    float jumpHeight;
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
} Enemy;

typedef struct Floor
{
    float x, y;
    float width, height;
} Floor;

typedef struct Camera
{
    float x, y;
} Camera;

typedef struct Collider
{
    float x, y;
    float width, height;
} Collider;


#endif