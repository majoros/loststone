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

#include "main.h"


int main(void)
{
    
    printf("And away we go.\n\r");
    activity = 1;
    run_mode.mode(PullUp);
    Ser25LCxxx *eeprom;
    
    #ifdef MBED
    SPI eeprom_spi( p11, p12, p13); // mosi, miso, sclk 
    #elif
    SPI eeprom_spi( P0_21, P0_22, P1_20); // mosi, miso, sclk 
    #endif
    
    eeprom_spi.format(8,3);
    eeprom_spi.frequency(1000000);
    
    #ifdef MBED
    //eeprom = new Ser25LCxxx( &eeprom_spi, p15, 0x10000, 0x20 ); 
    #elif
    eeprom = new Ser25LCxxx( &eeprom_spi, P1_27, 0x10000, 0x20 ); 
    #endif

    //retreave default and system settings from the EEPROM and override the default values.
    printf("Loading settings\n\r");
    //for( int i = 0; i < sizeof(s)/sizeof(uint16_t); i++ ){
    //    printf("Setting %d: %X\r\n", i, s[i]);
        //set_setting( eeprom, i, s[i], SETTINGS_BASE ); 
        //s[i] = get_setting( eeprom, i, SETTINGS_BASE );
    //    printf("Setting %d: %X\r\n", i, s[i]);
    //}
    
    //eeprom->write( s[ADNS_FW_OFFSET], ADNS9500_FIRMWARE_LEN, adns9500FWArray );
    
    if( run_mode ){
        printf("Tracking mode\n\r");
        track( eeprom );
    }
    else{
        printf("Programming mode\n\r");
        program( eeprom );
    }
}

