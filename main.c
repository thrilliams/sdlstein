#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <math.h>

int map[5][5] = {
    {1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1}};

int w = 5;
int h = 5;

typedef struct {
    double x;
    double y;
    double direction;
} emitter;

emitter e;

int screenWidth = 640;
int screenHeight = 480;

void background(SDL_Renderer *renderer, int r, int g, int b, int a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderClear(renderer);
}

void clear(SDL_Renderer *renderer) {
    background(renderer, 51, 51, 51, SDL_ALPHA_OPAQUE);
}

void rect(SDL_Renderer *renderer, int x, int y, int w, int h, int r, int g, int b, int a) {
    SDL_Rect rectangle;
    rectangle.x = x;
    rectangle.y = y;
    rectangle.w = w;
    rectangle.h = h;

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderDrawRect(renderer, &rectangle);
    SDL_RenderFillRect(renderer, &rectangle);
}

int cast(double direction, double *outX, double *outY) {
    int x = floor(e.x);
    int y = floor(e.y);
    int tileStepX = 1;
    int tileStepY = 1;
    double dx = fmod(e.x, 1);
    double dy = fmod(e.y, 1);

    if (direction < M_PI / 2) {
        tileStepY = -1;
        dx = 1 - dx;
    } else if (direction < M_PI) {
        tileStepX = -1;
        tileStepY = -1;
        dx = 1 - dx;
        dy = 1 - dy;
    } else if (direction < M_PI * 3 / 2) {
        tileStepX = -1;
        dy = 1 - dy;
    }

    double xStep = tan(direction) * tileStepX;
    double yStep = (1 / tan(direction)) * tileStepY;

    double xIntercept = x + dx + (-dy / tan(direction));
    double yIntercept = y + dy + (dx / tan(direction));

    while (1) {
        while ((tileStepY == 1 && yIntercept < y) || (tileStepY == -1 && yIntercept > y)) {
            int cy = yIntercept;
            // printf("Y intercept: %d, %d; Direction: %f\n", x, cy, direction);
            *outX = x;
            *outY = cy;
            if (cy < 0)
                return 1;
            if (cy > h)
                return 1;
            if (map[cy][x])
                return map[cy][x];
            x += tileStepX;
            yIntercept += yStep;
        }

        while ((tileStepX == 1 && xIntercept < x) || (tileStepX == -1 && xIntercept > x)) {
            int cx = xIntercept;
            *outX = cx;
            *outY = y;
            // printf("X intercept: %d, %d; Direction: %f\n", cx, y, direction);
            if (cx < 0)
                return 1;
            if (cx > w)
                return 1;
            if (map[y][cx])
                return map[y][cx];
            y += tileStepY;
            xIntercept += xStep;
        }
    }

    return 0;
}

void draw(SDL_Renderer *renderer) {
    // Uint32 millis = SDL_GetTicks();
    background(renderer, 100, 51, 51, SDL_ALPHA_OPAQUE);
    rect(renderer, 0, 0, screenWidth, screenHeight / 2, 135, 206, 235, SDL_ALPHA_OPAQUE);

    double fov = 90;
    fov *= M_PI;
    fov /= 180;

    for (int i = 0; i < screenWidth; i++) {
        double d = e.direction + ((i - (screenWidth / 2)) * (fov / screenWidth));
        while (d < 0) d += M_2_PI;
        while (d > M_2_PI) d -= M_2_PI;
        double dx, dy;
        cast(d, &dx, &dy);  // int tile =
        double dist = cos((i - (screenWidth / 2)) * (fov / screenWidth)) * sqrt(pow(dx, 2) + pow(dy, 2));
        if (screenHeight - dist > 0) {
            int height = screenHeight / dist;
            int color = 255 / dist;
            rect(renderer, i, (screenHeight - height) / 2, 1, height, color, color, color, SDL_ALPHA_OPAQUE);
        }
    }

    e.direction += 0.001;

    SDL_RenderPresent(renderer);
}

Uint32 drawCallback(Uint32 interval, void *pointer) {
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = &draw;
    userevent.data2 = pointer;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return interval;
}

int main(int argc, char *argv[]) {
    e.x = 2.05;
    e.y = 2.05;
    e.direction = 0.25 * M_PI;

    SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER);

    SDL_Window *window = SDL_CreateWindow("Raycasting", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, 0);
    if (window == NULL) printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RaiseWindow(window);

    // Draw loop, runs (fps) times every second.
    int fps = 60;
    SDL_AddTimer(1000 / fps, drawCallback, renderer);

    SDL_Event event;
    while (1) {
        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_USEREVENT: {
                void (*p)(void *) = event.user.data1;
                p(event.user.data2);
                break;
            }

            case SDL_QUIT: {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
        }
    }
}