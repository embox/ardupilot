//
// Unit tests for the AP_Math rotations code
//

#include <AP_HAL/AP_HAL.h>
#include <AP_Math/AP_Math.h>
//#include <AP_CustomRotations/AP_CustomRotations.h>

//AP_CustomRotations cust_rot;

void setup();
void loop();

const AP_HAL::HAL& hal = AP_HAL::get_HAL();
#if 0
static void print_vector(Vector3f &v)
{
    hal.console->printf("[%.4f %.4f %.4f]\n",
                        (double)v.x,
                        (double)v.y,
                        (double)v.z);
}
#endif

static void test_quaternion_normalize(void)
{
    for (int i = 0; i < 1000000000; i ++) {
        QuaternionF quat;
        quat.q1 = 1.0f;
        quat.q2 = 0.0f;
        quat.q3 = 0.0f;
        quat.q4 = 0.0f;
        quat.normalize();
        if (quat.check_has_nan() ) {
            hal.console->printf("test_quaternion_normalize break at[%d]\n", i); 
        }
    }
    hal.console->printf("test_quaternion_normalize done\n");
}



/*
 *  rotation tests
 */
void setup(void)
{
    hal.console->begin(115200);
    hal.console->printf("Quaternion unit tests\n\n");

    test_quaternion_normalize();
    hal.console->printf("\n\n");

    hal.console->printf("rotation unit tests done\n\n");

    return;
}

void loop(void) {}

AP_HAL_MAIN();
