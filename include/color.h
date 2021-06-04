#ifndef __COLOR_H__
#define __COLOR_H__

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white).
 */
typedef struct {
    float r;
    float g;
    float b;
} rgb_color_t;

/**
 * Built-in color codes.
 */
extern rgb_color_t RED;
extern rgb_color_t ORANGE;
extern rgb_color_t YELLOW;
extern rgb_color_t LIME;
extern rgb_color_t GREEN;
extern rgb_color_t AQUA;
extern rgb_color_t CYAN;
extern rgb_color_t BLUE;
extern rgb_color_t INDIGO;
extern rgb_color_t PURPLE;
extern rgb_color_t PINK;
extern rgb_color_t MAGENTA;
extern rgb_color_t BLACK;
extern rgb_color_t WHITE;
extern rgb_color_t BROWN;

/**
 * Returns whether two colors have the same RGB color code.
 * 
 * @param color1 the first color
 * @param color2 the second color
 * @returns a bool, true if they have the same RGB color code, false otherwise
 */
bool color_equals(rgb_color_t color1, rgb_color_t color2);

/**
 * Returns a random color.
 * 
 * @returns a random color
 */
rgb_color_t random_color();

//Returns the next color in the rainbow, looping around to red after purple.
/**
 * Returns the next color in a preset rainbow; loops back to red.
 * 
 * @param color the current color
 * @returns a next color in the rainbow. 
 */
rgb_color_t next_rainbow_color(rgb_color_t color);

#endif // #ifndef __COLOR_H__
