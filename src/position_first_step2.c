#include <cruckig/cruckig_config.h>
#include <math.h>
#include <float.h>

#include <cruckig/position.h>
#include <cruckig/block.h>
#include <cruckig/profile.h>
#include <cruckig/roots.h>

void cruckig_pos1_step2_init(CRuckigPositionFirstOrderStep2 *s,
                     double tf, double p0, double pf, double vMax, double vMin)
{
    s->tf = tf;
    s->_vMax = vMax;
    s->_vMin = vMin;
    s->pd = pf - p0;
}

bool cruckig_pos1_step2_get_profile(CRuckigPositionFirstOrderStep2 *s, CRuckigProfile *profile)
{
    const double vf = s->pd / s->tf;

    profile->t[0] = 0;
    profile->t[1] = 0;
    profile->t[2] = 0;
    profile->t[3] = s->tf;
    profile->t[4] = 0;
    profile->t[5] = 0;
    profile->t[6] = 0;

    return cruckig_profile_check_for_first_order_with_timing_full(profile, ControlSignsUDDU, ReachedLimitsNONE,
                                                          s->tf, vf, s->_vMax, s->_vMin);
}
