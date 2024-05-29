#ifndef WEAPON_H
#define WEAPON_H


typedef struct _wctl {                                           
        char            *type;          /* type of weapon at this station */
        int             info;           /* extra info (weapon specific) */
        int             info2;          /* "      " */
        int             info3;          /* "      " */
        }               weaponStation;

#define WK_M61A1        0x01            /* M-61A1 Vulcan 20mm cannon */
#define WK_AIM9M        0x02            /* Sidewinder missile */


#endif WEAPON_H
