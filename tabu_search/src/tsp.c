#include <tsp.h>
#include <log.h>
#include <compiler.h>
#include <common.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

#define TABU_LIST_MAX_TIME_PARAM    3
#define TABU_LIST_MAX_TIME(cities)  (TABU_LIST_MAX_TIME_PARAM * cities)

#define TABU_PUNSIHMENT_PARAM       0.2
#define TABU_PUNSIHMENT(TL, Cost, time) ((int)(((time) / (TL->maxtime)) * (Cost) * TABU_PUNSIHMENT_PARAM))

static __inline__ double tabu_search_new_euclidean_dist(City **sol, int i, int j, double cost)
{
    /* we want to swap neighbors */
    if (i + 1 == j - 1)
        return  cost -  city_euclidean_dist(sol[i], sol[i + 1])
                     -  city_euclidean_dist(sol[j], sol[j + 1])
                     +  city_euclidean_dist(sol[i], sol[j])
                     +  city_euclidean_dist(sol[i + 1], sol[j + 1]);

    /* normal swap */
    return cost - city_euclidean_dist(sol[i], sol[i + 1])
                - city_euclidean_dist(sol[i + 1], sol[i + 2])
                - city_euclidean_dist(sol[j - 1], sol[j])
                - city_euclidean_dist(sol[j], sol[j + 1])
                + city_euclidean_dist(sol[i], sol[j])
                + city_euclidean_dist(sol[j], sol[i+2])
                + city_euclidean_dist(sol[j - 1], sol[i + 1])
                + city_euclidean_dist(sol[i + 1], sol[j + 1]);
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
    @IN m - num of row
    @IN maxtime - max time on tabulist


    RETURN:
    NULL iff failure
    Pointer to TabuList iff success
*/
static TabuList *tabu_list_create(size_t n, size_t m, size_t maxtime);

/*
    Destroy TabuList

    PARAMS
    @IN tl - pointer to TabuList

    RETURN:
    This is a void function
*/
static void tabu_list_destroy(TabuList *tl);

static int __tabu_list_get(TabuList *tl, int i, int j)
{
    return tl->array[i * tl->nc + j];
}

static void __tabu_list_set(TabuList *tl, int i, int j, int val)
{
    tl->array[i * tl->nc + j] = val;
}

static TabuList *tabu_list_create(size_t n, size_t m, size_t maxtime)
{
    TabuList *tl;

    TRACE("");

    tl = malloc(sizeof(TabuList));
    if (tl == NULL)
        ERROR("malloc error\n", NULL, "");

    tl->array = (int *)malloc(sizeof(int) * n * m);
    if (tl->array == NULL)
        ERROR("malloc error\n", NULL, "");

    tl->allocated = n * m;
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

double tsp_solution_cost(City **solution, size_t n)
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
