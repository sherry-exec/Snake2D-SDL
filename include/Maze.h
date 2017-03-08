#ifndef MAZE_H
#define MAZE_H

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_timer.h"

struct Dimension
{
    int x, y;
};

class Maze
{
private:
    SDL_Surface *area;

    SDL_Rect *wall;

    int no_of_walls;

    Dimension start_position;

public:
    Maze();

    Maze(int w, int h);

    Maze(Maze &maze);       //copy constructor

    ~Maze();

    void SetStartPosition(int x, int y);

    Dimension GetStartPosition();

    bool AddWall(int x, int y, int w, int h);

    void ApplyMaze(SDL_Surface *surface);

    bool CheckCollision(SDL_Rect surface);

    Maze operator=(Maze &maze);

    int X_Start();
    int Y_Start();
};

#endif // MAZE_H
