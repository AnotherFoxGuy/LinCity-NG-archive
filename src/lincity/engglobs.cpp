/* ---------------------------------------------------------------------- *
 * engglobs.c
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * ---------------------------------------------------------------------- */
#include "engglobs.h"
//#include "common.h"
#include "fileutil.h"

/* load/save version for compatibility with(out) waterwell */
int ldsv_version;
int use_waterwell=true;

Map map;

int mappoint_array_x[WORLD_SIDE_LEN];
int mappoint_array_y[WORLD_SIDE_LEN];
int numof_shanties;
int numof_communes;
int main_screen_originx, main_screen_originy;

int pix_double = 0;

int cheat_flag;
int modern_windmill_flag = 0;

int askdir_lines;
char *askdir_path[4];

int monthgraph_size = 0;
int *monthgraph_pop;
int *monthgraph_starve;
int *monthgraph_nojobs;
int *monthgraph_ppool;

int sust_dig_ore_coal_tip_flag = 0, sust_port_flag = 0, sustain_flag = 0;
int sust_dig_ore_coal_count = 0, sust_port_count = 0, sust_old_money = 0;
int sust_old_money_count = 0, sust_old_population = 0, sust_old_population_count = 0;
int sust_old_tech = 0, sust_old_tech_count = 0, sust_fire_count = 0;

int total_time = 0;             /* game time */
int deadline = 0; 

int population, starving_population;
int housed_population;
int unemployed_population, people_pool;
int substationx[MAX_NUMOF_SUBSTATIONS], substationy[MAX_NUMOF_SUBSTATIONS];
int numof_substations = 0;
int marketx[MAX_NUMOF_MARKETS], markety[MAX_NUMOF_MARKETS], numof_markets = 0;
int numof_health_centres, max_pop_ever = 0, total_evacuated = 0, total_births = 0;
int numof_waterwell = 0;

int total_money = 0, income_tax_rate, coal_tax_rate;
int dole_rate, transport_cost_rate;
int goods_tax_rate;
int export_tax_rate, import_cost_rate;
int tech_level, highest_tech_level, unnat_deaths;

int total_pollution_deaths = 0, total_starve_deaths = 0, total_unemployed_days = 0;
int total_unemployed_years = 0;
float pollution_deaths_history = 0.0, starve_deaths_history = 0.0;
float unemployed_history = 0.0;

int university_intake_rate;
int power_made, power_used, coal_made, coal_used;
int goods_made, goods_used, ore_made, ore_used;
int rockets_launched, rockets_launched_success;
int coal_survey_done;

int selected_type_cost;

int ex_tax_dis[NUMOF_DISCOUNT_TRIGGERS] = {
    EX_DISCOUNT_TRIGGER_1,
    EX_DISCOUNT_TRIGGER_2,
    EX_DISCOUNT_TRIGGER_3,
    EX_DISCOUNT_TRIGGER_4,
    EX_DISCOUNT_TRIGGER_5,
    EX_DISCOUNT_TRIGGER_6,
};


int  global_aridity;
int  global_mountainity;
ground_struct ground[WORLD_SIDE_LEN][WORLD_SIDE_LEN];
int alt_min, alt_max, alt_step;

int fast_time_for_year;
