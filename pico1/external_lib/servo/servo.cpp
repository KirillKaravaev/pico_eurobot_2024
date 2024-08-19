#include "servo.h"

#include <stdlib.h>

#include "hardware/pwm.h"
#include "pico/stdlib.h"

/*https://www.webwork.co.uk/2023/06/raspberry-pi-pico-as-switching_21.html */

void servo(int number, int angle) {
    if (number == 1) {
        // множитель 11.11 = 2000/180. тогда при подаче сигнала поворота на 180 градусов получим значение level = 500+180*11.11 = 2500, 
        //что и отвечает необходимому углу
        pwm_set_gpio_level(SERVO_PWM_PIN_1, (500 + angle * 11.11));

    }

    else if (number == 2) {
        pwm_set_gpio_level(SERVO_PWM_PIN_2, (500 + angle * 11.11));

    }

    else if (number == 3) {
        pwm_set_gpio_level(SERVO_PWM_PIN_3, (500 + angle * 11.11));

    }

    else if (number == 4) {
        pwm_set_gpio_level(SERVO_PWM_PIN_4, (500 + angle * 11.11));

    }

    else if (number == 5) {
        pwm_set_gpio_level(SERVO_PWM_PIN_5, (500 + angle * 11.11));

    }

    else if (number == 6) {
        pwm_set_gpio_level(SERVO_PWM_PIN_6, (500 + angle * 11.11));
    }
}

// Функция инициализации ШИМ-выводов для управления сервоприводами
void servo_init() {
    // https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=1

    // Выделяем пины SERVO_PWM_PIN_i под генерацию ШИМА
    gpio_set_function(SERVO_PWM_PIN_1, GPIO_FUNC_PWM);
    gpio_set_function(SERVO_PWM_PIN_2, GPIO_FUNC_PWM);
    gpio_set_function(SERVO_PWM_PIN_3, GPIO_FUNC_PWM);
    gpio_set_function(SERVO_PWM_PIN_4, GPIO_FUNC_PWM);
    gpio_set_function(SERVO_PWM_PIN_5, GPIO_FUNC_PWM);
    gpio_set_function(SERVO_PWM_PIN_6, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice_num1 = pwm_gpio_to_slice_num(SERVO_PWM_PIN_1);
    uint slice_num2 = pwm_gpio_to_slice_num(SERVO_PWM_PIN_2);
    uint slice_num3 = pwm_gpio_to_slice_num(SERVO_PWM_PIN_3);
    uint slice_num4 = pwm_gpio_to_slice_num(SERVO_PWM_PIN_4);
    uint slice_num5 = pwm_gpio_to_slice_num(SERVO_PWM_PIN_5);
    uint slice_num6 = pwm_gpio_to_slice_num(SERVO_PWM_PIN_6);

    // Устанавливаем предделитель от тактовой частоты процессора (133[Мгц])
    pwm_set_clkdiv(slice_num1, SERVO_PWM_DIV);
    pwm_set_clkdiv(slice_num2, SERVO_PWM_DIV);
    pwm_set_clkdiv(slice_num3, SERVO_PWM_DIV);
    pwm_set_clkdiv(slice_num4, SERVO_PWM_DIV);
    pwm_set_clkdiv(slice_num5, SERVO_PWM_DIV);
    pwm_set_clkdiv(slice_num6, SERVO_PWM_DIV);

    // Устанавливаем частоту ШИМА равной 133[Мгц]/SERVO_PWM_DIV/SERVO_PWM_WRAP
    pwm_set_wrap(slice_num1, SERVO_PWM_WRAP);
    pwm_set_wrap(slice_num2, SERVO_PWM_WRAP);
    pwm_set_wrap(slice_num3, SERVO_PWM_WRAP);
    pwm_set_wrap(slice_num4, SERVO_PWM_WRAP);
    pwm_set_wrap(slice_num5, SERVO_PWM_WRAP);
    pwm_set_wrap(slice_num6, SERVO_PWM_WRAP);

    // Задаем начальное заполнение шим 2.5% (500/20 000), что отвечает ширине импульса 0.5 мс и повороту сервопривода на 0 градусов
    pwm_set_gpio_level(SERVO_PWM_PIN_1, 500);
    pwm_set_gpio_level(SERVO_PWM_PIN_2, 500);
    pwm_set_gpio_level(SERVO_PWM_PIN_3, 500);
    pwm_set_gpio_level(SERVO_PWM_PIN_4, 500);
    pwm_set_gpio_level(SERVO_PWM_PIN_5, 500);
    pwm_set_gpio_level(SERVO_PWM_PIN_6, 500);

    // Включаем генерацию ШИМ
    pwm_set_enabled(slice_num1, true);
    pwm_set_enabled(slice_num2, true);
    pwm_set_enabled(slice_num3, true);
    pwm_set_enabled(slice_num4, true);
    pwm_set_enabled(slice_num5, true);
    pwm_set_enabled(slice_num6, true);
}
