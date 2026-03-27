#ifndef CRUCKIG_BRAKE_H
#define CRUCKIG_BRAKE_H

#include <stdbool.h>

/* Two-phase brake profile */
typedef struct {
    double duration;
    double t[2];
    double j[2];
    double a[2];
    double v[2];
    double p[2];
} CRuckigBrakeProfile;

void cruckig_brake_init(CRuckigBrakeProfile *bp);

/* Calculate brake trajectories */
void cruckig_brake_get_position_brake_trajectory(CRuckigBrakeProfile *bp, double v0, double a0,
                                         double vMax, double vMin, double aMax, double aMin, double jMax);
void cruckig_brake_get_second_order_position_brake_trajectory(CRuckigBrakeProfile *bp, double v0,
                                                      double vMax, double vMin, double aMax, double aMin);
void cruckig_brake_get_velocity_brake_trajectory(CRuckigBrakeProfile *bp, double a0,
                                         double aMax, double aMin, double jMax);
void cruckig_brake_get_second_order_velocity_brake_trajectory(CRuckigBrakeProfile *bp);

/* Finalize by integrating */
void cruckig_brake_finalize(CRuckigBrakeProfile *bp, double *ps, double *vs, double *as);
void cruckig_brake_finalize_second_order(CRuckigBrakeProfile *bp, double *ps, double *vs, double *as);

#endif /* CRUCKIG_BRAKE_H */
