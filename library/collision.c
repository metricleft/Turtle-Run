#include <stdlib.h>
#include <math.h>
#include "collision.h"
#include "vector.h"
#include "polygon.h"
#include <assert.h>

list_t *get_axis(list_t *shape){
    list_t *axis_list =  list_init(list_size(shape), free);
    for (size_t i = 0; i < list_size(shape); i++){
        vector_t *axis = malloc(sizeof(vector_t));
        vector_t tangent = vec_subtract(
                *((vector_t *) list_get(shape, (i + 1) % list_size(shape))),
                *((vector_t *) list_get(shape, i)));
        tangent = vec_unit(tangent);
        *axis = (vector_t) {tangent.y, -tangent.x};
        list_add(axis_list, axis);
    }
    return axis_list;
}

typedef struct {
    double min;
    double max;
} min_max_t;

min_max_t shape_project(vector_t *axis, list_t *shape){
    min_max_t ret = {vec_dot(*((vector_t *) list_get(shape,0)),*axis),
                vec_dot(*((vector_t *) list_get(shape,0)),*axis)};
    for (size_t i = 1; i < list_size(shape); i++){
        double projection = vec_dot(*((vector_t *) list_get(shape,i)),*axis);
        ret.min  = fmin(ret.min, projection);
        ret.max  = fmax(ret.max, projection);
    }
    return ret;
}

typedef struct {
    bool collided;
    double overlap;
    vector_t axis;
} overlap_return_t;

overlap_return_t overlap(list_t *axis, list_t *shape1, list_t *shape2){
    //if there is an axis that separates the projections
    double min_overlap = INFINITY;
    vector_t min_axis = {INFINITY, INFINITY};
    for(int i = 0; i < list_size(axis); i++){
        min_max_t shape1_minmax = shape_project(list_get(axis,i),shape1);
        min_max_t shape2_minmax = shape_project(list_get(axis,i),shape2);
        //If there is an axis separating them
        if (!((shape2_minmax.max > shape1_minmax.min) &&
                (shape1_minmax.max > shape2_minmax.min))) {
                return (overlap_return_t) {false, min_overlap, min_axis};
        }
        double overlap = fmin(shape1_minmax.max, shape2_minmax.max)
                            - fmax(shape1_minmax.min, shape2_minmax.min);
        if (min_overlap > overlap) {
            assert(overlap > 0);
            min_overlap = overlap;
            min_axis = *((vector_t *) list_get(axis,i)); 
        }
    }
    return (overlap_return_t) {true, min_overlap, min_axis};
}


collision_info_t find_collision(list_t *shape1, list_t *shape2){
    vector_t shape1_min = *(vector_t *) list_get(shape1,0);
    vector_t shape1_max = *(vector_t *) list_get(shape1,0);

    vector_t shape2_min = *(vector_t *) list_get(shape2,0);
    vector_t shape2_max = *(vector_t *) list_get(shape2,0);

    for (int i = 0; i < list_size(shape1); i++) {
        shape1_min.x = fmin(shape1_min.x, (*(vector_t *) list_get(shape1,i)).x);
        shape1_max.x = fmax(shape1_max.x, (*(vector_t *) list_get(shape1,i)).x);
        shape1_min.y = fmin(shape1_min.y, (*(vector_t *) list_get(shape1,i)).y);
        shape1_max.y = fmax(shape1_max.y, (*(vector_t *) list_get(shape1,i)).y);
    }
    for (int i = 0; i < list_size(shape2); i++) {
        shape2_min.x = fmin(shape2_min.x, (*(vector_t *) list_get(shape2,i)).x);
        shape2_max.x = fmax(shape2_max.x, (*(vector_t *) list_get(shape2,i)).x);
        shape2_min.y = fmin(shape2_min.y, (*(vector_t *) list_get(shape2,i)).y);
        shape2_max.y = fmax(shape2_max.y, (*(vector_t *) list_get(shape2,i)).y);
    }

    if (shape1_max.x < shape2_min.x || shape1_max.y < shape2_min.y ||
                shape2_max.x < shape1_min.x || shape2_max.y < shape1_min.y) {
                    return (collision_info_t) {false, VEC_ZERO, 0.};
                }
    
    /*
    vector_t shape1_centroid = polygon_centroid(shape1);
    vector_t shape2_centroid = polygon_centroid(shape2);

    double shape1_r = 0;
    double shape2_r = 0;
    for (int i = 0; i < list_size(shape1); i++) {
        
        shape1_r = fmax(shape1_r, 
                vec_mag(vec_subtract(*(vector_t *) list_get(shape1,i), 
                                        shape1_centroid)));
    }
    for (int i = 0; i < list_size(shape2); i++) {
        shape2_r = fmax(shape2_r, 
                vec_mag(vec_subtract(*(vector_t *) list_get(shape2,i), 
                                        shape2_centroid)));
    }
    if (vec_mag(vec_subtract(shape1_centroid, shape2_centroid)) > 
            shape1_r + shape2_r) {
                return (collision_info_t) {false, VEC_ZERO};
    }
    */

    list_t *axis1 = get_axis(shape1);
    list_t *axis2 = get_axis(shape2);
    overlap_return_t shape1_overlap = overlap(axis1, shape1, shape2);
    if (!shape1_overlap.collided) {
        return (collision_info_t) {false, VEC_ZERO, 0};
    }
    overlap_return_t shape2_overlap = overlap(axis2, shape1, shape2);
    list_free(axis1);
    list_free(axis2);
    //if there is a separating axis from either
    if (shape1_overlap.collided && shape2_overlap.collided) {
        if (shape1_overlap.overlap < shape2_overlap.overlap) {
            return (collision_info_t) {true, vec_unit(shape1_overlap.axis),
                                        shape1_overlap.overlap};
        } else {
            return (collision_info_t) {true,
                                    vec_unit(vec_negate(shape2_overlap.axis)),
                                        shape2_overlap.overlap};
        }
    } else {
        return (collision_info_t) {false, VEC_ZERO, 0.};
    }
}