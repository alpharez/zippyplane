#include <SDL2/SDL.h>

class Entity
{
private:
    int x, y;
    int i_currentFrame;
    int angle;
    bool isMultiFrame;
    bool isInGame;
    SDL_Rect frame1, frame2, frame3;
    SDL_Rect currentFrame;
    SDL_Texture *tex;

public:
    Entity(int p_x, int p_y, int p_angle, SDL_Rect p_frame);
    Entity(int p_x, int p_y, int p_angle, SDL_Rect p_frame1, SDL_Rect p_frame2, SDL_Rect p_frame3);
    void init();
    int getX();
    int getY();
    int getAngle();
    int getWidth();
    int getHeight();
    bool inGame();
    void setX(int p_x);
    void setY(int p_y);
    void setAngle(int p_angle);
    void setOutofGame();
    SDL_Texture *getTex();
    SDL_Rect getBox();
    SDL_Rect *getCurrentFrame();
};