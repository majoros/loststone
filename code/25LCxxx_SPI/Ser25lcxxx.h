/*
* Ser25lcxxx library
* Copyright (c) 2010 Hendrik Lipka
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#ifndef __SER25LCXXX_H__
#define __SER25LCXXX_H__

#include "mbed.h"

/**
A class to read and write all 25* serial SPI eeprom devices from Microchip (from 25xx010 to 25xx1024).

One needs to provide total size and page size, since this cannot be read from the devices,
and the page size differs even by constant size (look up the data sheet for your part!)
*/
class Ser25LCxxx
{
    public:
        /**
            create the handler class
            @param spi the SPI port where the eeprom is connected. Must be set to format(8,3), and with a speed matching the one of your device (up to 5MHz should work)
            @param enable the pin name for the port where /CS is connected
            @param bytes the size of you eeprom in bytes (NOT bits, eg. a 25LC010 has 128 bytes)
            @param pagesize the size of a single page, to provide overruns
        */
        Ser25LCxxx(SPI *spi, PinName enable, uint32_t bytes, uint32_t pagesize);
        
        /**
            destroys the handler, and frees the /CS pin        
        */
        ~Ser25LCxxx();
        
        /**
            read a part of the eeproms memory. The buffer will be allocated here, and must be freed by the user
            @param startAdr the adress where to start reading. Doesn't need to match a page boundary
            @param len the number of bytes to read (must not exceed the end of memory)
            @return NULL if the adresses are out of range, the pointer to the data otherwise
        */
        uint8_t* read( uint32_t startAdr,  uint32_t len);
        
        /**
            writes the give buffer into the memory. This function handles dividing the write into 
            pages, and waites until the phyiscal write has finished
            @param startAdr the adress where to start writing. Doesn't need to match a page boundary
            @param len the number of bytes to read (must not exceed the end of memory)
            @return false if the adresses are out of range
        */
        bool write( uint32_t startAdr,  uint32_t len, const uint8_t* data);
        
        /**
            fills the given page with 0xFF
            @param pageNum the page number to clear
            @return if the pageNum is out of range        
        */
        bool clearPage( uint32_t pageNum);
        
        /**
            fills the while eeprom with 0xFF
        */
        void clearMem();
    private:
        bool writePage( uint32_t startAdr,  uint32_t len, const uint8_t* data);
        uint8_t readStatus();
        void waitForWrite();
        void enableWrite();
        

        SPI* _spi;
        DigitalOut* _enable;
        uint32_t _size,_pageSize;
        
};



#endif
