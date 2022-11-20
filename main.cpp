#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "hardware/gpio.h"
#include <stdint.h>
#include <tusb.h>
#include "pad_state.h"

constexpr int PIN_D0 = 3;
constexpr int PIN_D1 = 2;
constexpr int PIN_D2 = 8;
constexpr int PIN_D3 = 7;
constexpr int PIN_TH = 4; // (NOT OD)
constexpr int PIN_TR = 5;
constexpr int PIN_TL = 6;

void outGPIO(int pin, bool v)
{
    gpio_set_dir(pin, v ? GPIO_IN : GPIO_OUT);
}

void setData(int v)
{
    outGPIO(PIN_D0, v & 1);
    outGPIO(PIN_D1, v & 2);
    outGPIO(PIN_D2, v & 4);
    outGPIO(PIN_D3, v & 8);
}

bool getTH()
{
    return gpio_get(PIN_TH);
}

bool getTR()
{
    return gpio_get(PIN_TR);
}

void waitTR(bool pol)
{
    while (getTR() == pol)
    {
        tight_loop_contents();
    }
}

void setTL(bool v)
{
    outGPIO(PIN_TL, v);
}

void sendByte(int v)
{
    waitTR(true);
    setData(v >> 4);
    setTL(false);

    waitTR(false);
    setData(v);
    setTL(true);
}

int main()
{
    stdio_init_all();

    tusb_init();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    auto initInputPin = [](int pin)
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
    };
    auto initOutputPin = [](int pin)
    {
        gpio_init(pin);
        gpio_put(pin, 0);
        // gpio_set_dir(pin, GPIO_OUT);
        gpio_set_dir(pin, GPIO_IN);
        // 方向だけで制御する
    };

#if 0
    while (1)
        tuh_task();
#endif

    initOutputPin(PIN_D0);
    initOutputPin(PIN_D1);
    initOutputPin(PIN_D2);
    initOutputPin(PIN_D3);
    initInputPin(PIN_TH);
    initInputPin(PIN_TR);
    initOutputPin(PIN_TL);

    constexpr int id0 = 1;
    constexpr int id2_digital = 0;
    constexpr int id2_analog = 1;
    constexpr int len_digital = 2;
    constexpr int len_analog = 6;

    watchdog_enable(5000, true);

    setData(id0);
    setTL(1);

    while (1)
    {
        watchdog_update();
        bool analog = true;

        static int ct = 0;
        ++ct;

        if (ct > 120)
            ct = 0;
        gpio_put(PICO_DEFAULT_LED_PIN, ct < 60);

        while (getTH())
        {
            tight_loop_contents();
        }

        const auto *pad = getPadState();
        assert(pad);

        waitTR(false);
        if (analog)
        {
            sendByte((id2_analog << 4) | len_analog);

            sendByte(pad->getButtons());
            sendByte(pad->getButtons() >> 8);
            sendByte(pad->getAnalog(PadState::A_X));
            sendByte(pad->getAnalog(PadState::A_Y));
            sendByte(pad->getAnalog(PadState::A_RT));
            sendByte(pad->getAnalog(PadState::A_LT));
            sendByte(id0);
        }

        while (!getTH())
        {
            tight_loop_contents();
        }
        tuh_task();
    }
    return 0;
}
