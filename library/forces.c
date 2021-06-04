#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "forces.h"
#include "collision.h"
#include "entity.h"

//Gravity is not applied when two bodies are closer than this distance to each other.
const double SMALL_DISTANCE = 10;
//Normal force is not applied when two bodies are closer than this distance to each other.
const double SMALL_VALUE = 1e-6;

/**
 * Contains the information for a force creator,
 * and is used as input for the force handler.
 */
typedef struct param {
    void *constant;
    body_t *body1;
    body_t *body2;
    free_func_t const_freer;
} param_t;

/**
 * Frees a param_t, which is used as input for force handlers.
 */
void param_free(param_t *param) {
    param->const_freer(param->constant);
    free(param);
}

double calculate_reduced_mass(body_t *body1, body_t *body2) {
    double reduced_mass;
    if (body_get_mass(body1) == INFINITY) {
        reduced_mass = body_get_mass(body2);
    } else if (body_get_mass(body2) == INFINITY) {
        reduced_mass = body_get_mass(body1);
    } else {
        reduced_mass = body_get_mass(body1)*body_get_mass(body2) /
            (body_get_mass(body1)+body_get_mass(body2));
    }
    return reduced_mass;
}
 
/**
 * Gravity force handler for newtonian gravity between 2 bodies.
 * 
 * @param aux a param_t containing the information for the force.
 */
void gravity_creator(param_t *aux){
    vector_t r = vec_subtract(body_get_centroid(aux->body1),
                              body_get_centroid(aux->body2));
    double mass_product = body_get_mass(aux->body1)* body_get_mass(aux->body2);
    vector_t force = VEC_ZERO;
    if (sqrt(vec_dot(r, r)) > SMALL_DISTANCE){
        force = vec_multiply(- *(double *)aux->constant * mass_product /
                             pow(sqrt(vec_dot(r,r)), 3.0) , r);
    }
    body_add_force(aux->body1, force);
    body_add_force(aux->body2, vec_negate(force));
}

void create_newtonian_gravity(scene_t *scene, void *G, body_t *body1, body_t *body2,
                              free_func_t freer){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){G, body1, body2, freer};
    list_t *bodies = list_init(2, body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, gravity_creator, force_param, bodies,
                                   param_free);
}

/**
 * Force handler for a constant magnitude force that acts on a body
 * all the time.
 * 
 * @param aux auxilary param containing the information for the force.
 */
void const_force_creator(param_t *aux){
    vector_t force = vec_multiply(body_get_mass(aux->body1), *(vector_t *) aux->constant);
    body_add_force(aux->body1, force);
}

void create_constant_force(scene_t *scene, void *A, body_t *body, free_func_t freer){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){A, body, NULL, freer};
    list_t *bodies = list_init(1, body_free);
    list_add(bodies, body);
    scene_add_bodies_force_creator(scene, const_force_creator, force_param, bodies,
                                   param_free);
}

/**
 * Force handler for a spring force between 2 bodies.
 * 
 * @param aux auxilary parameter containing the information for the force.
 */
void spring_creator(param_t *aux){
    vector_t r = vec_subtract(body_get_centroid(aux->body1),
                              body_get_centroid(aux->body2));
    vector_t force = vec_multiply(-1 * *(double *) aux->constant, r);
    body_add_force(aux->body1, force);
    body_add_force(aux->body2, vec_negate(force));
} 

void create_spring(scene_t *scene, void *k, body_t *body1, body_t *body2,
                   free_func_t freer){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){k, body1, body2, freer};
    list_t *bodies = list_init(2, body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, spring_creator, force_param, bodies,
                                   param_free);
}

/**
 * Force handler for a drag force that is proportional to velocity
 * and acts opposite direction of travel.
 * 
 * @param aux auxilary parameter containing the information for the force.
 */
void drag_creator(param_t *aux){
    vector_t force = vec_multiply(- *(double *) aux->constant,
                                  body_get_velocity(aux->body1));
    body_add_force(aux->body1, force);
}

