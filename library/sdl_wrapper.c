#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include "sdl_wrapper.h"

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;
const rgb_color_t BACKGROUND = {28, 163, 236};

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;

/**
 * The keypress handler, or NULL if none has been configured.
 */
event_handler_t key_handler = NULL;
/**
 * The mousepress handler, or NULL if none has been configured.
 */
event_handler_t mouse_handler = NULL;

/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
    int *width = malloc(sizeof(*width)),
        *height = malloc(sizeof(*height));
    assert(width != NULL);
    assert(height != NULL);
    SDL_GetWindowSize(window, width, height);
    vector_t dimensions = {.x = *width, .y = *height};
    free(width);
    free(height);
    return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
    // Scale scene so it fits entirely in the window
    double x_scale = window_center.x / max_diff.x,
           y_scale = window_center.y / max_diff.y;
    return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
    // Scale scene coordinates by the scaling factor
    // and map the center of the scene to the center of the window
    vector_t scene_center_offset = vec_subtract(scene_pos, center);
    double scale = get_scene_scale(window_center);
    vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
    vector_t pixel = {
        .x = round(window_center.x + pixel_center_offset.x),
        // Flip y axis since positive y is down on the screen
        .y = round(window_center.y - pixel_center_offset.y)
    };
    return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT:  return LEFT_ARROW;
        case SDLK_UP:    return UP_ARROW;
        case SDLK_RIGHT: return RIGHT_ARROW;
        case SDLK_DOWN:  return DOWN_ARROW;
        case SDLK_a:  return LEFT_ARROW;
        case SDLK_w:    return UP_ARROW;
        case SDLK_d: return RIGHT_ARROW;
        case SDLK_s:  return DOWN_ARROW;
        default:
            // Only process 7-bit ASCII characters
            return key == (SDL_Keycode) (char) key ? key : '\0';
    }
}

char get_mousecode(SDL_MouseButtonEvent mouse) {
    switch (mouse.button) {
        case SDL_BUTTON_LEFT:   return LEFT_CLICK;
        case SDL_BUTTON_RIGHT:  return RIGHT_CLICK;
        case SDL_BUTTON_MIDDLE: return SCROLL_CLICK;
    }
}

sprite_t *sprite_image(char *image, double scale, SDL_Rect *in){
    sprite_t *sprite = malloc(sizeof(sprite_t));
    sprite->texture = IMG_LoadTexture(renderer, image);
    int *w = malloc(sizeof(int));
    int *h = malloc(sizeof(int));
    SDL_QueryTexture(sprite->texture, NULL, NULL, w, h);
    sprite->scale = scale;
    sprite->frames = 0;
    sprite->speed = 0;
    sprite->dt = 0;
    sprite->clock = 0;
    if (in == NULL){
        sprite->section = malloc(sizeof(SDL_Rect));
        *sprite->section = (SDL_Rect){0, 0, *w, *h};
    }
    else {
        assert(in->w <= *w && *w >= 0);
        assert(in->h <= *h && *h >= 0);
        sprite->section = in;
    }
    free(w);
    free(h);
    return sprite;
}

sprite_t *sprite_animated(char *image, double scale, int frames, int fps){
    sprite_t *sprite = sprite_image(image, scale, NULL);
    printf("%d %d\n", sprite->section->w, sprite->section->h );
    *sprite->section = (SDL_Rect){0, 0, sprite->section->w / frames, sprite->section->h};
    printf("%d %d\n", sprite->section->w, sprite->section->h );
    sprite->frames = frames;
    sprite->speed = fps;
    return sprite;
}

sprite_t *sprite_scroll(char *image, int scroll, SDL_Rect *in){
    sprite_t *sprite = sprite_image(image, 1, in);
    sprite->speed = scroll;
    return sprite;
}

void sprite_free(sprite_t *sprite){
    SDL_DestroyTexture(sprite->texture);
    free(sprite->section);
    free(sprite);
}

void sprite_set_speed(sprite_t *sprite, int speed){
    sprite->speed = speed;
}

void sprite_set_dt(sprite_t *sprite, double dt){
    sprite->dt = dt;
}


SDL_Renderer *sdl_init(vector_t min, vector_t max) {
    // Check parameters
    assert(min.x < max.x);
    assert(min.y < max.y);

    center = vec_multiply(0.5, vec_add(min, max));
    max_diff = vec_subtract(max, center);
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, 0);
    return renderer;
}

bool sdl_is_done(void *scene) {
    SDL_Event *event = malloc(sizeof(*event));
    assert(event != NULL);
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT:
                free(event);
                return true;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Skip the keypress if no handler is configured
                // or an unrecognized key was pressed
                if (key_handler == NULL) break;
                char key = get_keycode(event->key.keysym.sym);
                if (key == '\0') break;

                uint32_t timestamp = event->key.timestamp;
                if (!event->key.repeat) {
                    key_start_timestamp = timestamp;
                }
                key_event_type_t type =
                    event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
                double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
                key_handler(key, (void *)type, held_time, scene);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                if (mouse_handler == NULL) break;
                char button = get_mousecode(event->button);
                if (button == '\0') break;
                                
                mouse_event_type_t click =
                    event->type == SDL_MOUSEBUTTONUP ? BUTTON_PRESSED : BUTTON_RELEASED;
                mouse_handler(button, (void *)click, 0, scene);
                break;
        } 
    }
    free(event);
    return false;
}

