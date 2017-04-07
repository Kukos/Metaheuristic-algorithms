#ifndef TSP_H
#define TSP_H

/*
    Travel Saleman Problem

    Author: Michal Kukowski
    email: michalkukowski10.gmail.com

    LICENCE: GPL3.0

*/

#include <world.h>
#include <stdio.h>

/*
    Random solution

    PARAMS
    @IN w - pointer to world
    @OUT n - size of solution array

    RETURN
    NULL iff failure
    solution array iff success
*/
City **tsp_rand_solution(World *w, size_t *n);


/*
    Greedy solution

    PARAMS
    @IN w - pointer to world
    @OUT n - size of solution array

    RETURN
    NULL iff failure
    solution array iff success
*/
City **tsp_greedy_solution(World *w, size_t *n);

/*
    Tabu Search solution

    PARAMS
    @IN w - pointer to world
    @OUT n - size of solution array

    RETURN
    NULL iff failure
    solution array iff success
*/
City **tsp_tabusearch_solution(World *w, size_t *n);


/*
    Calculate cost of tsp solution

    PARAMS
    @IN solution - pointer to solution
    @IN n - size of solution array

    RETURN
    Cost
*/
double tsp_solution_cost(City **solution, size_t n);

/*
    print tsp solution on stderr

    PARAMS
    @IN solution - solution array
    @IN n - size of solution array

    RETURN:
    This is a void function
*/
__inline__ void tsp_solution_print(City **solution, size_t n)
{
    size_t i;
    --n;
    for (i = 0; i < n; ++i)
        fprintf(stderr, "%d ", solution[i]->id);

    fprintf(stderr, "%d\n", solution[i]->id);
}

__inline__ void tsp_cost_print(City **solution, size_t n)
{
    fprintf(stdout, "%lf\n", tsp_solution_cost(solution, n));
}

#endif
