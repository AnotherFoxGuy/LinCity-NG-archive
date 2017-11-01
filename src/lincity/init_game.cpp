/* ----------------------------------------------------------------------
 * init_game.cpp
 * This file is part of lincity-ng
 * see COPYING for license, and CREDITS for authors
 * ----------------------------------------------------------------------
 */

// This was part of simulate.cpp. 
// Moved in new file for clarification
//
// (re)initialise engine and UI data when
//  - load a saved game (or a scenario)
//  - start a random village (or a  void map)
//

#include <math.h>
#include <cstdlib>
#include "init_game.h"
#include "fileutil.h"
#include "simulate.h"
#include "gui_interface/shared_globals.h"
#include "lctypes.h"
#include "lin-city.h"
#include "engglobs.h"
#include "gui_interface/screen_interface.h"
#include "power.h"
#include "stats.h"
#include "gui_interface/pbar_interface.h"
#include "modules/all_modules.h"
#include "transport.h"

#define IS_RIVER(x,y) (MP_INFO(x,y).flags & FLAG_IS_RIVER)

static const int di[8] = { -1, 0, 1, 0, 1, 1, -1, -1};
static const int dj[8] = { 0, -1, 0, 1, 1, -1, 1, -1};

/* Private functions prototypes */

static void init_mappoint_array(void);
static void initialize_tax_rates(void);
static void nullify_mappoint(int x, int y);
static void random_start(int *originx, int *originy);
static void coal_reserve_setup(void);
static void ore_reserve_setup(void);
static void setup_river(void);
static void setup_river2(int x, int y, int d, int alt, int mountain);
static void setup_ground(void);
static void new_setup_river_ground(void);
static void new_setup_river(void);
static void sort_by_altitude(int n, int *tabx, int *taby);
static void new_setup_one_river(int num_river, int c, int *colx, int *coly, int t, int *topx, int *topy, int l, int *lakx, int *laky);
static void set_river_tile( int i, int j);

/* ---------------------------------------------------------------------- *
 * Public Functions
 * ---------------------------------------------------------------------- */
void clear_game(void)
{
    int x, y, i, p;

    init_mappoint_array ();
    initialize_tax_rates ();
    init_inventory();

    // Clear engine and UI data.
    for (y = 0; y < WORLD_SIDE_LEN; y++) {
        for (x = 0; x < WORLD_SIDE_LEN; x++) {
            nullify_mappoint(x, y);
        }
    }
    total_time = 0;
    coal_survey_done = 0;
    numof_shanties = 0;
    numof_communes = 0;
    numof_substations = 0;
    numof_health_centres = 0;
    numof_markets = 0;
    max_pop_ever = 0;
    total_evacuated = 0;
    total_births = 0;
    total_money = 0;
    tech_level = 0;
    highest_tech_level = 0;
    rockets_launched = 0;
    rockets_launched_success = 0;
    init_inventory();
    update_avail_modules(0);

    use_waterwell = true; // NG 1.91 : AL1
                          // unused now (it was used in branch waterwell)
                          // but useful to know how to add an optional module, so keep it for a while.
    numof_waterwell = 0;
    global_aridity = 0;
    global_mountainity =0;

    highest_tech_level = 0;
    total_pollution_deaths = 0;
    pollution_deaths_history = 0;
    total_starve_deaths = 0;
    starve_deaths_history = 0;
    total_unemployed_years = 0;
    unemployed_history = 0;
    /* Al1. NG 1.1 is this enough ? Are all global variables reseted ? */

    // UI stuff
    /* TODO check reset screen, sustain info ... */
    given_scene[0] = 0;
    for( i = 0; i < monthgraph_size; i++ ){
        monthgraph_pop[i] = 0;
        monthgraph_starve[i] = 0;
        monthgraph_nojobs[i] = 0;
        monthgraph_ppool[i] = 0;
    } 
    // reset PBARS
    // FIXME AL1 NG 1.92.svn pbars are reseted only 
    //          when we build something
    //          when some building already exist and modify one value
    //
    /* AL1 i don't understand why this does not work
    init_pbars(); // AL1: Why is this not enough and why do we need additional stuff ?
    */
 
    /*
   */
    housed_population=0;
    tech_level=0;
    tfood_in_markets=0;
    tjobs_in_markets=0;
    tcoal_in_markets=0;
    tgoods_in_markets=0;
    tore_in_markets=0;
    tsteel_in_markets=0;
    total_money=0;

    init_pbars();
    for (p = 0; p < NUM_PBARS; p++)
        pbars[p].data_size = PBAR_DATA_SIZE;

    for (p = 0; p < NUM_PBARS; p++) {
        pbars[p].oldtot = 0;
        pbars[p].diff = 0;
    }
        
    for (x = 0; x < PBAR_DATA_SIZE; x++) {
        update_pbar (PPOP, housed_population, 1);
        update_pbar (PTECH, tech_level, 1);
        update_pbar (PFOOD, tfood_in_markets , 1);
        update_pbar (PJOBS, tjobs_in_markets , 1);
        update_pbar (PCOAL, tcoal_in_markets , 1);
        update_pbar (PGOODS, tgoods_in_markets  , 1);
        update_pbar (PORE, tore_in_markets  , 1);
        update_pbar (PSTEEL, tsteel_in_markets , 1);
        update_pbar (PMONEY, total_money, 1);
    }
    data_last_month = 1;
    update_pbars_monthly();
}

