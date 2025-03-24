#include "syscon.h"
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

unsigned int syscon_get_tachyon_version()
{
    unsigned int ver = *(unsigned int*)(0xbc100040);
    
    if (ver & 0xFF000000)
        return (ver >> 8);

    return 0x100000;
}

int syscon_ctrl_power(unsigned int dev, unsigned int on)
{
    uint32_t device = ((on & 1) << 23) | (dev & 0x003FFFFF);
    return syscon_issue_command_write(SYSCON_CTRL_POWER, (unsigned char *)&device, 3);
}

int syscon_ctrl_led(SysconLed led, unsigned int on)
{
    unsigned char led_cmd = 0;

    // get the bit that represents the LED we're interested in
    switch (led) {
        case SYSCON_LED_MS:
            led_cmd = 0x40;
            break;

        case SYSCON_LED_WLAN:
            led_cmd = 0x80;
            break;

        case SYSCON_LED_POWER:
            led_cmd = 0x20;
            break;

        case SYSCON_LED_BT:
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
    return syscon_issue_command_write(SYSCON_CTRL_HR_POWER, &on_val, 1);
}

int syscon_ctrl_ms_power(unsigned int on)
{
    unsigned char on_val = on;

    // write message directly to syscon
    return syscon_issue_command_write(SYSCON_CTRL_MS_POWER, &on_val, 1);
}

int syscon_get_pommel_version(unsigned int *version)
{
    return syscon_issue_command_read(SYSCON_GET_POMMEL_VERSION, (unsigned char *)version);
}

int syscon_get_power_status(unsigned int *status)
{
    return syscon_issue_command_read(SYSCON_GET_POWER_STATUS, (unsigned char *)status);
}

int syscon_ctrl_voltage(unsigned int a0, unsigned int a1)
{
    uint32_t val = ((a0 & 0xFFFF) << 8) | (a1 & 0xFF);
    return syscon_issue_command_write(SYSCON_CTRL_VOLTAGE, (unsigned char *)&val, 3);
}

int syscon_get_digital_key(unsigned int *keys)
{
    return syscon_issue_command_read(SYSCON_GET_DIGITAL_KEY_KERNEL, (unsigned char *)keys);
}

int syscon_get_wakeup_factor(unsigned int *factor)
{
    return syscon_issue_command_read(SYSCON_GET_WAKEUP_FACTOR, (unsigned char *)factor);
}

int syscon_read_scratchpad(unsigned int src, unsigned int *dest)
{
    unsigned char data = (unsigned char)(src << 2) | (unsigned char)(sizeof(unsigned int) >> 1);

    return syscon_issue_command_read_write(SYSCON_READ_SCRATCHPAD, &data, sizeof(data), dest);
}

int syscon_write_scratchpad(unsigned int dest, unsigned int *src)
{
    unsigned char data[5];
    data[0] = (unsigned char)(dest << 2) | (unsigned char)(sizeof(unsigned int) >> 1);
    memcpy(&data[1], src, sizeof(unsigned int));

    return syscon_issue_command_write(SYSCON_WRITE_SCRATCHPAD, data, sizeof(data));
}

int syscon_reset_device(unsigned int a0, unsigned int a1)
{
    // valid options:
    // * 1, 1 => param is 1
    // * 1, 2 => param is 0x41
    // * x, 0 => param is x
    // * x, y => param is 0x80 | x
    uint8_t val;

    if (a0 == 1) {
        if (a1 == 1) {
            val = 1;
        } else if (a1 == 2) {
            val = 0x41;
        } else {
            return -1;
        }
    } else if (a1 == 0) {
        val = a0;
    } else {
        val = 0x80 | a0;
    }

    return syscon_issue_command_write(SYSCON_RESET_DEVICE, (unsigned char *)&val, 1);
}

int syscon_send_auth(unsigned char key, unsigned char *data)
{
    unsigned char tx_buf[0x10];

    tx_buf[0] = key;
    memcpy(&tx_buf[1], data, 8);
    int result = syscon_issue_command_write(0x30, tx_buf, 9);
    if (result < 0)
        return result;

    tx_buf[0] = key + 1;
    memcpy(&tx_buf[1], &data[8], 8);
    result = syscon_issue_command_write(0x30, tx_buf, 9);
    if (result < 0)
        return result;
        
    return 0;
    
}

int syscon_recv_auth(unsigned char key, unsigned char *data)
{
    unsigned char tx_buf[0x10],rx_buf[0x10];

    tx_buf[0] = key;
    int result = syscon_issue_command_read_write(0x30, tx_buf, 1, rx_buf);
    if (result < 0)
        return result;
    
    memcpy(data, rx_buf+1, 8);

    tx_buf[0] = key + 1;
    result = syscon_issue_command_read_write(0x30, tx_buf, 1, rx_buf);
    if (result < 0)
        return result;
    
    memcpy(&data[8], rx_buf+1, 8);
        
    return 0;
}