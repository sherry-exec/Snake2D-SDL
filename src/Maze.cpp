#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_timer.h"
#include "SDL/SDL_mixer.h"
#include "graphics/timer.h"
#include "graphics/graphics.h"

#include <iostream>
#include <string>
#include <sstream>

#include "Maze.h"

using namespace std;

const int Rmask = 0xff000000,
          Gmask = 0x00ff0000,
          Bmask = 0x0000ff00,
          Amask = 0x000000ff;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Maze::Maze()
{
    start_position.x = 0;
    start_position.y = 0;
    no_of_walls = 0;
    area = NULL;
    wall = NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Maze::Maze(int w, int h)
{
    start_position.x = 0;
    start_position.y = 0;
    no_of_walls = 0;

    area = NULL;
    wall = NULL;

    area = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, Rmask, Gmask, Bmask, Amask);

    if(! area)
        no_of_walls = -1;
    else
        SDL_FillRect(area, NULL, SDL_MapRGB(area->format, 0, 0, 0));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Maze::Maze(Maze &maze)
{
    start_position.x = maze.start_position.x;
    start_position.y = maze.start_position.y;

    wall = NULL;
    area = NULL;

    no_of_walls = maze.no_of_walls;

    area = SDL_CreateRGBSurface(SDL_SWSURFACE, maze.area->w, maze.area->h,
                                       32, Rmask, Gmask, Bmask, Amask);

    if(! area)
    {
        no_of_walls = -1;
        return;
    }
    else
    {
        SDL_FillRect(area, NULL, SDL_MapRGB(area->format, 0, 0, 0));

        wall = new(nothrow) SDL_Rect[no_of_walls];
        if(! wall)
        {
            no_of_walls = -1;
            return;
        }

        for(int i = 0; i < no_of_walls; i++)
        {
            wall[i].x = maze.wall[i].x;
            wall[i].y = maze.wall[i].y;
            wall[i].w = maze.wall[i].w;
            wall[i].h = maze.wall[i].h;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Maze::~Maze()
{
    /*
    if(area)
        SDL_FreeSurface(area);

    if(wall)
        delete wall;
    */
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Maze::SetStartPosition(int x, int y)
{
    start_position.x = x;
    start_position.y = y;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Dimension Maze::GetStartPosition()
{
    return start_position;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Maze::AddWall(int x, int y, int w, int h)
{
    if(no_of_walls == -1)
        return false;

    //If no walls created yet
    if(no_of_walls == 0)
    {
        no_of_walls++;

        wall = new(nothrow) SDL_Rect[no_of_walls];

        if(! wall)
        {
            no_of_walls--;
            return false;
        }
        else
        {
            wall[0].x = x;
            wall[0].y = y;
            wall[0].w = w;
            wall[0].h = h;
        }
    }
    else
    {
        SDL_Rect *temp = NULL;

        temp = new(nothrow) SDL_Rect[no_of_walls];

        if(! temp)
        {
            return false;
        }
        else
        {
            for(int i = 0; i < no_of_walls; i++)
            {
                temp[i].x = wall[i].x;
                temp[i].y = wall[i].y;
                temp[i].w = wall[i].w;
                temp[i].h = wall[i].h;
            }

            no_of_walls++;

            delete wall;
            wall = new(nothrow) SDL_Rect[no_of_walls];

            if(! wall)
            {
                no_of_walls--;
                delete temp;
                return false;
            }
            else
            {
                for(int i = 0; i < no_of_walls; i++)
                {
                    if(i == (no_of_walls - 1))
                    {
                        wall[i].x = x;
                        wall[i].y = y;
                        wall[i].w = w;
                        wall[i].h = h;
                    }
                    else
                    {
                        wall[i].x = temp[i].x;
                        wall[i].y = temp[i].y;
                        wall[i].w = temp[i].w;
                        wall[i].h = temp[i].h;
                    }
                }
            }
        }

        delete temp;
    }

    return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Maze::ApplyMaze(SDL_Surface *surface)
{
    for(int i = 0; i < no_of_walls; i++)
    {
        Apply_Surface(wall[i].x, wall[i].y, area, surface, &wall[i]);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Maze::CheckCollision(SDL_Rect head)
{
    bool collision = false;

    for(int i = 0; i < no_of_walls; i++)
    {
        collision = true;

        int top1    = head.y,                     top2    = wall[i].y,
            bottom1 = head.y + head.h,            bottom2 = wall[i].y + wall[i].h,
            left1   = head.x,                     left2   = wall[i].x,
            right1  = head.x + head.w,            right2  = wall[i].x + wall[i].w;

        if( bottom1 <= top2)
            collision = false;

        if( top1 >= bottom2)
            collision = false;

        if( right1 <= left2)
            collision = false;

        if( left1 >= right2)
            collision = false;

        if(collision == true)
            return collision;
    }

    return collision;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Maze::X_Start()
{
    return start_position.x;
}
int Maze::Y_Start()
{
    return start_position.y;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Maze Maze::operator=(Maze &maze)
{
    start_position.x = maze.start_position.x;
    start_position.y = maze.start_position.y;

    wall = NULL;
    area = NULL;

    no_of_walls = maze.no_of_walls;

    area = SDL_CreateRGBSurface(SDL_SWSURFACE, maze.area->w, maze.area->h,
                                       32, Rmask, Gmask, Bmask, Amask);

    if(! area)
    {
        no_of_walls = -1;
        return *this;
    }
    else
    {
        SDL_FillRect(area, NULL, SDL_MapRGB(area->format, 0, 0, 0));

        wall = new(nothrow) SDL_Rect[no_of_walls];
        if(! wall)
        {
            no_of_walls = -1;
            return *this;
        }

        for(int i = 0; i < no_of_walls; i++)
        {
            wall[i].x = maze.wall[i].x;
            wall[i].y = maze.wall[i].y;
            wall[i].w = maze.wall[i].w;
            wall[i].h = maze.wall[i].h;
        }
    }
}
