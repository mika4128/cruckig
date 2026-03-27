#ifndef CRUCKIG_H
#define CRUCKIG_H

#include <stdbool.h>
#include <stddef.h>

#include <cruckig/result.h>
#include <cruckig/input_parameter.h>
#include <cruckig/output_parameter.h>
#include <cruckig/trajectory.h>
#include <cruckig/calculator.h>

/* Main cruckig instance */
typedef struct {
    size_t degrees_of_freedom;
    double delta_time;
    size_t max_number_of_waypoints;

    CRuckigCalculator *calculator;
    CRuckigInputParameter *current_input;
    bool current_input_initialized;
} CRuckig;

/* Create and destroy (backward compatible: 0 waypoints) */
CRuckig* cruckig_create(size_t dofs, double delta_time);

/* Create with waypoint support */
CRuckig* cruckig_create_waypoints(size_t dofs, double delta_time, size_t max_waypoints);

void cruckig_destroy(CRuckig *r);

/* Reset (force recalculation on next update) */
void cruckig_reset(CRuckig *r);

/* Calculate trajectory (offline, auto-dispatches to waypoint calculator if needed) */
CRuckigResult cruckig_calculate(CRuckig *r, const CRuckigInputParameter *input,
                                CRuckigTrajectory *trajectory);

/* Update (online, call every delta_time) */
CRuckigResult cruckig_update(CRuckig *r, const CRuckigInputParameter *input,
                             CRuckigOutputParameter *output);

/* Validate input */
bool cruckig_validate_input(const CRuckig *r, const CRuckigInputParameter *input,
                            bool check_current_within_limits,
                            bool check_target_within_limits);

#endif /* CRUCKIG_H */
