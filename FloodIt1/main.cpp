//
//  main.cpp
//  FloodIt1
//
//  Created by Koissi Adjorlolo on 2/25/16.
//  Copyright © 2016 centuryapps. All rights reserved.
//

#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

typedef struct
{
    int x, y;
} Vector2i;

typedef int BoardCell;

static const BoardCell COLORID_1 = 1;
static const BoardCell COLORID_8 = 6;

static SDL_Color COLORS[8] = {
    { 255, 100, 100 },
    { 0, 100, 200 },
    { 200, 200, 0 },
    { 0, 200, 0 },
    { 100, 0, 200 },
    { 255, 0, 0 },
    { 255, 255, 128 },
    {155, 128, 255}
};

static const char *TITLE = "Flood It!";
static const int WINDOW_WIDTH = 1024;
static const int WINDOW_HEIGHT = 1024;
static const int WINDOW_POSX = SDL_WINDOWPOS_UNDEFINED;
static const int WINDOW_POSY = SDL_WINDOWPOS_UNDEFINED;
static const Uint32 WINDOW_FLAGS = 0;
static const Uint32 RENDERER_FLAGS = SDL_RENDERER_ACCELERATED |
                                     SDL_RENDERER_PRESENTVSYNC;

static double MS_PER_UPDATE = 1000 / 30;

static int BOARD_WIDTH = 14;
static int BOARD_HEIGHT = 14;

static void init();
static void initBoard();
static void quit();
static void update();
static void updateBoard();
static void render();
static void renderBoard();
static void renderMouseRect();
static int random(int min, int max);
static bool leftMouseDown();
static BoardCell getCellAtPosition(Vector2i position);
static BoardCell getCellAtCellPosition(Vector2i cellPosition);
static void floodBoardWithColor(BoardCell cellColor, Vector2i position = { 0, 0 });
static void setCellAtCellPosition(Vector2i cellPosition, BoardCell color);
static bool checkWin();

static SDL_Renderer *gRenderer = nullptr;
static SDL_Window *gWindow = nullptr;
static bool gRunning = false;
static SDL_Rect gMouseRect = {
    0,
    0,
    WINDOW_WIDTH / BOARD_WIDTH,
    WINDOW_HEIGHT / BOARD_HEIGHT
};
static BoardCell lastColor;
static int gMoves = 0;

static std::vector<std::vector<BoardCell>> gBoard;
static bool gRestarted = false;

int main(int argc, const char * argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_VIDEO))
    {
        std::cout << "Unable to init SDL" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        exit(1);
    }
    
    gWindow = SDL_CreateWindow(TITLE,
                               WINDOW_POSX,
                               WINDOW_POSY,
                               WINDOW_WIDTH,
                               WINDOW_HEIGHT,
                               WINDOW_FLAGS);
    
    if (gWindow == nullptr)
    {
        std::cout << "Unable to create window" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }
    
    gRenderer = SDL_CreateRenderer(gWindow, -1, RENDERER_FLAGS);
    
    if (gRenderer == nullptr)
    {
        std::cout << "Unable to create renderer" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        SDL_DestroyWindow(gWindow);
        gWindow = nullptr;
        SDL_Quit();
        exit(1);
    }
    
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    
    init();
    gRunning = true;
    SDL_Event event;
    
    double previous = (double)SDL_GetTicks();
    double lag = 0.0;
    
    while (gRunning)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    gRunning = false;
                    break;
                    
                case SDL_MOUSEMOTION:
                    gMouseRect.x = event.motion.x / (WINDOW_WIDTH / BOARD_WIDTH) *
                                   (WINDOW_WIDTH / BOARD_WIDTH);
                    gMouseRect.y = event.motion.y / (WINDOW_HEIGHT / BOARD_HEIGHT) *
                                   (WINDOW_HEIGHT / BOARD_HEIGHT);
                    break;
                    
                case SDL_KEYDOWN:
                    
                    if (event.key.keysym.sym == SDLK_RETURN &&
                        !gRestarted)
                    {
                        init();
                        gRestarted = true;
                    }
                    
                    break;
                    
                case SDL_KEYUP:
                    
                    if (event.key.keysym.sym == SDLK_RETURN &&
                        gRestarted)
                    {
                        gRestarted = false;
                    }
                    
                    break;
                    
                default:
                    break;
            }
        }
        
        double current = (double)SDL_GetTicks();
        double elapsed = current - previous;
        lag += elapsed;
        previous = current;
        
        while (lag >= MS_PER_UPDATE)
        {
            update();
            lag -= MS_PER_UPDATE;
        }
        
        render();
    }
    
    quit();
    
    return 0;
}

