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

#include "Ser25lcxxx.h"
#include "wait_api.h"

#define HIGH(x) ((x&0xff00)>>8)
#define LOW(x) (x&0xff)

Ser25LCxxx::Ser25LCxxx(SPI *spi, PinName enable, uint32_t bytes, uint32_t pagesize) {
    _spi=spi;
    _enable=new DigitalOut(enable);
    _size=bytes;
    _pageSize=pagesize;
    _enable->write(1);
}

Ser25LCxxx::~Ser25LCxxx() {
    delete _enable;
}

uint8_t* Ser25LCxxx::read( uint32_t startAdr,  uint32_t len) {
    // assertion
    if (startAdr+len>_size)
        return NULL;
    uint8_t* ret=(uint8_t*)malloc(len);
    _enable->write(0);
    wait_us(1);
    // send address
    if (_size<512) { // 256 and 128 bytes
        _spi->write(0x03);
        _spi->write(LOW(startAdr));
    } else if (512==_size) { // 4k variant adds 9th address bit to command
        _spi->write(startAdr>255?0xb:0x3);
        _spi->write(LOW(startAdr));
    } else if (_size<131072) { // everything up to 512k
        _spi->write(0x03);
        _spi->write(HIGH(startAdr));
        _spi->write(LOW(startAdr));
    } else { // 25xx1024, needs 3 byte address
        _spi->write(0x03);
        _spi->write(startAdr>>16);
        _spi->write(HIGH(startAdr));
        _spi->write(LOW(startAdr));
    }
    // read data into buffer
    for (uint8_t i=0;i<len;i++) {
        ret[i]=_spi->write(0);
    }
    wait_us(1);
    _enable->write(1);
    return ret;
}

bool Ser25LCxxx::write( uint32_t startAdr,  uint32_t len, const uint8_t* data) {
    if (startAdr+len>_size)
        return -1;

    uint8_t ofs=0;
    while (ofs<len) {
        // calculate amount of data to write into current page
        uint8_t pageLen=_pageSize-((startAdr+ofs)%_pageSize);
        if (ofs+pageLen>len)
            pageLen=len-ofs;
        // write single page
        bool b=writePage(startAdr+ofs,pageLen,data+ofs);
        if (!b)
            return false;
        // and switch to next page
        ofs+=pageLen;
    }
    return true;
}

bool Ser25LCxxx::writePage( uint32_t startAdr,  uint32_t len, const uint8_t* data) {
    enableWrite();

    _enable->write(0);
    wait_us(1);

    if (_size<512) { // 256 and 128 bytes
        _spi->write(0x02);
        _spi->write(LOW(startAdr));
    } else if (512==_size) { // 4k variant adds 9th address bit to command
        _spi->write(startAdr>255?0xa:0x2);
        _spi->write(LOW(startAdr));
    } else if (_size<131072) { // everything up to 512k
        _spi->write(0x02);
        _spi->write(HIGH(startAdr));
        _spi->write(LOW(startAdr));
    } else { // 25xx1024, needs 3 byte address
        _spi->write(0x02);
        _spi->write(startAdr>>16);
        _spi->write(HIGH(startAdr));
        _spi->write(LOW(startAdr));
    }

    // do real write
    for (uint8_t i=0;i<len;i++) {
        _spi->write(data[i]);
    }
    wait_us(1);
    // disable to start physical write
    _enable->write(1);
    
    waitForWrite();

    return true;
}

bool Ser25LCxxx::clearPage( uint32_t pageNum) {
    enableWrite();
    if (_size<65535) {
        uint8_t* s=(uint8_t*)malloc(_pageSize);
        for (uint8_t i=0;i<_pageSize;i++) {
            s[i]=0xff;
        }
        bool b=writePage(_pageSize*pageNum,_pageSize,s);
        delete s;
        return b;
    } else {
        _enable->write(0);
        wait_us(1);
        _spi->write(0x42);
        _spi->write(HIGH(_pageSize*pageNum));
        _spi->write(LOW(_pageSize*pageNum));
        wait_us(1);
        _enable->write(1);

        waitForWrite();
    }
    return true;
}

void Ser25LCxxx::clearMem() {
    enableWrite();
    if (_size<65535) {
        for (uint8_t i=0;i<_size/_pageSize;i++) {
            if (!clearPage(i))
                break;
        }
    }
    else
    {
        _enable->write(0);
        wait_us(1);
        _spi->write(0xc7);
        wait_us(1);
        _enable->write(1);

        waitForWrite();
    }
}

uint8_t Ser25LCxxx::readStatus() {
    _enable->write(0);
    wait_us(1);
    _spi->write(0x5);
    uint8_t status=_spi->write(0x00);
    wait_us(1);
    _enable->write(1);
    return status;
}

void Ser25LCxxx::waitForWrite() {
    while (true) {
        if (0==readStatus()&1)
            break;
        wait_us(10);
    }
}

void Ser25LCxxx::enableWrite()
{
    _enable->write(0);
    wait_us(1);
    _spi->write(0x06);
    wait_us(1);
    _enable->write(1);
}
