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
 
#ifndef ADNS9500_HPP_
#define ADNS9500_HPP_

#include <mbed.h>
#include <stdint.h>
#include <string>
#include "adns9500_firmware.hpp"

#define ADNS9500_CONFIGURATION_II_RPT_MOD   (1 << 2)
#define ADNS9500_CONFIGURATION_IV_SROM_SIZE (1 << 1)
#define ADNS9500_LASER_CTRL0_FORCE_DISABLED (1 << 0)
#define ADNS9500_OBSERVATION_CHECK_BITS     0x3f

#define ADNS9500_IF_MOTION(x)               (bool)(x & 0x80)
#define ADNS9500_IF_LASER_FAULT(x)          (bool)(x & 0x40)
#define ADNS9500_IF_RUNNING_SROM_CODE(x)    (bool)(x & 0x80)
#define ADNS9500_IF_FRAME_FIRST_PIXEL(x)    (bool)(x & 0x01)
#define ADNS9500_IF_OBSERVATION_TEST(x)     (bool)(x & ADNS9500_OBSERVATION_CHECK_BITS)
#define ADNS9500_UINT16(ub, lb)             (uint16_t)(((ub & 0xff) << 8) | (lb & 0xff))
#define ADNS9500_INT16(ub, lb)              (int16_t)(((ub & 0xff) << 8) | (lb & 0xff))

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0) 

namespace adns9500
{
    // Maximum SPI clock frequency supported by the sensor
    const int MAX_SPI_FREQUENCY = 2000000;
    
    // Internal oscillator norminal frequency
    const int INTERNAL_OSCILLATOR_FREQUENCY = 47000000;
    
    // Number of pixels per frame
    const int NUMBER_OF_PIXELS_PER_FRAME = 900;
    
    // Maximum surface quality
    const int MAX_SURFACE_QUALITY = 676;    // 169 * 4

    //
    // Sensor registers
    //
    
    enum Register
    {
        PRODUCT_ID         = 0x00,
        REVISION_ID        = 0x01,
        MOTION             = 0x02,
        DELTA_X_L          = 0x03,
        DELTA_X_H          = 0x04,
        DELTA_Y_L          = 0x05,
        DELTA_Y_H          = 0x06,
        SQUAL              = 0x07,
        PIXEL_SUM          = 0x08,
        MAXIMUM_PIXEL      = 0x09,
        MINIMUM_PIXEL      = 0x0a,
        SHUTTER_LOWER      = 0x0b,
        SHUTTER_UPPER      = 0x0c,
        FRAME_PERIOD_LOWER = 0x0d,
        FRAME_PERIOD_UPPER = 0x0e,
        CONFIGURATION_I    = 0x0f,
        CONFIGURATION_II   = 0x10,
        FRAME_CAPTURE      = 0x12,
        SROM_ENABLE        = 0x13,
        LASER_CTRL0        = 0x20,
        DATA_OUT_LOWER     = 0x25,
        DATA_OUT_UPPER     = 0x26,
        SROM_ID            = 0x2a,
        OBSERVATION        = 0x24,
        CONFIGURATION_V    = 0x2f,
        CONFIGURATION_IV   = 0x39,
        POWER_UP_RESET     = 0x3a,
        MOTION_BURST       = 0x50,
        SROM_LOAD_BURST    = 0x62,
        PIXEL_BURST        = 0x64
    };

    //
    // Supported resolutions
    //

    enum Resolution
    {
        CPI_90   = 0x01,
        CPI_1630 = 0x12,
        CPI_3240 = 0x24,
        CPI_5040 = 0x38
    };
    
    //
    // Motion burst data
    //
    
    struct MotionData
    {
        MotionData()
            : motion(0), observation(0), dx(0), dy(0), dxMM(0), dyMM(0),
              surfaceQuality(0), averagePixel(0), maximumPixel(0),
              minimumPixel(0), shutter(0), framePeriod(0)
        {}
    
        int motion;
        int observation;
        int dx;
        int dy;
        float dxMM;
        float dyMM;
        int surfaceQuality;
        float averagePixel;
        int maximumPixel;
        int minimumPixel;
        int shutter;
        int framePeriod;
    };

    //
    // Interface to access to ADNS-9500 mouse sensor
    //

    class ADNS9500
    {
        public:
        
            //
            // Create the sensor interface
            //
            // @param mosi MOSI pin for the SPI interface
            // @param miso MISO pin for the SPI interface
            // @param sclk SCLK pin for the SPI interface
            // @param spi_frequency SPI clock frequency in Hz up to MAX_SPI_PORT_FREQUENCY
            // @param ncs A digital active-low output pin for sensor chip select
            // @param motion A digital active-low input pin activated by the sensor when motion
            //               is detected
            //
            ADNS9500(PinName mosi, PinName miso, PinName sclk, PinName ncs,
                int spi_frequency = MAX_SPI_FREQUENCY, PinName motion = NC);

