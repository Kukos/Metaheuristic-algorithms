#include <stdio.h>
#include <log.h>
#include <compiler.h>
#include <world.h>
#include <tsp.h>
#include <stdlib.h>
#include <unistd.h>

/* init logging before main  */
void __before_main__(0) init(void)
{
    log_init(stdout, LOG_TO_FILE);
}

/* deinit logging after main */
void __after_main__(0) deinit(void)
{
    log_deinit();
}

/* read from stdin our world */
World *prepare_world(void)
{
    size_t n;
    World *world;

    City *city;
    int id;
    double x;
    double y;
    size_t i;

    /* read num entries */
    if (scanf("%zu", &n) != 1)
        ERROR("scanf error\n", NULL, "");

    LOG("SIZE = %zu\n", n);
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
    City **sol;
    size_t n;
    int time;

    w = prepare_world();
    if (scanf("%d", &time) != 1)
        ERROR("scanf error\n", 1, "");

    annealing_set_max_time(time);

    sol = tsp_annealing_solution(w, &n);
    tsp_cost_print(sol, n);
    tsp_solution_print(sol, n);
    free(sol);

    world_destroy(w);
    return 0;
}
