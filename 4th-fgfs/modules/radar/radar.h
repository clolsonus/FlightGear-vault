#ifndef RADAR_H
#define RADAR_H


/*
 *  Radar tracking information                                     
 */
 
typedef struct _radarInfo {                     
        int     beamID;                 /* index number of beam */
        VPoint  rel;                    /* location relative to radar set */
        double  d;                      /* distance to target (feet) */
        int     targetID;               /* craft id of target */
        int     locked;                 /* 1=we have a "lock" */
        } radarInfo;


#endif RADAR_H
