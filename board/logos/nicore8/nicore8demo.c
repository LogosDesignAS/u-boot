/*
 * This file holds the DEMO Mode functionality for Nicore8
 * Copyright (C) 2021 Logos Payment Solutions
 */

#include "nicore8demo.h"
#include "bootmelody.h"
#include <linux/delay.h>
#include <asm/gpio.h>

// Redefine enums for 4 GPIOs
enum LED_GPIO {
	GPIO_LED_2_1 		= IMX_GPIO_NR(6, 7),
	GPIO_LED_3_1 		= IMX_GPIO_NR(6, 9)
};

// Enum for GPIOs[2-3] on the LogosNi8 board
enum GPIO {
	GPIO_2_1 			= IMX_GPIO_NR(1, 3),
	GPIO_3_1 			= IMX_GPIO_NR(1, 4)
};

void led_logosni8_party_light(void)
{
	// This function will create a simple light demo - using the LED2 and LED3 - will run for 20 seconds
	for (int i = 0; i < 30; i++) {
		gpio_set_value(GPIO_LED_2_1, 1);
		gpio_set_value(GPIO_LED_3_1, 1);

		// Wait 0.5s
		mdelay(500);

		gpio_set_value(GPIO_LED_2_1, 0);
		gpio_set_value(GPIO_LED_3_1, 0);

		// Wait 0.5s
		mdelay(500);
	}

	// After the initial heartbeat start the more serious stuff will initiate - Namely "Something - rename
	// Insert some more LED config here, to make a nice demo.
	gpio_set_value(GPIO_LED_2_1, 0);
	gpio_set_value(GPIO_LED_3_1, 1);
}

/*
 * This function generate beeps using the Buzzer on the Test Carrier board and takes in two parameters
 * note     - This is the frequency of the requested note - Hz
 * duration - For how long should this tone be played     - ms
 *
 * The function generates a square wave that activates the buzzer
 */
int beep(int note, int duration)
{
	// Determine the Time Period
	int T_sound = 1000000 / note;
	// Determine the number of time periods to run to get the wanted duration.
	int runTime = (duration * 1000)/T_sound;

	for (int u = 0; u < runTime; u++) {
		gpio_direction_output(GPIO_2_1, 1);					// GPIO_2 -> SOUND2
		gpio_direction_output(GPIO_3_1, 0);					// GPIO_3 -> SOUND1
		udelay(T_sound >> 1);								// Divide by two

		gpio_direction_output(GPIO_2_1, 0);					// GPIO_2 -> SOUND2
		gpio_direction_output(GPIO_3_1, 1);					// GPIO_3 -> SOUND1
		udelay(T_sound >> 1);								// Divide by two
	}
	return 0;
}

int firstSection(void)
{
	beep(a,  500);
	beep(a,  500);
	beep(a,  500);
	beep(f,  350);
	beep(cH, 150);
	beep(a,  500);
	beep(f,  350);
	beep(cH, 150);
	beep(a,  650);

	mdelay(500);

	beep(eH, 500);
	beep(eH, 500);
	beep(eH, 500);
	beep(fH, 350);
	beep(cH, 150);
	beep(gS, 500);
	beep(f,  350);
	beep(cH, 150);
	beep(a,  650);

	mdelay(500);

	return 0;
}

int secondSection(void)
{
	beep(aH, 500);
	beep(a,  300);
	beep(a,  150);
	beep(aH, 500);
	beep(gSH,325);
	beep(gH, 175);
	beep(fSH,125);
	beep(fH, 125);
	beep(fSH,250);

	mdelay(325);

	beep(aS, 250);
	beep(dSH,500);
	beep(dH, 325);
	beep(cSH,175);
	beep(cH, 125);
	beep(b,  125);
	beep(cH, 250);

	mdelay(350);

	return 0;
}

int bootup_Song_Star_Wars(void)
{
	//Play first section
	firstSection();

	//Play second section
	secondSection();

	//Variant 1
	beep(f,  250);
	beep(gS, 500);
	beep(f,  350);
	beep(a,  125);
	beep(cH, 500);
	beep(a,  375);
	beep(cH, 125);
	beep(eH, 650);

	mdelay(500);

	//Repeat second section
	secondSection();

	//Variant 2
	beep(f,  250);
	beep(gS, 500);
	beep(f,  375);
	beep(cH, 125);
	beep(a,  500);
	beep(f,  375);
	beep(cH, 125);
	beep(a,  650);

	return 0;
}