static void init()
{
    srand((int)time(nullptr));
    initBoard();
}

static void initBoard()
{
    if (gBoard.size() != 0)
    {
        for (int y = 0; y < BOARD_HEIGHT; y++)
        {
            gBoard[y].clear();
        }
        
        gBoard.clear();
    }
    
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        std::vector<BoardCell> newRow;
        gBoard.push_back(newRow);
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            gBoard[y].push_back(random(COLORID_1 - 1, COLORID_8 - 1));
        }
    }
    
    // Color at top left of board.
    lastColor = gBoard[0][0];
}

static void quit()
{
}

static void update()
{
    updateBoard();
}

static void updateBoard()
{
    if (leftMouseDown())
    {
        Vector2i mousePosition = { gMouseRect.x, gMouseRect.y };
        floodBoardWithColor(getCellAtPosition(mousePosition));
        lastColor = getCellAtCellPosition({ 0, 0 });
        
        if (checkWin())
        {
            std::cout << "You Won!!" << std::endl;
            std::cout << "Moves: " << gMoves << std::endl;
            gRunning = false;
        }
    }
}

static void render()
{
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);
    
    renderBoard();
    renderMouseRect();
    
    SDL_RenderPresent(gRenderer);
}

static void renderBoard()
{
    SDL_Rect rect = {
        0,
        0,
        WINDOW_WIDTH / BOARD_WIDTH,
        WINDOW_HEIGHT / BOARD_HEIGHT
    };
    
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            rect.x = x * rect.w;
            rect.y = y * rect.h;
            
            SDL_Color correctColor = COLORS[gBoard[y][x]];
            SDL_SetRenderDrawColor(gRenderer,
                                   correctColor.r,
                                   correctColor.g,
                                   correctColor.b, 255);
            SDL_RenderFillRect(gRenderer, &rect);
        }
    }
}

static void renderMouseRect()
{
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 63);
    SDL_RenderFillRect(gRenderer, &gMouseRect);
}

static int random(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

static bool leftMouseDown()
{
    return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK;
}

static BoardCell getCellAtPosition(Vector2i position)
{
    return gBoard[position.y / (WINDOW_HEIGHT / BOARD_HEIGHT)]
                 [position.x / (WINDOW_WIDTH / BOARD_WIDTH)];
}

static BoardCell getCellAtCellPosition(Vector2i cellPosition)
{
    if (cellPosition.x < 0 || cellPosition.x >= BOARD_WIDTH ||
        cellPosition.y < 0 || cellPosition.y >= BOARD_HEIGHT)
    {
        return -1;
    }
    
    return gBoard[cellPosition.y][cellPosition.x];
}

static void floodBoardWithColor(BoardCell cellColor, Vector2i position)
{
    if (lastColor == cellColor)
    {
        return;
    }
    
    if (position.x < 0 || position.x >= BOARD_WIDTH ||
        position.y < 0 || position.y >= BOARD_HEIGHT)
    {
        return;
    }
    
    if (getCellAtCellPosition(position) == lastColor)
    {
        setCellAtCellPosition(position, cellColor);
    }
    else
    {
        return;
    }
    
    floodBoardWithColor(cellColor, { position.x - 1, position.y });
    floodBoardWithColor(cellColor, { position.x + 1, position.y });
    floodBoardWithColor(cellColor, { position.x, position.y - 1 });
    floodBoardWithColor(cellColor, { position.x, position.y + 1 });
}

static void setCellAtCellPosition(Vector2i cellPosition, BoardCell color)
{
    gBoard[cellPosition.y][cellPosition.x] = color;
}

static bool checkWin()
{
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            if (getCellAtCellPosition({ x, y }) != lastColor)
            {
                return false;
            }
        }
    }
    
    return true;
}
