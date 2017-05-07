#ifndef WORLD_H
#define WORLD_H

/*
    Our world is set of cities

    Author: Michal Kukowski
    email: michalkukowski10@gmail.com

    LICENCE: GPL3.0
*/

#include <stddef.h>
#include <compiler.h>
#include <math.h>

typedef struct City
{
    int     id;
    double  x;
    double  y;
}City;

typedef struct World
{
    size_t   num_cities;
    City     **cities; /* sorted city array by id: cities(k) has City with id = K */
}World;

/*
    Create city

    PARAMS
    @IN id - id
    @IN x - x pos
    @IN y - y pos

    RETURN
    NULL iff failure
    Pointer to City iff success
*/
City *city_create(int id, double x, double y);

/*
    Destroy City

    PARAMS
    @IN city - pointer to city

    RETURN
    This is void function
*/
void city_destroy(City *city);

/*
    Show City

    PARAMS
    @IN city - city

    RETURN
    This is void function
*/
void city_print(City *city);

/*
    Return euclidean distance between city C1 and C2
*/
double __inline__ __nonull__(1, 2) city_euclidean_dist(City *c1, City *c2)
{
    double x;
    double y;

    x = c1->x - c2->x;
    y = c1->y - c2->y;
    return sqrt((x * x) + (y * y));
}

/*
    Create world with @N cities

    PARAMS
    @IN n - number of cities in world

    RETURN
    NULL iff failure
    Pointer to City iff success
*/
World *world_create(size_t n);

/*
    Destroy world

    PARAMS
    @IN world - pointer to World

    RETURN
    This is void function
*/
void world_destroy(World *world);

/*
    Add city to World

    PARAMS
    @IN world - pointer to world
    @IN city - pointer to city

    RETURN
    0 iff success
    Non-zero value iff failure
*/
int world_add_city(World *world, City *city);

/*
    Show World ( All cities )

    PARAMS
    @IN world - world

    RETURN
    This is void function
*/
void world_print(World *world);

#endif
