#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <string.h> // for fprintf

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "../pivumeter.h"

#define CE 1

//#define DO_LOOP

unsigned char data[2] = { 0x0, 0x0 };
unsigned char backup_data[2] = { 0x0, 0x0 };

void bitWrite(int ab, int n, int b) { data[ab] ^= (-b ^ data[ab]) & (1 << n); }

void bitClear(int ab, int n) { data[ab] ^= (0 ^ data[ab]) & (1 << n); }

void bitSet(int ab, int n) { data[ab] ^= (-1 ^ data[ab]) & (1 << n); }

static void Off()
{
    data[0] = data[1] = 0x0;
    backup_data[0] = data[0];
    backup_data[1] = data[1];
    wiringPiSPIDataRW(CE, data, 2);
    data[0] = backup_data[0];
    data[1] = backup_data[1];
}

static void Off_r()
{
    data[1] = 0b00000000;
    backup_data[0] = data[0];
    backup_data[1] = data[1];
    wiringPiSPIDataRW(CE, data, 2);
    data[0] = backup_data[0];
    data[1] = backup_data[1];
}

static void Off_l()
{
    data[0] = 0b00000000;
    backup_data[0] = data[0];
    backup_data[1] = data[1];
    wiringPiSPIDataRW(CE, data, 2);
    data[0] = backup_data[0];
    data[1] = backup_data[1];
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
//    else if (num_l == 0) Off_l();
//    else if (num_r == 0) Off_r();
//    else if (num_l == 8) On_l();
//    else if (num_r == 8) On_r();
    else
    {
        // left side
        for (i = 0; i < 8; i++)
        {
            thisLed_r = i;
            thisLed_l = abs(7 - i);
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
                bitWrite(0, thisLed_l, toggle_l);
                backup_data[0] = data[0];
                backup_data[1] = data[1];
                wiringPiSPIDataRW(CE, data, 2);
                data[0] = backup_data[0];
                data[1] = backup_data[1];
            }
            if (con_r)
            {
                toggle_r = (thisLed_l < num_r);
                bitWrite(1, thisLed_r, toggle_r);
                backup_data[0] = data[0];
                backup_data[1] = data[1];
                wiringPiSPIDataRW(CE, data, 2);
                data[0] = backup_data[0];
                data[1] = backup_data[1];
            }
            con_l = con_r = 1;
        }
    }
}

#ifdef DO_LOOP
static void Loop()
{
    int i;
    for (i = 0; i < 8; i++)
    {
        doGraph(i, i);
        delay(100);
    }
}
#endif

static int init()
{
    wiringPiSetup();
    wiringPiSPISetup(CE, 500000);
#ifdef DO_LOOP
    Loop();
#endif
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

device bargraph_stereo()
{
    struct device _bargraph_stereo;
    _bargraph_stereo.init = &init;
    _bargraph_stereo.update = &update;
    return _bargraph_stereo;
}
