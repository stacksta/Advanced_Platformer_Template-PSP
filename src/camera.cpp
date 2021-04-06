#include "camera.h"

Camera::Camera(float xPos, float yPos, float width, float height)
{
    x = xPos;
    y = yPos;
    w = width;
    h = height;
}

void Camera::followTarget(float targetX, float targetY, float targetWidth, float worldSizeX, float worldSizeY, float tileSize)
{
    x = targetX - (worldSizeX + targetWidth * tileSize * 0.5);
    y = targetY - (worldSizeY * tileSize * 0.5);

    if(x < 0)
       x = 0;
    if(y < 0)
       y = 0;

    if(x > w)// camera.x > camera.width
       x = w;
    if(y > h)//camera.y > camera.height
       y = h;
}

void Camera::getPosition(float *xPos, float *yPos)
{
    *xPos = x;
    *yPos = y;
}

float Camera::getX()
{
    return x;
}

float Camera::getY()
{
    return y;
}

float Camera::getWidth()
{
    return w;
}

float Camera::getHeight()
{
    return h;
}
