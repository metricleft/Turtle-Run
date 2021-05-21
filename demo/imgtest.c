#include <time.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_mouse.h"
#include "SDL2/sdl_image.h"

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;

clock_t last_clock = 0;

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock
        ? (double) (now - last_clock) / CLOCKS_PER_SEC
        : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}

void animate(SDL_Texture *texture, int frames, int FPS){
    double time  = (double)clock() /CLOCKS_PER_SEC;
    int tick = MS_PER_S / FPS;
    int *w = malloc(sizeof(int));
    int *h = malloc(sizeof(int));
    SDL_QueryTexture(texture, NULL, NULL, w, h);
    SDL_Rect *in = malloc(sizeof(SDL_Rect));
    SDL_Rect *out = malloc(sizeof(SDL_Rect));
    int frame = ((int)(time * MS_PER_S) / tick) % frames;
    printf("%d\n", frame);
    *in = (SDL_Rect) { frame* (*w/frames), 0, *w/frames, *h };
    *out = (SDL_Rect) {0,0, *w/frames *2, *h*2};
    SDL_RenderCopy(renderer, texture, in, out );
    free(w);
    free(h);
    free(in);
    free(out);
}

int main(int argc, char *argv[]) {   
    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, 0);
    
    SDL_Texture *test = IMG_LoadTexture(renderer,"static/turtle_spritesheet.png");
    double time  = 0.0;
    while (time <= 10) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        time += time_since_last_tick();
        animate(test, 8, 12);
        SDL_RenderPresent (renderer);
        SDL_Delay(50);

    }
    SDL_DestroyTexture( test );
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow ( window );

    SDL_Quit();
}