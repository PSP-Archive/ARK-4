#include <cpu.h>
#include <gpio.h>
#include <sysreg.h>
#include <interrupt.h>
#include <kirk.h>
#include <uart.h>
#include <utils.h>
#include <syscon.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int delay_us(int delay){
    int ret = 0;
    for (int i=0; i<delay; i++){
        ret++;
    }
    return ret;
}

int _init(){
    return 0;
}

int _fini(){
    return 0;
}

int main(void)
{
    sysreg_io_enable_gpio();

    // initialise syscon
    syscon_init();
    syscon_handshake_unlock();

    // turn on control for MS and WLAN leds
    syscon_ctrl_led(0, 1);
    syscon_ctrl_led(1, 1);

    // enable GPIO to control leds
    sysreg_io_enable_gpio_port(GPIO_PORT_MS_LED);
    sysreg_io_enable_gpio_port(GPIO_PORT_WLAN_LED);
    gpio_set_port_mode(GPIO_PORT_MS_LED, GPIO_MODE_OUTPUT);
    gpio_set_port_mode(GPIO_PORT_WLAN_LED, GPIO_MODE_OUTPUT);

    // turn off both LEDs
    gpio_set(GPIO_PORT_MS_LED);
    gpio_set(GPIO_PORT_WLAN_LED);
    delay_us(4*250000);
    delay_us(4*250000);
    gpio_set(GPIO_PORT_MS_LED);
    gpio_set(GPIO_PORT_WLAN_LED);

    while (1) {
        gpio_set(GPIO_PORT_MS_LED);
        gpio_clear(GPIO_PORT_WLAN_LED);
        delay_us(250000);
        gpio_clear(GPIO_PORT_MS_LED);
        gpio_set(GPIO_PORT_WLAN_LED);
        delay_us(250000);
    }
}
