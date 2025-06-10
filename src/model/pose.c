#include "log.c/log.h"
#include "model_internal.h"
#include <stdlib.h>

void cstrl_model_pose_init(pose_t *pose, int count)
{
    pose->joints = malloc(sizeof(transform_t) * count);
    if (!pose->joints)
    {
        log_error("Failed to malloc joints");
        return;
    }
    pose->parents = malloc(sizeof(int) * count);
    if (!pose->parents)
    {
        free(pose);
        log_error("Failed to malloc parents");
        return;
    }
    pose->count = count;
}

void cstrl_model_pose_free(pose_t *pose)
{
    free(pose->joints);
    pose->joints = NULL;
    free(pose->parents);
    pose->parents = NULL;
    pose->count = 0;
}

transform_t cstrl_model_pose_get_global_transform(pose_t *pose, int joint)
{
    transform_t result = pose->joints[joint];
    for (int p = pose->parents[joint]; p >= 0; p = pose->parents[p])
    {
        result = combine(pose->joints[p], result);
    }

    return result;
}
