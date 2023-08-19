#include "comms.h"
#include "gpio.h"
#include "sysreg.h"
#include "spi.h"

unsigned int g_baryon_version = 0;

int syscon_init(void)
{
    // enable GPIO used for syscon sync
    sysreg_io_enable_gpio_port(GPIO_PORT_SYSCON_REQUEST);
    sysreg_io_enable_gpio_port(GPIO_PORT_TACHYON_SPI_CS);
    gpio_clear(GPIO_PORT_SYSCON_REQUEST);
    gpio_set_port_mode(GPIO_PORT_SYSCON_REQUEST, GPIO_MODE_OUTPUT);
    gpio_set_port_mode(GPIO_PORT_TACHYON_SPI_CS, GPIO_MODE_INPUT);
	sysreg_io_enable_gpio();

    // syscon communicates over an SPI channel to the allegrex processor.
    // syscon holds the "master" device status, and the allegrex uses a
    // GPIO to signal to syscon when to initiate a message transfer to it.
    sysreg_clock_select_spi(0, 1);
    sysreg_clock_enable_spi(0);
    sysreg_io_enable_spi(0);

    // initialise the SPI hardware for communication with syscon
    spi_init(SPI_SYSCON);

    // initialise GPIO to known state
    gpio_clear(GPIO_PORT_SYSCON_REQUEST);
    gpio_unmask_interrupt(GPIO_PORT_TACHYON_SPI_CS);
    gpio_clear_unk14(GPIO_PORT_TACHYON_SPI_CS);
    gpio_set_unk18(GPIO_PORT_TACHYON_SPI_CS);
    gpio_acquire_interrupt(GPIO_PORT_TACHYON_SPI_CS);

    // get the baryon version and store it for quick access. its not like
    // this value will change... right?
    if (syscon_issue_command_read(SYSCON_GET_BARYON_VERSION, (unsigned char *)&g_baryon_version) < 0)
    {
        return -1;
    }

    return 0;
}

unsigned int syscon_get_baryon_version(void)
{
    // this value doesn't change. it is cached on syscon_init so we
    // don't need to xchg across SPI everytime we want to know
    // the version
    return g_baryon_version;
}

int syscon_ctrl_led(unsigned int led, unsigned int on)
{
    unsigned char led_cmd = 0;

    // get the bit that represents the LED we're interested in
    switch (led) {
        // memory stick
        case 0:
            led_cmd = 0x40;
            break;
        // wlan
        case 1:
            led_cmd = 0x80;
            break;
        // power
        case 2:
            led_cmd = 0x20;
            break;
        // bluetooth
        case 3:
            // TODO: check pommel type. error if < 0x300
            led_cmd = 0x10;
            break;
        default:
            return -1;
    }

    if (on) 
    {
        // the "on" bit changed between PSP-1000, and beyond select the
        // correct bit as appropriate
        unsigned int baryon = syscon_get_baryon_version();

        const unsigned char psp_model = (baryon >> 16) & 0xF0;
        const unsigned char is_psp1000 = psp_model == 0x00 || psp_model == 0x10;
        led_cmd |= (is_psp1000) ? (0x10) : (0x1);
    }

    // write message to syscon to modify LED state
    return syscon_issue_command_write(SYSCON_CTRL_LED, &led_cmd, 1);
}

int syscon_ctrl_hr_power(unsigned int on)
{
    unsigned char on_val = on;

    // write message directly to syscon
    syscon_issue_command_write(SYSCON_CTRL_HR_POWER, &on_val, 1);
}

int sceSysconCtrlMsPower(unsigned char sw){ return syscon_issue_command_write(0x4c, &sw, 1); }			// 4C : 3 : 3

int pspSysconGetCtrl1(unsigned int *ctrl){ return syscon_issue_command_read(0x07, ctrl); }				// 07 : 2 : 7