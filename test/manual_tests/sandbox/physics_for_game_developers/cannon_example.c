//
// Created by 12105 on 12/5/2024.
//

#include "cstrl/cstrl_math.h"
#include "log.c/log.h"

static float Vm;        // Magnitude of the muzzle velocity, m/s
static float Alpha;     // Angle of y-axis (upward) to the cannon
                        // When angle is 0, cannon is pointing up
                        // When angle is 90, cannon is pointing horizontally
static float Gamma;     // Angle from x-axis, in the x-z plane to the cannon
                        // When angle is 0, cannon is pointing in the positive x-direction
                        // Positive values are toward the positive z-axis
static float L;         // This is the length of the cannon, m
static float Yb;        // This is the base elevation of the cannon, m
static float X;         // The x-position of the center of the target, m
static float Y;         // The y-position of the center of the target, m
static float Z;         // The z-position of the center of the target, m
static float Length;    // The length of the target measured along the x-axis, m
static float Width;     // The width of the target measured along the z-axis, m
static float Height;    // The height of the target measured along the y-axis, m
static vec3 s;          // The shell position (displacement) vector
static float shot_time; // The time from the instant the shell leaves the cannon, seconds
static float t_inc;     // The time increment to use when stepping through the simulation, seconds
static float g;         // Acceleration due to gravity, m/s^2

int do_simulation()
{
    float cos_x;
    float cos_y;
    float cos_z;
    float xe, ze;
    float b, Lx, Ly, Lz;
    float tx1, tx2, ty1, ty2, tz1, tz2;

    // step to the next time in the simulation
    shot_time += t_inc;

    // First calculate the direction cosines for the cannon orientation.
    // NOTE: since calculation is fixed (does not change), in real world applications
    // it would be better to calculate this once outside the function
    b = L * cosf((90.0f - Alpha) * cstrl_pi_180); // Projection of barrel onto x-z plane
    Lx = b * cosf(Gamma * cstrl_pi_180);          // x-component of barrel length
    Ly = L * cosf(Alpha * cstrl_pi_180);          // y-component of barrel length
    Lz = b * sinf(Gamma * cstrl_pi_180);          // z-component of barrel length

    cos_x = Lx / L;
    cos_y = Ly / L;
    cos_z = Lz / L;

    // These are the x and z coordinates of the very end of the cannon barrel
    // Use these as the initial x and z displacements
    xe = L * cosf((90.0f - Alpha) * cstrl_pi_180) * cosf(Gamma * cstrl_pi_180);
    ze = L * cosf((90.0f - Alpha) * cstrl_pi_180) * sinf(Gamma * cstrl_pi_180);

    // Calculate the position vector at this time
    s.x = Vm * cos_x * shot_time + xe;
    s.y = (Yb + L * cosf(Alpha * cstrl_pi_180)) + (Vm * cos_y * shot_time) - (0.5f * g * shot_time * shot_time);
    s.z = Vm * cos_z * shot_time + ze;

    // Check for collision with target
    // Get extents (bounding coordinates) of the target
    tx1 = X - Length / 2.0f;
    tx2 = X + Length / 2.0f;
    ty1 = Y - Height / 2.0f;
    ty2 = Y + Height / 2.0f;
    tz1 = Z - Width / 2.0f;
    tz2 = Z + Width / 2.0f;

    // Check to see if the shell has passed through the target
    // NOTE: using rudimentary collision detection; a practical problem is you may miss a collision if for a given time
    // step the shell's change in position is large enough to allow it to skip over the target.
    // A better approach would be to look at the previous time step's position data and to check the line from the
    // previous position to the current position to see if that line intersects the target bounding box
    if (s.x >= tx1 && s.x <= tx2 && s.y >= ty1 && s.y <= ty2 && s.z >= tz1 && s.z <= tz2)
    {
        return 1;
    }
    if (s.y <= 0.0f)
    {
        return 2;
    }
    if (shot_time > 3600)
    {
        return 3;
    }

    return 0;
}

void cannon_example()
{
    // Cannon Properties
    Vm = 50.0f;
    Alpha = 25.0f;
    Gamma = 8.0f;
    L = 65.0f;
    Yb = 10.0f;

    // Target Properties
    X = 400.0f;
    Y = 75.0f;
    Z = 45.0f;
    Length = 60.0f;
    Width = 50.0f;
    Height = 80.0f;

    // Other
    t_inc = 0.1f;
    g = 9.8f;
    int status = 0;
    shot_time = 0.0f;

    while (status == 0)
    {
        status = do_simulation();
        switch (status)
        {
        case 1:
            log_info("Target Hit!");
            break;
        case 2:
            log_info("Target Missed");
            break;
        case 3:
            log_info("Timed out");
            break;
        default:
            break;
        }
    }
}
