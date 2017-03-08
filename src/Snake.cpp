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

#include "Snake.h"
#include "Maze.h"

#define head_part   length - 1
#define tail_part   0

using namespace std;

const int Rmask = 0xff000000,
          Gmask = 0x00ff0000,
          Bmask = 0x0000ff00,
          Amask = 0x000000ff;

int score_level = 100;

bool grow = false;

///////////////////////////////////////////////////////////////////////
int X_START = 0, Y_START = 0,
    X_BORDER = 500, Y_BORDER = 440;
///////////////////////////////////////////////////////////////////////
//Constructor
Snake::Snake(int x_start, int y_start, int x_border, int y_border,
             SDL_Surface *head_image, SDL_Surface *body_image,
             SDL_Surface *food_image, SDL_Surface *combo_image,
             Maze _maze,
             int difficulty_level)
{
    head = body = NULL;
    part = NULL;

    //Set play area attributes
    if(x_start >= 0)
        X_START = x_start;

    if(y_start >= 0)
        Y_START = y_start;

    if(x_border > X_BORDER)
        X_BORDER = x_border;

    if(y_border > Y_BORDER)
        Y_BORDER = y_border;

    //Set snake attributes
    score = 0;
    streak = 0;

    length = 10;
    difficulty = difficulty_level * difficulty_pivot;
    speed = 20;
    delay_speed = 60;
    direction = Right;

    food_on_maze = false;
    combo_active = false;

    //Game Maze
    maze = _maze;

    //Load images
    head = head_image;
    body = body_image;
    food = food_image;
    combo = combo_image;

    //Make snake
    InitializeSnake();
}
///////////////////////////////////////////////////////////////////////
//Destructor
Snake::~Snake()
{
    head = body =
    food = combo = NULL;
    delete part;
}
///////////////////////////////////////////////////////////////////////
void Snake::InitializeSnake()
{
    int box_area = 20;

    delete part;
    part = new SDL_Rect[length];
    for(int i = 0; i < length; i++)
    {
        part[i].x = maze.X_Start() + (box_area * (i + 1));
        part[i].y = maze.Y_Start() + box_area;
        part[i].w = box_area;
        part[i].h = box_area;
    }
}
///////////////////////////////////////////////////////////////////////
void Snake::Grow(SDL_Rect NewPart)
{
    SDL_Rect *temp = new(nothrow) SDL_Rect[length];

    //Store snake position
    for(int i = 0; i < length; i++)
    {
        temp[i].x = part[i].x;
        temp[i].y = part[i].y;
        temp[i].w = part[i].w;
        temp[i].h = part[i].h;
    }

    length++;

    //Make new larger Snake
    delete part;
    part = new(nothrow) SDL_Rect[length];

    //Restore positions
    for(int i = 1; i < length; i++)
    {
        part[i].x = temp[i-1].x;
        part[i].y = temp[i-1].y;
        part[i].w = temp[i-1].w;
        part[i].h = temp[i-1].h;
    }

    //Add the last new part and Make new tail
    part[tail_part].x = NewPart.x;
    part[tail_part].y = NewPart.y;
    part[tail_part].w = NewPart.w;
    part[tail_part].h = NewPart.h;

    /*
    //Load previous head
    part[length-1].w = temp[length-2].w;
    part[length-1].h = temp[length-2].h;

    switch(direction)
    {
        case Up:
        {
            part[length-1].x = part[length-2].x;
            part[length-1].y = part[length-2].y - speed;

            if(part[length-1].y < Y_START)
                part[length-1].y = Y_BORDER - 20;

            break;
        }
        case Down:
        {
            part[length-1].x = part[length-2].x;
            part[length-1].y = part[length-2].y + speed;

            if(part[length-1].y + part[length-1].h > Y_BORDER)
                part[length-1].y = Y_START;

            break;
        }
        case Left:
        {
            part[length-1].x = part[length-2].x - speed;
            part[length-1].y = part[length-2].y;

            if(part[length-1].x < X_START)
                part[length-1].x = X_BORDER - 20;

            break;
        }
        case Right:
        {
            part[length-1].x = part[length-2].x + speed;
            part[length-1].y = part[length-2].y;

            if(part[length-1].x + part[length-1].w > X_BORDER)
                part[length-1].x = X_START;

            break;
        }
    }
    */

    delete temp;
}
///////////////////////////////////////////////////////////////////////
void Snake::PlaceFood(SDL_Surface *surface)
{
    //if combo food timer exceeded
    if(combo_active && comboTimer.GetSeconds() > 2)
    {
        //disappear combo food
        combo_active = false;
        food_on_maze = false;
    }

    if(! food_on_maze)
    {
        if(combo_active)
        {
            food_pos.w = 30;
            food_pos.h = 30;
        }
        else
        {
            food_pos.w = 20;
            food_pos.h = 20;
        }

        //Create new position
        do
        {
            food_pos.x = X_START + (rand() % ((X_BORDER - food_pos.w) - X_START + 1));
            food_pos.y = Y_START + (rand() % ((Y_BORDER - food_pos.h) - Y_START + 1));
        }
        while(maze.CheckCollision(food_pos) || CheckBodyCollision(&food_pos));   //while if food is placed on wall, create new position

        food_on_maze = true;
    }

    if(combo_active)
    {
        Apply_Surface(food_pos.x, food_pos.y, combo, surface);
    }
    else
    {
        Apply_Surface(food_pos.x, food_pos.y, food, surface);
    }
}
///////////////////////////////////////////////////////////////////////
bool Snake::CheckBorderCollision()
{
    switch(direction)
    {
        case Up:
        {
            if(part[head_part].y < Y_START)
                part[head_part].y = Y_BORDER - 20;

            return true;
        }
        case Down:
        {
            if(part[head_part].y + part[head_part].h > Y_BORDER)
                part[head_part].y = Y_START;

            return true;
        }
        case Left:
        {
            if(part[head_part].x < X_START)
                part[head_part].x = X_BORDER - 20;

            return true;
        }
        case Right:
        {
            if(part[head_part].x + part[head_part].w > X_BORDER)
                part[head_part].x = X_START;

            return true;
        }
    }

    return false;
}
///////////////////////////////////////////////////////////////////////
bool Snake::CheckFoodCollision()
{
    bool collision = true;

    int top1    = part[head_part].y,                         top2    = food_pos.y,
        bottom1 = part[head_part].y + part[head_part].h,     bottom2 = food_pos.y + food_pos.h,
        left1   = part[head_part].x,                         left2   = food_pos.x,
        right1  = part[head_part].x + part[head_part].w,     right2  = food_pos.x + food_pos.w;

    if( bottom1 <= top2)
        collision = false;

    if( top1 >= bottom2)
        collision = false;

    if( right1 <= left2)
        collision = false;

    if( left1 >= right2)
        collision = false;

    //If Collision Found
    if(collision == true)
    {
        if(combo_active)    //if collided with combo food
            score += (difficulty * 18);
        else                //collided with normal food
            score += (difficulty * 3);

        if(score > score_level) //if score reached score level
        {
            if(delay_speed > 20)    //if speed not least
            {
                delay_speed -= difficulty;  //increase speed according to difficulty
                score_level += 100; //increase score pivot level
            }
        }

        streak++;

        //if snake eats 5 contiguous food
        if(streak % 5 == 0)
        {
            //then activate combo
            combo_active = true;
            comboTimer.Start();
        }
        else
        {
            combo_active = false;
            comboTimer.Stop();
        }

        //If snake eats 2 contiguous food
        if(streak % 2 == 0)
        {
            //then grow by 1
            grow = true;
        }

        food_on_maze = false;
    }

    return collision;
}
///////////////////////////////////////////////////////////////////////
bool Snake::CheckBodyCollision(SDL_Rect *head = NULL)
{
    bool collision = false;

    if(head == NULL)
        head = &part[head_part];

    for(int i = 0; i < length - 1; i++)
    {
        collision = true;

        int top1    = head->y,               top2    = part[i].y,
            bottom1 = head->y + head->h,      bottom2 = part[i].y + part[i].h,
            Left1   = head->x,               Left2   = part[i].x,
            Right1  = head->x + head->w,      Right2  = part[i].x + part[i].w;

        if( bottom1 <= top2)
            collision = false;

        if( top1 >= bottom2)
            collision = false;

        if( Right1 <= Left2)
            collision = false;

        if( Left1 >= Right2)
            collision = false;

        if(collision == true)
            return collision;
    }

    return collision;
}
///////////////////////////////////////////////////////////////////////
bool Snake::CheckMazeCollision()
{
    //Check For Collision With Maze Obstacles with Head
    return maze.CheckCollision(part[head_part]);
}
///////////////////////////////////////////////////////////////////////
int Snake::Move()
{
    //delay movement
    SDL_Delay(delay_speed);

    SDL_Rect last_part;
    if(grow)
    {
        last_part.x = part[0].x;
        last_part.y = part[0].y;
        last_part.w = part[0].w;
        last_part.h = part[0].h;
    }

    for(int i = 0; i < length; i++)
    {
        if(i == length-1)   //if head
        {
            switch(direction)
            {
                case Up:
                {
                    part[i].y -= speed;
                    break;
                }
                case Down:
                {
                    part[i].y += speed;
                    break;
                }
                case Left:
                {
                    part[i].x -= speed;
                    break;
                }
                case Right:
                {
                    part[i].x += speed;
                    break;
                }
            }
        }
        else    //if body, then follow box ahead
        {
            part[i].x = part[i+1].x;
            part[i].y = part[i+1].y;
        }
    }

    if(grow)
    {
        Grow(last_part);
        grow = false;
    }

    CheckBorderCollision();

    if(CheckMazeCollision())
        return Collision_Wall;

    if(CheckBodyCollision())
        return Collision_Body;

    if(CheckFoodCollision())
        return Collision_Food;

    return 0;
}
///////////////////////////////////////////////////////////////////////
void Snake::Show(SDL_Surface *surface)
{
    maze.ApplyMaze(surface);

    for(int i = 0; i < length; i++)
    {
        if( i == head_part ) //head
        {
            Apply_Surface(part[i].x, part[i].y, head, surface);
        }
        else    //body
        {
            Apply_Surface(part[i].x, part[i].y, body, surface);
        }
    }

    PlaceFood(surface);
}
///////////////////////////////////////////////////////////////////////
void Snake::ChangeDirection(int key)
{
    if(key == Up && (direction == Left || direction == Right))
    {
        direction = Up;
    }
    else if(key == Down && (direction == Left || direction == Right))
    {
        direction = Down;
    }
    else if(key == Left && (direction == Up || direction == Down))
    {
        direction = Left;
    }
    else if(key == Right && (direction == Up || direction == Down))
    {
        direction = Right;
    }
}
///////////////////////////////////////////////////////////////////////
int Snake::GetScore()
{
    return score;
}
///////////////////////////////////////////////////////////////////////
void Snake::Reset()
{
    //Reset snake attributes
    score = 0;
    streak = 0;

    length = 10;
    direction = Right;
    speed = 20;
    delay_speed = 60;

    food_on_maze = false;
    combo_active = false;

    //Initialize Snake
    InitializeSnake();
}
///////////////////////////////////////////////////////////////////////
void Snake::ChangeDifficultyLevel(int difficulty_level)
{
    if(difficulty_level >= Easy && difficulty_level <= Hard)
        difficulty = difficulty_level * difficulty_pivot;
    else
        difficulty = Medium * difficulty_pivot;
}
///////////////////////////////////////////////////////////////////////
void Snake::ChangeMaze(Maze newMaze)
{
    maze = newMaze;
    InitializeSnake();
}
///////////////////////////////////////////////////////////////////////
Dimension Snake::GetHeadPos()
{
    Dimension head_pos;

    head_pos.x = part[head_part].x;
    head_pos.y = part[head_part].y;

    return head_pos;
}
