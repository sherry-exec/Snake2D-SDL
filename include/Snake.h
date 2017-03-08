#ifndef SNAKE_H
#define SNAKE_H

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_timer.h"

#include "Maze.h"

const
int difficulty_pivot = 2;

//Difficulty Levels
#define Easy    1
#define Medium  2
#define Hard    3

//CollisionType
#define Collision_Food    1
#define Collision_Body    2
#define Collision_Wall    3

//Direction
#define Up      1
#define Down    2
#define Left    3
#define Right   4

//Snake Class
class Snake
{
private:
    SDL_Surface *head,  //image for head
                *body,  //image for body
                *food,  //image for food
                *combo; //image for combo food

    SDL_Rect    *part,  //screen offsets for part of snake(head and body)
                 food_pos;  //offsets for food and combo

    bool combo_active,
         food_on_maze;

    long score,
         streak;

    int length,
        speed,
        delay_speed,
        direction,
        difficulty;

    Timer comboTimer;

    Maze maze;

public:
    Snake(int x_start, int y_start, int x_border, int y_border,
          SDL_Surface *_head, SDL_Surface *_body,
          SDL_Surface *_food, SDL_Surface *_comboFood,
          Maze _maze,
          int difficulty_level = Medium);

    ~Snake();

private:
    void InitializeSnake();

    void PlaceFood(SDL_Surface *);

    void Grow(SDL_Rect );

    bool CheckBorderCollision();

    bool CheckFoodCollision();

    bool CheckBodyCollision(SDL_Rect *);

    bool CheckMazeCollision();

public:
    int  Move();

    void Show(SDL_Surface *surface);

    int  GetScore();

    void Reset();

    void ChangeDirection(int key);

    void ChangeDifficultyLevel(int difficulty_level);

    void ChangeMaze(Maze newMaze);

    Dimension GetHeadPos();
};

#endif // SNAKE_H