void new_city(int *originx, int *originy, int random_village)
{
    int old_setup_ground = true;
    clear_game();
    coal_reserve_setup();

    global_mountainity= 100 + rand () % 300; // roughly water slope = 25m / 1km (=from N to S)
    if (old_setup_ground) {
        setup_river();
        setup_ground();
    } else {
        new_setup_river_ground();
    }
    setup_land();
    ore_reserve_setup();
    init_pbars();

    /* Initial population is 100 for empty board or 200 
       for random village (100 are housed). */
    people_pool = 100;

    if (random_village != 0) {
        random_start(originx, originy);
        update_pbar(PPOP, 200, 1);      /* So pbars don't flash */
    } else {
        *originx = *originy = WORLD_SIDE_LEN / 2;
        update_pbar(PPOP, 100, 1);
    }
    connect_transport(1, 1, WORLD_SIDE_LEN - 2, WORLD_SIDE_LEN - 2);
    /* Fix desert frontier for old saved games and scenarios */
    desert_frontier(0, 0, WORLD_SIDE_LEN, WORLD_SIDE_LEN);

    refresh_pbars(); // AL1: does nothing in NG !
}

void setup_land(void)
{
    int x, y, xw, yw;
    int aridity = rand() % 450 - 150;

    global_aridity = aridity;
    

    for (y = 0; y < WORLD_SIDE_LEN; y++) {
        for (x = 0; x < WORLD_SIDE_LEN; x++) {
            int d2w_min = 2 * WORLD_SIDE_LEN * WORLD_SIDE_LEN;
            int r;
            int arid = aridity;
            int alt0 = 0;

            /* test against IS_RIVER to prevent terrible recursion */
            if (IS_RIVER(x, y) || !GROUP_IS_BARE(MP_GROUP(x, y)))
                continue;

            for (yw = 0; yw < WORLD_SIDE_LEN; yw++) {
                for (xw = 0; xw < WORLD_SIDE_LEN; xw++) {
                    int d2w;
                    if (!IS_RIVER(xw, yw))
                        continue;
                    d2w = (xw - x) * (xw - x) + (yw - y) * (yw - y);
                    if (d2w < d2w_min) {
                        d2w_min = d2w;
                        alt0 = ALT(xw,yw); // altitude of the river
                    }
                    /* TODO ? Store square of distance to river for each tile */
                }
            }

            /* near river lower aridity */
            if (aridity > 0) {
                if (d2w_min < 5)
                    arid = aridity / 3;
                else if (d2w_min < 17)
                    arid = (aridity * 2) / 3;
            }
            /* Altitude has same effect as distance */
            if( alt_step == 0 ){
                 alt_step = 400; // TODO: Why can alt_step be zero here? Quick hack to prevent crash WolfgangB 2008-09-13
            }
            r = rand() % (d2w_min / 3 + 1) + arid +
                abs((ALT(x,y) - alt0) * 19 / alt_step) + 3 * (ALT(x,y) * ALT(x,y)) /1000000 ;
            ground[x][y].ecotable=r;
            /* needed to setup quasi randome land. The flag is set below */
            MP_INFO(x, y).flags |= FLAG_HAS_UNDERGROUND_WATER;
            do_rand_ecology(x,y);
            MP_POL(x, y) = 0;

            /* preserve rivers, so that we can connect port later */
            if (MP_TYPE(x, y) == CST_WATER) {
                int navigable = MP_INFO(x, y).flags & FLAG_IS_RIVER;
                set_mappoint(x, y, CST_WATER);
                MP_INFO(x, y).flags |= navigable;
                MP_INFO(x, y).flags |= FLAG_HAS_UNDERGROUND_WATER;
            }
            /* set undergroung water according to first random land setup */
            if (MP_TYPE(x, y) == CST_DESERT) {
                MP_INFO(x, y).flags &= (0xffffffff - FLAG_HAS_UNDERGROUND_WATER);
            }
        }
    }
    for (y = 0; y < WORLD_SIDE_LEN; y++)
        for (x = 0; x < WORLD_SIDE_LEN; x++)
            if (MP_TYPE(x, y) == CST_WATER)
                MP_INFO(x, y).flags |= FLAG_HAS_UNDERGROUND_WATER;

    connect_rivers();
}

/* ---------------------------------------------------------------------- *
 * Private Functions
 * ---------------------------------------------------------------------- */
static void initialize_tax_rates(void)
{
    income_tax_rate = INCOME_TAX_RATE;
    coal_tax_rate = COAL_TAX_RATE;
    goods_tax_rate = GOODS_TAX_RATE;
    dole_rate = DOLE_RATE;
    transport_cost_rate = TRANSPORT_COST_RATE;
    import_cost_rate = IM_PORT_COST_RATE;
}

