#include <math.h>
#include <stdlib.h>
#include "vector.h"

const vector_t VEC_ZERO = {.x = 0, .y = 0};

vector_t *polar_to_cartesian(double r, double theta) {
    vector_t *cartesian = malloc(sizeof(vector_t));
    cartesian->x = r * cos(theta);
    cartesian->y = r * sin(theta);
    return cartesian;
}

vector_t vec_add(vector_t v1, vector_t v2) {
    vector_t return_vector = {.x = v1.x + v2.x, .y = v1.y + v2.y};
    return return_vector;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
    vector_t return_vector = {.x = v1.x - v2.x, .y = v1.y - v2.y};
    return return_vector;
}

vector_t vec_negate(vector_t v) {
    vector_t return_vector = {.x = -v.x, .y = -v.y};
    return return_vector;
}

vector_t vec_multiply(double scalar, vector_t v) {
    vector_t return_vector = {.x = scalar * v.x, .y = scalar * v.y};
    return return_vector;
}

double vec_dot(vector_t v1, vector_t v2) {
    return v1.x*v2.x + v1.y*v2.y;
}

double vec_cross(vector_t v1, vector_t v2) {
    return v1.x*v2.y - v1.y*v2.x;
}

vector_t vec_rotate(vector_t v, double angle) {
    vector_t return_vector = {.x = v.x*cos(angle) - v.y*sin(angle),
                              .y = v.x*sin(angle) + v.y*cos(angle)};
    return return_vector;
}

vector_t vec_unit(vector_t v){
    vector_t unit  = vec_multiply(1 / sqrt(vec_dot(v,v)) , v);
    return unit;
}

double vec_mag(vector_t v){
    double ret = sqrt(vec_dot(v, v));
    return ret;
}
