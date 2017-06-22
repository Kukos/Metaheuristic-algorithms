#include <tsp.h>
#include <log.h>
#include <compiler.h>
#include <common.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define GENERIC_MAX_ITERATION       1000
#define GENERIC_REPEAT_IN_LOOP      100
#define GENERIC_POPULATION_SIZE     4
#define GENERIC_TIME_FACTOR         (double)0.7

#define GENERIC_FORCE_ALGO_END_IF_MUST \
    do { \
        if (generic_is_end) \
            goto generic_end; \
    } while(0)

static int generic_max_time;
static bool generic_is_end;

/*
    Thread Function
    If time is over set generic_is_end to true

    PARAMS
    @IN time in [s] (void *)&time

    RETURN
    This is a void function

*/
static void *generic_watchdog_life(void *time);

/*
    Check cities at i and j position in cycle

    PARAMS
    @IN n - cycle size
    @IN i - first index
    @IN j - second index

    RETURN
    true iff @i and @j are neighbors in cycle
    false iff not
*/
static __inline__ bool are_cities_neighbors(int n, int i, int j);

static __inline__ bool are_cities_neighbors(int n, int i, int j)
{
    /* begining, so XY.... or X...Y */
    if (i == 0)
        return (i + 1 == j || n - 1 == j);

    /* end so YX.... or Y...X */
    if (i == n - 1)
        return (j == 0 || i - 1 == j);

    /* middle so ...XY... or ...YX... */
    return (i + 1 == j || i - 1 == j);
}

static int find_city(City **t, int n, int id)
{
    int i;
    for (i = 0; i < n; ++i)
        if (t[i]->id == id)
            return i;

    return -1;
}

static void reverse_cities(City **t, int n, int index1, int index2)
{
    int i;
    int j;

    if (index1 < index2)
    {
        for (i = index1, j = 0; i <= index2; ++i, ++j)
            SWAP(t[i], t[index2 - j]);
    }
    else
    {
        for (i = index1, j = 1; i < n; ++i, ++j)
            SWAP(t[i], t[n - j]);

        for (i = 0; i < index2; ++i)
            SWAP(t[i], t[index2 - i - 1]);
    }
}

static void *generic_watchdog_life(void *time)
{
    /* wait time in micro  */
    useconds_t wtime;

    wtime = (useconds_t)((*(int *)time * GENERIC_TIME_FACTOR) * 1000000);
    LOG("Watchdog waiting for %ld micro seconds\n", wtime);
    (void)usleep(wtime);
    LOG("Watchdog kicking !!!\n", "");

    generic_is_end = true;

    return NULL;
}

void generic_set_max_time(int time)
{
    generic_max_time = time;
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

City **tsp_generic_solution(World *w, size_t *n)
{
    City **populations[GENERIC_POPULATION_SIZE];
    double costs[GENERIC_POPULATION_SIZE];

    City **new_population;
    double cost;

    City **solusion;
    City **greedy;

    pthread_t watchdog;

    int i;
    int j;

    int max_iter;
    int repeat_iter;
    int pop;
    int pop2;

    int index1;
    int index2;

    int size;

    TRACE("");

    assert(w == NULL);
    assert(n == NULL);

    /* Feed Watchdog */
    (void)pthread_create(&watchdog, NULL,
                generic_watchdog_life, (void *)&generic_max_time);

    srand(time(NULL));
    size = w->num_cities;

    LOG("INIT populations with random solusion\n", "");
    /* init populations with random solusions */
    for (i = 0; i < GENERIC_POPULATION_SIZE; ++i)
    {
        populations[i] = tsp_rand_solution(w, n);
        if (populations[i] == NULL)
            ERROR("tsp_rand_solution error\n", NULL, "");

        costs[i] = tsp_solution_cost(populations[i], size);
    }

    LOG("INIT DONE\n", "");
    new_population = (City **)malloc(sizeof(City *) * size);
    if (new_population == NULL)
        ERROR("malloc error\n", NULL, "");

    for (max_iter = 0; max_iter < GENERIC_MAX_ITERATION; ++max_iter)
        for (pop = 0; pop < GENERIC_POPULATION_SIZE; ++pop)
        {
            /* let's create new population from this pop */
            (void)memcpy(new_population, populations[pop], size * sizeof(City *));

            index1 = rand() % size;
            for (repeat_iter = 0; repeat_iter < GENERIC_REPEAT_IN_LOOP; ++repeat_iter)
            {
                do {
                    pop2 = rand() % GENERIC_POPULATION_SIZE;
                } while (pop2 == pop);

                /* city 2 is after city 1 in pop2 */
                index2 = find_city(populations[pop2], size, new_population[index1]->id);
                index2 = (index2 + 1) % size;

                /* city 2 is city2 in pop */
                index2 = find_city(new_population, size, populations[pop2][index2]->id);

                /*  dont reverse neighbors */
                if (are_cities_neighbors(size, index1, index2))
                    break;

                reverse_cities(new_population, size, index1, index2);
                index1 = index2;

                GENERIC_FORCE_ALGO_END_IF_MUST;
            }

            cost = tsp_solution_cost(new_population, size);
            if (cost < costs[pop])
            {
                costs[pop] = cost;
                (void)memcpy(populations[pop], new_population, size * sizeof(City *));
            }
        }

generic_end:
    LOG("END\n", "");
    FREE(new_population);

    cost = costs[0];
    index1 = 0;
    for (i = 1; i < GENERIC_POPULATION_SIZE; ++i)
        if (costs[i] < cost)
        {
            cost = costs[i];
            index1 = i;
        }

    greedy = tsp_greedy_solution(w, n);
    if (tsp_solution_cost(greedy, *n) < cost)
    {
        LOG("RETURN GREEDY\n", "");
        for (i = 0; i < GENERIC_POPULATION_SIZE; ++i)
            FREE(populations[i]);

        return greedy;
    }
    else
    {
        LOG("RETURN GENERIC\n", "");

        solusion = (City **)malloc(sizeof(City *) * (w->num_cities + 1));
        if (solusion == NULL)
            ERROR("malloc error\n", NULL, "");

        index2 = find_city(populations[index1], size, 1);
        solusion[w->num_cities] = populations[index1][index2];

        for (i = index2, j = 0; i < size; ++i, ++j)
            solusion[j] = populations[index1][i];

        for (i = 0; i < index2; ++i, ++j)
            solusion[j] = populations[index1][i];

        for (i = 0; i < GENERIC_POPULATION_SIZE; ++i)
            FREE(populations[i]);

        return solusion;
    }
}
