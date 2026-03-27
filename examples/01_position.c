#include <stdio.h>
#include <cruckig/cruckig.h>

int main(void) {
    /* Create a 3-DOF cruckig instance with 10ms control cycle */
    CRuckig *otg = cruckig_create(3, 0.01);
    CRuckigInputParameter *inp = cruckig_input_create(3);
    CRuckigOutputParameter *out = cruckig_output_create(3);

    /* Set current state */
    inp->current_position[0] = 0.0;
    inp->current_position[1] = 0.0;
    inp->current_position[2] = 0.5;

    /* Set target state */
    inp->target_position[0] = 5.0;
    inp->target_position[1] = -2.0;
    inp->target_position[2] = -3.5;

    /* Set limits */
    inp->max_velocity[0] = 3.0;
    inp->max_velocity[1] = 1.0;
    inp->max_velocity[2] = 0.6;
    inp->max_acceleration[0] = 3.0;
    inp->max_acceleration[1] = 2.0;
    inp->max_acceleration[2] = 1.0;
    inp->max_jerk[0] = 4.0;
    inp->max_jerk[1] = 3.0;
    inp->max_jerk[2] = 2.0;

    printf("cruckig - C port of Ruckig trajectory generation\n\n");

    /* Run the trajectory */
    CRuckigResult result;
    int step = 0;
    while (1) {
        result = cruckig_update(otg, inp, out);

        if (step % 100 == 0 || result == CRuckigFinished) {
            printf("t=%.3f  pos=[%.4f, %.4f, %.4f]  vel=[%.4f, %.4f, %.4f]\n",
                   out->time,
                   out->new_position[0], out->new_position[1], out->new_position[2],
                   out->new_velocity[0], out->new_velocity[1], out->new_velocity[2]);
        }

        cruckig_output_pass_to_input(out, inp);
        step++;

        if (result == CRuckigFinished) {
            printf("\nTrajectory finished after %.4f seconds (%d steps)\n",
                   cruckig_trajectory_get_duration(out->trajectory), step);
            break;
        }

        if (result != CRuckigWorking) {
            printf("\nError: %d\n", result);
            break;
        }
    }

    cruckig_output_destroy(out);
    cruckig_input_destroy(inp);
    cruckig_destroy(otg);
    return 0;
}
