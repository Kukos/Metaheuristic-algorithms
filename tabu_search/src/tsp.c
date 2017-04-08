#include <tsp.h>
#include <log.h>
#include <compiler.h>
#include <common.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>

/* How long city can be in tabu list  */
#define TABU_LIST_MAX_TIME_PARAM    3
#define TABU_LIST_MAX_TIME(cities)  (TABU_LIST_MAX_TIME_PARAM * (cities))

/* punishment param, needed in aspiration */
#define TABU_PUNSIHMENT_PARAM       0.2
#define TABU_PUNSIHMENT(TL, Cost, time) ((int)(((time) / (TL->maxtime)) * (Cost) * TABU_PUNSIHMENT_PARAM))

/* how many main loop we have in tabu search */
#define TABU_MAX_LOOPS  1

/* how many iteration we have in 1 main loop  */
#define TABU_MAX_ITERATION_PARAM    0.003
#define TABU_MAX_ITERATION_PARAM2   0.2
#define TABU_MAX_ITERATION(cities)  ((int)(cities > 2000 ? \
                                            (TABU_MAX_ITERATION_PARAM * cities) \
                                         : TABU_MAX_ITERATION_PARAM2 * cities))

/* Calculate new cost after swap city on index @i with index @j on solution @sol when we have cost @cost  */
static __inline__ double tabu_search_new_cost(City **sol, int i, int j, double cost)
{
    /* we want to swap neighbors */
    if (i == j - 1)
        return  cost -  city_euclidean_dist(sol[i - 1], sol[i])
                     -  city_euclidean_dist(sol[j], sol[j + 1])
                     +  city_euclidean_dist(sol[i - 1], sol[j])
                     +  city_euclidean_dist(sol[i], sol[j + 1]);

    /* normal swap */
    return cost - city_euclidean_dist(sol[i - 1], sol[i])
                - city_euclidean_dist(sol[i], sol[i + 1])
                - city_euclidean_dist(sol[j - 1], sol[j])
                - city_euclidean_dist(sol[j], sol[j + 1])
                + city_euclidean_dist(sol[i - 1], sol[j])
                + city_euclidean_dist(sol[j], sol[i + 1])
                + city_euclidean_dist(sol[j - 1], sol[i])
                + city_euclidean_dist(sol[i], sol[j + 1]);
}

typedef struct TabuList
{
    int     *array;
    size_t  allocated;
    size_t  maxtime;
    size_t  nc;

    int     (*get)(struct TabuList *tl, int i, int j);
    void    (*set)(struct TabuList *tl, int i, int j, int val);

}TabuList;

/*
    Get array[i][j]

    PARAMS
    @IN tl - pointer to TabuList
    @IN i - index i
    @IN j - index j

    RETURN
    array[i][j]
*/
static int __tabu_list_get(TabuList *tl, int i, int j);

/*
    array[i][j] = val

    PARAMS
    @IN tl - pointer to TabuList
    @IN i - index i
    @IN j - index j
    @IN val - value

    RETURN:
    This is a void function
*/
static void __tabu_list_set(TabuList *tl, int i, int j, int val);

/*
    Create tabu list

    PARAMS
    @IN n - num of column
    @IN maxtime - max time on tabulist


    RETURN:
    NULL iff failure
    Pointer to TabuList iff success
*/
static TabuList *tabu_list_create(size_t n, size_t maxtime);

/*
    Destroy TabuList

    PARAMS
    @IN tl - pointer to TabuList

    RETURN:
    This is a void function
*/
static void tabu_list_destroy(TabuList *tl);

/*
    Random solusion on existing solusion

    PARAMS
    @IN cities - array of cities or exisiting solusion
    @IN n - size of array

    RETURN
    NULL iff failure
    Solusion iff success
*/
static City **tabu_search_random_solusion(City **cities, size_t n);

static int __tabu_list_get(TabuList *tl, int i, int j)
{
    return tl->array[((i * (i - 1)) >> 1) + j];
}

static void __tabu_list_set(TabuList *tl, int i, int j, int val)
{
    tl->array[((i * (i - 1)) >> 1) + j] = val;
}

static TabuList *tabu_list_create(size_t n, size_t maxtime)
{
    TabuList *tl;

    TRACE("");

    tl = malloc(sizeof(TabuList));
    if (tl == NULL)
        ERROR("malloc error\n", NULL, "");

    tl->allocated = (n * (n - 1)) >> 1;
    tl->array = (int *)malloc(sizeof(int) * tl->allocated);
    if (tl->array == NULL)
        ERROR("malloc error\n", NULL, "");

    (void)memset(tl->array, 0, sizeof(int) * tl->allocated);

    tl->nc = n;
    tl->maxtime = maxtime;
    tl->get = __tabu_list_get;
    tl->set = __tabu_list_set;

    return tl;
}