static void init_mappoint_array(void)
{
    int x;
    for (x = 0; x < WORLD_SIDE_LEN; x++) {
        mappoint_array_x[x] = x;
        mappoint_array_y[x] = x;
    }
}

static void coal_reserve_setup(void)
{
    int i, j, x, y, xx, yy;
    for (i = 0; i < NUMOF_COAL_RESERVES / 5; i++) {
        x = (rand() % (WORLD_SIDE_LEN - 12)) + 6;
        y = (rand() % (WORLD_SIDE_LEN - 10)) + 6;
        do {
            xx = (rand() % 3) - 1;
            yy = (rand() % 3) - 1;
        }
        while (xx == 0 && yy == 0);
        for (j = 0; j < 5; j++) {
            MP_INFO(x, y).coal_reserve += rand() % COAL_RESERVE_SIZE;
            x += xx;
            y += yy;
        }
    }
}

static void ore_reserve_setup(void)
{
    int x, y;
    for (y = 0; y < WORLD_SIDE_LEN; y++)
        for (x = 0; x < WORLD_SIDE_LEN; x++)
            MP_INFO(x, y).ore_reserve = ORE_RESERVE;
}

static void new_setup_river_ground(void)
{
    const int NLOOP = 7;
    const int SZ=128; // must be = 2^NLOOP
    const int SHIFT = (SZ - WORLD_SIDE_LEN) / 2; // center the visible map in the big one
    const float sigma = 3.5; // gaussian smoothing
    const float ods2 = 1. / (2. * sigma * sigma);
    const int mask_size = 11; // useless to be larger than 3*sigma && Must be < SHIFT 
    const float fract = 0.9;

    float mat[2 * mask_size + 1][2 * mask_size + 1];
    float g[SZ][SZ];
    float tmp[SZ][SZ];
    float min = 10000000000000000000.;
    float norm;
    int i,j,k,l,m,n,size,h;

    // build gaussian mask 
    norm = 0;
    for ( i = 0; i < 2 * mask_size + 1; i++) {
        for ( j = 0; j < 2 * mask_size + 1; j++) {
            float r2 = (i - mask_size) * (i - mask_size) + (j - mask_size) * (j - mask_size);
            mat[i][j] = exp(-r2 * ods2);
            norm += mat[i][j];
        }
    }
    norm = 1. / norm;

   // intialisation
#ifdef DEBUG
    fprintf(stderr," mountainity = %i \n", global_mountainity);
#endif
    h = ( rand() % 10 + rand() % 10 ) * global_mountainity / 20;
    for (i = 0; i < SZ; i++) {
        for (j = 0; j < SZ; j++) {
            g[i][j]=h;
            tmp[i][j]=0;
        }
    }

    /* fractal iteration for height */
    n = 1;
    for (k = 1; k <= NLOOP; k++) {
        n *= 2;
        size = SZ / n;
        // n x n block of size
        for ( l = 0; l < n; l++ ) {
            for ( m = 0; m < n; m++ ) {
                // one block
                h = int ( double((rand() % 10 + rand() % 10) * global_mountainity) * pow(fract,k));
                for (i = 0 ; i < size; i++)
                    for (j = 0 ; j < size; j++)
                        g[l * size + i][ m * size + j ] += h;
            }
        }
    }

    //smooth is iterated to propagate a little the lowering of borders
    for (n = 0; n < 2 ; n++) {
        // apply the mask
        for (i = mask_size; i < SZ - mask_size; i++) 
            for (j = mask_size; j < SZ - mask_size; j++) {
                tmp[i][j] = 0;
                for ( k = -mask_size; k <= mask_size; k++ )
                    for ( l = -mask_size; l <= mask_size; l++ )
                        tmp[i][j] += g[i + k][j - l] * mat[mask_size + k][mask_size + l];
            }

        for (i = mask_size; i< SZ - mask_size; i++) 
            for (j = mask_size; j< SZ - mask_size; j++)
                g[i][j] = tmp[i][j] * norm;

        if (n == 0) {
            // find the lowest borders
            // switch the map to have lowest borders in SE an SW in ISO view
            float Nmin = 0;
            float Smin = 0;
            float Emin = 0;
            float Wmin = 0;
            for ( i = 0; i < WORLD_SIDE_LEN ; i++) {
                Nmin += g[SHIFT + i][SHIFT];
                Smin += g[SHIFT + i][SHIFT + WORLD_SIDE_LEN];
                Wmin += g[SHIFT][SHIFT + i];
                Emin += g[SHIFT + WORLD_SIDE_LEN][SHIFT + i];
            }
            if (Nmin < Smin) {
                for ( i = 0; i < SZ; i++)
                    for ( j = 0; j < SZ; j++)
                        tmp[i][j] = g[i][SZ - j -1];

                for ( i = 0; i < SZ; i++)
                    for ( j = 0; j < SZ; j++)
                        g[i][j] = tmp[i][j];
            }

            if (Wmin < Emin) {
                for ( i = 0; i < SZ; i++)
                    for ( j = 0; j < SZ; j++)
                        tmp[i][j] = g[SZ - i -1][j];

                for ( i = 0; i < SZ; i++)
                    for ( j = 0; j < SZ; j++)
                        g[i][j] = tmp[i][j];
            }
        }

        // put the south and east border of the "big" map at the minimum visible height
        for ( i = 0; i < WORLD_SIDE_LEN ; i++)
            for ( j = 0; j < WORLD_SIDE_LEN ; j++)
                if (g[SHIFT + i][SHIFT + j] < min)
                    min = g[SHIFT + i][SHIFT + j];

        for ( i = 0; i < SZ; i++)
            for (j = 0; j < (SZ - SHIFT - WORLD_SIDE_LEN); j++) {
                g[i][SZ - 1 - j] = min; // south
                g[SZ - 1 - j][i] = min; // east
            }
    }

    alt_min =  int (min);
    alt_max = 0;
    // pick our map in the fractal one
    for ( i = 0; i < WORLD_SIDE_LEN; i++)
        for ( j = 0; j < WORLD_SIDE_LEN; j++) {
            ALT(i,j) = int (g[SHIFT + i][SHIFT + j]) - alt_min + 1;
            if ( ALT(i,j) > alt_max)
                alt_max = ALT(i,j);
        }

    // take visible value for maximum color dynamic
    alt_min = 0; // visible alt_min is 1, we will use 0 for gray border
    alt_step = (alt_max - alt_min)/10;
    
#ifdef DEBUG
    fprintf(stderr," alt min = %i; max = %i\n", alt_min, alt_max);
#endif
    new_setup_river();

}
void new_setup_river(void)
{


    int colx[WORLD_SIDE_LEN * WORLD_SIDE_LEN], coly[WORLD_SIDE_LEN * WORLD_SIDE_LEN];
    int topx[WORLD_SIDE_LEN * WORLD_SIDE_LEN], topy[WORLD_SIDE_LEN * WORLD_SIDE_LEN];
    int lakx[WORLD_SIDE_LEN * WORLD_SIDE_LEN], laky[WORLD_SIDE_LEN * WORLD_SIDE_LEN];

    int i, j, c, t, l;


    // Put lakes/seas in the lowest part of the map
    for ( i = 0; i < WORLD_SIDE_LEN; i++)
        for ( j = 0; j < WORLD_SIDE_LEN; j++)
            if (ALT(i,j) < 2 * alt_step)
                set_river_tile(i,j);

    // Put the gray border (not visible) at alt_min, for easier rivers handling.
    for ( i = 0; i < WORLD_SIDE_LEN; i++) {
        ALT(i, 0) = alt_min;
        ALT(i, WORLD_SIDE_LEN - 1) = alt_min;
        ALT(0, i) = alt_min;
        ALT(WORLD_SIDE_LEN - 1, i) = alt_min;
    }

    // Hessian = second order derivatives in visible map
    // => know curvature, local max, min, saddle points and find col (mountain pass)
    c = 0;
    l = 0;
    t = 0;
    for ( i = 1; i < WORLD_SIDE_LEN - 1 ; i++)
        for ( j = 1; j < WORLD_SIDE_LEN - 1 ; j++) {
            float e1, e2;
            float dx2, dy2, dxy, dyx, delta;

            dx2 = float (ALT(i + 1,j) + ALT(i - 1,j) - 2 * ALT(i,j));
            dy2 = float (ALT(i,j + 1) + ALT(i,j - 1) - 2 * ALT(i,j));
            dxy = float ( (ALT(i+1 , j+1) + ALT(i-1, j-1) - ALT(i+1,j-1) - ALT(i-1,j+1)) * 0.25);
            dyx = dxy;
            // e1 e2 are the eigenvalues of Hessian, ie solutions of:
            // X^2 - (dx2 + dy2).X + dx2.dy2 - dxy.dyx = 0     (wrt X)
            delta =  (dx2 + dy2)*(dx2 + dy2) - 4 * (dx2 * dy2 - dxy * dyx);
            e1 = (dx2 + dy2) + sqrt(delta);
            e2 = (dx2 + dy2) - sqrt(delta);

            if (e1 * e2 < 0) {
                // saddle point = mountain pass  _IF_ tangent plane is _nearly_ horizontal !

                /* Tangent plane has equation alpha.(x-x0) + beta.(y-y0) - z = 0
                 * Least square method to find alpha and beta considering 8 neighbours
                 *  (just partial derivatives along x and y are not enought)
                 * After some calculus, alpha = (Sax.Syy - Say.Sxy)/(2.*(Sxx.Syy-Sxy*Syx))
                 * beta is symetrical wrt x <-> y
                 */
                    // precomputed terms for least square, on 8 neighbours
                    // (for i = 0; i < 8; i++) Sxy += (xi -x0) * (yi - y0);
                    // => Sxy = Syx = 0 :-)
                    // => simple result for alpha and beta.

                const float Sxx = 6.; // Syy = Sxx
                float alpha, beta;
                float Sax = 0.;
                float Say = 0.;
                for (int n = 0; n < 8; n++) {
                    Sax += float ((ALT(i + di[n], j + dj[n]) - ALT(i,j)) * di[n]);
                    Say += float ((ALT(i + di[n], j + dj[n]) - ALT(i,j)) * dj[n]);
                }
                alpha = Sax / Sxx;
                beta  = Say / Sxx; // because Syy = Sxx

                // Normal vector is (alpha, beta, -1)
                // so plane is nearly horizontal if alpha^2 + beta^2 is "small"
                if ( (alpha * alpha + beta * beta) < float(global_mountainity / 2) ) {
                    // mountain pass = col
                    colx[c] = i;
                    coly[c] = j;
                    c++;
//#define DEBUG_LAND
#ifdef DEBUG_LAND
                    fprintf(stderr," x %i, y %i, norm %f\n", i, j, alpha * alpha + beta * beta);
                    if (GROUP_IS_BARE(MP_GROUP(i,j)))
                        set_mappoint(i,j, CST_ROAD_LR);
                    //XXX AL1: why is there a segfault if we use CST_POWERL_H_D ?
#endif
                }
            } else if (e1 * e2 != 0) {
                if ( e1 < 0) {
                    // local top
                    if (  ALT(i + 1,j) < ALT(i,j) && ALT(i - 1,j) < ALT(i,j) &&
                            ALT(i,j + 1) < ALT(i,j) && ALT(i,j - 1) < ALT(i,j)  )  {

                        topx[t] = i;
                        topy[t] = j;
                        t++;
#ifdef DEBUG_LAND
                        if (GROUP_IS_BARE(MP_GROUP(i,j)))
                            set_mappoint(i,j, CST_FIRE_1);
#endif
                    }
                } else {
                    // local min = potential lake => has water.
                    if (  ALT(i + 1,j) > ALT(i,j) && ALT(i - 1,j) > ALT(i,j) &&
                            ALT(i,j + 1) > ALT(i,j) && ALT(i,j - 1) > ALT(i,j)  )  {

                        lakx[l] = i;
                        laky[l] = j;
                        l++;
                        if (GROUP_IS_BARE(MP_GROUP(i,j))) {
                            set_mappoint(i,j, CST_PARKLAND_LAKE);
                            MP_INFO(i,j).flags |= (FLAG_HAS_UNDERGROUND_WATER + FLAG_IS_RIVER);
                        }
                    }
                }
            } else {
                // parabolic point
#ifdef DEBUG_LAND
                if (GROUP_IS_BARE(MP_GROUP(i,j)))
                    set_mappoint(i,j, CST_RAIL_LR);
#endif
            }


        }

#ifdef DEBUG_LAND
    fprintf(stderr," pass c = %i, cx = %i, cy = %i\n", c, colx[c - 1], coly[c -1]);
    fprintf(stderr," top t = %i, tx = %i, ty = %i\n", t, topx[t-1], topy[t-1]);
    fprintf(stderr," lak l = %i, lx = %i, ly = %i\n", l, lakx[l-1], laky[l-1]);
#endif

    // put one river from each top.
    sort_by_altitude(t, topx, topy);
    for (i = 0; i < t; i++)
        new_setup_one_river(i, c, colx, coly, t, topx, topy, l, lakx, laky);
}

