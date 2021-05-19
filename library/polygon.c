#include <math.h>
#include <stdlib.h>
#include "polygon.h"

double polygon_area(list_t *polygon) {
    double tot = 0;
    size_t size = list_size(polygon);
    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t cur = *(vector_t *)list_get(polygon, i);
        vector_t next = *(vector_t *)list_get(polygon, (i+1) % size);
        tot += vec_cross(cur, next);
    }
    return tot / 2.0;
}

vector_t polygon_centroid(list_t *polygon) {
    vector_t centroid = VEC_ZERO;
    size_t size = list_size(polygon);
    for (size_t i = 0; i < size; i++) {
        vector_t cur = *(vector_t *)list_get(polygon, i);
        vector_t next = *(vector_t *)list_get(polygon, (i+1) % size);
        double intermediate = vec_cross(cur, next);
        centroid.x += (cur.x + next.x) * intermediate;
        centroid.y += (cur.y + next.y) * intermediate;
    }
    double area = polygon_area(polygon);
    centroid.x /= 6 * area;
    centroid.y /= 6 * area;
    return centroid;
}

void polygon_translate(list_t *polygon, vector_t translation) {
    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t translated = vec_add(*(vector_t *)list_get(polygon, i), translation);
        ((vector_t *)list_get(polygon, i))->x = translated.x;
        ((vector_t *)list_get(polygon, i))->y = translated.y;
    }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
    polygon_translate(polygon, vec_negate(point));
    for (size_t i = 0; i < list_size(polygon); i++) {
        vector_t rotated = vec_rotate(*(vector_t *)list_get(polygon, i), angle);
        ((vector_t *)list_get(polygon, i))->x = rotated.x;
        ((vector_t *)list_get(polygon, i))->y = rotated.y;
    }
    polygon_translate(polygon, point);
}
