#include "shapelib.h"

list_t *compute_circle_points(vector_t center, double radius, double arc_resolution) {
     list_t *coords = list_init((size_t) arc_resolution, free);

    double d_theta = (2*M_PI / arc_resolution);
    for (int i = 0; i < arc_resolution; i++) {
        vector_t *next_point = polar_to_cartesian(radius, i*d_theta);
        next_point->x = next_point->x + center.x;
        next_point->y = next_point->y + center.y;
        list_add(coords, next_point);
    }
    return coords;
}

list_t *compute_rect_points(vector_t center, double width, double height) {
    vector_t half_width  = {.x = width / 2, .y = 0.0},
             half_height = {.x = 0.0, .y = height / 2};
    list_t *rect = list_init(4, free);
    vector_t *v = malloc(sizeof(*v));
    *v = vec_add(half_width, half_height);
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_subtract(half_height, half_width);
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_negate(*(vector_t *) list_get(rect, 0));
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_subtract(half_width, half_height);
    list_add(rect, v);

    polygon_translate(rect, center);
    return rect;
}

list_t *compute_sector_points(vector_t center, double radius, double angle,
                              double arc_resolution) {
    list_t *coords = list_init((size_t)(arc_resolution + 2), free);

    vector_t *c = malloc(sizeof(vector_t));
    c->x = center.x;
    c->y = center.y;
    list_add(coords, c);
    
    vector_t *top_mouth = polar_to_cartesian(radius, angle / 2);
    top_mouth->x = top_mouth->x + center.x;
    top_mouth->y = top_mouth->y + center.y;
    list_add(coords, top_mouth);

    double d_theta = (M_PI - angle) / arc_resolution;
    for (int i = 1; i < arc_resolution; i++) {
        vector_t *next_point = polar_to_cartesian(radius, (angle/2) + i*d_theta);
        next_point->x = next_point->x + center.x;
        next_point->y = next_point->y + center.y;
        list_add(coords, next_point);
    }

    vector_t *bottom_mouth = polar_to_cartesian(radius, M_PI - angle / 2);
    bottom_mouth->x = bottom_mouth->x + center.x;
    bottom_mouth->y = bottom_mouth->y + center.y;
    list_add(coords, bottom_mouth);

    return coords;
}