static void set_river_tile( int i, int j)
{
    MP_TYPE(i, j) = CST_WATER;
    MP_GROUP(i, j) = GROUP_WATER;
    MP_INFO(i, j).flags |= FLAG_IS_RIVER;
}

static void sort_by_altitude(int n, int *tabx, int *taby)
{
    int tmp_x, tmp_y;
    bool sorted = false;

    // bubble sort. n is near 10 so ...
    for (int i = 0; i < n && !sorted ; i++) {
        sorted = true;
        for (int j=1; j < n - i; j++)
            if (ALT(tabx[j],taby[j]) < ALT(tabx[j-1], taby[j-1])) {
                tmp_x = tabx[j-1];
                tmp_y = taby[j-1];
                tabx[j-1] =  tabx[j];
                taby[j-1] =  taby[j];
                tabx[j] = tmp_x;
                taby[j] = tmp_y;
                sorted = false;
            }
        /*fprintf(stderr," sorted = %i, n - i -1 = %i, ALT() = %i\n", 
         *       sorted, n - i -1, ALT(tabx[n-i-1], taby[n -i -1]));
         */
    }
}

static void new_setup_one_river(int num_river, int c, int *colx, int *coly, int t, int *topx, int *topy, int l, int *lakx, int *laky)
{
    int x, y, xx, yy, alt, alt_max;

    /* find a place in altitude near top */
    xx = topx[t - num_river] + (1 + rand() % 2) * di[rand() % 8];
    yy = topy[t - num_river] + (1 + rand() % 2) * dj[rand() % 8];
    if ( xx < 0 || xx >= WORLD_SIDE_LEN)
        xx =  topx[t - num_river];

    if ( yy < 0 || yy >= WORLD_SIDE_LEN)
        yy =  topy[t - num_river];

    set_river_tile(xx,yy);
    alt_max = ALT(xx, yy);

    /* follow most important slope and go downward */
    do {
        int m = 0;
        x = xx;
        y = yy;
        alt = ALT(x,y);
        for (int n = 0; n < 8; n++) {
            if (ALT(x + di[n], y + dj[n]) < alt) {
                xx = x + di[n];
                yy = y + dj[n];
                alt = ALT(xx, yy);
                m = n;
            }
        }
        set_river_tile(xx,yy);
        if (m>3) {
            // we did diagonal move, so we need to connect river
            if (ALT(x + di[m], y) > ALT(x, y + dj[m]))
                set_river_tile(x, y + dj[m]);
            else
                set_river_tile(x + di[m], y);
        }
    } while ( (xx != x) || (yy != y) ); 
    // We are in a local minimum

    if ( x == 0 || x == WORLD_SIDE_LEN - 1 || y == 0 || y == WORLD_SIDE_LEN - 1) {
        // borders of the map are strictly the lowest points 
        return;
    }

    // TODO connect lakes to outside of the map
    // make a small lake
    sort_by_altitude(c, colx, coly);
    sort_by_altitude(l, lakx, laky);

}

