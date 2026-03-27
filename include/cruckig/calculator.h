#ifndef CRUCKIG_CALCULATOR_H
#define CRUCKIG_CALCULATOR_H

#include <stdbool.h>
#include <stddef.h>
#include <cruckig/result.h>
#include <cruckig/block.h>
#include <cruckig/input_parameter.h>
#include <cruckig/trajectory.h>

typedef struct {
    size_t degrees_of_freedom;

    double *new_phase_control;
    double *pd;
    double *possible_t_syncs;
    size_t *idx;

    CRuckigBlock *blocks;
    double *inp_min_velocity;
    double *inp_min_acceleration;
    CRuckigControlInterface *inp_per_dof_control_interface;
    CRuckigSynchronization *inp_per_dof_synchronization;

    /* Scratch space for waypoint calculation */
    CRuckigInputParameter *segment_input;  /* Reusable per-segment input */
} CRuckigCalculator;

CRuckigCalculator* cruckig_calculator_create(size_t dofs);
void cruckig_calculator_destroy(CRuckigCalculator *calc);

/* Single-segment calculation (existing, backward compatible) */
CRuckigResult cruckig_calculator_calculate(CRuckigCalculator *calc,
                                           const CRuckigInputParameter *inp,
                                           CRuckigTrajectory *traj,
                                           double delta_time,
                                           bool *was_interrupted);

/* Multi-segment waypoint calculation */
CRuckigResult cruckig_calculator_calculate_waypoints(CRuckigCalculator *calc,
                                                     const CRuckigInputParameter *inp,
                                                     CRuckigTrajectory *traj,
                                                     double delta_time,
                                                     bool *was_interrupted);

/* Continue an interrupted calculation */
CRuckigResult cruckig_calculator_continue(CRuckigCalculator *calc,
                                          const CRuckigInputParameter *inp,
                                          CRuckigTrajectory *traj,
                                          double delta_time,
                                          bool *was_interrupted);

#endif /* CRUCKIG_CALCULATOR_H */