void sdl_clear(void) {
    SDL_SetRenderDrawColor(renderer, BACKGROUND.r, BACKGROUND.g, BACKGROUND.b, 255);
    SDL_RenderClear(renderer);
}

void sdl_draw_polygon(body_t *body, rgb_color_t *color) {
    // Check parameters
    list_t *points = body_get_shape(body);
    size_t n = list_size(points);
    assert(n >= 3);
    assert(0 <= (*color).r && color->r <= 1);
    assert(0 <= (*color).g && color->g <= 1);
    assert(0 <= (*color).b && color->b <= 1);

    vector_t window_center = get_window_center();

    // Convert each vertex to a point on screen
    int16_t *x_points = malloc(sizeof(*x_points) * n),
            *y_points = malloc(sizeof(*y_points) * n);
    assert(x_points != NULL);
    assert(y_points != NULL);
    for (size_t i = 0; i < n; i++) {
        vector_t *vertex = list_get(points, i);
        vector_t pixel = get_window_position(*vertex, window_center);
        x_points[i] = pixel.x;
        y_points[i] = pixel.y;
    }

    // Draw polygon with the given color
    filledPolygonRGBA(
        renderer,
        x_points, y_points, n,
        (*color).r * 255, (*color).g * 255, (*color).b * 255, 255
    );
    free(x_points);
    free(y_points);
}

void sdl_draw_image(body_t *body, sprite_t *sprite) {
    vector_t window_center = get_window_center();
    vector_t center = get_window_position(body_get_centroid(body), window_center);
    SDL_Rect *out = malloc(sizeof(SDL_Rect));
    *out = (SDL_Rect) {center.x - sprite->scale * sprite->section->w/2,
                       center.y - sprite->scale * sprite->section->h/2, 
                       sprite->scale * sprite->section->w, 
                       sprite->scale * sprite->section->w};
    SDL_RenderCopy(renderer, sprite->texture, sprite->section, out);
    free(out);
}

void sdl_draw_animated(body_t *body, sprite_t *sprite){
    double time  = (double)clock() /CLOCKS_PER_SEC;
    vector_t window_center = get_window_center();
    vector_t center = get_window_position(body_get_centroid(body), window_center);
    int frame = (int)(time * sprite->speed) % sprite->frames;
    assert((frame < sprite->frames) && (frame >= 0));
    int width = sprite->section->w;
    SDL_Rect *out = malloc(sizeof(SDL_Rect));
    *sprite->section = (SDL_Rect) { frame * (width), 0, width, sprite->section->h };
    *out = (SDL_Rect) {center.x - (sprite->scale * width/2),
                       center.y - (sprite->scale* sprite->section->h/2), 
                       sprite->scale * width, 
                       sprite->scale * sprite->section->h};
    SDL_RenderCopy(renderer, sprite->texture, sprite->section, out);
    free(out);
}

void sdl_draw_scroll(body_t *body, sprite_t *sprite){
    double time  = (double)clock() /CLOCKS_PER_SEC;
    if(sprite->dt == 0){
        sprite->clock = time;
        sprite->dt = 0.00000001;
        *sprite->section = (SDL_Rect) {sprite->frames,
                                       0, 
                                       sprite->section->w, 
                                       sprite->section->h};
    }
    else{
        sprite->dt = time - sprite->clock;
    }
    int *w = malloc(sizeof(int));
    SDL_QueryTexture(sprite->texture, NULL, NULL, w, NULL);
    int width = sprite->section->w;
    int frame = (int)( sprite->dt * sprite->speed + sprite->section->x) % 
                (*w - width);
    printf("%f %d\n", (sprite->dt * sprite->speed) , sprite->section->x);
    assert((frame < *w - width) && (frame >= 0));
    sprite->frames = frame;
    SDL_Rect *in = malloc(sizeof(SDL_Rect));
    *in = (SDL_Rect) {frame, 0, width, sprite->section->h};   
    SDL_RenderCopy(renderer, sprite->texture, in, NULL);
}

void sdl_show(void) {
    // Draw boundary lines
    vector_t window_center = get_window_center();
    vector_t max = vec_add(center, max_diff),
             min = vec_subtract(center, max_diff);
    vector_t max_pixel = get_window_position(max, window_center),
             min_pixel = get_window_position(min, window_center);
    SDL_Rect *boundary = malloc(sizeof(*boundary));
    boundary->x = min_pixel.x;
    boundary->y = max_pixel.y;
    boundary->w = max_pixel.x - min_pixel.x;
    boundary->h = min_pixel.y - max_pixel.y;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, boundary);
    free(boundary);
    SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene) {
    sdl_clear();
    size_t body_count = scene_bodies(scene);
    for (size_t i = 0; i < body_count; i++) {
        body_t *body = scene_get_body(scene, i);
        body_draw(body);
    }
    sdl_show();
}


void sdl_on_key(event_handler_t handler) {
    key_handler = handler;
}

void sdl_on_click(event_handler_t handler) {
    mouse_handler = handler;
}

vector_t sdl_mouse_pos(){
    vector_t window_center = get_window_center();
    int *mouse_x = malloc(sizeof(int));
    int *mouse_y = malloc(sizeof(int));
    SDL_GetMouseState(mouse_x, mouse_y);
    vector_t mouse = (vector_t){*mouse_x, *mouse_y};
    free(mouse_x);
    free(mouse_y);
    return get_window_position(mouse, window_center);

}

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock
        ? (double) (now - last_clock) / CLOCKS_PER_SEC
        : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}
