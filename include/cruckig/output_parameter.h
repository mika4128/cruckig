#ifndef CRUCKIG_OUTPUT_PARAMETER_H
#define CRUCKIG_OUTPUT_PARAMETER_H

#include <stdbool.h>
#include <stddef.h>
#include <cruckig/trajectory.h>
#include <cruckig/input_parameter.h>

typedef struct {
    size_t degrees_of_freedom;

    CRuckigTrajectory *trajectory;

    double *new_position;
    double *new_velocity;
    double *new_acceleration;
    double *new_jerk;

    double time;
    size_t new_section;
    bool did_section_change;
    bool new_calculation;
    bool was_calculation_interrupted;
    double calculation_duration; /* microseconds */
} CRuckigOutputParameter;

CRuckigOutputParameter* cruckig_output_create(size_t dofs);
void cruckig_output_destroy(CRuckigOutputParameter *out);
void cruckig_output_pass_to_input(const CRuckigOutputParameter *out, CRuckigInputParameter *inp);

#endif /* CRUCKIG_OUTPUT_PARAMETER_H */
