#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <string.h> // for fprintf

#include <wiringPi.h>
#include "../pivumeter.h"


const int gpioLedPins[10] = { 1, 4, 5, 6, 21, 22, 23, 24, 25, 27 };

static void AllOn() { int i; for (i = 0; i < 10; i++) digitalWrite(gpioLedPins[i], HIGH); }

static void AllOff() { int i; for (i = 0; i < 10; i++) digitalWrite(gpioLedPins[i], LOW); }

static double map(float x, float x0, float x1, float y0, float y1)
{
    float y = y0 + ((y1 - y0) * ((x - x0) / (x1 - x0)));
    double z = (double)y;
    return z;
}

static void doGraph(int num)
{
    int i;
    if (num < 0 || num > 10) return;
    else if (num == 0) AllOff();
    else if (num == 10) AllOn();
    else for (i = 0; i < 10; i++) digitalWrite(gpioLedPins[i], (i < num));
}

static int init()
{
    int i;
    wiringPiSetup();
    for (i = 0; i < 10; i++) pinMode(gpioLedPins[i], OUTPUT);
    AllOff();
    atexit(AllOff);
    return 0;
}

static void update(int meter_level_l, int meter_level_r, snd_pcm_scope_ameter_t *level)
{
    int led, meter_level, brightness, bar;
    meter_level = meter_level_l;
    if (meter_level_r > meter_level) meter_level = meter_level_r;
    brightness = level->led_brightness;
    bar = (meter_level / 15000.0f) * (brightness * 10.0f);
    led = map(bar, 0, 2796, 0, 10);
    doGraph(led);
}

device gpio_10_bargraph()
{
    struct device _gpio_10_bargraph;
    _gpio_10_bargraph.init = &init;
    _gpio_10_bargraph.update = &update;
    return _gpio_10_bargraph;
}