static void setup_river(void)
{
    int x, y, i, j;
    int alt = 1; //lowest altitude in the map = surface of the river at mouth.
    x = (1 * WORLD_SIDE_LEN + rand() % WORLD_SIDE_LEN) / 3;
    y = WORLD_SIDE_LEN - 1;
    ground[x][y].water_alt = alt; // 1 unit = 1 cm , 
                        //for rivers .water_alt = .altitude = surface of the water
                        //for "earth tile" .water_alt = alt of underground water
                        //                 .altitude = alt of the ground
                        //            so .water_alt <= .altitude

    /* Mouth of the river, 3 tiles wide, 6 + %12 long */
    i = (rand() % 12) + 6;
    for (j = 0; j < i; j++) {
        x += (rand() % 3) - 1;
        MP_TYPE(x, y) = CST_WATER;
        MP_GROUP(x, y) = GROUP_WATER;
        MP_INFO(x, y).flags |= FLAG_IS_RIVER;
        ground[x][y].altitude=alt;

        MP_TYPE(x + 1, y) = CST_WATER;
        MP_GROUP(x + 1, y) = GROUP_WATER;
        MP_INFO(x + 1, y).flags |= FLAG_IS_RIVER;
        ground[x + 1][y].altitude=alt;

        MP_TYPE(x - 1, y) = CST_WATER;
        MP_GROUP(x - 1, y) = GROUP_WATER;
        MP_INFO(x - 1, y).flags |= FLAG_IS_RIVER;
        ground[x -1][y].altitude=alt;

        y--;
        alt += 1; // wide river, so very small slope
    }

    MP_TYPE(x, y) = CST_WATER;
    MP_GROUP(x, y) = GROUP_WATER;
    MP_INFO(x, y).flags |= FLAG_IS_RIVER;
    ground[x][y].altitude=alt;

    MP_TYPE(x + 1, y) = CST_WATER;
    MP_GROUP(x + 1, y) = GROUP_WATER;
    MP_INFO(x + 1, y).flags |= FLAG_IS_RIVER;
    ground[x + 1][y].altitude=alt;

    MP_TYPE(x - 1, y) = CST_WATER;
    MP_GROUP(x - 1, y) = GROUP_WATER;
    MP_INFO(x - 1, y).flags |= FLAG_IS_RIVER;
    ground[x -1][y].altitude=alt;

    alt += 2;

#ifdef DEBUG
    fprintf(stderr," x= %d, y=%d, altitude = %d, mountainity = %d\n", x, y, alt, global_mountainity);
#endif
    setup_river2(x - 1, y, -1, alt, global_mountainity); /* left tributary */
    setup_river2(x + 1, y, 1, alt, global_mountainity);  /* right tributary */
}

