#include <tsp.h>
#include <log.h>
#include <compiler.h>
#include <common.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

/* How long city can be in tabu list  */
#define TABU_LIST_MAX_TIME_PARAM    3
#define TABU_LIST_MAX_TIME(cities)  (TABU_LIST_MAX_TIME_PARAM * cities)

/* punishment param, needed in aspiration */
#define TABU_PUNSIHMENT_PARAM       0.2
#define TABU_PUNSIHMENT(TL, Cost, time) ((int)(((time) / (TL->maxtime)) * (Cost) * TABU_PUNSIHMENT_PARAM))

/* how many main loop we have in tabu search */
#define TABU_MAX_LOOPS  4



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
                + city_euclidean_dist(sol[j], sol[i + 2])
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

/* need to create pairs of neighbors */
typedef struct CitiesPair
{
    City *c1;
    City *c2;
}CitiesPair;

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

/*
    Random solusion on existing solusion

    PARAMS
    @IN cities - array of cities or exisiting solusion
    @IN n - size of array

    RETURN
    NULL iff failure
    Solusion iff success
*/
static City **tabu_list_random_solusion(City **cities, size_t n);

/*
    Create Pair of cities @c1 and @c2

    PARAMS
    @IN c1 - pointer to first city
    @IN c2 - pointer to second city

    RETURN:
    NULL iff failure
    Pointer iff success
*/
static CitiesPair *cities_pair_create(City *c1, City *c2);

/*
    Destroy Pair

    PARAMS
    @IN pair - cities  pair

    RETURN:
    This is a void function
*/
static void cities_pair_destroy(CitiesPair *pair);

static CitiesPair *cities_pair_create(City *c1, City *c2)
{
    CitiesPair *pair;

    assert(c1 == NULL);
    assert(c2 == NULL);

    TRACE("");

    pair = (CitiesPair *)malloc(sizeof(CitiesPair));
    if (pair == NULL)
        ERROR("malloc error\n", NULL, "");

    pair->c1 = c1;
    pair->c2 = c2;

    return pair;
}

static void cities_pair_destroy(CitiesPair *pair)
{
    TRACE("");

    if (pair == NULL)
        return;

    FREE(pair);
}

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

    (void)memset(tl->array, 0, sizeof(int) * n * m);

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

static City **tabu_list_random_solusion(City **cities, size_t n)
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

City **tsp_tabusearch_solution(World *w, size_t *n)
{
    City **global_solution;
    City **local_solution;
    City **best_local_solution;
    CitiesPair **pairs;
    TabuList *tl;

    TRACE("");

    assert(w == NULL);
    assert(n == NULL);

    tl = tabu_list_create(w->num_cities, w->num_cities, TABU_LIST_MAX_TIME(w->num_cities));
    if (tl == NULL)
        ERROR("tabu_list_create error\n", NULL, "");


}
