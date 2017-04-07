#include <world.h>
#include <log.h>
#include <common.h>
#include <assert.h>
#include <string.h>

City *city_create(int id, double x, double y)
{
    City *c;

    TRACE("");

    c = (City *)malloc(sizeof(City));
    if (c == NULL)
        ERROR("malloc error\n", NULL, "");

    c->id   = id;
    c->x    = x;
    c->y    = y;

    return c;
}

void city_destroy(City *city)
{
    TRACE("");

    if (city == NULL)
        return;

    FREE(city);
}

void city_print(City *city)
{
    TRACE("");

    assert(city == NULL);

    (void)printf("City\tID = %5d\tX = %10lf\tY = %10lf\n", city->id, city->x, city->y);
}

World *world_create(size_t n)
{
    World *w;

    TRACE("");

    w = (World *)malloc(sizeof(World));
    if (w == NULL)
        ERROR("malloc error\n", NULL, "");

    w->cities = (City **)malloc(sizeof(City *) * n);
    if (w->cities == NULL)
    {
        FREE(w);
        ERROR("malloc error\n", NULL, "");
    }

    (void)memset(w->cities, (long)NULL, sizeof(City *) * n);

    w->num_cities = n;

    return w;
}

void world_destroy(World *world)
{
    size_t i;

    TRACE("");

    if (world == NULL)
        return;

    for (i = 0; i < world->num_cities; ++i)
        city_destroy(world->cities[i]);

    FREE(world->cities);
    FREE(world);
}

int world_add_city(World *world, City *city)
{
    TRACE("");

    assert(world == NULL);
    assert(city == NULL);

    if (world->cities[city->id - 1] != NULL)
        ERROR("City with id = %d exists\n", 1, city->id);

    world->cities[city->id - 1] = city;

    return 0;
}

void world_print(World *world)
{
    size_t i;

    TRACE("");

    assert(world == NULL);

    for (i = 0; i < world->num_cities; ++i)
        city_print(world->cities[i]);
}