            //
            // Destroy de sensor interface
            //
            ~ADNS9500();

            //
            // Power up/reset the sensor
            // Terminate with error if the connection can not be established
            //
            // @param firmware If the firmware has to be downloaded, C-string containing the name
            //                 of the file where it is stored, or NULL in other case
            //
            void reset(const uint8_t* fw = NULL, uint16_t fw_len = 0 );

            //
            // Shutdown the sensor
            //
            void shutdown();

            //
            // Read the value of a sensor register
            //
            // @param lregister The register which to read its value
            // @return The value of the register
            //
            int read(Register lregister);
            
            //
            // Read the values of sensor registers
            //
            // @param uregister The register which to read the upper byte
            // @param lregister The register which to read the lower byte
            // @return The values of registers as a 16-bit integer, putting the value
            //         of uregister in the upper byte and the value of lregister in the
            //         lower byte
            //
            int read(Register uregister, Register lregister);

            //
            // Get information about sensor status
            //
            // @return The value of MOTION register. It tells if motion or laser fault
            //         conditions have ocurred, laser power setting status and operating
            //         mode in current frame
            //
            int status()
                { return read(MOTION); }
           
            //
            // Download the firmware to the sensor SROM
            //
            // @param filename The name of the file which contains the sensor firmware
            // @return The SROM CRC value
            //
            int sromDownload(const uint8_t*, uint16_t);

            //
            // Enable the laser
            //
            // @param enable True if laser must be enabled, or false if laser must be disabled
            //
            void enableLaser(bool enable=true);
            void getLaser(void);

            //
            // Get motion deltas from sensor
            //
            // @param dx The component X of displacement
            // @param dy The component Y of displacement
            // @return True if motion was occurred since the last time the function was called,
            //         or false in other case
            //
            bool getMotionDelta(int16_t& dx, int16_t& dy);

            //
            // Get motion deltas in mm. from sensor
            //
            // @param dx The component X of displacement in mm.
            // @param dy The component Y of displacement in mm.
            // @return True if motion was occurred since the last time the function was called,
            //         or false in other case
            //
            bool getMotionDeltaMM(float& dx, float& dy);
            
            //
            // Get all information about motion
            //
            // @param data The struct where sensor data will be stored
            // @return True if motion was occurred since the last time the function was called,
            //         or false in other case
            //
            bool getMotionData(MotionData& data);

            //
            // Set the resolution on XY axes together
            //
            // @param xy_resolution The resolution for X-axis and Y-axis
            //
            void setResolution( uint16_t xy_resolution);

            //
            // Set the resolutions on X-axis and Y-axis
            //
            // @param x_resolution The resolution for X-axis
            // @param y_resolution The resolution for Y-axis
            //
            void setResolution(uint16_t x_resolution, uint16_t y_resolution);

            //
            // Get a full array of pixel values from a single frame.
            // This disables navigation and overwrites any donwloaded firmware,
            // so call to reset() is needed to restore them
            //
            // @param pixels A pointer to the array where pixel values will be stored
            //
            void captureFrame(uint8_t* pixels);

            //
            // Member function invoked when motion has ocurred and if a motion pin
            // was specified when the object constructor was called.
            // By default it invokes the function specified by a previous call to attach()
            //
            virtual void motionTrigger()
                { motionTrigger_.call(); }

            //
            // Attach a function to call when a falling edge occurs on motion pin
            //
            // @param function A pointer to a function or 0 to set the attached function as none
            //
            void attach(void (*function)(void))
                { motionTrigger_.attach(function); }

            //
            // Attach a member function to call when a falling edge occurs on motion pin
            //
            // @param object A reference to the object to call the member function on
            // @param function A pointer to the member function to be called
            //
            template<typename T>
            void attach(T& object, void (T::*member)(void))
                { motionTrigger_.attach(object, member); }

            int cpi_to_res( uint16_t cpi ){
                int res = cpi / 90;
        
                if( res < 0x01 ){
                    res = 0x01;
                }
                else if( res > 0x38 ){
                    res = 0x38;
                }
                return res;
            }
            
        private:
            SPI spi_;
            InterruptIn motion_;
            DigitalOut ncs_;

            bool enabled_;            
            int xCpi_, yCpi_;
            
            FunctionPointer motionTrigger_;
            
            //
            // Write a byte to the specified register
            //
            // @param address The register address
            // @param value The value to be written to the register
            //
            void spiSend(Register address, int value);

            //
            // Read a byte from the specified register
            //
            // @param address The register address
            // @return The value of the register
            //
            int spiReceive(Register address);
    };
}

#endif /* ADNS9500_HPP_ */