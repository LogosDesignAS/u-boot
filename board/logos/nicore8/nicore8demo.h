/*
 * This file holds the DEMO Mode functions for Nicore8
 * Copyright (C) 2021 Logos Payment Solutions
 */

#ifndef U_BOOT_NICORE8DEMO_H
#define U_BOOT_NICORE8DEMO_H

// Function for Blinking the LED
void led_logosni8_party_light(void);

// Function for making beep sounds with the Buzzer
int beep(int note, int duration);

// First Section of Star Wars
int firstSection(void);

// Second Section of Star Wars
int secondSection(void);

// Star Wars Song
int bootup_Song_Star_Wars(void);

#endif //U_BOOT_NICORE8DEMO_H
