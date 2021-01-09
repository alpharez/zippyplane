#include "entity.hpp"

Entity::Entity(int p_x, int p_y, int p_angle, SDL_Rect p_frame)
{
    x = p_x;
    y = p_y;
    angle = p_angle;
    currentFrame = p_frame;
    isMultiFrame = false;
    isInGame = true;
}
Entity::Entity(int p_x, int p_y, int p_angle, SDL_Rect p_frame1, SDL_Rect p_frame2, SDL_Rect p_frame3)
{
    x = p_x;
    y = p_y;
    angle = p_angle;
    frame1 = p_frame1;
    frame2 = p_frame2;
    frame3 = p_frame3;
    currentFrame = frame1;
    isMultiFrame = true;
    i_currentFrame = 1;
    isInGame = true;
}
void Entity::init()
{
}

int Entity::getX()
{
    return x;
}

int Entity::getY()
{
    return y;
}

int Entity::getAngle()
{
    return angle;
}

int Entity::getWidth()
{
    return currentFrame.w;
}
int Entity::getHeight()
{
    return currentFrame.h;
}
bool Entity::inGame()
{
    return isInGame;
}
void Entity::setX(int p_x)
{
    x = p_x;
}
void Entity::setY(int p_y)
{
    y = p_y;
}
void Entity::setAngle(int p_angle)
{
    angle = p_angle;
}
void Entity::setOutofGame()
{
    isInGame = false;
}
/*
SDL_Texture *Entity::getTex()
{
}*/
SDL_Rect *Entity::getCurrentFrame()
{
    if (isMultiFrame)
    {
        // rotate frames
        if (i_currentFrame > 2)
            i_currentFrame = 1;
        else
            i_currentFrame++;
        switch (i_currentFrame)
        {
        case 1:
            currentFrame = frame1;
            break;
        case 2:
            currentFrame = frame2;
            break;
        case 3:
            currentFrame = frame3;
            break;
        default:
            currentFrame = frame1;
            break;
        }
    }
    return &currentFrame;
}

SDL_Rect Entity::getBox()
{
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = currentFrame.w;
    r.h = currentFrame.h;
    return r;
}
