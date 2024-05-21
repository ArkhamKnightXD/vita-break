#include <psp2/kernel/processmgr.h>
#include <SDL2/SDL.h>
#include <vector>

// Screen dimension constants
enum {
  SCREEN_WIDTH  = 960,
  SCREEN_HEIGHT = 544
};

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_GameController* controller = NULL;

const int FRAME_RATE = 60; // Desired frame rate (frames per second)

SDL_Rect player;
SDL_Rect ball;

int playerSpeed = 800;
int ballVelocityX = 425;
int ballVelocityY = 425;

typedef struct
{
    SDL_Rect bounds;
    bool isDestroyed;
} Brick;

std::vector<Brick> bricks;

std::vector<Brick> createBricks()
{
    std::vector<Brick> bricks;

    int positionX;
    int positionY = 40;

    for (int i = 0; i < 8; i++)
    {
        positionX = 0;

        for (int j = 0; j < 15; j++)
        {
            Brick actualBrick = {{positionX, positionY, 60, 20}, false};

            bricks.push_back(actualBrick);
            positionX += 64;
        }

        positionY += 22;
    }

    return bricks;
}
 

// Exit the game and clean up
void quitGame() {
    SDL_GameControllerClose(controller);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Function to handle events
void handleEvents() {

    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {

            quitGame();
            exit(0);
        }
    }
}

bool hasCollision(SDL_Rect bounds, SDL_Rect ball)
{
    return bounds.x < ball.x + ball.w && bounds.x + bounds.w > ball.x &&
           bounds.y < ball.y + ball.h && bounds.y + bounds.h > ball.y;
}
 
// Function to update rectangle movement
void update(float deltaTime) {

    SDL_GameControllerUpdate();

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) && player.x > 0) {
        player.x -= playerSpeed * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) && player.x < SCREEN_WIDTH - player.w) {
        player.x += playerSpeed * deltaTime;
    }

    if (ball.y > SCREEN_HEIGHT + ball.h)
    {
        ball.x = SCREEN_WIDTH / 2 - ball.w;
        ball.y = SCREEN_HEIGHT / 2 - ball.h;

        ballVelocityX *= -1;
    }

    if (ball.y < 0)
    {
        ballVelocityY *= -1;
    }
    
    if (ball.x < 0 || ball.x > SCREEN_WIDTH - ball.w)
    {
        ballVelocityX *= -1;
    }

    if (hasCollision(player, ball))
    {
        ballVelocityY *= -1;
    }

    for (unsigned int i = 0; i < bricks.size(); i++)
    {
        if (!bricks[i].isDestroyed && hasCollision(bricks[i].bounds, ball))
        {
            ballVelocityY *= -1;
            bricks[i].isDestroyed = true;
        }
    }

    ball.x += ballVelocityX * deltaTime;
    ball.y += ballVelocityY * deltaTime;
}

// Function to render graphics
void render() {
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);

    for (Brick brick : bricks)
    {
        if (!brick.isDestroyed)
            SDL_RenderFillRect(renderer, &brick.bounds);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_RenderFillRect(renderer, &player);
    SDL_RenderFillRect(renderer, &ball);

    SDL_RenderPresent(renderer);
}

void capFrameRate(Uint32 frameStartTime) {

    Uint32 frameTime = SDL_GetTicks() - frameStartTime;
    
    if (frameTime < 1000 / FRAME_RATE) {
        SDL_Delay(1000 / FRAME_RATE - frameTime);
    }
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        return -1;
    }

    if ((window = SDL_CreateWindow("RedRectangle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN)) == NULL) {
        return -1;
    }

    if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
        return -1;
    }

    if (SDL_NumJoysticks() < 1) {
        printf("No game controllers connected!\n");
        return -1;
    } else {
        controller = SDL_GameControllerOpen(0);
        if (controller == NULL) {
            printf("Unable to open game controller! SDL Error: %s\n", SDL_GetError());
            return -1;
        }
    }

    bricks = createBricks();

    player = {SCREEN_WIDTH / 2, SCREEN_HEIGHT - 32, 74, 16};

    ball = {SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2 - 20, 20, 20};

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime;
    float deltaTime;

    // Main loop
    while (1) {

        currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;
        previousFrameTime = currentFrameTime;

        handleEvents();
        update(deltaTime);
        render();

        capFrameRate(currentFrameTime);
    }

    quitGame();
    return 0;
}
