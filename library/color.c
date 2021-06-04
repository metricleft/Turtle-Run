#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "color.h"
#include "list.h"

const int MAX_COLOR = 255;

rgb_color_t RED = {1, 0, 0};
rgb_color_t ORANGE = {1, 0.5, 0};
rgb_color_t YELLOW = {1, 1, 0};
rgb_color_t LIME = {0.5, 1, 0};
rgb_color_t GREEN = {0, 1, 0};
rgb_color_t AQUA = {0, 1, 0.5};
rgb_color_t CYAN = {0, 1, 1};
rgb_color_t BLUE = {0, 0.5, 1};
rgb_color_t INDIGO = {0, 0, 1};
rgb_color_t PURPLE = {0.5, 0, 1};
rgb_color_t PINK = {1, 0, 1};
rgb_color_t MAGENTA = {1, 0, 0.5};
rgb_color_t BLACK = {0, 0, 0};
rgb_color_t WHITE = {1, 1, 1};
rgb_color_t BROWN = {0.361, 0.173, 0.024};

bool color_equals(rgb_color_t color1, rgb_color_t color2) {
    return color1.r == color2.r && color1.g == color2.g && color1.b == color2.b;
}

rgb_color_t random_color() {
    rgb_color_t out = (rgb_color_t) {(float) (rand()%MAX_COLOR)/MAX_COLOR,
                                     (float) (rand()%MAX_COLOR)/MAX_COLOR,
                                     (float) (rand()%MAX_COLOR)/MAX_COLOR};
    return out;
}

rgb_color_t next_rainbow_color(rgb_color_t color) {
    if (color_equals(color, RED)) {
        return ORANGE;
    }
    else if (color_equals(color, ORANGE)) {
        return YELLOW;
    }
    else if (color_equals(color, YELLOW)) {
        return GREEN;
    }
    else if (color_equals(color, GREEN)) {
        return AQUA;
    }
    else if (color_equals(color, AQUA)) {
        return CYAN;
    }
    else if (color_equals(color, CYAN)) {
        return BLUE;
    }
    else if (color_equals(color, BLUE)) {
        return INDIGO;
    }
    else if (color_equals(color, INDIGO)) {
        return PURPLE;
    }
    else if (color_equals(color, PURPLE)) {
        return PINK;
    }
    else if (color_equals(color, PINK)) {
        return MAGENTA;
    }
    else if (color_equals(color, MAGENTA)) {
        return RED;
    }
    return BLACK;
}
