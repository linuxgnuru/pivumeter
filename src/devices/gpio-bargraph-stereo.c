#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <string.h> // for fprintf

#include <wiringPi.h>
#include "../pivumeter.h"


const int ledPins[2][8] = {
    {  0,  1,  2,  3,  4,  5,  6,  7 },
    { 21, 22, 23, 24, 25, 26, 27, 28 }
};

static void On()
{
    int i, j;

    for (i = 0; i < 2; i++)
        for (j = 0; j < 8; j++)
            digitalWrite(ledPins[i][j], HIGH);
}

static void Off()
{
    int i, j;

    for (i = 0; i < 2; i++)
        for (j = 0; j < 8; j++)
            digitalWrite(ledPins[i][j], LOW);
}

static void On_r()
{
    int i;

    for (i = 0; i < 8; i++)
        digitalWrite(ledPins[1][i], HIGH);
}

static void Off_r()
{
    int i;

    for (i = 0; i < 8; i++)
        digitalWrite(ledPins[1][i], LOW);
}

static void On_l()
{
    int i;

    for (i = 0; i < 8; i++)
        digitalWrite(ledPins[0][i], HIGH);
}

static void Off_l()
{
    int i;

    for (i = 0; i < 8; i++)
        digitalWrite(ledPins[0][i], LOW);
}

static double map(float x, float x0, float x1, float y0, float y1)
{
    float y = y0 + ((y1 - y0) * ((x - x0) / (x1 - x0)));
    double z = (double)y;
    return z;
}

static void doGraph(int num_r, int num_l)
{
    _Bool toggle_l, toggle_r;
    int i, thisLed_l, thisLed_r;
    _Bool con_l, con_r;

    con_l = con_r = 1;
    if (num_r < 0 || num_r > 8) return;
    else if (num_l < 0 || num_l > 8) return;
    else
    {
        for (i = 0; i < 8; i++)
        {
            thisLed_r = abs(7 - i);
            thisLed_l = i;
            if (num_r == 0)
            {
                Off_r(); 
                con_r = 0;
            }
            if (num_l == 0)
            {
                Off_l();
                con_l = 0;
            }
            if (con_l)
            {
                toggle_l = (thisLed_l < num_l);
                //toggle_l = (i < num_l);
                digitalWrite(ledPins[0][i], toggle_l);
            }
            if (con_r)
            {
                toggle_r = (thisLed_l < num_r);
                //toggle_r = (i < num_r);
                digitalWrite(ledPins[1][i], toggle_r);
            }
            con_l = con_r = 1;
        }
    }
}

static int init()
{
    int i, j;

    wiringPiSetup();
    for (i = 0; i < 2; i++)
        for (j = 0; j < 8; j++)
            pinMode(ledPins[i][j], OUTPUT);
    Off();
    atexit(Off);
    return 0;
}

static void update(int meter_level_l, int meter_level_r, snd_pcm_scope_ameter_t *level)
{
    int led_l, led_r;
    int brightness;
    int bar_r, bar_l;

    brightness = level->led_brightness;
    bar_r = (meter_level_r / 15000.0f) * (brightness * 10.0f);
    bar_l = (meter_level_l / 15000.0f) * (brightness * 10.0f);
    led_r = map(bar_r, 0, 2796, 0, 8);
    led_l = map(bar_l, 0, 2796, 0, 8);
    doGraph(led_r, led_l);
}

device gpio_bargraph_stereo()
{
    struct device _gpio_bargraph_stereo;
    _gpio_bargraph_stereo.init = &init;
    _gpio_bargraph_stereo.update = &update;
    return _gpio_bargraph_stereo;
}
