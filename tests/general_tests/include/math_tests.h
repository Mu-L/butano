/*
 * Copyright (c) 2020 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see LICENSE file.
 */

#ifndef MATH_TESTS_H
#define MATH_TESTS_H

#include "bn_math.h"
#include "bn_fixed.h"
#include "bn_rule_of_three_approximation.h"
#include "tests.h"

class math_tests : public tests
{

public:
    math_tests() :
        tests("math")
    {
        BN_ASSERT(bn::rule_of_three_approximation(360, 65536).calculate(0) == 0);
        BN_ASSERT(bn::rule_of_three_approximation(360, 65536).calculate(90) == 16384);
        BN_ASSERT(bn::rule_of_three_approximation(360, 65536).calculate(180) == 32768);
        BN_ASSERT(bn::rule_of_three_approximation(360, 65536).calculate(270) == 49152);
        BN_ASSERT(bn::rule_of_three_approximation(360, 65536).calculate(360) == 65536);

        BN_ASSERT(bn::rule_of_three_approximation(bn::fixed(360).data(), 2048).calculate(bn::fixed(0).data()) == 0);
        BN_ASSERT(bn::rule_of_three_approximation(bn::fixed(360).data(), 2048).calculate(bn::fixed(90).data()) == 512);
        BN_ASSERT(bn::rule_of_three_approximation(bn::fixed(360).data(), 2048).calculate(bn::fixed(180).data()) == 1024);
        BN_ASSERT(bn::rule_of_three_approximation(bn::fixed(360).data(), 2048).calculate(bn::fixed(270).data()) == 1024 + 512);
        BN_ASSERT(bn::rule_of_three_approximation(bn::fixed(360).data(), 2048).calculate(bn::fixed(360).data()) == 2048);

        BN_ASSERT(bn::rule_of_three_approximation(bn::fixed(360).data(), 65536).calculate(bn::fixed(0).data()) == 0);
        BN_ASSERT(bn::rule_of_three_approximation(bn::fixed(360).data(), 65536).calculate(bn::fixed(90).data()) == 16384);
        BN_ASSERT(bn::rule_of_three_approximation(bn::fixed(360).data(), 65536).calculate(bn::fixed(180).data()) == 32768);
        BN_ASSERT(bn::rule_of_three_approximation(bn::fixed(360).data(), 65536).calculate(bn::fixed(270).data()) == 49152);
        BN_ASSERT(bn::rule_of_three_approximation(bn::fixed(360).data(), 65536).calculate(bn::fixed(360).data()) == 65536);

        BN_ASSERT(bn::degrees_sin(0) == 0);
        BN_ASSERT(bn::degrees_sin(30) == 0.5);
        BN_ASSERT(bn::degrees_sin(90) == 1);
        BN_ASSERT(bn::degrees_sin(150) == 0.5);
        BN_ASSERT(bn::degrees_sin(180) == 0);
        BN_ASSERT(bn::degrees_sin(210) == -0.5);
        BN_ASSERT(bn::degrees_sin(270) == -1);
        BN_ASSERT(bn::degrees_sin(330) == -0.5);
        BN_ASSERT(bn::degrees_sin(360) == 0);
        BN_ASSERT(bn::degrees_sin(360 + 90) == 1);
        BN_ASSERT(bn::degrees_sin(-90) == -1);

        BN_ASSERT(bn::degrees_cos(0) == 1);
        BN_ASSERT(bn::degrees_cos(90) == 0);
        BN_ASSERT(bn::degrees_cos(180) == -1);
        BN_ASSERT(bn::degrees_cos(270) == 0);
        BN_ASSERT(bn::degrees_cos(360) == 1);
        BN_ASSERT(bn::degrees_cos(360 + 90) == 0);
        BN_ASSERT(bn::degrees_cos(-90) == 0);

        BN_ASSERT(bn::lut_sin(0) == 0);
        BN_ASSERT(bn::lut_sin(512) == 1);
        BN_ASSERT(bn::lut_sin(1024) == 0);
        BN_ASSERT(bn::lut_sin(1024 + 512) == -1);
        BN_ASSERT(bn::lut_sin(2048) == 0);

        BN_ASSERT(bn::lut_cos(0) == 1);
        BN_ASSERT(bn::lut_cos(512) == 0);
        BN_ASSERT(bn::lut_cos(1024) == -1);
        BN_ASSERT(bn::lut_cos(1024 + 512) == 0);
        BN_ASSERT(bn::lut_cos(2048) == 1);

        BN_ASSERT(bn::degrees_lut_sin(0) == 0);
        BN_ASSERT(bn::degrees_lut_sin(90) == 1);
        BN_ASSERT(bn::degrees_lut_sin(180) == 0);
        BN_ASSERT(bn::degrees_lut_sin(270) == -1);
        BN_ASSERT(bn::degrees_lut_sin(360) == 0);

        BN_ASSERT(bn::degrees_lut_cos(0) == 1);
        BN_ASSERT(bn::degrees_lut_cos(90) == 0);
        BN_ASSERT(bn::degrees_lut_cos(180) == -1);
        BN_ASSERT(bn::degrees_lut_cos(270) == 0);
        BN_ASSERT(bn::degrees_lut_cos(360) == 1);
    }
};

#endif