void track( Ser25LCxxx *eeprom ){
    activity = 0;

    mouse = new USBMouse( REL_MOUSE, s[VID], s[PID], s[RELEASE]) ;

    /* 
     * mosi == p5 / P0_9 -- 6
     * miso == p6 / P0_8 -- 4
     * sclk == p7 / P0_10 -- 5
     * ncs  == p8 / P1_16 -- 3
     * spi_frequency = MAX_SPI_FREQUENCY
     * motion == p14 / P0_22 -- 7
     */


    #ifdef MBED
    printf("Creating sensor object for mbed\n\r");
    sensor = new adns9500::ADNS9500(p5, p6, p7, p8, adns9500::MAX_SPI_FREQUENCY);
    #elif
    printf("Creating sensor object raw chip\n\r");
    sensor = new adns9500::ADNS9500(P0_9, P0_8, P0_10, P1_16, adns9500::MAX_SPI_FREQUENCY); 
    #endif
    
    static fn press_funcs[] = {
        btn_l_press,
        btn_m_press,
        btn_r_press,
        btn_f_press,
        btn_b_press,
        btn_z_press,
        btn_hr_press};
        
    static fn release_funcs[] = {
        btn_l_release,
        btn_m_release,
        btn_r_release,
        btn_f_release,
        btn_b_release,
        btn_z_release,
        btn_hr_release};
        
    printf("Inisializing buttons\n\r");

// Mouse buttons

    //motion_in.mode(PullDown);
    //motion_in.fall(motion_get);
     
    btn_a.mode(PullNone);
    btn_a.fall(press_funcs[s[BTN_A]]);
    btn_a.rise(release_funcs[s[BTN_A]]);
    
    btn_b.mode(PullNone);
    btn_b.fall(press_funcs[s[BTN_B]]);
    btn_b.rise(release_funcs[s[BTN_B]]);

    btn_c.mode(PullNone);
    btn_c.fall(press_funcs[s[BTN_C]]);
    btn_c.rise(release_funcs[s[BTN_C]]);
    
    btn_d.mode(PullDown);
    btn_d.fall(press_funcs[s[BTN_D]]);
    btn_d.rise(release_funcs[s[BTN_D]]);
    
    btn_e.mode(PullNone);
    btn_e.fall(press_funcs[s[BTN_E]]);
    btn_e.rise(release_funcs[s[BTN_E]]);
    
    btn_f.mode(PullNone);
    btn_f.fall(press_funcs[s[BTN_F]]);
    btn_f.rise(release_funcs[s[BTN_F]]);

    btn_g.mode(PullNone);
    btn_g.fall(press_funcs[s[BTN_G]]);
    btn_g.rise(release_funcs[s[BTN_G]]);

// Profile buttons
    prfl_a.mode(PullUp);
    prfl_a.fall(&prfl_a_set);
    prfl_a.rise(&prfl_stub);
    
    prfl_b.mode(PullUp);
    prfl_b.fall(&prfl_b_set);
    prfl_b.rise(&prfl_stub);
    
    prfl_c.mode(PullUp);
    prfl_c.fall(&prfl_c_set);
    prfl_c.rise(&prfl_stub);
    
    prfl_d.mode(PullUp);
    prfl_d.fall(&prfl_d_set);
    prfl_d.rise(&prfl_stub);
    
    prfl_e.mode(PullUp);
    prfl_e.fall(&prfl_e_set);
    prfl_e.rise(&prfl_stub);
    
    debug.mode(PullUp);
    debug.fall(&debug_out);
    debug.rise(&debug_out);
    
    int16_t dx, dy;
    float mx, my;


    sensor->reset();

    uint8_t *adns_fw;

    //eeprom->write(ADNS_FW_OFFSET, ADNS_FW_LEN, adns9500FWArray);
    //adns_fw = eeprom->read( ADNS_FW_OFFSET, ADNS_FW_LEN );
//eeprom->write( s[ADNS_FW_OFFSET], ADNS9500_FIRMWARE_LEN, adns9500FWArray );

    activity = 1;
    printf("Loading sensor firmware\r\n");
    uint16_t crc = sensor->sromDownload( adns9500FWArray, ADNS9500_FIRMWARE_LEN ); //FIXME

    if( ADNS6010_FIRMWARE_CRC != crc ){ //ADNS6010_FIRMWARE_CRC
        printf("Firmware CRC does not match [%X] [%X]\n\r", ADNS6010_FIRMWARE_CRC, crc);             
        while (true){
            activity = 0;
            wait(0.2);
            activity = 1;
            wait(0.2);
        }
    }
    else{
        activity = 0;
        printf("Firmware CRC matches [%X] [%X]\n\r", ADNS6010_FIRMWARE_CRC, crc);
    }

    // The firmware was set correctly. Freeing the 3K! of ram.
    //delete adns_fw;

    printf("Enableing lazer\n\r");
    sensor->enableLaser();
    printf("setting inishal resolution %d %d\n\r", 5670, s[CPI_Y] );
    sensor->setResolution( s[CPI_X], s[CPI_Y] );

    printf("Starting Loop\n\r");
    activity = 1;
    //Timer st;
    //st.start();
    int scroll_counter = 0;
    while (true){
        
        //rest_counter++;
        /*
         * Moved the setResolution calls out of the interupt callbacks as they
         * havequite a few waits in them.
         */
        if( set_res_hr ){
            set_res_hr = false;
            sensor->setResolution( s[CPI_HR_X], s[CPI_HR_Y] );
        }
        
        if( set_res_z ){
            set_res_z = false;
            sensor->setResolution( s[CPI_Z], s[CPI_H] );
        }
        
        if( set_res_default ){
            set_res_default = false;
            sensor->setResolution( s[CPI_X], s[CPI_Y] );
        }
     
        if( !motion_in){

            motion_triggered = false;

            sensor->getMotionDelta(dx, dy);

            if( z_axis_active ){

                if(scroll_counter >= s[SCROLL_SKIP]){
                    scroll_counter = 0;
                    mouse->scroll( - dy, dx );
                }
                scroll_counter++;
            }
            else{
                /*
                 * If the cpi multiplyer values are not zero they we modify the
                 * x and y values accordingly.
                 */
                if(s[CPI_X_MULITIPLYER] != 0 and s[CPI_Y_MULITIPLYER] != 0){
                    mx = (float(abs(dx))/float(s[CPI_X_MULITIPLYER]) + 1.0);
                    my = (float(abs(dy))/float(s[CPI_Y_MULITIPLYER]) + 1.0);
                    dx = mx * dx;
                    dy = my * dy;
                }
                
                /*
                 * If the coord skew values are not zero than we will skew the
                 * coordanite place accordingy.
                 */
                if(s[COORD_X_SKEW] != 0 and s[COORD_X_SKEW]){                
                    float rad_x = degree2rad(s[COORD_X_SKEW]);
                    float rad_y = degree2rad(s[COORD_Y_SKEW]);

                    float new_x = float(dx) * cos(rad_x) - float(dy) * sin(rad_x);
                    float new_y = float(dy) * cos(rad_y) + float(dx) * sin(rad_y);               
                    dx = int(new_x);
                    dy = int(new_y);
                }  
                mouse->move(  int(dx), -int(dy) );
            }
        }
            
        //}
        

        // Load the current profile.
        // This rights the first 10 16bit values of the s array from the selected profile.
        //
        // Base is calculated...
        //     selected profile * profile length * 2 + profile base
        //
        //     (3 * 7 * 2) + 64  (multiplying by two because they are 16bit values.
        // TODO: should i move this to a function?
        if( profile_load ){
        //    for( int i = 0; i < PROFILE_LEN; i++ ){
        //        //s[i] = get_setting( eeprom, i, (s[PROFILE_CURRENT] * PROFILE_LEN) + PROFILE_BASE );
        //    }
        //    sensor->setResolution( s[CPI_X], s[CPI_Y] );
            profile_load = false;
        }
    }
}