void create_drag(scene_t *scene, void *gamma, body_t *body, free_func_t freer){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){ gamma, body, NULL, freer};
    list_t *bodies = list_init(1, body_free);
    list_add(bodies, body);
    scene_add_bodies_force_creator(scene, drag_creator, force_param, bodies, param_free);
}

/**
 * Struct containing the information of a collision, is passed to the force
 * creator and the collision handler is called on the bodies.
 */
typedef struct {
    collision_handler_t handler;
    body_t *body1;
    body_t *body2;
    void *aux;
    bool collided;
    free_func_t aux_freer;
} collision_param_t;

/**
 * Frees a collision parameter.
 */
void collision_param_free(collision_param_t *param) {
    if (param->aux != NULL) {
        param->aux_freer(param->aux);
    }
    free(param);
}

/**
 * Force creator that calls the collision handler if 2 objects are collided.
 * 
 * @param param a collision parameter that contains the information of the
 *      collision.
 */
void collision_force_creator(collision_param_t *param) {
    list_t *shape1 = body_get_shape(param->body1);
    list_t *shape2 = body_get_shape(param->body2);
    collision_info_t collision = find_collision(shape1, shape2);
    if (collision.collided && !(param->collided)) {
        param->handler(param->body1, param->body2, collision.axis, param->aux);
        param->collided = true;
    } else if (!collision.collided) {
        param->collided = false;
    }
    free(shape1);
    free(shape2);
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
            collision_handler_t handler, void *aux, free_func_t freer) {
    list_t *bodies = list_init(2, body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    collision_param_t *force_param = malloc(sizeof(collision_param_t));
    *force_param = (collision_param_t) {handler, body1, body2, aux, false,
                                        freer};
    scene_add_bodies_force_creator(scene, collision_force_creator, force_param,
                                        bodies, collision_param_free);
}

/**
 * Collision handler for a collision between 2 bodies; applies an impulse
 * to both bodies that resolves the collision.
 * 
 * @param body1 the first body
 * @param body2 the second body
 * @param axis the axis of the collision
 * @param aux auxilary value that contains the elasticity of the collision.
 */
void physics_collision_handler(body_t *body1, body_t *body2,
                                vector_t axis, void *aux) {
    double reduced_mass = calculate_reduced_mass(body1, body2);
    vector_t impulse = 
                vec_multiply(
                    reduced_mass * 
                    (1.0 + *(double *) aux) *
                    (vec_dot(body_get_velocity(body2),axis)-
                    vec_dot(body_get_velocity(body1),axis)),
                    axis);
    if (body_get_mass(body1) != INFINITY) {
        body_add_impulse(body1, impulse);
    }
    if (body_get_mass(body2) != INFINITY) {
        body_add_impulse(body2, vec_negate(impulse));
    }
}

/**
 * Struct containing the information for normal collisions.
 */
typedef struct normal_param {
    normal_handler_t handler;
    body_t *body1;
    body_t *body2;
    void *aux;
    bool collided;
    free_func_t aux_freer;
} normal_param_t;

/**
 * Frees the information for a normal collision.
 */
void normal_param_free(normal_param_t *param) {
    param->aux_freer(param->aux);
    free(param);
}

/**
 * Force handler for normal forces.
 * 
 * @param param parameter containing the information for the normal collision.
 */
void normal_handler(normal_param_t *param){
    list_t *shape1 = body_get_shape(param->body1);
    list_t *shape2 = body_get_shape(param->body2);

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

    if (body_get_velocity(param->body1).y < 0 &&
            shape1_max.x > shape2_min.x && shape1_min.x < shape2_max.x &&
            shape1_min.y > shape2_max.y &&
            shape1_min.y - shape2_max.y < SMALL_DISTANCE) {
        player_entity_t *entity1 = body_get_info(param->body1); //This must be the player.
        entity_set_colliding(entity1, true);
        vector_t new_centroid = {body_get_centroid(param->body1).x,
                shape2_max.y + 0.5*(shape1_max.y - shape1_min.y)+SMALL_DISTANCE};
        body_set_centroid(param->body1, new_centroid);
        vector_t new_velocity = {body_get_velocity(param->body1).x, 0};
        body_set_velocity(param->body1, new_velocity);
        vector_t force = vec_multiply(body_get_mass(param->body1),
                                          *(vector_t *) param->aux);
        body_add_force(param->body1, force);
    } else {
        if (!strcmp(entity_get_type(body_get_info(param->body2)),"TERRAIN")){
            collision_info_t collision = find_collision(shape1,shape2);
            if (collision.collided) {
                if (fabs(collision.axis.y) < SMALL_VALUE){
                
                    if (body_get_centroid(param->body1).x <
                            body_get_centroid(param->body2).x) {
                                body_translate(param->body1,
                                        vec_multiply(collision.overlap,
                                            vec_negate(collision.axis)));
                    } else {
                            body_translate(param->body1,
                                    vec_multiply(collision.overlap,
                                        (collision.axis)));
                    }
                    vector_t old_velocity = body_get_velocity(param->body1);
                    vector_t terrain_vel = body_get_velocity(param->body2);
                    body_set_velocity(param->body1, (vector_t) {
                            terrain_vel.x, old_velocity.y});
                } else {
                    body_translate(param->body1,
                        vec_multiply(collision.overlap, collision.axis));
                    vector_t old_velocity = body_get_velocity(param->body1);
                    body_set_velocity(param->body1, (vector_t) {
                            old_velocity.x, 0});
                }
            }
        }
    }
    free(shape1);
    free(shape2);
}

/**
 * Collision handler for a one way destrcutive collision, where the second
 * body is destroyed.
 * 
 * @param body1 the first body, which is bounced off
 * @param body2 the second body, which is destroyed on collision.
 * @param axis the axis of the collision.
 * @param aux auxilary value containing the elasticity of the collision.
 */
void one_way_destroy_handler(body_t *body1, body_t *body2,
                                vector_t axis, void *aux) {
    physics_collision_handler(body1, body2, axis, aux);
    body_remove(body2);
}

/**
 * Collision handler for a two way destrcutive collision, where both bodies
 * are destroyed
 * 
 * @param body1 the first body
 * @param body2 the second body
 * @param axis the axis of the collision.
 * @param aux auxilary value containing the elasticity of the collision.
 */
void two_way_destroy_handler(body_t *body1, body_t *body2,
                                vector_t axis, void *aux) {
    body_remove(body1);
    body_remove(body2);
}

void create_destructive_collision(scene_t *scene, body_t *body1, body_t *body2){
    create_collision(scene, body1, body2, two_way_destroy_handler,
                        NULL, free);
}

void create_oneway_destructive_collision(scene_t *scene, double elasticity, 
                                            body_t *body1, body_t *body2) {
    double *elasticity_param = malloc(sizeof(double));
    *elasticity_param = elasticity;
    create_collision(scene, body1, body2, one_way_destroy_handler,
                        elasticity_param, free);
}

void create_physics_collision(scene_t *scene, double elasticity,
                                    body_t *body1, body_t *body2) {
    double *elasticity_param = malloc(sizeof(double));
    *elasticity_param = elasticity;
    create_collision(scene, body1, body2, physics_collision_handler,
                        elasticity_param, free);
}

void create_normal_collision(scene_t *scene, vector_t grav,
                                    body_t *body1, body_t *body2) {
    vector_t *grav_param = malloc(sizeof(vector_t));
    *grav_param = grav;
    list_t *bodies = list_init(2, body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    normal_param_t *force_param = malloc(sizeof(normal_param_t));
    *force_param = (normal_param_t) {normal_handler, body1,
                                        body2, grav_param, false, free};
    scene_add_bodies_force_creator(scene, normal_handler, force_param,
                                        bodies, normal_param_free);
}