static void setup_river2(int x, int y, int d, int alt, int mountain)
{
    int i, j, r;
    i = (rand() % 55) + 15;
    for (j = 0; j < i; j++) {
        r = (rand() % 3) - 1 + (d * (rand() % 3));
        if (r < -1) {
            alt += rand() % (mountain / 10);
            r = -1;
        } else if (r > 1) {
            alt += rand() % (mountain / 10);
            r = 1;
        }
        x += r;
        if (!GROUP_IS_BARE(MP_GROUP(x + (d + d), y))
            || !GROUP_IS_BARE(MP_GROUP(x + (d + d + d), y)))
            return;
        if (x > 5 && x < WORLD_SIDE_LEN - 5) {
            MP_TYPE(x, y) = CST_WATER;
            MP_GROUP(x, y) = GROUP_WATER;
            MP_INFO(x, y).flags |= FLAG_IS_RIVER;
            ground[x][y].altitude = alt;
            alt += rand() % (mountain / 10);

            MP_TYPE(x + d, y) = CST_WATER;
            MP_GROUP(x + d, y) = GROUP_WATER;
            MP_INFO(x + d, y).flags |= FLAG_IS_RIVER;
            ground[x + d][y].altitude = alt;
            alt += rand () % (mountain / 10);
        }
        if (--y < 10 || x < 5 || x > WORLD_SIDE_LEN - 5)
            break;
    }
#ifdef DEBUG
    fprintf(stderr," x= %d, y=%d, altitude = %d\n", x, y, alt);
#endif

    if (y > 20) {
        if (x > 5 && x < WORLD_SIDE_LEN - 5) {
#ifdef DEBUG
            fprintf(stderr," x= %d, y=%d, altitude = %d\n", x, y, alt);
#endif
            setup_river2(x, y, -1, alt, (mountain * 3)/2 );
        }
        if (x > 5 && x < WORLD_SIDE_LEN - 5) {
#ifdef DEBUG
            fprintf(stderr," x= %d, y=%d, altitude = %d\n", x, y, alt);
#endif
            setup_river2(x, y, 1, alt, (mountain *3)/2 );
        }
    }
}