static void tabu_list_destroy(TabuList *tl)
{
    TRACE("");

    if (tl == NULL)
        return;

    FREE(tl->array);
    FREE(tl);
}

static City **tabu_search_random_solusion(City **cities, size_t n)
{
    size_t i;
    size_t randd;

    assert(cities == NULL);
    assert(n == 0);

    TRACE("");

    /* shuffle, but without first and last */
    for (i = 1; i < n - 1; ++i)
    {
        randd = rand() % (n - i - 1) + i + 1;
        SWAP(cities[i], cities[randd]);
    }

    return cities;
}

City **tsp_rand_solution(World *w, size_t *n)
{
    City **sol;
    size_t i;
    size_t randd;

    TRACE("");
    assert(w == NULL);
    assert(n == NULL);

    srand(time(NULL));

    *n = w->num_cities + 1;

    sol = (City **)malloc(sizeof(City *) * *n);
    if (sol == NULL)
        ERROR("malloc error\n", NULL, "");

    (void)memcpy(sol, w->cities, w->num_cities * sizeof(City*));
    sol[w->num_cities] = sol[0];

    /* shuffle, but without first and last */
    for (i = 1; i < w->num_cities - 1; ++i)
    {
        randd = rand() % (w->num_cities - i - 1) + i + 1;
        SWAP(sol[i], sol[randd]);
    }

    return sol;
}

City **tsp_greedy_solution(World *w, size_t *n)
{
    City **sol;

    size_t i;
    size_t j;
    size_t swap_id;

    double min_cost;
    double temp_cost;

    TRACE("");

    assert(w == NULL);
    assert(n == NULL);

    *n = w->num_cities + 1;

    sol = (City **)malloc(sizeof(City *) * *n);
    if (sol == NULL)
        ERROR("malloc error\n", NULL, "");

    (void)memcpy(sol, w->cities, w->num_cities * sizeof(City*));
    sol[w->num_cities] = sol[0];

    for (i = 1; i < w->num_cities - 1; ++i)
    {
        min_cost = city_euclidean_dist(sol[i], sol[i + 1]);
        swap_id = i + 1;
        for (j = i + 2; j < w->num_cities; ++j)
        {
            temp_cost = city_euclidean_dist(sol[i], sol[j]);
            if (temp_cost < min_cost)
            {
                min_cost = temp_cost;
                swap_id = j;
            }
        }

        SWAP(sol[i + 1], sol[swap_id]);
    }

    return sol;
}

__inline__ double tsp_solution_cost(City **solution, size_t n)
{
    double cost = 0.0;
    size_t i;

    TRACE("");

    assert(solution == NULL);

    --n;
    for (i = 0; i < n; ++i)
        cost += city_euclidean_dist(solution[i], solution[i + 1]);

    return cost;
}

