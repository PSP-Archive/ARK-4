#include "led.h"

#include <syscon.h>
#include <sysreg.h>
#include <gpio.h>
#include <model.h>
#include <interrupt.h>
#include <cpu.h>

typedef enum {
    ACTIVE_LOW,
    ACTIVE_HIGH,
} LogicLevel;

typedef struct {
    enum Led id;
    enum Ports gpio_port;
    LogicLevel logic_level;
    int initialised;
    enum LedMode mode;
    unsigned int cur_tick;
    unsigned int on_time;
    unsigned int off_time;
    unsigned int blink_time;
    int req_action;
} LedState;

static LedState g_ms_led = {
    .id = LED_MEMORY_STICK,
    .gpio_port = GPIO_PORT_MS_LED,
    .initialised = 0,
};

static LedState g_wlan_led = {
    .id = LED_WLAN,
    .gpio_port = GPIO_PORT_WLAN_LED,
    .initialised = 0,
};

static LedState g_bt_led = {
    .id = LED_BLUETOOTH,
    .gpio_port = GPIO_PORT_BT_LED,
    .initialised = 0,
};


static LedState *g_led_states[] = {
    &g_ms_led,
    &g_wlan_led,
    &g_bt_led
};

#define NUM_LED_STATES  (sizeof(g_led_states)/sizeof(*g_led_states))

static LedState *state_for_led(enum Led led)
{
    for (size_t i = 0; i < NUM_LED_STATES; ++i) {
        if (g_led_states[i]->id == led && g_led_states[i]->initialised) {
            return g_led_states[i];
        }
    }

    return NULL;
}

static void turn_on_led(enum Ports port, LogicLevel level)
{
    switch (level) {
        case ACTIVE_HIGH:
            gpio_set(port);
            break;

        case ACTIVE_LOW:
            gpio_clear(port);
            break;
    }
}

static void turn_off_led(enum Ports port, LogicLevel level)
{
    switch (level) {
        case ACTIVE_HIGH:
            gpio_clear(port);
            break;

        case ACTIVE_LOW:
            gpio_set(port);
            break;
    }
}

static void blink_led(LedState *led)
{
    if (led->blink_time == 0) {
        turn_off_led(led->gpio_port, led->logic_level);
        led->mode = LED_MODE_OFF;
        led->req_action = 0;
        return;
    } else {
        led->blink_time -= 1;
    }

    // wrap our ticker around the period of the blink
    led->cur_tick = (led->cur_tick + 1) % (led->on_time + led->off_time);

    if (led->cur_tick <= led->on_time) {
        turn_on_led(led->gpio_port, led->logic_level);
    } else {
        turn_off_led(led->gpio_port, led->logic_level);
    }
}

static void init_ms_led(LogicLevel logic_level)
{
    g_ms_led.logic_level = logic_level;
    g_ms_led.initialised = 1;
    syscon_ctrl_led(SYSCON_LED_MS, 1);
    sysreg_io_enable_gpio_port(GPIO_PORT_MS_LED);
    gpio_set_port_mode(GPIO_PORT_MS_LED, GPIO_MODE_OUTPUT);
    turn_off_led(g_ms_led.gpio_port, g_ms_led.logic_level);
    g_ms_led.mode = LED_MODE_OFF;
    g_ms_led.req_action = 1;
    g_ms_led.cur_tick = 0;
}

static void init_wlan_led(LogicLevel logic_level)
{
    g_wlan_led.logic_level = logic_level;
    g_wlan_led.initialised = 1;
    syscon_ctrl_led(SYSCON_LED_WLAN, 1);
    sysreg_io_enable_gpio_port(GPIO_PORT_WLAN_LED);
    gpio_set_port_mode(GPIO_PORT_WLAN_LED, GPIO_MODE_OUTPUT);
    turn_off_led(g_wlan_led.gpio_port, g_wlan_led.logic_level);
    g_wlan_led.mode = LED_MODE_OFF;
    g_wlan_led.req_action = 1;
    g_wlan_led.cur_tick = 0;
}

static void init_power_led(void)
{
    g_wlan_led.initialised = 1;
    syscon_ctrl_led(SYSCON_LED_POWER, 1);
}

static void init_bt_led(LogicLevel logic_level)
{
    g_bt_led.logic_level = logic_level;
    g_bt_led.initialised = 1;
    syscon_ctrl_led(SYSCON_LED_BT, 1);
    sysreg_io_enable_gpio_port(GPIO_PORT_BT_LED);
    gpio_set_port_mode(GPIO_PORT_BT_LED, GPIO_MODE_OUTPUT);
    turn_off_led(g_bt_led.gpio_port, g_bt_led.logic_level);
    g_bt_led.mode = LED_MODE_OFF;
    g_bt_led.req_action = 1;
    g_bt_led.cur_tick = 0;
}

static void on_vsync_led(LedState *led)
{
    switch (led->mode) {
        case LED_MODE_OFF:
            turn_off_led(led->gpio_port, led->logic_level);
            led->req_action = 0;
            break;

        case LED_MODE_ON:
            turn_on_led(led->gpio_port, led->logic_level);
            led->req_action = 0;
            break;

        case LED_MODE_BLINK:
            blink_led(led);
            break;
    }
}

static enum IrqHandleStatus on_vsync(void)
{
    for (size_t i = 0; i < NUM_LED_STATES; ++i) {
        if (!g_led_states[i]->initialised || !g_led_states[i]->req_action) {
            continue;
        }

        on_vsync_led(g_led_states[i]);
    }

    return IRQ_HANDLE_NO_RESCHEDULE;
}

void led_set_mode(enum Led led, enum LedMode mode, const LedConfig *config)
{
    LedState *state = state_for_led(led);

    if (!state) {
        return;
    }

    unsigned int intr = cpu_suspend_interrupts();

    state->mode = mode;
    state->req_action = 1;
    state->cur_tick = 0;

    if (state->mode == LED_MODE_BLINK) {
        state->on_time = config->on_time;
        state->off_time = config->off_time;
        state->blink_time = config->blink_time;
    }

    cpu_resume_interrupts(intr);
}

void led_init(void)
{
    const PspModelIdentity *identity = model_get_identity();

    switch (identity->model) {
        case PSP_MODEL_01G:
        case PSP_MODEL_04G:
        case PSP_MODEL_07G:
        case PSP_MODEL_09G:
            init_ms_led(ACTIVE_HIGH);
            init_wlan_led(ACTIVE_HIGH);
            init_power_led();
            break;
        
        case PSP_MODEL_02G:
        case PSP_MODEL_03G:
            init_ms_led(ACTIVE_HIGH);
            init_wlan_led(ACTIVE_LOW);
            init_power_led();
            break;
        
        case PSP_MODEL_05G:
            init_ms_led(ACTIVE_HIGH);
            init_wlan_led(ACTIVE_LOW);
            init_power_led();
            init_bt_led(ACTIVE_HIGH);
            break;
        
        case PSP_MODEL_11G:
            init_ms_led(ACTIVE_HIGH);
            init_power_led();
            break;
    }

    interrupt_set_handler(IRQ_VSYNC, on_vsync);
}
