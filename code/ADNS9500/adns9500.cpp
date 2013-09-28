/*
 * adns9500.hpp - Interface to access to Avago ADNS-9500 laser mouse sensors
 *
 *   Copyright 2012 Jesus Torres <jmtorres@ull.es>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdio>
#include <cstdlib>
#include <mbed.h>
#include <string>

#include <adns9500.hpp>

#define WAIT_TSRAD()        wait_us(100)
#define WAIT_TSRR()         wait_us(20)
#define WAIT_TSRW()         wait_us(20)
#define WAIT_TSWR()         wait_us(120)
#define WAIT_TSWW()         wait_us(120)
#define WAIT_TBEXIT()       wait_us(1)      // 500ns
#define WAIT_TNCSSCLK()     wait_us(1)      // 120ns
#define WAIT_TSCLKNCS()     wait_us(20)
#define WAIT_TLOAD()        wait_us(15)

#define LONG_WAIT_MS(x)     \
    WAIT_TSCLKNCS(); ncs_.write(1); wait_ms(x); ncs_.write(0); WAIT_TNCSSCLK()
#define LONG_WAIT_US(x)     \
    WAIT_TSCLKNCS(); ncs_.write(1); wait_us(x); ncs_.write(0); WAIT_TNCSSCLK()

#define DEFAULT_MAX_FPS             1958
#define DEFAULT_MAX_FRAME_PERIOD    (1000000 / DEFAULT_MAX_FPS + 1) // in us
#define DEFAULT_X_CPI               1620
#define DEFAULT_Y_CPI               1620
#define CPI_CHANGE_UNIT             90

#define SPI_BITS_PER_FRAME          8
#define SPI_MODE                    3
#define SPI_WRITE_MODE              0x80

#define SET_BIT(word, mask)         (word | mask)
#define CLEAR_BIT(word, mask)       (word & (~mask))

namespace adns9500 {

    ADNS9500::ADNS9500(PinName mosi, PinName miso, PinName sclk, PinName ncs,
        int spi_frequency, PinName motion)
        : spi_(mosi, miso, sclk),
          motion_(motion),
          ncs_(ncs),
          enabled_(false),
          xCpi_(DEFAULT_X_CPI), yCpi_(DEFAULT_Y_CPI)
    {
        spi_.format(SPI_BITS_PER_FRAME, SPI_MODE);
        spi_.frequency(spi_frequency);

        motion_.mode(PullUp);
        motion_.fall(this, &ADNS9500::motionTrigger);
    }

    ADNS9500::~ADNS9500()
    {
        shutdown();
    }

    void ADNS9500::reset(const uint8_t* fw, uint16_t fw_len)
    {
        // SPI port reset
        ncs_.write(1);
        WAIT_TNCSSCLK();
        ncs_.write(0);
        WAIT_TNCSSCLK();
        
        // send 0x5a to POWER_UP_RESET and wait for at least 50ms
        spiSend(POWER_UP_RESET, 0x5a);
        LONG_WAIT_MS(50);
        
        // clear observation register. Only required to deassert shutdown mode.
        spiSend(OBSERVATION, 0x00);
        LONG_WAIT_US(DEFAULT_MAX_FRAME_PERIOD);

        // check observation register bits [5:0]
        int observation = spiReceive(OBSERVATION);
        if (! ADNS9500_IF_OBSERVATION_TEST(observation)) {
            WAIT_TSCLKNCS();
            ncs_.write(1);

            error("ADNS9500::reset : observation register test failed: 0x%x\n", observation);
        }

        // read motion data
        WAIT_TSRR();
        spiReceive(MOTION);
        WAIT_TSRR();
        spiReceive(DELTA_X_L);
        WAIT_TSRR();
        spiReceive(DELTA_X_H);
        WAIT_TSRR();
        spiReceive(DELTA_Y_L);
        WAIT_TSRR();
        spiReceive(DELTA_Y_H);
        
        // read product and revision id to test the connection
        WAIT_TSRR();
        int product_id = spiReceive(PRODUCT_ID);
        WAIT_TSRR();
        int revision_id = spiReceive(REVISION_ID);

        WAIT_TSCLKNCS();
        ncs_.write(1);

        if (product_id != 0x33) {
            error("ADNS9500::reset : bad product ID: 0x%x\n", product_id);
        }

        if (revision_id != 0x03) {
            error("ADNS9500::reset : bad revision ID: 0x%x\n", revision_id);
        }

        enabled_ = true;

        if (fw) {
            sromDownload(fw, fw_len);
            enableLaser();
        }
    }

    void ADNS9500::shutdown()
    {
        if (! enabled_)
            error("ADNS9500::shutdown : the sensor is not enabled\n");
        
        ncs_.write(0);
        WAIT_TNCSSCLK();
        
        // send 0x5a to POWER_UP_RESET
        spiSend(POWER_UP_RESET, 0x5a);
        
        WAIT_TSCLKNCS();
        ncs_.write(1);
        
        enabled_ = false;
    }

    int ADNS9500::read(Register lregister)
    {
        if (! enabled_)
            error("ADNS9500::read : the sensor is not enabled\n");
    
        ncs_.write(0);
        WAIT_TNCSSCLK();
        
        // send the command to read the register
        int value = spiReceive(lregister);
        
        WAIT_TSCLKNCS();
        ncs_.write(1);

        return value;
    }

    int ADNS9500::read(Register uregister, Register lregister)
    {
        if (! enabled_)
            error("ADNS9500::read : the sensor is not enabled\n");

        ncs_.write(0);
        WAIT_TNCSSCLK();

        // send the command to read the registers
        int lvalue = spiReceive(lregister);
        WAIT_TSRR();
        int uvalue = spiReceive(uregister);

        WAIT_TSCLKNCS();
        ncs_.write(1);

        return ADNS9500_UINT16(uvalue, lvalue);
    }
    
    int ADNS9500::sromDownload(const uint8_t* fw, uint16_t fw_len)
    {
        if (! enabled_)
            error("ADNS9500::sromDownload : the sensor is not enabled\n");
    
        ncs_.write(0);
        WAIT_TNCSSCLK();

        // SROM download
        spiSend(CONFIGURATION_IV, ADNS9500_CONFIGURATION_IV_SROM_SIZE);
        WAIT_TSWW();
        spiSend(SROM_ENABLE, 0x1d);
        LONG_WAIT_US(DEFAULT_MAX_FRAME_PERIOD);

        spiSend(SROM_ENABLE, 0x18);
        WAIT_TSWW();
        spi_.write(SET_BIT(SROM_LOAD_BURST, SPI_WRITE_MODE));
       
        for( uint16_t i = 0; i < fw_len; i++){
            WAIT_TLOAD();
            spi_.write(fw[i]);
        }
        WAIT_TSCLKNCS();
        ncs_.write(1);
        WAIT_TBEXIT();

        // test if SROM was downloaded successfully
        wait_us(160);
        ncs_.write(0);
        WAIT_TNCSSCLK();
        
        int srom_id = spiReceive(SROM_ID);
        
        printf("SROM ID: %x %i\r\n", srom_id, srom_id );
        WAIT_TSCLKNCS();
        ncs_.write(1);
        
        if (! srom_id)
            error("ADNS9500::sromDownload : the firmware was not successful downloaded\n");

        // test laser fault condition
        ncs_.write(0);
        WAIT_TNCSSCLK();
        
        int motion = spiReceive(MOTION);
        
        WAIT_TSCLKNCS();
        ncs_.write(1);
        
        if (ADNS9500_IF_LASER_FAULT(motion))
            error("ADNS9500::sromDownload : laser fault condition detected\n");
        
        // return the SROM CRC value
        ncs_.write(0);
        WAIT_TNCSSCLK();
        
        spiSend(SROM_ENABLE, 0x15);
        LONG_WAIT_MS(10);
        
        int lcrc = spiReceive(DATA_OUT_LOWER);
        WAIT_TSRR();
        int ucrc = spiReceive(DATA_OUT_UPPER);
        
        WAIT_TSCLKNCS();
        ncs_.write(1);
        return ADNS9500_UINT16(ucrc, lcrc);
    }

    void ADNS9500::enableLaser(bool enable)
    {
        if (! enabled_ ){
            error("ADNS9500::enableLaser : the sensor is not enabled\n");
        }
        
        ncs_.write(0);
        WAIT_TNCSSCLK();

        int laser_ctrl0 = spiReceive(LASER_CTRL0);
        
        ncs_.write(0);
        WAIT_TNCSSCLK();
  
        if (enable) {
            int laser_ctrl0 = CLEAR_BIT(0x00, ADNS9500_LASER_CTRL0_FORCE_DISABLED);
            spiSend(LASER_CTRL0, laser_ctrl0);
        }
        else {
            int laser_ctrl0 = SET_BIT(0x01, ADNS9500_LASER_CTRL0_FORCE_DISABLED);
            spiSend(LASER_CTRL0, laser_ctrl0);
        }        

        WAIT_TSCLKNCS();
        ncs_.write(1);
    }
    
    bool ADNS9500::getMotionDelta(int16_t& dx, int16_t& dy)
    {
        if (! enabled_)
            error("ADNS9500::getMotionDelta : the sensor is not enabled\n");

        ncs_.write(0);
        WAIT_TNCSSCLK();

        dx = 0;
        dy = 0;

        int motion = spiReceive(MOTION);
        
        if (ADNS9500_IF_MOTION(motion)) {
            WAIT_TSRR();
            int dxl = spiReceive(DELTA_X_L);
            WAIT_TSRR();
            dx = ADNS9500_INT16(spiReceive(DELTA_X_H), dxl);
            
            WAIT_TSRR();
            
            int dyl = spiReceive(DELTA_Y_L);
            WAIT_TSRR();
            dy = ADNS9500_INT16(spiReceive(DELTA_Y_H), dyl);
        }
        WAIT_TSCLKNCS();
        ncs_.write(1);
        
        return ADNS9500_IF_MOTION(motion);
    }
    
    bool ADNS9500::getMotionDeltaMM(float& dx, float& dy)
    {
        int16_t rawDx, rawDy;
        bool motion = getMotionDelta(rawDx, rawDy);
        dx = (float)rawDx / xCpi_ * 25.4;
        dy = (float)rawDy / yCpi_ * 25.4;

        return motion;
    }

    bool ADNS9500::getMotionData(MotionData& data)
    {
        if (! enabled_)
            error("ADNS9500::getMotionData : the sensor is not enabled\n");
    
        ncs_.write(0);
        WAIT_TNCSSCLK();
        
        // activate motion burst mode
        spi_.write(0x50);
        WAIT_TSRAD();   // see the chronogram
        
        // read motion burst data
        data.motion = spi_.write(0x00);
        data.observation = spi_.write(0x00);
        
        int ldx = spi_.write(0x00);
        data.dx = ADNS9500_INT16(spi_.write(0x00), ldx);
        
        int ldy = spi_.write(0x00);
        data.dy = ADNS9500_INT16(spi_.write(0x00), ldy);
        
        data.surfaceQuality = spi_.write(0x00) * 4;
        data.averagePixel = spi_.write(0x00) / 1.76;
        data.maximumPixel = spi_.write(0x00);
        data.minimumPixel = spi_.write(0x00);
        
        int ushutter = spi_.write(0x00);
        data.shutter = ADNS9500_UINT16(ushutter, spi_.write(0x00));
        
        int uframe_period = spi_.write(0x00);
        data.framePeriod = ADNS9500_UINT16(uframe_period, spi_.write(0x00));

        WAIT_TSCLKNCS();
        ncs_.write(1);
        WAIT_TBEXIT();

        data.dxMM = (float)data.dx / xCpi_ * 25.4;
        data.dyMM = (float)data.dy / yCpi_ * 25.4;

        // write a value to Motion register to clear motion bit
        ncs_.write(0);
        WAIT_TNCSSCLK();
        
        spiSend(MOTION, 0x00);

        WAIT_TSCLKNCS();
        ncs_.write(1);

        return ADNS9500_IF_MOTION(data.motion);
    }
    
    void ADNS9500::setResolution( uint16_t cpi_xy )
    {
        
        if (! enabled_)
            error("ADNS9500::setResolution : the sensor is not enabled\n");

        int res_xy = cpi_to_res(cpi_xy);
        
        ncs_.write(0);
        WAIT_TNCSSCLK();
        
        // enable XY axes CPI in sync mode
        int rpt_mod = spiReceive(CONFIGURATION_II);
        rpt_mod = CLEAR_BIT(rpt_mod, ADNS9500_CONFIGURATION_II_RPT_MOD);
        WAIT_TSRW();
        spiSend(CONFIGURATION_II, rpt_mod);

        // set resolution for X-axis and Y-axis
        WAIT_TSWW();
        spiSend(CONFIGURATION_I, res_xy);

        WAIT_TSCLKNCS();
        ncs_.write(1);
        
        xCpi_ = res_xy * CPI_CHANGE_UNIT;
        yCpi_ = res_xy * CPI_CHANGE_UNIT;
    }
    
    void ADNS9500::setResolution(uint16_t cpi_x, uint16_t cpi_y)
    {
        if (! enabled_)
            error("ADNS9500::setResolution : the sensor is not enabled\n");
        
        int res_x = cpi_to_res(cpi_x);
        int res_y = cpi_to_res(cpi_y);
           
        ncs_.write(0);
        WAIT_TNCSSCLK();

        // disable XY axes CPI in sync mode
        int rpt_mod = spiReceive(CONFIGURATION_II);
        rpt_mod = SET_BIT(rpt_mod, ADNS9500_CONFIGURATION_II_RPT_MOD);
        WAIT_TSRW();
        spiSend(CONFIGURATION_II, rpt_mod);
        
        // set resolution for X-axis
        WAIT_TSWW();
        spiSend(CONFIGURATION_I, res_x);
                
        // set resolution for Y-axis
        WAIT_TSWW();
        spiSend(CONFIGURATION_V, res_y);

        WAIT_TSCLKNCS();
        ncs_.write(1);

        xCpi_ = res_x * CPI_CHANGE_UNIT;
        yCpi_ = res_y * CPI_CHANGE_UNIT;
    }
    
    void ADNS9500::captureFrame(uint8_t* pixels)
    {
        if (! enabled_)
            error("ADNS9500::captureFrame : the sensor is not enabled\n");
            
        ncs_.write(0);
        WAIT_TNCSSCLK();
        
        spiSend(FRAME_CAPTURE, 0x93);
        WAIT_TSWW();
        spiSend(FRAME_CAPTURE, 0xc5);
        LONG_WAIT_US(2*DEFAULT_MAX_FRAME_PERIOD);

        // check for first pixel reading motion bit
        int motion = spiReceive(MOTION);
        WAIT_TSRR();
        while(! ADNS9500_IF_FRAME_FIRST_PIXEL(motion)) {
            int motion = spiReceive(MOTION);
            WAIT_TSRR();
        }

        // read pixel values
        spi_.write(PIXEL_BURST);
        WAIT_TSRAD();
        for (uint8_t* p = pixels; p != pixels + NUMBER_OF_PIXELS_PER_FRAME; ++p) {
            *p = spi_.write(0x00);
            WAIT_TLOAD();
        }

        // burst exit
        ncs_.write(1);
        WAIT_TBEXIT();
    }
    
    void ADNS9500::spiSend(Register address, int value)
    {
        spi_.write(SET_BIT(address, SPI_WRITE_MODE));
        spi_.write(value);
    }

    int ADNS9500::spiReceive(Register address)
    {
        spi_.write(CLEAR_BIT(address, SPI_WRITE_MODE));
        WAIT_TSRAD();
        return spi_.write(0x00);
    }
}