void program( Ser25LCxxx *eeprom ){

    
    //USBHID *hid = new USBHID( 64, 64, 0x192f, 0x0, 0x0);
    USBHID *hid = new USBHID();
    //This report will contain data to be sent
    HID_REPORT send_rep;
    HID_REPORT recv_rep;
    
    send_rep.length = 64;
    uint16_t base;
    uint16_t len;
    //Fill the report
    for (int i = 0; i < send_rep.length; i++) {
        send_rep.data[i] = rand() & 0xff;
    }
            
    //Send the report
    //hid->send(&send_rep);

    uint8_t *tmp;

    printf("Entering loop\n\r");
    while (1) {
        if( hid->readNB(&recv_rep)) {
            printf("Receaved some data.\n\r");
            switch ((recv_rep.data[0])){
                case SET:
                    //set_setting( eeprom, recv_rep.data[1], UINT16(recv_rep.data[2],recv_rep.data[3]));
                    break;
                case GET:
                    //tmp = get_setting( eeprom, recv_rep.data[1] );
                        
                    //send_rep.data[0] = (int)tmp;
                    //send_rep.data[1] = (int)(tmp >> 8);
                        
                    //hid->send(&send_rep);
                    
                    break;
                case CLEAR:
                    //clear_setting( eeprom, recv_rep.data[1] );
                    break;
                case INIT:
                    eeprom->clearMem();
                    break;
                case LOAD_DATA:
                    printf("LOADING DATA\n\r");
                    base = UINT16( recv_rep.data[1], recv_rep.data[2] );
                    len  = recv_rep.data[3];
                    printf("BASE: %X LEN: %X\n\r", base, len);
                    load_data( eeprom, base, len, &recv_rep.data[4] );
                    wait(0.1);
                    tmp = get_data( eeprom, base, len );
                    printf("BASE: %X LEN: %X\n\r", base, len);
                    for( uint16_t i=0; i < (len); i++){
                        printf("%X\n\r", tmp[i]);
                        send_rep.data[i] = tmp[i]; //FIXME
                    }
                    hid->send(&send_rep);
                    delete( tmp);
                    break;
                case GET_DATA:
                    //base = UINT16( recv_rep.data[1], recv_rep.data[2] );
                    //len  = UINT16( recv_rep.data[3], recv_rep.data[4] );
                    //tmp = get_data( Ser25LCxxx *eeprom, uint16_t base, uint16_t len ){
                default:
                    // FIXME: error handling.
                    break;
            }
        }
    }
}

void btn_l_press(){
    printf("button,left,press\n\r");
    mouse->press(MOUSE_LEFT);

}
void btn_l_release(){
    printf("button,left,release\n\r");
    mouse->release(MOUSE_LEFT);

}

void btn_m_press(){
    printf("button,middle,press\n\r");
    mouse->press(MOUSE_MIDDLE);
}
void btn_m_release(){
    printf("button,middle,release\n\r");
    mouse->release(MOUSE_MIDDLE);
}

void btn_r_press(){
    printf("button,right,press\n\r");
    mouse->press(MOUSE_RIGHT);
}