static void setup_ground(void)
{
    int x,y;
    int hmax =0;
    int tmp[WORLD_SIDE_LEN][WORLD_SIDE_LEN];

    /* fill the corrects fields: ground[x][y).stuff, global_aridity, global_mountainity */
    /* currently only dummy things in order to compile */

#define TMP(x,y) tmp[x][y]

    for (x = 1; x < WORLD_SIDE_LEN - 1; x++) {
         for (y = 1; y < WORLD_SIDE_LEN - 1; y++) {
                if ( !IS_RIVER(x,y) ) {
                    ALT(x,y) = 0;
                    TMP(x,y) = 0;
                } else {
                    ground[x][y].water_alt = ALT(x,y);
                    //shore is higher than water
                    ALT(x,y) += 10 + rand() % (global_mountainity/7);
                    TMP(x,y) = ALT(x,y);
                    if (ALT(x,y) >= hmax)
                        hmax = ALT(x,y);
                }
         }
    }
#ifdef DEBUG
    fprintf(stderr,"\n river max = %d\n\n", hmax);
    hmax=0;
#endif

    for (int i =0; i < 90; i++ ) {
        int tot_cnt = 0;
        for (x = 1; x < WORLD_SIDE_LEN - 1; x++) {
            for (y = 1; y < WORLD_SIDE_LEN - 1; y++) {
                if ( ALT(x,y) != 0 )
                    continue;
                int count = 0;
                int lmax = 0;
                tot_cnt ++;
                for ( int k = -1; k <= 1; k++ )
                    for ( int l = -1; l <= 1; l++) 
                        if ( ALT(x+k, y+l) != 0 ) {
                            count ++;
                            if ( ALT(x+k, y+l) >= lmax )
                                lmax = ALT(x+k, y+l);
                        }

                if (count != 0)
                    TMP(x,y) = lmax + rand () % (global_mountainity/3);

                if (TMP(x,y) >= hmax)
                    hmax = TMP(x,y);
            }
        }
        for (x = 1; x < WORLD_SIDE_LEN - 1; x++)
            for (y = 1; y < WORLD_SIDE_LEN - 1; y++)
                ALT(x,y)=TMP(x,y);

#ifdef DEBUG
        if ( (i%5) == 1 )
        fprintf(stderr," i= %2d, alt max = %d, tot_cnt = %d\n", i, hmax, tot_cnt);
#endif
    }
    alt_min = 2000000000;
    alt_max = -alt_min;
    for (x = 1; x < WORLD_SIDE_LEN - 1; x++)
         for (y = 1; y < WORLD_SIDE_LEN - 1; y++) {
             if (alt_min > ALT(x,y))
                 alt_min = ALT(x,y);
             if (alt_max < ALT(x,y))
                 alt_max = ALT(x,y);
         }
    alt_step = (alt_max - alt_min) /10;

}


static void nullify_mappoint(int x, int y)
{
    MP_TYPE(x, y) = CST_GREEN;
    MP_GROUP(x, y) = GROUP_BARE;
    MP_SIZE(x, y) = 1;
    MP_POL(x, y) = 0;
    MP_INFO(x, y).population = 0;
    MP_INFO(x, y).flags = 0;
    MP_INFO(x, y).coal_reserve = 0;
    MP_INFO(x, y).ore_reserve = 0;
    MP_INFO(x, y).int_1 = 0;
    MP_INFO(x, y).int_2 = 0;
    MP_INFO(x, y).int_3 = 0;
    MP_INFO(x, y).int_4 = 0;
    MP_INFO(x, y).int_5 = 0;
    MP_INFO(x, y).int_6 = 0;
    MP_INFO(x, y).int_7 = 0;

    ground[x][y].altitude = 0;
    ground[x][y].ecotable = 0;
    ground[x][y].wastes = 0;
    ground[x][y].pollution = 0;
    ground[x][y].water_alt = 0;
    ground[x][y].water_pol = 0;
    ground[x][y].water_wast = 0;
    ground[x][y].water_next = 0;
    ground[x][y].int1 = 0;
    ground[x][y].int2 = 0;
    ground[x][y].int3 = 0;
    ground[x][y].int4 = 0;

}

