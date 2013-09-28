/* Copyright (c) 2010-2011 mbed.org, MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdint.h"
#include "USBMouse.h"

bool USBMouse::update(int16_t x, int16_t y, uint8_t button, int8_t z, int8_t h) {
    switch (mouse_type) {
        case REL_MOUSE:
            return mouseSend(x, y, button, z, h);
        case ABS_MOUSE:
            HID_REPORT report;

            report.data[0] = x & 0xff;
            report.data[1] = (x >> 8) & 0xff;
            report.data[2] = y & 0xff;
            report.data[3] = (y >> 8) & 0xff;
            report.data[4] = -z;
            report.data[5] = button & 0x07;

            report.length = 6;

            return send(&report);
        default:
            return false;
    }
}

bool USBMouse::mouseSend(int16_t x, int16_t y, uint8_t buttons, int8_t z, int8_t h) {
    HID_REPORT report;

    report.data[0] = buttons;// & 0x07;

    report.data[1] = (unsigned int) x & 0x00FF;
    report.data[2] = (unsigned int) x >> 8;
    report.data[3] = (unsigned int) y & 0x00FF;
    report.data[4] = (unsigned int) y >> 8;
    report.data[5] = -z; // >0 to scroll down, <0 to scroll up
    report.data[6] = h;

    report.length = 7;

    return send(&report);
}

bool USBMouse::move(int16_t x, int16_t y) {
    return update(x, y, button, 0, 0);
}

bool USBMouse::scroll(int8_t z, int8_t h) {
    return update(0, 0, button, z, h);
}


bool USBMouse::doubleClick() {
    if (!click(MOUSE_LEFT))
        return false;
    wait(0.1);
    return click(MOUSE_LEFT);
}

bool USBMouse::click(uint8_t button) {
    if (!update(0, 0, button, 0, 0))
        return false;
    wait(0.01);
    return update(0, 0, 0, 0, 0);
}

bool USBMouse::press(uint8_t button_) {
    printf("btn_press\n\r");
    button = button_ & 0x07;
    return update(0, 0, button, 0, 0);
}

bool USBMouse::release(uint8_t button_) {
    printf("btn_release\n\r");
    button = (button & (~button_)) & 0x07;
    return update(0, 0, button, 0, 0);
}


uint8_t * USBMouse::reportDesc() {

    if (mouse_type == REL_MOUSE) {
//
// Wheel Mouse - simplified version - 5 button, vertical and horizontal wheel
//
// Input report - 5 bytes
//
//     Byte | D7      D6      D5      D4      D3      D2      D1      D0
//    ------+---------------------------------------------------------------------
//      0   |  0       0       0    Forward  Back    Middle  Right   Left (Buttons)
//      1   |                             X High
//      2   |                             X Low
//      3   |                             Y High
//      4   |                             Y Low
//      5   |                       Vertical Wheel
//      6   |                    Horizontal (Tilt) Wheel
//
// Feature report - 1 byte
//
//     Byte | D7      D6      D5      D4   |  D3      D2  |   D1      D0
//    ------+------------------------------+--------------+----------------
//      0   |  0       0       0       0   |  Horizontal  |    Vertical
//                                             (Resolution multiplier)
//
// Reference
//    Wheel.docx in "Enhanced Wheel Support in Windows Vista" on MS WHDC
//    http://www.microsoft.com/whdc/device/input/wheel.mspx
//
        static uint8_t reportDescriptor[] = {
            0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
            0x09, 0x02,        // USAGE (Mouse)
            0xa1, 0x01,        // COLLECTION (Application)
            0x09, 0x02,        //   USAGE (Mouse)
            0xa1, 0x02,        //   COLLECTION (Logical)
            0x09, 0x01,        //     USAGE (Pointer)
            0xa1, 0x00,        //     COLLECTION (Physical)
                               // ------------------------------  Buttons
            0x05, 0x09,        //       USAGE_PAGE (Button)
            0x19, 0x01,        //       USAGE_MINIMUM (Button 1)
            0x29, 0x05,        //       USAGE_MAXIMUM (Button 5)
            0x15, 0x00,        //       LOGICAL_MINIMUM (0)
            0x25, 0x01,        //       LOGICAL_MAXIMUM (1)
            0x75, 0x01,        //       REPORT_SIZE (1)
            0x95, 0x05,        //       REPORT_COUNT (5)
            0x81, 0x02,        //       INPUT (Data,Var,Abs)
                               // ------------------------------  Padding
            0x75, 0x03,        //       REPORT_SIZE (3)
            0x95, 0x01,        //       REPORT_COUNT (1)
            0x81, 0x03,        //       INPUT (Cnst,Var,Abs)
                               // ------------------------------  X,Y position
            0x05, 0x01,        //       USAGE_PAGE (Generic Desktop)
            0x09, 0x30,        //       USAGE (X)
            0x09, 0x31,        //       USAGE (Y)
            0x16, 0x00, 0x81,  //       LOGICAL_MINIMUM (-32768)
            0x26, 0xff, 0x7f,  //       LOGICAL_MAXIMUM (32767)
            0x75, 0x10,        //       REPORT_SIZE (16)
            0x95, 0x02,        //       REPORT_COUNT (2)
            0x81, 0x06,        //       INPUT (Data,Var,Rel)
            0xa1, 0x02,        //       COLLECTION (Logical)
                               // ------------------------------  Vertical wheel res multiplier
            0x09, 0x48,        //         USAGE (Resolution Multiplier)
            0x15, 0x00,        //         LOGICAL_MINIMUM (0)
            0x25, 0x01,        //         LOGICAL_MAXIMUM (1)
            0x35, 0x01,        //         PHYSICAL_MINIMUM (1)
            0x45, 0x04,        //         PHYSICAL_MAXIMUM (4)
            0x75, 0x02,        //         REPORT_SIZE (2)
            0x95, 0x01,        //         REPORT_COUNT (1)
            0xa4,              //         PUSH
            0xb1, 0x02,        //         FEATURE (Data,Var,Abs)
                               // ------------------------------  Vertical wheel
            0x09, 0x38,        //         USAGE (Wheel)
            0x15, 0x81,        //         LOGICAL_MINIMUM (-127)
            0x25, 0x7f,        //         LOGICAL_MAXIMUM (127)
            0x35, 0x00,        //         PHYSICAL_MINIMUM (0)        - reset physical
            0x45, 0x00,        //         PHYSICAL_MAXIMUM (0)
            0x75, 0x08,        //         REPORT_SIZE (8)
            0x81, 0x06,        //         INPUT (Data,Var,Rel)
            0xc0,              //       END_COLLECTION
            0xa1, 0x02,        //       COLLECTION (Logical)
                               // ------------------------------  Horizontal wheel res multiplier
            0x09, 0x48,        //         USAGE (Resolution Multiplier)
            0xb4,              //         POP
            0xb1, 0x02,        //         FEATURE (Data,Var,Abs)
                               // ------------------------------  Padding for Feature report
            0x35, 0x00,        //         PHYSICAL_MINIMUM (0)        - reset physical
            0x45, 0x00,        //         PHYSICAL_MAXIMUM (0)
            0x75, 0x04,        //         REPORT_SIZE (4)
            0xb1, 0x03,        //         FEATURE (Cnst,Var,Abs)
                               // ------------------------------  Horizontal wheel
            0x05, 0x0c,        //         USAGE_PAGE (Consumer Devices)
            0x0a, 0x38, 0x02,  //         USAGE (AC Pan)
            0x15, 0x81,        //         LOGICAL_MINIMUM (-127)
            0x25, 0x7f,        //         LOGICAL_MAXIMUM (127)
            0x75, 0x08,        //         REPORT_SIZE (8)
            0x81, 0x06,        //         INPUT (Data,Var,Rel)
            0xc0,              //       END_COLLECTION
            0xc0,              //     END_COLLECTION
            0xc0,              //   END_COLLECTION
           0xc0               // END_COLLECTION
        };

        reportLength = sizeof(reportDescriptor);
        return reportDescriptor;
    } else if (mouse_type == ABS_MOUSE) {
        static uint8_t reportDescriptor[] = {

            USAGE_PAGE(1), 0x01,           // Generic Desktop
            USAGE(1), 0x02,                // Mouse
            COLLECTION(1), 0x01,           // Application
            USAGE(1), 0x01,                // Pointer
            COLLECTION(1), 0x00,           // Physical

            USAGE_PAGE(1), 0x01,            // Generic Desktop
            USAGE(1), 0x30,                 // X
            USAGE(1), 0x31,                 // Y
            LOGICAL_MINIMUM(1), 0x00,       // 0
            LOGICAL_MAXIMUM(2), 0xff, 0x7f, // 32767
            REPORT_SIZE(1), 0x10,
            REPORT_COUNT(1), 0x02,
            INPUT(1), 0x02,                 // Data, Variable, Absolute

            USAGE_PAGE(1), 0x01,            // Generic Desktop
            USAGE(1), 0x38,                 // scroll
            LOGICAL_MINIMUM(1), 0x81,       // -127
            LOGICAL_MAXIMUM(1), 0x7f,       // 127
            REPORT_SIZE(1), 0x08,
            REPORT_COUNT(1), 0x01,
            INPUT(1), 0x06,                 // Data, Variable, Relative

            USAGE_PAGE(1), 0x09,            // Buttons
            USAGE_MINIMUM(1), 0x01,
            USAGE_MAXIMUM(1), 0x03,
            LOGICAL_MINIMUM(1), 0x00,       // 0
            LOGICAL_MAXIMUM(1), 0x01,       // 1
            REPORT_COUNT(1), 0x03,
            REPORT_SIZE(1), 0x01,
            INPUT(1), 0x02,                 // Data, Variable, Absolute
            REPORT_COUNT(1), 0x01,
            REPORT_SIZE(1), 0x05,
            INPUT(1), 0x01,                 // Constant

            END_COLLECTION(0),
            END_COLLECTION(0)
        };
        reportLength = sizeof(reportDescriptor);
        return reportDescriptor;
    }
    return NULL;
}

#define DEFAULT_CONFIGURATION (1)
#define TOTAL_DESCRIPTOR_LENGTH ((1 * CONFIGURATION_DESCRIPTOR_LENGTH) \
                               + (1 * INTERFACE_DESCRIPTOR_LENGTH) \
                               + (1 * HID_DESCRIPTOR_LENGTH) \
                               + (2 * ENDPOINT_DESCRIPTOR_LENGTH))

uint8_t * USBMouse::configurationDesc() {
    static uint8_t configurationDescriptor[] = {
        CONFIGURATION_DESCRIPTOR_LENGTH,// bLength
        CONFIGURATION_DESCRIPTOR,       // bDescriptorType
        LSB(TOTAL_DESCRIPTOR_LENGTH),   // wTotalLength (LSB)
        MSB(TOTAL_DESCRIPTOR_LENGTH),   // wTotalLength (MSB)
        0x01,                           // bNumInterfaces
        DEFAULT_CONFIGURATION,          // bConfigurationValue
        0x00,                           // iConfiguration
        C_RESERVED | C_SELF_POWERED,    // bmAttributes
        C_POWER(0),                     // bMaxPowerHello World from Mbed

        INTERFACE_DESCRIPTOR_LENGTH,    // bLength
        INTERFACE_DESCRIPTOR,           // bDescriptorType
        0x00,                           // bInterfaceNumber
        0x00,                           // bAlternateSetting
        0x02,                           // bNumEndpoints
        HID_CLASS,                      // bInterfaceClass
        1,                              // bInterfaceSubClass
        2,                              // bInterfaceProtocol (mouse)
        0x00,                           // iInterface

        HID_DESCRIPTOR_LENGTH,          // bLength
        HID_DESCRIPTOR,                 // bDescriptorType
        LSB(HID_VERSION_1_11),          // bcdHID (LSB)
        MSB(HID_VERSION_1_11),          // bcdHID (MSB)
        0x00,                           // bCountryCode
        0x01,                           // bNumDescriptors
        REPORT_DESCRIPTOR,              // bDescriptorType
        LSB(reportDescLength()),        // wDescriptorLength (LSB)
        MSB(reportDescLength()),        // wDescriptorLength (MSB)

        ENDPOINT_DESCRIPTOR_LENGTH,     // bLength
        ENDPOINT_DESCRIPTOR,            // bDescriptorType
        PHY_TO_DESC(EPINT_IN),          // bEndpointAddress
        E_INTERRUPT,                    // bmAttributes
        LSB(MAX_PACKET_SIZE_EPINT),     // wMaxPacketSize (LSB)
        MSB(MAX_PACKET_SIZE_EPINT),     // wMaxPacketSize (MSB)
        1,                              // bInterval (milliseconds)

        ENDPOINT_DESCRIPTOR_LENGTH,     // bLength
        ENDPOINT_DESCRIPTOR,            // bDescriptorType
        PHY_TO_DESC(EPINT_OUT),         // bEndpointAddress
        E_INTERRUPT,                    // bmAttributes
        LSB(MAX_PACKET_SIZE_EPINT),     // wMaxPacketSize (LSB)
        MSB(MAX_PACKET_SIZE_EPINT),     // wMaxPacketSize (MSB)
        1,                              // bInterval (milliseconds)
    };
    return configurationDescriptor;
}
