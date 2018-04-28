#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <string.h> // for fprintf

#include <wiringPi.h>
#include <sr595.h>
#include "../pivumeter.h"

#define ADDR_SR595 100
#define BITS 8

#define DATAPIN  25 // data blue (pin 14)
#define LATCHPIN 26 // latch green (pin 12)
#define CLOCKPIN 27 // clock yellow (pin 11)

const int ledPins[] = { 23, 24 }; // for bar graph leds 9 and 10

static void Off()
{
    int thisLed;
    for (thisLed = 0; thisLed < 10; thisLed++)
    {
        if (thisLed < 8) digitalWrite(ADDR_SR595 + thisLed, 0);
#if 0
        if (thisLed < 8)
        {
            switch (thisLed)
            {
                case 0: system("gpio -x sr595:200:8:25:27:26 write 200 0"); break;
                case 1: system("gpio -x sr595:200:8:25:27:26 write 201 0"); break;
                case 2: system("gpio -x sr595:200:8:25:27:26 write 202 0"); break;
                case 3: system("gpio -x sr595:200:8:25:27:26 write 203 0"); break;
                case 4: system("gpio -x sr595:200:8:25:27:26 write 204 0"); break;
                case 5: system("gpio -x sr595:200:8:25:27:26 write 205 0"); break;
                case 6: system("gpio -x sr595:200:8:25:27:26 write 206 0"); break;
                case 7: system("gpio -x sr595:200:8:25:27:26 write 207 0"); break;
            }
        }
#endif
        else digitalWrite(ledPins[thisLed - 8], 0);
    }
}

static void On()
{
    int thisLed;
    for (thisLed = 0; thisLed < 10; thisLed++)
    {
        if (thisLed < 8) digitalWrite(ADDR_SR595 + thisLed, 1);
#if 0
        if (thisLed < 8)
        {
            switch (thisLed)
            {
                case 0: system("gpio -x sr595:200:8:25:27:26 write 200 1"); break;
                case 1: system("gpio -x sr595:200:8:25:27:26 write 201 1"); break;
                case 2: system("gpio -x sr595:200:8:25:27:26 write 202 1"); break;
                case 3: system("gpio -x sr595:200:8:25:27:26 write 203 1"); break;
                case 4: system("gpio -x sr595:200:8:25:27:26 write 204 1"); break;
                case 5: system("gpio -x sr595:200:8:25:27:26 write 205 1"); break;
                case 6: system("gpio -x sr595:200:8:25:27:26 write 206 1"); break;
                case 7: system("gpio -x sr595:200:8:25:27:26 write 207 1"); break;
            }
        }
#endif
        else digitalWrite(ledPins[thisLed - 8], HIGH);
    }
}

static double map(float x, float x0, float x1, float y0, float y1)
{
    float y = y0 + ((y1 - y0) * ((x - x0) / (x1 - x0)));
    double z = (double)y;
    return z;
}


static void doGraph(int num)
{
    int thisLed;
    _Bool toggle;

    if (num < 0 || num > 10) return;
    if (num == 0) Off();
    else if (num == 10) On();
    else
    {
        for (thisLed = 0; thisLed < 10; thisLed++)
        {
            toggle = (thisLed < num);
            if (thisLed < 8) digitalWrite(ADDR_SR595 + thisLed, toggle);
#if 0
            if (thisLed < 8)
            {
                switch (thisLed)
                {
                    case 0:
                        if (toggle) system("gpio -x sr595:200:8:25:27:26 write 200 1");
                        else system("gpio -x sr595:200:8:25:27:26 write 200 0");
                        break;
                    case 1:
                        if (toggle) system("gpio -x sr595:200:8:25:27:26 write 201 1");
                        else system("gpio -x sr595:200:8:25:27:26 write 201 0");
                        break;
                    case 2:
                        if (toggle) system("gpio -x sr595:200:8:25:27:26 write 202 1");
                        else system("gpio -x sr595:200:8:25:27:26 write 202 0");
                        break;
                    case 3:
                        if (toggle) system("gpio -x sr595:200:8:25:27:26 write 203 1");
                        else system("gpio -x sr595:200:8:25:27:26 write 203 0");
                        break;
                    case 4:
                        if (toggle) system("gpio -x sr595:200:8:25:27:26 write 204 1");
                        else system("gpio -x sr595:200:8:25:27:26 write 204 0");
                        break;
                    case 5:
                        if (toggle) system("gpio -x sr595:200:8:25:27:26 write 205 1");
                        else system("gpio -x sr595:200:8:25:27:26 write 205 0");
                        break;
                    case 6:
                        if (toggle) system("gpio -x sr595:200:8:25:27:26 write 206 1");
                        else system("gpio -x sr595:200:8:25:27:26 write 206 0");
                        break;
                    case 7:
                        if (toggle) system("gpio -x sr595:200:8:25:27:26 write 207 1");
                        else system("gpio -x sr595:200:8:25:27:26 write 207 0");
                        break;
                }
            }
#endif
            else digitalWrite(ledPins[thisLed - 8], toggle);
        }
    }
}

static int init()
{
    wiringPiSetup();
    sr595Setup(ADDR_SR595, BITS, DATAPIN, CLOCKPIN, LATCHPIN);
    pinMode(ledPins[0], OUTPUT);
    pinMode(ledPins[1], OUTPUT);
    Off();
    atexit(Off);
    return 0;
}

static void update(int meter_level_l, int meter_level_r, snd_pcm_scope_ameter_t *level)
{
    int led;
    int meter_level;
    int brightness;
    int bar;

    meter_level = meter_level_l;
    if (meter_level_r > meter_level) meter_level = meter_level_r;
    brightness = level->led_brightness;
    bar = (meter_level / 15000.0f) * (brightness * 10.0f);
    //led = map(bar, 0, 3000, 0, 10);
    led = map(bar, 0, 2796, 0, 10);
    fprintf(stderr, "bar: %d\n", bar);
    doGraph(led);
}

device bargraph()
{
    struct device _bargraph;
    _bargraph.init = &init;
    _bargraph.update = &update;
    return _bargraph;
}
