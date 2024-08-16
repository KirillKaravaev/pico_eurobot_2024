// https://forum.arduino.cc/t/good-news-dmp-from-mpu6050-can-be-used-without-interrupt-pin/393797

#define I2C_PORT i2c0
#define SCL 13
#define SDA 12
// Для снижения нагрузки на процессор в файле MPU6050_6Axis_Motion увеличиваем значение делителя #define MPU6050_DMP_FIFO_RATE_DIVISOR 0x05

class IMU {
   public:
    struct data {
        // velocities
        //            float lin_vel_x;
        //            float lin_vel_y;
        //            float ang_vel_z;
        // angles
        float ang_x;
        float ang_y;
        float ang_z;
        // accelerations
        float lin_accel_x;
        float lin_accel_y;
        //            float ang_accel_z;
    };
    struct velocities {
        float lin_vel_x;
        float lin_vel_y;
    };

    void I2C_init();
    void imu_init();
    data get_data();
};
