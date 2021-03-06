#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <string.h> // for fprintf

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "../pivumeter.h"

#define CE 1

const int extraLedPins[] = { 23, 24 }; // for bar graph leds 9 and 10

unsigned char data_1[1] = { 0x0 };
unsigned char backup_data_1[1] = { 0x0 };

void bitWrite_1(int n, int b) { if (n <= 7 && n >= 0) data_1[0] ^= (-b ^ data_1[0]) & (1 << n); }

void bitClear_1(int n) { if (n <= 7 && n >= 0) data_1[0] ^= (0 ^ data_1[0]) & (1 << n); }

void bitSet_1(int n) { if (n <= 7 && n >= 0) data_1[0] ^= (-1 ^ data_1[0]) & (1 << n); }

static void Off_1()
{
    data_1[0] = 0b00000000;
    backup_data_1[0] = data_1[0];
    wiringPiSPIDataRW(CE, data_1, 1);
    data_1[0] = backup_data_1[0];
    digitalWrite(23, LOW);
    digitalWrite(24, LOW);
}

static void On_1()
{
    data_1[0] = 0b11111111;
    backup_data_1[0] = data_1[0];
    wiringPiSPIDataRW(CE, data_1, 1);
    data_1[0] = backup_data_1[0];
    digitalWrite(23, HIGH);
    digitalWrite(24, HIGH);
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
    int lednum;
    _Bool toggle;

    if (num < 0 || num > 10) return;
    else if (num == 0) Off_1();
    else if (num == 10) On_1();
    else
    {
        for (thisLed = 0; thisLed < 10; thisLed++)
        {
            lednum = thisLed;
            toggle = (thisLed < num);
            if (thisLed < 8)
            {
                lednum = abs(lednum - 7);
                bitWrite_1(lednum, toggle);
                backup_data_1[0] = data_1[0];
                wiringPiSPIDataRW(CE, data_1, 1);
                data_1[0] = backup_data_1[0];
            }
            else
            {
                lednum -= 8;
                digitalWrite(extraLedPins[lednum], toggle);
            }
        }
    }
}

static int init()
{
    wiringPiSetup();
    wiringPiSPISetup(CE, 500000);
    pinMode(extraLedPins[0], OUTPUT);
    pinMode(extraLedPins[1], OUTPUT);
    digitalWrite(extraLedPins[0], LOW);
    digitalWrite(extraLedPins[1], LOW);
    Off_1();
    atexit(Off_1);
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
    //fprintf(stderr, "bar: %d\n", bar);
    doGraph(led);
}

device bargraph()
{
    struct device _bargraph;
    _bargraph.init = &init;
    _bargraph.update = &update;
    return _bargraph;
}
