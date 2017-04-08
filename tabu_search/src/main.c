#include <stdio.h>
#include <log.h>
#include <compiler.h>
#include <world.h>
#include <tsp.h>
#include <stdlib.h>

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
    City **sol;
    size_t n;

    w = prepare_world();

    /*
    sol = tsp_greedy_solution(w, &n);
    tsp_cost_print(sol, n);
    free(sol);

    sol = tsp_rand_solution(w, &n);
    tsp_cost_print(sol, n);
    free(sol);
    */

    sol = tsp_tabusearch_solution(w, &n);
    tsp_cost_print(sol, n);
    free(sol);

    world_destroy(w);
    return 0;
}
