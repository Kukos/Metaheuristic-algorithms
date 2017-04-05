#include <stdio.h>
#include <log.h>
#include <compiler.h>
#include <world.h>

void __before_main__(0) init(void)
{
    log_init(stdout, LOG_TO_FILE);
}

void __after_main__(0) deinit(void)
{
    log_deinit();
}

World *prepare_world(void)
{
    long n;
    World *world;

    City *city;
    int id;
    double x;
    double y;
    long i;

    /* read num entries */
    if (scanf("%ld", &n) != 1)
        ERROR("scanf error\n", NULL, "");

    world = world_create((size_t)n);
    if (world == NULL)
        ERROR("world_create error\n", NULL, "");

    for (i = 0; i < n; ++i)
    {
        if (scanf("%d %lf %lf", &id, &x, &y) != 3)
        {
            world_destroy(world);
            ERROR("scanf error\n", NULL, "");
        }

        city = city_create(id, x, y);
        if (city == NULL)
        {
            world_destroy(world);
            ERROR("city_create error\n", NULL, "");
        }

        if (world_add_city(world, city))
        {
            world_destroy(world);
            ERROR("world_add_city error\n", NULL, "");
        }
    }

    return world;
}

int main(void)
{
    World *w;

    w = prepare_world();
    world_print(w);

    world_destroy(w);
    return 0;
}
