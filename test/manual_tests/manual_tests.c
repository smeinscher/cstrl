//
// Created by 12105 on 12/1/2024.
//

#include "manual_tests.h"

#include "../test_manager/test_types.h"
#include "basic/test_4_cubes_scene.h"

int run_manual_tests()
{
    int result = 0;

    result += test_4_cubes_scene();

    return result;
}
