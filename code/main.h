/*
 *  loststone is free sofware: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License 3 as published by
 *  the Free Software Foundation.
 *
 *  loststone is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with loststone. If not, see <http://www.gnu.org/licenses/gpl.txt>.
 *
 *  Copyright (c) 2012-2013 Chris Majoros(chris@majoros.us), GNU3
 */

#include "mbed.h"
#include "USBHID.h"
#include "USBMouse.h"
#include "Ser25lcxxx.h"


#include <stdint.h>

#define ADNS9500_SROM_91
#define ADNS9500_CRCHI (0xBE)
#define ADNS9500_CRCLO (0xEF)
#define ADNS9500_ID (0x56)
#define ADNS6010_FIRMWARE_CRC (0xBEEF)
#define ADNS9500_FIRMWARE_LEN 3070

#include "adns9500.hpp"


#define UINT16(ub, lb)             (uint16_t)(((ub & 0xff) << 8) | (lb & 0xff))
#define INT16(ub, lb)              (int16_t)(((ub & 0xff) << 8) | (lb & 0xff))

#define SETTINGS_BASE 0x00
#define PROFILE_BASE 0xff
#define PROFILE_LEN 0x13

#define degree2rad(deg)  (deg * (3.141592/180))
#define MBED

typedef void (*fn)(void);

enum cli_actions {
    SET       = 0x01,
    GET       = 0x02,
    LOAD_DATA = 0x03,
    GET_DATA  = 0x04,
    CLEAR     = 0x05,
    INIT      = 0x06
};

enum cli_replies {
    retval  = 0x01,
    message = 0x02
};

enum buttons {
    BUTTON_LEFT = 0x00, //explisitly showing we start at zero
    BUTTON_MIDDLE,
    BUTTON_RIGHT,
    BUTTON_FORWORD,
    BUTTON_BACK,
    BUTTON_Z,
    BUTTON_HIGH_RES,
};
enum settings {
    CPI_X = 0x00, //explisitly showing we start at zero
    CPI_Y,
    CPI_X_MULITIPLYER,
    CPI_Y_MULITIPLYER,
    COORD_X_SKEW,
    COORD_Y_SKEW,
    SCROLL_SKIP,
    CPI_MAX, // Not currently used.
    CPI_MIN, // Not currently used.
    CPI_STEP, // Not currently used.
    CPI_Z,
    CPI_H,
    CPI_HR_X,
    CPI_HR_Y,

    BTN_A,
    BTN_B,
    BTN_C,
    BTN_D,
    BTN_E,
    BTN_F,
    BTN_G,

    LED_ACTION, // Not currently used

    VID,
    PID,
    RELEASE,

    PROFILE_DEFAULT,
    PROFILE_CURRENT,

    ADNS_CRC,
    ADNS_ID,
    ADNS_FW_LEN,
    ADNS_FW_OFFSET,
};




#ifdef MBED
DigitalIn run_mode(p36);

DigitalIn motion_in(p14);
//InterruptIn motion_in(p14);


DigitalOut activity(p35);

InterruptIn debug(p29);

InterruptIn btn_a(p21);
InterruptIn btn_b(p22);
InterruptIn btn_c(p23);
InterruptIn btn_d(p24);
InterruptIn btn_e(p25);
InterruptIn btn_f(p26);
InterruptIn btn_g(p27);

InterruptIn prfl_a(p20);
InterruptIn prfl_b(p19);
InterruptIn prfl_c(p18);
InterruptIn prfl_d(p17);
InterruptIn prfl_e(p16);
#elif
DigitalIn run_mode(P1_29);

DigitalIn motion_in(P0_22);

DigitalOut activity(P1_28);

InterruptIn btn_a(P0_18);
InterruptIn btn_b(P0_16);
InterruptIn btn_c(P0_17);
InterruptIn btn_d(P0_23);
InterruptIn btn_e(P1_15);
//InterruptIn btn_f(P1_??);
//InterruptIn btn_g(P1_??);

InterruptIn prfl_a(P0_4);
InterruptIn prfl_b(P0_5);
InterruptIn prfl_c(P0_21);
InterruptIn prfl_d(P1_23);
InterruptIn prfl_e(P1_24);
#endif

// We are global for the callbacks
USBMouse *mouse;
adns9500::ADNS9500 *sensor;
bool motion_triggered = true;
bool z_axis_active = false;
bool high_rez_active = false;
bool profile_load = true; // Always inishally load the profile even if it might be the same.
bool set_res_hr = false;
bool set_res_z = false;
bool set_res_default = false;
//uint32_t rest_counter;

uint16_t s[31] = {
    5670,    // CPI_X
    5670,    // CPI_Y
    0,       // CPI_X_MULITIPLYER
    0,       // CPI_Y_MULITIPLYER
    0,       // COORD_X_SKEW
    0,       // COORD_Y_SKEW
    4,       // SCROLL_SKIP
    5670,    // CPI_MAX
    0,       // CPI_MIN
    90,      // CPI_STEP
    0,       // CPI_Z
    0,       // CPI_H
    360,     // CPI_HR_X
    360,     // CPI_HR_Y
    BUTTON_Z,
    BUTTON_MIDDLE,
    BUTTON_RIGHT,
    BUTTON_LEFT,
    BUTTON_HIGH_RES,
    BUTTON_FORWORD,
    BUTTON_BACK,
    0x00,    // LED_ACTION
    0x192f,  // VID  (No default, must be set)
    0x0000,  // PID  (No default, must be set)
    0x00,    // PROFILE_DEFAULT
    0x00,    // PROFILE_CURRENT
    0x0000,  // RELEASE
    0xffff,  // ADNS_CRC    (No default, must be set)
    0xffff,  // ADNS_ID     (No default, must be set)
    0xffff,  // ADNS_FW_LEN (No default, must be set)
    0xea60   // ADNS_FW_OFFSET
};



void track( Ser25LCxxx *eeprom );
void program( Ser25LCxxx *eeprom );

void motionCallback( void );

void btn_hr_press( void );
void btn_hr_release( void );

void btn_z_press( void );
void btn_z_release( void );

void btn_l_press(void);
void btn_l_release( void );

void btn_m_press( void );
void btn_m_release( void );

void btn_r_press( void );
void btn_r_release( void );

void btn_f_press( void );
void btn_f_release( void );

void btn_b_press( void );
void btn_b_release( void );

void prfl_a_set( void );
void prfl_b_set( void );
void prfl_c_set( void );
void prfl_d_set( void );
void prfl_e_set( void );

void motion_get(void);
void prfl_stub( void );
void debug_out(void);

int set_setting( Ser25LCxxx *eeprom, uint16_t attrib, uint16_t val, uint16_t base_address );
uint16_t get_setting( Ser25LCxxx *eeprom, uint16_t attrib, uint16_t base_address );
void clear_setting( Ser25LCxxx *eeprom, uint16_t attrib, uint16_t base_address );

void load_data( Ser25LCxxx *eeprom, uint16_t base, uint16_t len, const uint8_t* data );

uint8_t* get_data( Ser25LCxxx *eeprom, uint16_t base, uint16_t len );
