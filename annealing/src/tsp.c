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

#define ANNEALING_MAX_LOOPS(n) \
    __extension__ \
    ({ \
        int ______temp; \
        if (n < 4000) \
            ______temp = 1000; \
        else if (n < 10000) \
            ______temp = 4000; \
        else \
            ______temp = 6000; \
    ______temp; \
    })

#define ANNEALING_RAND_MAX_LOOP     10
#define ANNEALING_START_TEMP        (double)1.0
#define ANNEALING_END_TEMP          (double)0.0000001
#define ANNEALING_TEMP_FACTOR       (double)0.995
#define ANNEALING_TIME_FACTOR       (double)0.9

#define ANNEALING_FORCE_ALGO_END_IF_MUST \
    do { \
        if (annealing_is_end) \
            goto annealing_end; \
    } while(0)

static int annealing_max_time;
static bool annealing_is_end;

/*
    Thread Function
    If time is over set annealing_is_end to true

    PARAMS
    @IN time in [s] (void *)&time

    RETURN
    This is a void function

*/
static void *annealing_watchdog_life(void *time);

/*
    Annealing cost (Use this when new_cost > cost to check acceptance by algo)

    PARAMS
    @IN temp - current temperature
    @IN cost - current cost
    @IN new_cost - new cost

    RETURN
    true iff accept new_cost
    false iff doesn't accept new cost
*/
static __inline__ bool annealing_cond(double temp, double cost, double new_cost)
{
    return ((double)rand() / (double)RAND_MAX) < exp((cost - new_cost) / temp);
}

/* Calculate new cost after swap city on index @i with index @j on solution @sol when we have cost @cost  */
static __inline__ double annealing_new_cost(City **sol, int i, int j, double cost)
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

static void *annealing_watchdog_life(void *time)
{
    /* wait time in micro  */
    useconds_t wtime;

    wtime = (useconds_t)((*(int *)time * ANNEALING_TIME_FACTOR) * 1000000);
    LOG("Watchdog waiting for %ld micro seconds\n", wtime);
    (void)usleep(wtime);
    LOG("Watchdog kicking !!!\n", "");

    annealing_is_end = true;

    return NULL;
}

void annealing_set_max_time(int time)
{
    annealing_max_time = time;
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

City **tsp_annealing_solution(World *w, size_t *n)
{
    pthread_t watchdog;

    /* solutions ( permutation of cities ) */
    City **local_solution;
    City **greedy_solution;

    /* costs (dists) of solutions */
    double local_solution_cost;
    double greedy_solution_cost;

    /* main loop iterator */
    int annealing_main_loop;
    int annealing_max_loops;

    /* temperatures */
    double cur_temp;
    double end_temp;
    double temp_factor;

    double temp_cost;

    /* indexes of candidates to swap during annealing */
    int annealing_swap_candidate1;
    int annealing_swap_candidate2;

    /* iterators to rand loop */
    int rand_loop;
    int rand_max_loop;

    /* some big sizes */
    size_t copy_solution_bytes;

    TRACE("");

    assert(w == NULL);
    assert(n == NULL);

    /* Feed Watchdog */
    (void)pthread_create(&watchdog, NULL,
                annealing_watchdog_life, (void *)&annealing_max_time);

    srand(time(NULL));

    /******* init Annealing ******/
    LOG("Start greedy\n", "");

    /* the best solution for now is a greddy solution */
    greedy_solution = tsp_greedy_solution(w, n);
    if (greedy_solution == NULL)
        ERROR("tsp_greedy_solution error\n", NULL, "");

    LOG("Greedy DONE\n", "");

    copy_solution_bytes = sizeof(City *) * *n;
    local_solution = (City **)malloc(copy_solution_bytes);
    if (local_solution == NULL)
        ERROR("malloc error\n", NULL, "");

    /* copy this solution to local */
    (void)memcpy(local_solution, greedy_solution, copy_solution_bytes);

    /* calc cost and again copy to local */
    greedy_solution_cost = tsp_solution_cost(greedy_solution, *n);
    local_solution_cost = greedy_solution_cost;

    LOG("Greedy solution cost = %lf\n", greedy_solution_cost);

    /* init some const */
    end_temp            = ANNEALING_END_TEMP;
    temp_factor         = ANNEALING_TEMP_FACTOR;
    rand_max_loop       = ANNEALING_RAND_MAX_LOOP;
    annealing_max_loops = ANNEALING_MAX_LOOPS(w->num_cities);

    LOG("WORLD SIZE = %zu\n\tANNEALING_MAX_LOOPS = %d\n",
        w->num_cities, annealing_max_loops);

    for (annealing_main_loop = 0;
         annealing_main_loop < annealing_max_loops;
         ++annealing_main_loop)
    {
        cur_temp = ANNEALING_START_TEMP;
        while (cur_temp > end_temp)
        {
            for (rand_loop = 0; rand_loop < rand_max_loop; ++rand_loop)
            {
                do {
                    annealing_swap_candidate1 = rand() % (w->num_cities - 1) + 1;
                    annealing_swap_candidate2 = rand() % (w->num_cities - 1) + 1;
                } while (annealing_swap_candidate1 == annealing_swap_candidate2);

                if (annealing_swap_candidate1 > annealing_swap_candidate2)
                    SWAP(annealing_swap_candidate1, annealing_swap_candidate2);

                temp_cost = annealing_new_cost( local_solution,
                                                annealing_swap_candidate1,
                                                annealing_swap_candidate2,
                                                local_solution_cost);

                if (temp_cost < local_solution_cost ||
                    annealing_cond(cur_temp, local_solution_cost, temp_cost))
                {
                    local_solution_cost = temp_cost;
                    SWAP(local_solution[annealing_swap_candidate1],
                         local_solution[annealing_swap_candidate2]);
                }

                ANNEALING_FORCE_ALGO_END_IF_MUST;
            }

            cur_temp *= temp_factor;
        }
    }

annealing_end:
    if (local_solution_cost < greedy_solution_cost)
    {
        FREE(greedy_solution);

        return local_solution;
    }
    else
    {
        FREE(local_solution);

        return greedy_solution;
    }
}
