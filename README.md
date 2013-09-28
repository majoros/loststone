#loststone
=========

The loststone project is a fully open source trackball. loststone was designed
from ground up. It is not based on any existing trackballs or mice. It does however take
inspiration of existing open source projects such as [mbed](http://mbed.org) and
[USBug](http://squonk42.github.com/USBug/)

##Fetures
=========

loststone is being designed to perform as well as any trackball/mouse on the
market today. In addition it is going to have several features that are not found
on any product yet offered.

   * Fully Open-Source:<br>
     Hardware & Software (CC-BY-SA, GPL3)
   * No drivers:<br>
     Drivers are annoying. Especially when there is no need for them. Lost stone
     is being designed to use the OS level HID drivers.
   * Fully configurable:<br>
     Anything that could be configurable will be configurable, without having
     to change the code and flash the chip. [Default configuration file.](https://github.com/Majoros/loststone/blob/master/config/loststone.cfg)
   * Easily programmable:<br>
     If there is a need to flash the firmware it is as easy as copying a file to
     a flash drive (Thank you NXP).
   * Firmware cursor acceleration:<br>
     Cursor acceleration is implemented within the firmware. It is configurable and optional.
   * Fully programmable buttons:<br>
     You want the left button to be on the right and the right button on the
     left? go for it. No need to change any hardware or software.
   * Profiles:<br>
     There are 5 selectable profiles. Each profile has the ability to configure
     any and all programmable features. This is good for multiple users or a
     single users that like different settings for different situations.

##Specifications

####Hardware

   * **Sensor**([ADNS-9500](http://www.pixart.com.tw/product_data_table.asp?ToPage=1&productclassify_id=1&productclassify2_id=3)):
     * High speed motion: 150ips and 30G
     * Frame rate: Up to 11,750fps
     * Selectable resolution: Up to 5040cpi with 90cpi step size
     * Independent X and Y resolutions
   * **MCU**([LPC11U24](http://www.nxp.com/products/microcontrollers/cortex_m0_m0/LPC11U24FBD48.html)):
     * Processing Power: Cortex-M0, which is, to say the least complete overkill.
     * Easily programmable: Thanks to the LPC11U24 reprograming is as easy as
       copying a file to a flash drive.
     * GPIO: with 40 GPIO pins there are more than enough pins for buttons and
       LED's and anything else that may come up.

## System Requirements

   * Any OS released in the past decade.
     Ok, to be a bit more specific an OS that supports USB HID

## Support

If anyone else actually decides to make one of these and you come accross any issues use the ([github issue
tracker](https://github.com/Majoros/loststone/issues)).

