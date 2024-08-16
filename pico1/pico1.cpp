#include <rcl/error_handling.h>
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <rmw_microros/rmw_microros.h>
#include <std_msgs/msg/int32.h>
#include <stdio.h>

#include "kinematics.h"
#include "motors.h"
#include "pico/stdlib.h"
#include "pico_uart_transports.c"  //только так (расширене не .h ,а .c) работает передача данных по юарт.
#include "rpm.h"
#include "servo.h"
#include "imu.h"

#include <geometry_msgs/msg/quaternion.h>
#include <geometry_msgs/msg/twist.h>
#include <geometry_msgs/msg/vector3.h>
#include <sensor_msgs/msg/imu.h>

rpm current_rpm;
imp_num current_imp_num;
struct repeating_timer timer;

const uint LED_PIN = 25;
int cnt = 1;

rcl_publisher_t imu_publisher;
sensor_msgs__msg__Imu imu_msg;

rcl_publisher_t rpm_publisher;
geometry_msgs__msg__Quaternion rpm_msg;

rcl_subscription_t twist_subscriber;
geometry_msgs__msg__Twist twist_msg;

rcl_subscription_t servo_subscriber;
geometry_msgs__msg__Vector3 servo_msg;

 IMU imu;
 IMU::data imu_dat;

Kinematics kinematics(
    Kinematics::LINO_BASE,
    MOTOR_MAX_RPM,
    MAX_RPM_RATIO,
#ifdef BRUSH_MOTORS
    MOTOR_OPERATING_VOLTAGE,
    MOTOR_POWER_MAX_VOLTAGE,
#endif
#ifdef BRUSHLESS_MOTORS
    MAX_RPM,
#endif
    WHEEL_DIAMETER,
    LR_WHEELS_DISTANCE);

Kinematics::velocities cmd_vel;

void blink() {
    if (cnt == 1) {
        gpio_put(LED_PIN, 1);
        cnt = 0;
    } else if (cnt == 0) {
        gpio_put(LED_PIN, 0);
        cnt = 1;
    }
}

void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
    rpm_msg.w = current_rpm.rpm1;
    rpm_msg.x = current_rpm.rpm2;
    rpm_msg.y = current_rpm.rpm3;
    rpm_msg.z = current_rpm.rpm4;

    rcl_ret_t ret1 = rcl_publish(&rpm_publisher, &rpm_msg, NULL);

    imu_dat = imu.get_data();

    imu_msg.angular_velocity.z = imu_dat.ang_z;  //Отправляется не угловая скорость, а угол!!!!!!
    imu_msg.linear_acceleration.x = imu_dat.lin_accel_x;
    imu_msg.linear_acceleration.y = imu_dat.lin_accel_y;

    rcl_ret_t ret = rcl_publish(&imu_publisher, &imu_msg, NULL);
}

void twist_subscriber_callback(const void *msgin) {
    const geometry_msgs__msg__Twist *msg = (const geometry_msgs__msg__Twist *)msgin;

    // Kinematics::velocities cmd_vel;
    cmd_vel.linear_x = msg->linear.x;
    cmd_vel.linear_y = msg->linear.y;
    cmd_vel.angular_z = msg->angular.z;

    Kinematics::rpm req_rpm = kinematics.getRPM(
        msg->linear.x,
        msg->linear.y,
        msg->angular.z);



    
    motor1_controller((int)req_rpm.motor1);  // обращаемся к элементу motor1
  
    motor2_controller((int)req_rpm.motor2);
    
    motor3_controller((int)req_rpm.motor3);
    
    motor4_controller((int)req_rpm.motor4);

    blink();
}

void servo_subscriber_callback(const void * msgin)
{
     const geometry_msgs__msg__Vector3 * msg = (const geometry_msgs__msg__Vector3 *)msgin;
     servo(msg->x, msg->y);
     blink();
}

int main() {
    rmw_uros_set_custom_transport(
        true,
        NULL,
        pico_serial_transport_open,
        pico_serial_transport_close,
        pico_serial_transport_write,
        pico_serial_transport_read);

    motors_init();
    servo_init();
    imu.imu_init();
    impulse_counter_init();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);



    rcl_timer_t timer;
    rcl_node_t node;
    rcl_allocator_t allocator;
    rclc_support_t support;
    rclc_executor_t executor;

    allocator = rcl_get_default_allocator();

    // Wait for agent successful ping for 2 minutes.
    const int timeout_ms = 1000;
    const uint8_t attempts = 120;

    rcl_ret_t ret = rmw_uros_ping_agent(timeout_ms, attempts);

    if (ret != RCL_RET_OK) {
        // Unreachable agent, exiting program.
        return ret;
    }

    rclc_support_init(&support, 0, NULL, &allocator);

    rclc_node_init_default(&node, "pico_node", "", &support);

    rclc_subscription_init_default(
        &twist_subscriber,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
        "cmd_vel");

    rclc_subscription_init_default(
        &servo_subscriber,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Vector3),
        "servo_topic");

    rclc_publisher_init_default(
        &imu_publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
        "imu_topic");

    rclc_publisher_init_default(
        &rpm_publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Quaternion),
        "rpm_topic");

    rclc_timer_init_default(
        &timer,
        &support,
        RCL_MS_TO_NS(200),
        timer_callback);


    rclc_executor_init(&executor, &support.context, 3, &allocator);
    rclc_executor_add_timer(&executor, &timer);
   

    rclc_executor_add_subscription(&executor, &twist_subscriber, &twist_msg, &twist_subscriber_callback, ON_NEW_DATA);
    rclc_executor_add_subscription(&executor, &servo_subscriber, &servo_msg, &servo_subscriber_callback, ON_NEW_DATA);
   
    while (true) {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
    }
    return 0;
}
