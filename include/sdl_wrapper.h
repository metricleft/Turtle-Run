#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include <stdbool.h>
#include "color.h"
#include "list.h"
#include "scene.h"
#include "vector.h"

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
void sdl_init(vector_t min, vector_t max);

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
 * @param points the list of vertices of the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(list_t *points, rgb_color_t color);

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

#endif // #ifndef __SDL_WRAPPER_H__
