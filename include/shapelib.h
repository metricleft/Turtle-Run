#ifndef __SHAPELIB_H__
#define __SHAPELIB_H__

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "vector.h"
#include "polygon.h"
#include "list.h"

/**
 * Computes the list of vertices of a circle with a set center, radius, and resolution.
 * 
 * @param center the center of the circle
 * @param radius the radius of the circle
 * @param arc_resolution the number of points that make up the circle
 * @return a list of vector_t vertices representing the points in a circle
 */
list_t *compute_circle_points(vector_t center, double radius, double arc_resolution);

/**
 * Computes the list of vertices of a rectangle with a set center, width, and height.
 * 
 * @param center the center of the rectangle
 * @param width the width of the rectangle
 * @param height the height of the rectangle
 * @return a list of vector_t vertices that define a rectangle
 */
list_t *compute_rect_points(vector_t center, double width, double height);

/**
 * Computes the list of vertices of a sector of a circle with a set center,
 * radius, and angle.
 * Note that the centroid of the sector does not lie at the center.
 * 
 * @param center the center of the sector
 * @param radius the radius of the sector
 * @param angle the angle of the sector
 * @param arc_resolution the number of points that define an arc
 * @return a list of vector_t vertices that define a sector
 */
list_t *compute_sector_points(vector_t center, double radius, double angle,
                              double arc_resolution);

#endif // #ifndef __SHAPELIB_H__
