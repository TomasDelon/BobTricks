#include "tests/TestSupport.h"

#include <cmath>

#include "core/locomotion/LegIK.h"
#include "core/physics/Geometry.h"

int main()
{
    TestSuite suite("core_unit_tests");

    {
        const double L = 0.36;
        const double d_pref = 0.90;
        const double ratio = 0.75;
        const double expected = std::sqrt(4.0 * L * L - std::pow(0.5 * d_pref * L, 2.0))
                              + ratio * L;
        TEST_EXPECT_NEAR(suite, computeNominalY(L, d_pref, ratio), expected, 1e-9);
    }

    {
        const double low = computeNominalY(0.36, 0.90, 0.60);
        const double high = computeNominalY(0.36, 0.90, 0.85);
        TEST_EXPECT(suite, high > low);
    }

    {
        const Vec2 pelvis{0.0, 0.9};
        const Vec2 foot{0.3, 0.1};
        const double L = 0.5;
        const LegIKResult ik = computeKnee(pelvis, foot, L, 1.0);

        TEST_EXPECT_NEAR(suite, (ik.knee - pelvis).length(), L, 1e-6);
        TEST_EXPECT_NEAR(suite, (ik.knee - ik.foot_eff).length(), L, 1e-6);
        TEST_EXPECT_NEAR(suite, ik.foot_eff.x, foot.x, 1e-9);
        TEST_EXPECT_NEAR(suite, ik.foot_eff.y, foot.y, 1e-9);
    }

    {
        const Vec2 pelvis{0.0, 0.9};
        const Vec2 far_foot{2.0, 0.0};
        const double L = 0.5;
        const LegIKResult ik = computeKnee(pelvis, far_foot, L, -1.0);

        TEST_EXPECT(suite, (ik.foot_eff - pelvis).length() <= 2.0 * L);
        TEST_EXPECT_NEAR(suite, (ik.knee - pelvis).length(), L, 1e-6);
        TEST_EXPECT_NEAR(suite, (ik.knee - ik.foot_eff).length(), L, 1e-6);
        TEST_EXPECT(suite, ik.knee.x < pelvis.x + 0.5 * (ik.foot_eff.x - pelvis.x));
    }

    return suite.finish();
}
