/* ---------------------------------------------------------------------- *
 * commune.c
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * (c) Corey Keasling, 2004
 * ---------------------------------------------------------------------- */

#include "modules.h"
#include "commune.h"
#include <cstdlib>
#include <math.h>

void do_commune(int x, int y)
{
    /*
       // int_1 unused
       // int_2 is the steelflag/trackflag
       // int_3 is the coal sold in the last 100 days 200 units is 100%
       // steel adds more.
       // int_4 is the months without selling much coal,steel,ore
       // int_5 is the coal, ore, steel waste flags for last month
       // int_6 is the coal, ore, steel waste flags so far this month
       //
       // MP_ANIM is the animation trigger time  since 1.91
     */
    /* GCS -- I folded the trackflag into int_2, changing the logic slightly.
       This change only affects the animation. */
    int trackflag = 0;
    int coalprod = 2;
    /* stick coal and ore on tracks, in SMALL doses. */
    if (use_waterwell) {
        int w = 0;
        int n = 0;
        /* Check underground water, and reduce production accordingly */
        for (int i = 0; i < MP_SIZE(x, y); i++) {
            for (int j = 0; j < MP_SIZE(x, y); j++) {
                n++;
                if (HAS_UGWATER(x + i, y + j))
                    w++;
            }
        }
        if (w < n / 3)
            coalprod = 0;
        else if (w < (2 * n) / 3)
            coalprod = 1;
    }
    if (coalprod > 0) {
        if (put_coal(x, y, coalprod) != 0) {
            trackflag = 1;
            MP_INFO(x, y).int_3++;
            MP_INFO(x, y).int_6 |= 1;
        }
    }
    if (put_ore(x, y, 6) != 0) {
        trackflag = 1;
        MP_INFO(x, y).int_3++;
        MP_INFO(x, y).int_6 |= 2;
    }
    /* recycle a bit of waste */
    if (get_waste(x, y, 20) != 0) {
        trackflag = 1;
        MP_INFO(x, y).int_3++;
        MP_INFO(x, y).int_6 |= 8;
    }
    if (total_time % 10 == 0) {
        MP_INFO(x, y).int_2 = 1;
        if (put_steel(x, y, 2) != 0) {
            MP_INFO(x, y).int_3++;
            MP_INFO(x, y).int_6 |= 4;
        } else {
            MP_INFO(x, y).int_2 = 0;
        }
        if (trackflag) {
            MP_INFO(x, y).int_2 = 0;
        }
    }

    /* each month. /AL1: is there a reason for day 48 ? */
    if (total_time % 100 == 1) {
        MP_INFO(x, y).int_5 = MP_INFO(x, y).int_6;
        MP_INFO(x, y).int_6 = 0;
        if (MP_INFO(x, y).int_5 & 4) {  //producing steel
            if (MP_TYPE(x, y) < CST_COMMUNE_7) {
                MP_TYPE(x, y) += 5;
            }
        } else {
            if (MP_TYPE(x, y) >= CST_COMMUNE_7) {
                MP_TYPE(x, y) -= 5;
            }
        }

        if (MP_INFO(x, y).int_3 > 0) {  /*  >0% */
            MP_INFO(x, y).int_3 = 0;
            if (--MP_INFO(x, y).int_4 < 0)
                MP_INFO(x, y).int_4 = 0;
        } else {
            MP_INFO(x, y).int_3 = 0;
            MP_INFO(x, y).int_4++;
            /* Communes without production only last 10 years */
            if (MP_INFO(x, y).int_4 > 120) {    /* 10 years */
                if (use_waterwell) {
                    int s = MP_SIZE(x, y);
                    /* first destroy the commune, then fill it with correct tiles
                     * maybe paranoid, but tile is 4x4 thus becomes 16 tiles 1x1
                     */
                    do_bulldoze_area(CST_DESERT, x, y);
                    for (int i = 0; i < s; i++) {
                        for (int j = 0; j < s; j++)
                            if (HAS_UGWATER(x + i, y + j))
                                do_bulldoze_area(CST_PARKLAND_PLANE, x + i, y + j);
                            else
                                do_bulldoze_area(CST_DESERT, x + i, y + j);
                    }
                } else {
                    do_bulldoze_area(CST_PARKLAND_PLANE, x, y);
                }
                return;
            }
        }
    }

    /* animate */
    if (real_time >= MP_ANIM(x, y)) {
        MP_ANIM(x, y) = real_time + COMMUNE_ANIM_SPEED - 25 + (rand() % 50);
        switch (MP_TYPE(x, y)) {
        case (CST_COMMUNE_1):
            MP_TYPE(x, y) = CST_COMMUNE_2;
            break;
        case (CST_COMMUNE_2):
            MP_TYPE(x, y) = CST_COMMUNE_3;
            break;
        case (CST_COMMUNE_3):
            MP_TYPE(x, y) = CST_COMMUNE_4;
            break;
        case (CST_COMMUNE_4):
            MP_TYPE(x, y) = CST_COMMUNE_5;
            break;
        case (CST_COMMUNE_5):
            MP_TYPE(x, y) = CST_COMMUNE_6;
            break;
        case (CST_COMMUNE_6):
            MP_TYPE(x, y) = CST_COMMUNE_2;
            break;
        case (CST_COMMUNE_7):
            MP_TYPE(x, y) = CST_COMMUNE_8;
            break;
        case (CST_COMMUNE_8):
            MP_TYPE(x, y) = CST_COMMUNE_9;
            break;
        case (CST_COMMUNE_9):
            MP_TYPE(x, y) = CST_COMMUNE_10;
            break;
        case (CST_COMMUNE_10):
            MP_TYPE(x, y) = CST_COMMUNE_11;
            break;
        case (CST_COMMUNE_11):
            MP_TYPE(x, y) = CST_COMMUNE_7;
            break;
        default:
            MP_TYPE(x, y) = CST_COMMUNE_1;
            break;
        }
        if (MP_INFO(x, y).int_5 == 0) { //not producing anything
            MP_TYPE(x, y) = CST_COMMUNE_1;
        }
    }

}

void mps_commune(int x, int y)
{
    int i = 0;
    const char *p;

    mps_store_title(i++, _("Commune"));
    i++;
    i++;
    mps_store_title(i++, _("Activity"));
    i++;
    p = (MP_INFO(x, y).int_5 & 1) ? _("Yes") : _("No");
    mps_store_ss(i++, _("Coal"), p);
    p = (MP_INFO(x, y).int_5 & 2) ? _("Yes") : _("No");
    mps_store_ss(i++, _("Ore"), p);
    p = (MP_INFO(x, y).int_5 & 4) ? _("Yes") : _("No");
    mps_store_ss(i++, _("Steel"), p);
    p = (MP_INFO(x, y).int_5 & 8) ? _("Yes") : _("No");
    mps_store_ss(i++, _("Waste"), p);
}