static void random_start(int *originx, int *originy)
{
    int x, y, xx, yy, flag, watchdog;

    /* first find a place that has some water. */
    watchdog = 90;              /* if too many tries, random placement. */
    do {
        do {
            xx = rand() % (WORLD_SIDE_LEN - 25);
            yy = rand() % (WORLD_SIDE_LEN - 25);
            flag = 0;
            for (y = yy + 2; y < yy + 23; y++)
                for (x = xx + 2; x < xx + 23; x++)
                    if (IS_RIVER(x, y)) {
                        flag = 1;
                        x = xx + 23;    /* break out of loop */
                        y = yy + 23;    /* break out of loop */
                    }
        } while (flag == 0 && (--watchdog) > 1);
        for (y = yy + 4; y < yy + 22; y++)
            for (x = xx + 4; x < xx + 22; x++)
                /* Don't put the village on a river, but don't care of
                 * isolated random water tiles putted by setup_land
                 */
                if (IS_RIVER(x, y)) {
                    flag = 0;
                    x = xx + 22;        /* break out of loop */
                    y = yy + 22;        /* break out of loop */
                }
    } while (flag == 0 && (--watchdog) > 1);
#ifdef DEBUG
    fprintf(stderr, "random village watchdog = %i\n", watchdog);
#endif

    /* These are going to be the main_screen_origin? vars */
    *originx = xx;
    *originy = yy;

    /*  Draw the start scene. */
    set_mappoint(xx + 5, yy + 5, CST_FARM_O0);
    /* The first two farms have more underground water */
    for (int i = 0; i < MP_SIZE(xx + 5, yy + 5); i++)
        for (int j = 0; j < MP_SIZE(xx + 5, yy + 5); j++)
            if (!HAS_UGWATER(xx + 5 + i, yy + 5 + j) && (rand() % 2))
                MP_INFO(xx + 5 + i, yy + 5 + j).flags |= FLAG_HAS_UNDERGROUND_WATER;

    set_mappoint(xx + 9, yy + 6, CST_RESIDENCE_ML);
    MP_INFO(xx + 9, yy + 6).population = 50;
    MP_INFO(xx + 9, yy + 6).flags |= (FLAG_FED + FLAG_EMPLOYED + FLAG_WATERWELL_COVER);

    set_mappoint(xx + 9, yy + 9, CST_POTTERY_0);

    set_mappoint(xx + 16, yy + 9, CST_WATERWELL);
    do_waterwell_cover(xx + 16, yy + 9);

    set_mappoint(xx + 14, yy + 6, CST_RESIDENCE_ML);
    MP_INFO(xx + 14, yy + 6).population = 50;
    MP_INFO(xx + 14, yy + 6).flags |= (FLAG_FED + FLAG_EMPLOYED + FLAG_WATERWELL_COVER);

    /* The first two farms have more underground water */
    set_mappoint(xx + 17, yy + 5, CST_FARM_O0);
    for (int i = 0; i < MP_SIZE(xx + 17, yy + 5); i++)
        for (int j = 0; j < MP_SIZE(xx + 17, yy + 5); j++)
            if (!HAS_UGWATER(xx + 17 + i, yy + 5 + j) && (rand() % 2))
                MP_INFO(xx + 17 + i, yy + 5 + j).flags |= FLAG_HAS_UNDERGROUND_WATER;

    set_mappoint(xx + 14, yy + 9, CST_MARKET_EMPTY);
    marketx[numof_markets] = xx + 14;
    markety[numof_markets] = yy + 9;
    numof_markets++;
    /* Bootstrap markets with some stuff. */
    MP_INFO(xx + 14, yy + 9).int_1 = 2000;
    MP_INFO(xx + 14, yy + 9).int_2 = 10000;
    MP_INFO(xx + 14, yy + 9).int_3 = 100;
    MP_INFO(xx + 14, yy + 9).int_5 = 10000;
    MP_INFO(xx + 14, yy + 9).flags
        |= (FLAG_MB_FOOD + FLAG_MS_FOOD + FLAG_MB_JOBS
            + FLAG_MS_JOBS + FLAG_MB_COAL + FLAG_MS_COAL + FLAG_MB_ORE
            + FLAG_MS_ORE + FLAG_MB_GOODS + FLAG_MS_GOODS + FLAG_MB_STEEL + FLAG_MS_STEEL);

    /* build tracks */
    for (x = 2; x < 23; x++) {
        set_mappoint(xx + x, yy + 11, CST_TRACK_LR);
        MP_INFO(xx + x, yy + 11).flags |= FLAG_IS_TRANSPORT;
    }
    for (y = 2; y < 11; y++) {
        set_mappoint(xx + 13, yy + y, CST_TRACK_LR);
        MP_INFO(xx + 13, yy + y).flags |= FLAG_IS_TRANSPORT;
    }
    for (y = 12; y < 23; y++) {
        set_mappoint(xx + 15, yy + y, CST_TRACK_LR);
        MP_INFO(xx + 15, yy + y).flags |= FLAG_IS_TRANSPORT;
    }

    /* build communes */
    set_mappoint(xx + 6, yy + 12, CST_COMMUNE_1);
    set_mappoint(xx + 6, yy + 17, CST_COMMUNE_1);
    set_mappoint(xx + 11, yy + 12, CST_COMMUNE_1);
    set_mappoint(xx + 11, yy + 17, CST_COMMUNE_1);
    set_mappoint(xx + 16, yy + 12, CST_COMMUNE_1);
    set_mappoint(xx + 16, yy + 17, CST_COMMUNE_1);
}


