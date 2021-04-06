#ifndef CAMERA_H
#define CAMERA_H

class Camera
{
    public:
        Camera(float xPos, float yPos, float width, float height);
        void followTarget(float targetX, float targetY, float targetWidth, float worldSizeX, float worldSizeY, float tileSize);
        void getPosition(float *xPos, float *yPos);

        float getX();
        float getY();
        float getWidth();
        float getHeight();

    private:
        float x, y;
        float w, h;
};


#endif