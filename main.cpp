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
#include <fstream>

#include "Snake.h"
#include "Maze.h"

#define asc     1
#define desc    2

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum Screens
{
    Screen_InGame, Screen_Options, Screen_Highscores
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum MenuChoices
{
    Menu_None, Menu_Play, Menu_Options, Menu_Highscores, Menu_Exit
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum OptionChoices
{
    Options_Difficulty, Options_Maze, Options_Sound
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global Constants

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 440;
const int SCREEN_BPP = 32;

const int Rmask = 0xff000000,
          Gmask = 0x00ff0000,
          Bmask = 0x0000ff00,
          Amask = 0x000000ff;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global Database Variables

unsigned int scores[6];

SDL_Surface *scoreLabels[5] = {NULL, NULL, NULL, NULL, NULL};

ofstream outfile;
ifstream infile;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Mazes

Maze    classic(500, 440),
        box(500, 440),
        scope(500, 440),
        horizontal_pillars(500, 440),
        vertical_pillars(500, 440);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
void BubbleSort(T *base, int length)
{
	bool swap;

	do
	{
		swap = false;

		for(int i = 0; i < length-1; i++)
		{
            if(base[i] < base[i+1])
            {
                T temp = base[i];
                base[i] = base[i+1];
                base[i+1] = temp;
                swap = true;
            }
		}
	}
	while(swap);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool isOver(SDL_Rect rect, int x, int y)
{
    if(x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h)
    {
        return true;
    }

    return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WriteScores()
{
    if(outfile.is_open())
        outfile.close();

    outfile.open("data/data.dat", ios::out | ios::trunc | ios::binary);

    for(int i = 0; i < 5; i++)
    {
        outfile.write((char *) &scores[i], sizeof(unsigned int));
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LoadScores()
{
    if(infile.is_open())
        infile.close();

    infile.open("data/data.dat", ios::in | ios::binary);

    int element = 0;
    while(! infile.eof())
    {
        infile.read((char *) &scores[element++], sizeof(unsigned int));
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WriteScoresLabels(TTF_Font *font, SDL_Color color)
{
    stringstream s;

    for(int i = 0; i < 5; i++)
    {
        s.str("");
        s << i+1 << ".  " << scores[i] ;
        scoreLabels[i] = TTF_RenderText_Blended(font, s.str().c_str(), color);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Apply_Colorkey(SDL_Surface *surface, int R, int G, int B, int A = 255)
{
    Uint32 key = SDL_MapRGBA(surface->format, R, G, B, A);

    SDL_SetColorKey(surface, SDL_SRCCOLORKEY, key);
}
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
int main( int argc, char **args )
{
    //Initialize SDL systems
    if(InitializeSystems() == false)
        return 1;

    if(Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024 ) < 0)
    {
        return false;
    }

    //Local Variables
    SDL_Event e;

    SDL_Surface *head = NULL,
                *body = NULL,
                *food = NULL,
                *combo = NULL,
                *game_over = NULL,
                *GameMaze = NULL,
                *GameVersion = NULL;

    stringstream scorelabel;
    // --

    //Sound Effects
    Mix_Chunk   *eat_sound = NULL,
                *game_over_sound = NULL;
    // --

    //status variables
    bool quit = false;
    int  difficulty_level = Medium,
         maze_level = 1,    //Classic
         sound_level = 1;   //On

    MenuChoices menu_choice = Menu_None;
    // --

    //Display Screen(Window)
    SDL_Surface *screen = NULL;
    // --

    //Fonts and Colors
    TTF_Font    *fontTiny = NULL,
                *fontSmall = NULL,
                *fontMedium = NULL,
                *fontBig = NULL;

    SDL_Color   black = {0, 0, 0},
                white = {255, 255, 255};
    // --

    //Main Menu Screen Items
    SDL_Surface *home = NULL,
                *title = NULL,
                *play = NULL,
                *options = NULL,
                *highscores = NULL,
                *exit = NULL;
    // --

    //In Game Screen Items
    SDL_Surface *maze = NULL,
                *scorebar = NULL,
                *line = NULL;
    // --

    //Options Screen Items
    SDL_Surface *difficulty_label[4] = {NULL ,NULL, NULL, NULL},
                *maze_label[6] = {NULL, NULL, NULL, NULL, NULL, NULL},
                *sound_label[3] = {NULL, NULL, NULL},
                *arrow_left_right = NULL,
                *arrow_up_down = NULL;

    // --

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //SetUp Mazes
    classic.SetStartPosition(20, 40);

    box.SetStartPosition(40, 80);
    if( ! box.AddWall(0,   40,  500, 20)  ||    //Top
        ! box.AddWall(0,   420, 500, 20)  ||    //Bottom
        ! box.AddWall(0,   40,  20,  440) ||    //Left
        ! box.AddWall(480, 40,  20,  440) )     //Right
    return 1;

    scope.SetStartPosition(20, 160);
    if( ! scope.AddWall(0,   220, 120, 20)  ||  //Left
        ! scope.AddWall(240, 40,  20,  120) ||  //Top
        ! scope.AddWall(380, 220, 120, 20)  ||  //Right
        ! scope.AddWall(240, 320, 20,  120) )   //Bottom
    return 1;

    horizontal_pillars.SetStartPosition(40, 80);
    if( ! horizontal_pillars.AddWall(0, 140, 200, 20)   ||
        ! horizontal_pillars.AddWall(320, 240, 200, 20) ||
        ! horizontal_pillars.AddWall(0, 340, 200, 20)   )
    return 1;

    vertical_pillars.SetStartPosition(20, 200);
    if( ! vertical_pillars.AddWall(120, 260, 20, 180) ||
        ! vertical_pillars.AddWall(240, 40, 20, 180)  ||
        ! vertical_pillars.AddWall(380, 260, 20, 180) )
    return 1;
    // --

    //SetUp Display Screen
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
    SDL_WM_SetCaption("Snake - PC", NULL);

    if(!screen)
        return 1;
    // --

    //Load Music Sounds
    eat_sound = Mix_LoadWAV("audio/eat.wav");
    game_over_sound = Mix_LoadWAV("audio/game over.wav");

    //if(!eat_sound || !game_over_sound)
    //    return 1;
    // --

    //Load Snake's Head and Body image, Food and ComboFood
    head = Load_Image("pics/head diamond.png");
    body = Load_Image("pics/body diamond.png");
    food = Load_Image("pics/food.png");
    combo = Load_Image("pics/combo.png");

    if(!head || !body || !food || !combo)
        return 1;

    Apply_Colorkey(head, 255, 255, 255, 255);
    Apply_Colorkey(body, 255, 255, 255, 255);

    //and Create snake
    Snake snake(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT, head, body, food, combo, classic);
    // --

    //SetUp Fonts for Labels
    fontTiny = TTF_OpenFont("fonts/opensans.ttf", 12);
    fontSmall = TTF_OpenFont("fonts/opensans.ttf", 18);
    fontMedium = TTF_OpenFont("fonts/opensans.ttf", 26);
    fontBig = TTF_OpenFont("fonts/opensans.ttf", 30);

    if(! fontSmall || !fontMedium || !fontBig)
        return 1;
    // --

    //Load Other Images
    game_over = Load_Image("pics/game over.png");

    if(! game_over)
        return 1;
    // --

    //SetUp Main Menu Screen
    home = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, Rmask, Gmask, Bmask, Amask);
    title       = Load_Image("pics/title.png");
    play        = Load_Image("pics/play.png");
    options     = Load_Image("pics/options.png");
    highscores  = Load_Image("pics/high scores.png");
    exit        = Load_Image("pics/exit.png");

    if(!home || !title || !play || !options || !highscores || !exit)
    {
        return 1;
    }
    else
    {
        title->clip_rect.x = SCREEN_WIDTH/2 - title->w/2;
        title->clip_rect.y = 30;

        play->clip_rect.x = SCREEN_WIDTH/2 - play->w/2;
        play->clip_rect.y = 150;

        options->clip_rect.x = SCREEN_WIDTH/2 - options->w/2;
        options->clip_rect.y = 220;

        highscores->clip_rect.x = SCREEN_WIDTH/2 - highscores->w/2;
        highscores->clip_rect.y = 290;

        exit->clip_rect.x = SCREEN_WIDTH/2 - exit->w/2;
        exit->clip_rect.y = 360;

        Apply_Surface(title->clip_rect.x,       title->clip_rect.y,         title,      home, NULL);
        Apply_Surface(play->clip_rect.x,        play->clip_rect.y,          play,       home, NULL);
        Apply_Surface(options->clip_rect.x,     options->clip_rect.y,       options,    home, NULL);
        Apply_Surface(highscores->clip_rect.x,  highscores->clip_rect.y,    highscores, home, NULL);
        Apply_Surface(exit->clip_rect.x,        exit->clip_rect.y,          exit,       home, NULL);
    }
    // --

    //SetUp In-Game Screen
    GameMaze = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, Rmask, Gmask, Bmask, Amask);

    maze = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, Rmask, Gmask, Bmask, Amask);
    SDL_FillRect(maze, NULL, SDL_MapRGB(maze->format, 190, 190, 190));

    line = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, 3, SCREEN_BPP, Rmask, Gmask, Bmask, Amask);
    SDL_FillRect(line, NULL, SDL_MapRGB(maze->format, 0, 0, 0));

    if(!GameMaze || !maze || !line)
        return 1;
    else
        Apply_Surface(0, 40, line, maze);
    // --

    //SetUp Options Screen
    difficulty_label[0] = TTF_RenderText_Blended(fontSmall, "Difficulty : ", white);
    difficulty_label[1] = TTF_RenderText_Blended(fontSmall, "Easy", white);
    difficulty_label[2] = TTF_RenderText_Blended(fontSmall, "Medium", white);
    difficulty_label[3] = TTF_RenderText_Blended(fontSmall, "Hard", white);

    maze_label[0] = TTF_RenderText_Blended(fontSmall, "Game Maze : ", white);
    maze_label[1] = TTF_RenderText_Blended(fontSmall, "Classic", white);
    maze_label[2] = TTF_RenderText_Blended(fontSmall, "Box", white);
    maze_label[3] = TTF_RenderText_Blended(fontSmall, "Scope", white);
    maze_label[4] = TTF_RenderText_Blended(fontSmall, "Horizontal Pillars", white);
    maze_label[5] = TTF_RenderText_Blended(fontSmall, "Vertical Pillars", white);

    sound_label[0] = TTF_RenderText_Blended(fontSmall, "Sound Effects : ", white);
    sound_label[1] = TTF_RenderText_Blended(fontSmall, "On", white);
    sound_label[2] = TTF_RenderText_Blended(fontSmall, "Off", white);

    GameVersion = TTF_RenderText_Blended(fontTiny, "Snake 2D - PC - Version 1.2 - Stable Release", white);

    arrow_left_right = Load_Image("pics/arrows_left_right.png");
    arrow_up_down    = Load_Image("pics/arrows_up_down.png");

    for(int i = 0; i < 4; i++)
    {
        if(! difficulty_label[i])
            return 1;
    }

    for(int i = 0; i < 6; i++)
    {
        if(! maze_label[i])
            return 1;
    }

    for(int i = 0; i < 3; i++)
    {
        if(! sound_label[i])
            return 1;
    }
    // --

    //Load Scores from Database
    infile.open("data/data.dat", ios::in | ios::binary);

    if(!infile) //if file does not exist
    {
        outfile.open("data/data.dat", ios::out | ios::binary);  //create new file for write
        infile.open("data/data.dat", ios::in | ios::binary);    //and read

        //Initialize Scores
        for(int i = 0; i < 5; i++)
        {
            scores[i] = 0;
            outfile.write((char *) &scores[i], sizeof(unsigned int));
        }
    }
    else    //if file exists
    {
        //Load all previous scores
        LoadScores();
    }

    WriteScoresLabels(fontMedium, white);
    // --

    //GAME START
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //MAIN LOOP
    while(menu_choice != Menu_Exit)
    {
        //reset choice status FLAGS
        menu_choice = Menu_None;
        quit = false;

        //Apply Main Menu
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        Apply_Surface(0, 0, home, screen);

        //IN MAIN MENU
        while(menu_choice == Menu_None)
        {
            while(SDL_PollEvent(&e))
            {
                if(e.type == SDL_MOUSEBUTTONDOWN)
                {
                    if(e.button.button == SDL_BUTTON_LEFT)
                    {
                        if(isOver(play->clip_rect, e.button.x, e.button.y))
                        {
                            menu_choice = Menu_Play;
                        }
                        else if(isOver(options->clip_rect, e.button.x, e.button.y))
                        {
                            menu_choice = Menu_Options;
                        }
                        else if(isOver(highscores->clip_rect, e.button.x, e.button.y))
                        {
                            menu_choice = Menu_Highscores;
                        }
                        else if(isOver(exit->clip_rect, e.button.x, e.button.y))
                        {
                            menu_choice = Menu_Exit;
                        }
                    }
                }
                else if(e.type == SDL_KEYDOWN)
                {
                    switch(e.key.keysym.sym)
                    {
                    case SDLK_1:
                        {
                            menu_choice = Menu_Play;
                            break;
                        }
                    case SDLK_SPACE:
                        {
                            menu_choice = Menu_Play;
                            break;
                        }
                    case SDLK_2:
                        {
                            menu_choice = Menu_Options;
                            break;
                        }
                    case SDLK_3:
                        {
                            menu_choice = Menu_Highscores;
                            break;
                        }
                    case SDLK_4:
                        {
                            menu_choice = Menu_Exit;
                            break;
                        }
                    case SDLK_ESCAPE:
                        {
                            menu_choice = Menu_Exit;
                            break;
                        }
                    }
                }
                else if(e.type == SDL_QUIT)
                {
                    menu_choice = Menu_Exit;
                }
            }
        }
        // -- MAIN MENU LOOP ENDS


        //If choice status is Play
        if(menu_choice == Menu_Play)
        {
            //Reset Snake
            snake.Reset();

            //IN GAME MAIN-LOOP
            //PLAY LOOP
            while(! quit)
            {
                SDL_FillRect(GameMaze, NULL, SDL_MapRGB(maze->format, 0, 0, 0));
                Apply_Surface(0, 0, maze, GameMaze);

                //Place Snake and Food on Game Maze
                snake.Show(GameMaze);

                //Place Score On Game Maze
                scorelabel.str("");
                scorelabel << "Score  :  " << snake.GetScore() ;
                scorebar = TTF_RenderText_Blended(fontSmall, scorelabel.str().c_str(), black);
                Apply_Surface(50, 40/2 - scorebar->h/2, scorebar, GameMaze);

                //Check for Key Movement
                while(SDL_PollEvent(&e))
                {
                    if(e.type == SDL_KEYDOWN)
                    {
                        switch(e.key.keysym.sym)
                        {
                        case SDLK_UP:
                            snake.ChangeDirection(Up);  break;

                        case SDLK_DOWN:
                            snake.ChangeDirection(Down);  break;

                        case SDLK_LEFT:
                            snake.ChangeDirection(Left);  break;

                        case SDLK_RIGHT:
                            snake.ChangeDirection(Right);  break;
                        }
                    }
                    else if(e.type == SDL_QUIT)
                    {
                        menu_choice = Menu_Exit;
                        quit = true;
                    }
                }

                //Refresh Play Maze
                Apply_Surface(0, 0, GameMaze, screen);

                //Move Snake and check for collisions
                int collision_status = snake.Move();    //return true if snake has eaten itself

                if(collision_status == Collision_Food)
                {
                    if(sound_level == 1)
                        Mix_PlayChannel(-1, eat_sound, 0);
                }
                else if(collision_status == Collision_Body || collision_status == Collision_Wall)
                {
                    quit = true;
                }
            }

            //Let User View the Collision
            if(sound_level == 1)
                Mix_PlayChannel(-1, game_over_sound, 0);

            SDL_Delay(700);

            //Apply Game Over and Score
            {
                SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
                scorebar = TTF_RenderText_Blended(fontMedium, scorelabel.str().c_str(), white);

                Apply_Surface(screen->w/2 - game_over->w/2, 100, game_over, screen);
                Apply_Surface(screen->w/2 - scorebar->w/2, 250, scorebar, screen);
            }

            //Update Database with scores
            {
                scores[5] = snake.GetScore();

                BubbleSort(scores, 6);

                WriteScores();
                WriteScoresLabels(fontMedium, white);
            }

            //Wait for 1.2 secs
            SDL_Delay(1200);
        }
        // -- IN GAME, PLAY LOOP ENDS

        //If choice status is Options
        else if(menu_choice == Menu_Options)
        {
            bool applied = false;
            while(! quit)
            {
                if(! applied)
                {
                    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
                    Apply_Surface(screen->w/2 - options->w/2, 50, options, screen);

                    //Apply Difficulty Options
                    Apply_Surface(100, 150, difficulty_label[0], screen);
                    Apply_Surface(300, 150, difficulty_label[difficulty_level], screen);
                    Apply_Surface(100 - arrow_left_right->w - 10, 149, arrow_left_right, screen);

                    //Apply Maze Options
                    Apply_Surface(100, 200, maze_label[0], screen);
                    Apply_Surface(300, 200, maze_label[maze_level], screen);
                    Apply_Surface(100 - arrow_up_down->w - 10, 199, arrow_up_down, screen);

                    //Apply Sound Options
                    Apply_Surface(100, 250, sound_label[0], screen);
                    Apply_Surface(300, 250, sound_label[sound_level], screen);

                    //Apply Version Label of Game
                    Apply_Surface(screen->w/2 - GameVersion->w/2, screen->h - GameVersion->h, GameVersion, screen);

                    applied = true;
                }

                while(SDL_PollEvent(&e))
                {
                    if(e.type == SDL_KEYDOWN)
                    {
                        switch(e.key.keysym.sym)
                        {
                        case SDLK_LEFT:
                            if(difficulty_level > 1)
                            {
                                difficulty_level--;

                                snake.ChangeDifficultyLevel(difficulty_level);

                                applied = false;
                            }

                            break;

                        case SDLK_RIGHT:
                            if(difficulty_level < 3)
                            {
                                difficulty_level++;

                                snake.ChangeDifficultyLevel(difficulty_level);

                                applied = false;
                            }

                            break;

                        case SDLK_UP:
                            if(maze_level > 1)
                            {
                                maze_level--;

                                switch(maze_level)
                                {
                                    case 1: snake.ChangeMaze(classic); break;
                                    case 2: snake.ChangeMaze(box); break;
                                    case 3: snake.ChangeMaze(scope); break;
                                    case 4: snake.ChangeMaze(horizontal_pillars); break;
                                    case 5: snake.ChangeMaze(vertical_pillars); break;
                                }

                                applied = false;
                            }

                            break;

                        case SDLK_DOWN:
                            if(maze_level < 5)
                            {
                                maze_level++;

                                switch(maze_level)
                                {
                                    case 1: snake.ChangeMaze(classic); break;
                                    case 2: snake.ChangeMaze(box); break;
                                    case 3: snake.ChangeMaze(scope); break;
                                    case 4: snake.ChangeMaze(horizontal_pillars); break;
                                    case 5: snake.ChangeMaze(vertical_pillars); break;
                                }

                                applied = false;
                            }

                            break;

                        case SDLK_BACKSPACE:
                            quit = true;

                        case SDLK_ESCAPE:
                            quit = true;
                        }
                    }
                    else if(e.type == SDL_QUIT)
                    {
                        menu_choice = Menu_Exit;
                        quit = true;
                    }
                }
            }
        }
        // -- OPTIONS END

        //If choice status is HighScores
        else if(menu_choice == Menu_Highscores)
        {
            bool applied = false;

            while(! quit)
            {
                if(!applied)
                {
                    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
                    Apply_Surface(screen->w/2 - highscores->w/2, 50, highscores, screen);

                    for(int y = 150, i = 0; y <= 350; y += 50, i++)
                    {
                        Apply_Surface(150, y, scoreLabels[i], screen);
                    }

                    applied = true;
                }

                while(SDL_PollEvent(&e))
                {
                    if(e.type == SDL_KEYDOWN)
                    {
                        switch(e.key.keysym.sym)
                        {
                        case SDLK_BACKSPACE:
                            quit = true;

                        case SDLK_ESCAPE:
                            quit = true;
                        }
                    }
                    else if(e.type == SDL_QUIT)
                    {
                        menu_choice = Menu_Exit;
                        quit = true;
                    }
                }
            }
        }
        // -- HIGHSCORES ENDS
    }
    // -- MAIN LOOP ENDS

    //Close DB files
    outfile.close();
    infile.close();

    //Delete All Surfaces
    SDL_FreeSurface(screen);

    SDL_FreeSurface(head);
    SDL_FreeSurface(body);
    SDL_FreeSurface(food);
    SDL_FreeSurface(combo);

    SDL_FreeSurface(home);
    SDL_FreeSurface(title);
    SDL_FreeSurface(play);
    SDL_FreeSurface(highscores);
    SDL_FreeSurface(options);
    SDL_FreeSurface(exit);
    SDL_FreeSurface(game_over);

    SDL_FreeSurface(GameMaze);
    SDL_FreeSurface(maze);
    SDL_FreeSurface(line);
    SDL_FreeSurface(scorebar);

    SDL_FreeSurface(GameVersion);

    for(int i = 0; i < 5; i++)
    SDL_FreeSurface(scoreLabels[i]);

    for(int i = 0; i < 4; i++)
    SDL_FreeSurface(difficulty_label[i]);

    for(int i = 0; i < 6; i++)
    SDL_FreeSurface(maze_label[i]);

    for(int i = 0; i < 3; i++)
    SDL_FreeSurface(sound_label[i]);

    SDL_FreeSurface(arrow_up_down);
    SDL_FreeSurface(arrow_left_right);

    //Delete Sounds
    Mix_FreeChunk(eat_sound);
    Mix_FreeChunk(game_over_sound);

    //Delete Fonts
    TTF_CloseFont(fontSmall);
    TTF_CloseFont(fontMedium);
    TTF_CloseFont(fontBig);

    ShutdownSystems();
    Mix_CloseAudio();

	return 0;
}
