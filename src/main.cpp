#include <raylib.h>
#include <raymath.h>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <iostream>
// #include "gameover.cpp"
// #include "Menu.cpp"

using namespace std;

Color darkGreen = Color{40, 160, 50, 255};
Color gold = {255, 203, 0, 255};
const int cellSize = 30;
const int cellCount = 25;
int offset = 75;
double lastUpdateTime = 0;

Vector2 start1 = {0.0f, 80.0f};               // Start point (x, y)
Vector2 end1 = {cellSize * cellCount, 80.0f}; // End point (x, y)

enum GameState
{
    MENU,
    GAMEPLAY,
    GAME_OVER
};

GameState gameState = MENU;

void ResizeImageToFit(Image &image, int width, int height)
{
    ImageResize(&image, width, height); // Resize image in place
}

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

class Event
{
public:
    bool eventhandler(double interval)
    {
        double currentTime = GetTime();
        if (currentTime - lastUpdateTime >= interval)
        {
            lastUpdateTime = currentTime;
            return true;
        }
        return false;
    }
};

class Snake
{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;

public:
    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{x * cellSize, y * cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(segment, 0.5, 6, gold);
        }
    }

    void controlsHandling()
    {
        if (IsKeyPressed(KEY_UP) && direction.y != 1)
        {
            direction = {0, -1};
        }
        if (IsKeyPressed(KEY_DOWN) && direction.y != -1)
        {
            direction = {0, 1};
        }
        if (IsKeyPressed(KEY_LEFT) && direction.x != 1)
        {
            direction = {-1, 0};
        }
        if (IsKeyPressed(KEY_RIGHT) && direction.x != -1)
        {
            direction = {1, 0};
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (!addSegment)
        {
            body.pop_back(); // If snake is not eating food remove the last elemnt to keep the snake length constant
        }
        else
        {
            addSegment = false; // If snake is eating we skip the pop func
        }
    }

    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Food
{
public:
    const float minY = 90.0f;
    Vector2 position;
    Image fdimage;
    Texture2D fdtexture;

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(minY / cellSize, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(std::deque<Vector2> snakeBody)
    {
        Vector2 pos = GenerateRandomCell();
        while (ElementInDeque(pos, snakeBody))
        {
            pos = GenerateRandomCell();
        }
        return pos;
    }

    Food(std::deque<Vector2> snakeBody)
        : position(GenerateRandomPos(snakeBody))
    {
        Image fdimage = LoadImage("assets/food.png");
        ResizeImageToFit(fdimage, cellSize, cellSize); // Resize image to fit cell size
        fdtexture = LoadTextureFromImage(fdimage);
        UnloadImage(fdimage); // Unload image after creating texture
    }

    void Draw()
    {
        DrawTexture(fdtexture, position.x * cellSize, position.y * cellSize, WHITE);
    }

    ~Food()
    {
        // UnloadImage(fdimage); // Having Segmentation Fault
        UnloadTexture(fdtexture);
    }

public:
};

class GameStart
{
public:
    Snake snake = Snake();
    Event event1 = Event();
    Food food = Food(snake.body);
    int score = 0;
    Sound eatSound;
    Sound wallSound;
    float setGameSpeed = 0.2;
    float scoreFac = 1;
    float factor = 0.01;
    Texture2D bgTexture;

public:
    GameStart()
    {

        eatSound = LoadSound("sounds/eat.mp3");
        wallSound = LoadSound("sounds/wall.mp3");
    }
    void Draw()
    {
        food.Draw();
        snake.Draw();
    }

    void gameUpdate()
    {
        snake.controlsHandling();
    }

    void gameSpeedControl() // Change the value to change speed
    {
        if (score >= 3 * scoreFac)
        {
            setGameSpeed -= factor;
            scoreFac *= 2;
        }
        if (event1.eventhandler(setGameSpeed))
        {
            snake.Update();
        }
    }
    void checkCollisionWithFood()
    {

        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    void checkCollisionWithEdges()
    {

        if (snake.body[0].y <= 2 && snake.body[0].y <= 2)
        {
            GameOver();
        }

        if (snake.body[0].x == cellCount || snake.body[0].x == -1 || snake.body[0].y == cellCount || snake.body[0].y == -1)
        {
            GameOver();
        }
    }

    void checkCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }

    void GameOver()
    {
        snake.Reset();
        score = 0;
        setGameSpeed = 0.2;
        gameState = GAME_OVER;
        // food.position = food.GenerateRandomPos(snake.body);
        PlaySound(wallSound);
    }

    ~GameStart()
    {
        // UnloadImage(bgImage);
        UnloadTexture(bgTexture);
        UnloadSound(wallSound);
        UnloadSound(eatSound);
    }
};

class Menu
{
public:
    Image menuImage;
    Texture2D menuTexture;

    Menu()
    {
        menuImage = LoadImage("assets/backgroundmenu.png");
        ResizeImageToFit(menuImage, cellSize * cellCount, cellSize * cellCount);
        menuTexture = LoadTextureFromImage(menuImage);
        UnloadImage(menuImage); // We can unload the image now that we have the texture
    }

    ~Menu()
    {
        UnloadTexture(menuTexture); // Unload the texture to free up VRAM
    }

    void drawMenu()
    {
        BeginDrawing();
        DrawTexture(menuTexture, 0, 0, WHITE);
        DrawText("Snake Game", 100, 100, 40, GOLD);
        DrawText("Press Enter to Start", 100, 200, 20, GOLD);
        EndDrawing();
    }
};

class GameOver
{
public:
    void drawEndingScreen()
    {
        BeginDrawing();
        ClearBackground(darkGreen);
        DrawText("Game Over", 100, 100, 40, GOLD);
        DrawText("Press Enter to Restart", 100, 200, 20, GOLD);
        EndDrawing();
    }
};

int main()
{

    InitWindow(cellSize * cellCount, cellSize * cellCount, "Snake Game");
    SetTargetFPS(60);
    Image icon = LoadImage("assets/logo.png");
    SetWindowIcon(icon);

    GameStart game1 = GameStart();
    float thickness = 10.0f;
    Menu menu;
    GameOver gameover;

    Image bgImage = LoadImage("assets/background3.png");
    ResizeImageToFit(bgImage, cellSize * cellCount, cellSize * cellCount);
    Texture2D bgTexture = LoadTextureFromImage(bgImage);
    /*----------------------------------------------------------------------------------*/
    int FPS;
    /*----------------------------------------------------------------------------------*/

    while (!WindowShouldClose())
    {
        // FPS = GetFPS();
        // cout << FPS << endl;
        switch (gameState)
        {
        case MENU:
            menu.drawMenu();
            if (IsKeyPressed(KEY_ENTER))
            {
                gameState = GAMEPLAY;
            }
            break;
        case GAMEPLAY:

            BeginDrawing();
            DrawTexture(bgTexture, 0, 0, WHITE);
            game1.Draw();
            game1.gameUpdate();
            game1.gameSpeedControl();
            game1.checkCollisionWithFood();
            game1.checkCollisionWithTail();
            game1.checkCollisionWithEdges();
            ClearBackground(darkGreen);
            DrawLineEx(start1, end1, thickness, RED);
            DrawText(TextFormat("%i", game1.score), 10.0f, 2.0f, 40, gold);
            EndDrawing();
            break;
        case GAME_OVER:
            gameover.drawEndingScreen();
            if (IsKeyPressed(KEY_ENTER))
            {
                gameState = GAMEPLAY;
            }
        default:
            break;
        }
    }
    UnloadImage(icon);
    CloseWindow();
    return 0;
}
