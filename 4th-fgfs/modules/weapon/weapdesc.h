typedef struct _wdsc {
        int             mask;           /* key mask */
        int             (*select)();    /* weapon select procedure */
        int             (*update)();    /* per tick update procedure */
        int             (*display)();   /* display update procedure */
        int             (*firePress)(); /* fire button pressed procedure */
        int             (*fireRelease)(); /* fire button released */
        craftType       *w;             /* description of this weapon */
        }               weaponDesc;
