#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <string.h> // for fprintf

#include <wiringPi.h>
#include <sr595.h>
//#include <wiringPiI2C.h>
#include "../pivumeter.h"

#define ADDR_SR595 100
#define BITS 8

#define DATAPIN  21 // data blue (pin 14)
#define LATCHPIN 22 // latch green (pin 12)
#define CLOCKPIN 23 // clock yellow (pin 11)

const int ledPins[] = { 25, 26 }; // for bar graph leds 9 and 10

//static int stupid_led_mappings[10] = {0, 1, 2, 4, 6, 8, 10, 12, 14, 16};

static void Off()
{
    int thisLed;
    for (thisLed = 0; thisLed < 10; thisLed++)
    {
        if (thisLed < 8) digitalWrite(ADDR_SR595 + thisLed, 0);
        else digitalWrite(ledPins[thisLed - 8], 0);
    }
}

static void onOff(_Bool b)
{
    int thisLed;
    for (thisLed = 0; thisLed < 10; thisLed++)
    {
        if (thisLed < 8) digitalWrite(ADDR_SR595 + thisLed, b);
        else digitalWrite(ledPins[thisLed - 8], b);
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
    else if (num == 10) onOff(1);
    else
    {
        for (thisLed = 0; thisLed < 10; thisLed++)
        {
            toggle = (thisLed < num);
            if (thisLed < 8) digitalWrite(ADDR_SR595 + thisLed, toggle);
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
    //led = map(bar, 0, 2796, 0, 10);
    led = map(bar, 0, 3000, 0, 10);
    //fprintf(stderr, "bar: %d\n", bar);
    doGraph(led);
    /*
    if (bar < 0)
        bar = 0;
    if (bar > (brightness*10)) bar = (brightness*10);
    for (led = 0; led < 10; led++)
    {
        int val = 0, index = led;
        if (bar > brightness)
        {
            val = brightness;
            bar -= brightness;
        }
        else if (bar > 0)
        {
            val = bar;
            bar = 0;
        }
        if (level->bar_reverse == 1) index = 9 - led;
        wiringPiI2CWriteReg8(i2c, 0x01 + stupid_led_mappings[index], val);
    }
    wiringPiI2CWriteReg8(i2c, 0x16, 0x01);
    */
}

device wiggins()
{
    struct device _wiggins;
    _wiggins.init = &init;
    _wiggins.update = &update;
    return _wiggins;
}
