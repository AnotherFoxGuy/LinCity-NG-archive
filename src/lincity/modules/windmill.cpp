/* ---------------------------------------------------------------------- *
 * windmill.c
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * (c) Corey Keasling, 2004
 * ---------------------------------------------------------------------- */
#include "modules.h"
#include "../power.h"
#include "windmill.h"

/*** Windmills ***/
/*
  // int_1 is the rated capacity 
  // int_2 unused
  // int_3 is the sail count - to choose the right sail.
  // int_4 reserved = local power demand for substations (like substations)
  // int_5 is the power produced (basically _if_ power produced)
  // int_6 is the grid it's on
  // int_7 is a timestamp for mapping
  // 
  // MP_ANIM(x,y) is the last real time that a sail was turned  (since 1.91)
  // MP_TECH(x,y) is the tech_level at build time
*/
void do_windmill(int x, int y)
{
    int anim_tile;

    if (get_jobs(x, y, WINDMILL_JOBS) != 0) {
        MP_INFO(x, y).int_5 = MP_INFO(x, y).int_1;
        grid[MP_INFO(x, y).int_6]->avail_power += MP_INFO(x, y).int_1;
    } else {
        MP_INFO(x,y).int_5 = 0;
        MP_ANIM(x, y) = real_time + MODERN_WINDMILL_ANIM_SPEED;
        return;
    }

    /* update animation */
    if (real_time > MP_ANIM(x , y)) {
        MP_INFO(x, y).int_3++;
        if (MP_TECH(x, y) < MODERN_WINDMILL_TECH) {
            MP_ANIM(x,y) = real_time + ANTIQUE_WINDMILL_ANIM_SPEED;
        } else {
            MP_ANIM(x,y) = real_time + MODERN_WINDMILL_ANIM_SPEED;
        }
    }

    /* figure out which tile to use */
    anim_tile = (MP_INFO(x, y).int_3 % 3);

    if (MP_TECH(x, y) < MODERN_WINDMILL_TECH)
        MP_TYPE(x, y) = CST_WINDMILL_1_W + anim_tile;
    else
        switch (grid[MP_INFO(x, y).int_6]->powered) {
        case -1:
            MP_TYPE(x, y) = CST_WINDMILL_1_R + anim_tile;
            break;
        case 0:
            MP_TYPE(x, y) = CST_WINDMILL_1_RG + anim_tile;
            break;
        case 1:
            MP_TYPE(x, y) = CST_WINDMILL_1_G + anim_tile;
            break;
        default:
            printf("Default case in do_power_substation\n");
            break;
        }
}

void mps_windmill(int x, int y)
{
    int i = 0;
    char s[12];

    mps_store_title(i++, _("Windmill"));
    mps_store_sfp(i++, _("Tech"), (MP_TECH(x, y) * 100.0) / MAX_TECH_LEVEL);
    mps_store_sfp(i++, _("Jobs"), (MP_INFO(x, y).int_5 * 100.0) / MP_INFO(x, y).int_1); // either 0 or 100%
    i++;

    if (MP_TECH(x, y) >= MODERN_WINDMILL_TECH) {
        mps_store_title(i++, _("Local Status"));

        format_power(s, sizeof(s), MP_INFO(x, y).int_5);
        mps_store_ss(i++, _("Prod."), s);

        format_power(s, sizeof(s), MP_INFO(x, y).int_4);
        mps_store_ss(i++, _("Demand"), s);
        i++;

        mps_store_title(i++, _("Grid Status"));

        format_power(s, sizeof(s), grid[MP_INFO(x, y).int_6]->max_power);
        mps_store_ss(i++, _("T. Cap."), s);

        format_power(s, sizeof(s), grid[MP_INFO(x, y).int_6]->avail_power);
        mps_store_ss(i++, _("A. Cap."), s);

        format_power(s, sizeof(s), grid[MP_INFO(x, y).int_6]->demand);
        mps_store_ss(i++, _("Demand"), s);
        mps_store_sd(i++, _("Grid ID"), MP_INFO(x, y).int_6);
    }
}
