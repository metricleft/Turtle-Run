#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include <stdbool.h>
#include "list.h"
#include "scene.h"
#include "vector.h"
#include "color.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

/**
 * Contains all the info needed to draw a sprite, including the texture used
 * scaling of the image, and number of frames and fps animating
 */

typedef struct sprite{
    SDL_Texture *texture;
    double scale;
    int frames;
    int speed;
    double dt;
    double clock;
    SDL_Rect *section;
}sprite_t;

/**
 * Creates the info of animated sprite .
 * 
 * @param image file link of image used to create texture 
 * @param scale the scaling of the image
 * @param frames number of frames of animation 
 * @param fps how fast the animation is
 * @return a pointer to info for a sprite
 */
sprite_t *sprite_animated(const char *image, double scale, int frames, int fps);

/**
 * Creates the info of unanimated sprite .
 * 
 * @param image file link of image used to create texture 
 * @param scale the scaling of the image
 * @param in rectangular section of image drawn
 * @return a pointer to info for a sprite
 */
sprite_t *sprite_image(const char *image, double scale, SDL_Rect *in);

/**
 * Creates the info of unanimated sprite .
 * 
 * @param image file link of image used to create texture 
 * @param scroll speed of scrolling
 * @param in initial frame of scroll
 * @return a pointer to info for a sprite
 */
sprite_t *sprite_scroll(const char *image, int scroll, SDL_Rect *in);

/**
 * Releases the memory allocated for sprite.
 *
 * @param body a pointer to a sprite
 */
void sprite_free(sprite_t *sprite);

void sprite_set_speed(sprite_t *sprite, int speed);

void sprite_set_dt(sprite_t *sprite, double dt);

Mix_Music *loadMedia(char *music_name);
Mix_Chunk *loadEffects(char *effect_name);

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
    LEFT_ARROW = 1, 
    UP_ARROW = 2, 
    RIGHT_ARROW = 3, 
    DOWN_ARROW = 4
} arrow_key_t;

// Values passed to a mouse handler when the given button is clicked
typedef enum {
    LEFT_CLICK = 1, 
    RIGHT_CLICK = 2, 
    SCROLL_CLICK = 3 
} mouse_button_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum {
    KEY_PRESSED,
    KEY_RELEASED
} key_event_type_t;

/**
 * The possible types of mouse button events.
 */
typedef enum {
    BUTTON_PRESSED,
    BUTTON_RELEASED
} mouse_event_type_t;

/**
 * A event handler.
 * When a key or button is pressed or released, the handler is passed its char value.
 * 
 * @param event a character indicating which key or button was pressed
 * @param type the type of event (KEY_PRESSED/KEY_RELEASED or BUTTONP_PRESSED/BUTTON_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 * @param scene an aux variable, usually a pointer to a scene_t
 */
typedef void (*event_handler_t)(char event, void *type, double held_time,
                              void *scene);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
SDL_Window *sdl_init(vector_t min, vector_t max);

/**
 * Sets the focused renderer.
 * 
 * @param renderer the new renderer.
 */
void sdl_set_window(SDL_Renderer *window);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle keypresses.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void *scene);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param body body associated with the sprite
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon( body_t *body, rgb_color_t *color);

/**
 * Draws an image from the given info about a sprite.
 *
 * @param body body associated with the sprite
 * @param sprite info needed to draw the image
 */
void sdl_draw_image(body_t *body, sprite_t *sprite);

/**
 * Draws an animation from the given info about a sprite.
 *
 * @param body body associated with the sprite
 * @param sprite info needed to draw the animation
 */
void sdl_draw_animated( body_t *body, sprite_t *sprite);


void sdl_draw_scroll(body_t *body, sprite_t *sprite);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 */
void sdl_render_scene(scene_t *scene);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(event_handler_t handler);


/**
 * Registers a function to be called every time a mouse button is pressed.
 * Overwrites any existing handler.
 *
 * @param handler the function to call with each button press
 */

void sdl_on_click(event_handler_t handler);

/**
 * Gets the current position of the mouse
 *
 * @return vector_t of the mouse position
 */

vector_t sdl_mouse_pos();    

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

/**
 * Draws text.
 * 
 * @param window a pointer to an SDL_Window that the text will be drawn in
 * @param text the text to be drawn
 * @param font the font type
 * @param color the color of the text in RGB
 * @param size the font size of the text
 * @param coords the coordinates of the top-left corner of the text
 */
void sdl_draw_text(SDL_Window *window, char *text, const char *font, rgb_color_t color,
                   int size, vector_t coords);

/**
 * Draws text with an outline.
 * 
 * @param window a pointer to an SDL_Window that the text will be drawn in
 * @param text the text to be drawn
 * @param font the font type
 * @param color the color of the text in RGB
 * @param size the font size of the text
 * @param thickness the thickness of the outline
 * @param coords the coordinates of the top-left corner of the text
 */
void sdl_draw_outlined_text(SDL_Window *window, char *text, const char *font,
                            rgb_color_t color, rgb_color_t outline_color,
                            int size, int thickness, vector_t coords);

/**
 * Gives the width of the text that would be drawn.
 * 
 * @param text the text to be centered
 * @param font the font type
 * @param size the font size of the text
 * @return the width of the text
 */
int sdl_text_width(char *text, const char *font, int size);

/**
 * Gives the x-coordinate that would center the text on the screen.
 * 
 * @param MIN the minimum coordinates of the screen
 * @param MAX the maximum coordinates of the screen
 * @param text the text to be centered
 * @param font the font type
 * @param size the font size of the text
 * @return the x-coordinate of the top-left corner that would center the text
 */
int sdl_text_center(vector_t min, vector_t max, char *text, const char *font, int size);

#endif // #ifndef __SDL_WRAPPER_H__