void btn_r_release(){
    printf("button,right,release\n\r");
    mouse->release(MOUSE_RIGHT);
}

void btn_f_press(){
    printf("button,forword,press\n\r");
    mouse->press(MOUSE_FORWORD);
}
void btn_f_release(){
    printf("button,forword,release\n\r");
    mouse->release(MOUSE_FORWORD);
}

void btn_b_press(){
    printf("button,back,press\n\r");
    mouse->press(MOUSE_BACK);
}

void btn_b_release(){
    printf("button,back,release\n\r");
    mouse->release(MOUSE_BACK);
}

void btn_hr_press(){
    printf("button,high_res,press\n\r");
    set_res_hr = true;
    set_res_default = false;
    high_rez_active = true;
}
void btn_hr_release(){
    printf("button,high_res,release\n\r");
    set_res_hr = false;
    set_res_default = true;
    high_rez_active = false;
}

void btn_z_press(){
    printf("button,z,press\n\r");
    set_res_z = true;
    set_res_default = false;
    z_axis_active = true;
}
void btn_z_release(){
    printf("button,z,release\n\r");
    set_res_z = false;
    set_res_default = true;
    z_axis_active = false;
}

void prfl_a_set(){
    s[PROFILE_CURRENT] = 0;
    profile_load = true;
}

void prfl_b_set(){
    s[PROFILE_CURRENT] = 1;
    profile_load = true;
}

void prfl_c_set(){
    s[PROFILE_CURRENT] = 2;
    profile_load = true;
}

void prfl_d_set(){
    s[PROFILE_CURRENT] = 3;
    profile_load = true;
}

void prfl_e_set(){
    s[PROFILE_CURRENT] = 4;
    profile_load = true;
}

void prfl_stub(){
}

void debug_out(){
printf("motion_triggerd %d\n\r" , motion_triggered);
printf("z_axis_active %d\n\r", z_axis_active);
printf("high_rez_active %d\n\r", high_rez_active);
printf("profile_load %d\n\r", profile_load); // Always inishally load the profile even if it might be the same.
printf("set_res_hr %d\n\r", set_res_hr);
printf("set_res_z %d\n\r" , set_res_z);
printf("set_res_default %d\n\r", set_res_default);
}

/*
 * The settings are kept in the first 'N' addresses of the of the EEPROM.
 * They are 16bits long. The default settings are in a uint16_t array. So
 * to set/get a setting you use the array value and multiple it by two.
 * This way I can easily get all setting at the start of the program by
 * simply looping through the array.
 */


int set_setting( Ser25LCxxx *eeprom, uint16_t attrib, uint16_t val, uint16_t base_address ){

    uint8_t hl[2];
    
    // TODO: Its working but is it working the way i think its working
    hl[0] = (int)val;
    hl[1] = (int)(val >> 8);
    
    printf("Set Setting %d: L[%X] H[%X]\n\r", attrib, hl[0], hl[1] );

    if ( eeprom->write( attrib * 2, 0x2, hl )){
        return true;
    }
    return false;
}

uint16_t get_setting( Ser25LCxxx *eeprom, uint16_t attrib, uint16_t base_address ){
    
    uint16_t val;
    uint8_t *hl;
    
    hl = eeprom->read( (attrib * 2) + base_address, 0x2 );
    printf("Get Setting %d: L[%X] H[%X]\n\r", attrib, hl[0], hl[1] );
    val = UINT16( hl[1], hl[0] );
    if( val != 0xFFFF ){
        return val;
    }

    return s[attrib];
}

void clear_setting( Ser25LCxxx *eeprom, uint16_t attrib, uint16_t base_address ){
    uint8_t val[2];
    val[0] = val[1] = 0xFF;
    eeprom->write( (attrib * 2) + base_address , 0x2, val );
}


/*
 * Getting and setting the firmware is a bit easer as it is all 8bit.
 */
void load_data( Ser25LCxxx *eeprom, uint16_t base, uint16_t len, const uint8_t* data ){
    eeprom->write( base , len, data );
}

uint8_t* get_data( Ser25LCxxx *eeprom, uint16_t base, uint16_t len ){
    return (uint8_t*)eeprom->read( base, len );
}