City **tsp_tabusearch_solution(World *w, size_t *n)
{
    /* solutions ( permutation of cities ) */
    City **global_solution;
    City **local_solution;
    City **best_local_solution;

    /* costs (dists) of solutions */
    double global_solution_cost;
    double local_solution_cost;
    double best_local_solution_cost;
    double cur_cost;
    double temp_cost;


    /* tabu list (triangle array 2D in 1D array) */
    TabuList *tl;

    /* some iterators */
    int i;
    int j;

    /* tabu loops iterators */
    int tabu_main_loop;
    int tabu_iteration;

    /* index in tabu list */
    int tabu_index1;
    int tabu_index2;

    /* candidates to swap cities ( index in solution ) */
    int tabu_swap_candidate1;
    int tabu_swap_candidate2;

    /* some big sizes */
    size_t copy_solution_bytes;

    TRACE("");

    assert(w == NULL);
    assert(n == NULL);

    srand(time(NULL));

    /******* init tabu ******/

    tl = tabu_list_create(w->num_cities, TABU_LIST_MAX_TIME(w->num_cities));
    if (tl == NULL)
        ERROR("tabu_list_create error\n", NULL, "");

    LOG("Start greedy\n", "");
    /* the best solution for now is a greddy solution */
    global_solution = tsp_greedy_solution(w, n);
    if (global_solution == NULL)
        ERROR("tsp_greedy_solution error\n", NULL, "");

    LOG("Greedy DONE\n", "");

    copy_solution_bytes = sizeof(City *) * *n;
    local_solution = (City **)malloc(copy_solution_bytes);
    best_local_solution = (City **)malloc(copy_solution_bytes);

    /* copy this solution to local and best local */
    (void)memcpy(local_solution, global_solution, copy_solution_bytes);
    (void)memcpy(best_local_solution, global_solution, copy_solution_bytes);

    /* calc cost and again copy to local and best_local_solution */
    global_solution_cost = tsp_solution_cost(global_solution, *n);
    local_solution_cost = global_solution_cost;
    best_local_solution_cost = global_solution_cost;

    LOG("Greedy solution cost = %lf\n", global_solution_cost);

    for (tabu_main_loop = 0; tabu_main_loop < TABU_MAX_LOOPS; ++tabu_main_loop)
    {
        /* in 1st time we init by greedy, else init by random */
        if (tabu_main_loop)
        {
            LOG("Init tabu by random solution\n", "");
            local_solution = tabu_search_random_solusion(local_solution, *n - 1);
            (void)memcpy(best_local_solution, local_solution, copy_solution_bytes);

            local_solution_cost = tsp_solution_cost(local_solution, *n);
            best_local_solution_cost = local_solution_cost;

            LOG("in %d loop random solution cost = %lf\n", tabu_main_loop,
                best_local_solution_cost);
        }

        cur_cost = best_local_solution_cost;

        for (tabu_iteration = 0;
             tabu_iteration < TABU_MAX_ITERATION(w->num_cities);
             ++tabu_iteration)
        {

            local_solution_cost = DBL_MAX;

            tabu_swap_candidate1 = 0;
            tabu_swap_candidate2 = 0;


            for (i = 1; i < w->num_cities - 1; ++i)
                for (j = i + 1; j < w->num_cities; ++j)
                {
                    /* we have triangle array instead of matrix so we need (i, j) i < j */
                    if (local_solution[i]->id < local_solution[j]->id)
                    {
                        tabu_index1 = local_solution[i]->id - 1;
                        tabu_index2 = local_solution[j]->id - 1;
                    }
                    else
                    {
                        tabu_index1 = local_solution[j]->id - 1;
                        tabu_index2 = local_solution[i]->id - 1;
                    }

                    /* swap is better ? */
                    temp_cost = tabu_search_new_cost(local_solution,
                                i, j, cur_cost);

                    /* we don't swap cities or we swapped long time ago */
                    if (tl->get(tl, tabu_index1, tabu_index2) == 0 ||
                        tabu_iteration - tl->get(tl, tabu_index1, tabu_index2) >
                            TABU_LIST_MAX_TIME(w->num_cities))
                    {
                        /* yes is better do it */
                        if (temp_cost < local_solution_cost)
                        {
                            tabu_swap_candidate1 = i;
                            tabu_swap_candidate2 = j;
                            local_solution_cost = temp_cost;
                        }
                    }
                    else /* tabu move */
                    {
                        if (temp_cost < best_local_solution_cost -
                            TABU_PUNSIHMENT(tl, best_local_solution_cost, (
                            tabu_iteration - tl->get(tl, tabu_index1, tabu_index2) < 0 ?
                            tabu_iteration :
                            tabu_iteration - tl->get(tl, tabu_index1, tabu_index2)))
                            )
                        {

                            local_solution_cost = temp_cost;
                            tabu_swap_candidate1 = i;
                            tabu_swap_candidate2 = j;
                        }
                    }
                }

            /* swap the BEST cities */
            SWAP(   local_solution[tabu_swap_candidate1],
                    local_solution[tabu_swap_candidate2]);

            /* we swap cities so update tabu list */
            if (local_solution[tabu_swap_candidate1]->id <
                    local_solution[tabu_swap_candidate2]->id)
                tl->set(tl, local_solution[tabu_swap_candidate1]->id - 1,
                            local_solution[tabu_swap_candidate2]->id - 1,
                            tabu_iteration);
            else
                tl->set(tl, local_solution[tabu_swap_candidate2]->id - 1,
                            local_solution[tabu_swap_candidate1]->id - 1,
                            tabu_iteration);

            cur_cost = local_solution_cost;

            /* we have new best local solution */
            if (local_solution_cost < best_local_solution_cost)
            {
                best_local_solution_cost = local_solution_cost;
                (void)memcpy(best_local_solution, local_solution, copy_solution_bytes);
            }

        }

        /* update global solution */
        if (best_local_solution_cost < global_solution_cost)
        {
            global_solution_cost = best_local_solution_cost;
            (void)memcpy(global_solution, best_local_solution, copy_solution_bytes);

            LOG("in %d loop tabu search solution cost = %lf\n", tabu_main_loop,
                global_solution_cost);
        }
    }

    FREE(local_solution);
    FREE(best_local_solution);

    tabu_list_destroy(tl);

    return global_solution;

}
