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

## Design

The circuitry was designed using ([DipTrace](http://www.diptrace.com/)). The files are available in the git repository.

#### PCB Rendering

<table>
  <tr>
    <td>
        <img alt="Main board PCB" src="https://raw.github.com/Majoros/loststone/master/images/main_board_top.png"
    </td>
    <td>
        <img alt="Sensor board PCB" src="https://raw.github.com/Majoros/loststone/master/images/sensor_board_top.png" >
    </td>
  </tr>
</table>

#### Schematic

<table>
  <tr>
    <td>
        <a href="https://github.com/Majoros/loststone/raw/master/docs/main_board_schematic.pdf" >
        <img alt="Main board schematic" src="https://raw.github.com/Majoros/loststone/master/images/main_board_schematic.png" >
        </a>
    </td>
    <td>
        <a href="https://github.com/Majoros/loststone/raw/master/docs/sensor_board_schematic.pdf" >
        <img alt="Sensor board schematic" src="https://raw.github.com/Majoros/loststone/master/images/sensor_board_schimatic.png" >
        </a>
    </td>
  </tr>
</table>

## Bill Of Materials

| Value |  Manufacturer |   Mouser Part # |  Quantity |
| -------------------- | ----------------------- | -------------------- | --- | 
| 0.01uF/630V | TDK | 810-CGA5L4C0G2J103J | 1
| 4.7uF |   TDK | 810-C1608X6S1C475K  | 2
| 470pF |   TDK | 810-C0603X7R1H471K  | 1
| 0.1uF |   TDK | 810-CGJ3E2X7R1C104K | 9
| 1uF/16V | TDK | 810-C1608X7R1C105K  | 1
| 10uF |    TDK | 810-C1608X5R1C106M  | 3
| 0.01uF |  TDK | 810-CGJ3E2X7R1C103K | 2
| 18pF/50V |    TDK | 810-C1608C0G1H180J  | 2
| 1000pF |  TDK | 810-C1608C0G1H121J  | 2
| 1.5nH |   Skyworks Solutions  | 873-SMV1233-011LF   | 4
| FPF2123 | Fairchild Semiconductor | 512-FPF2123 | 1
| LD1117S33TR | STMicroelectronics  | 511-LD1117S33   | 1
| PRTR5V0U2X |  NXP Semiconductors  | 771-PRTR5V0U2X-T/R  | 1
| 64K EEPROM |  Microchip   | 579-25LC640A-E/SN   | 1
| LPC11U24FBD48/401 |   NXP | 771-LPC11U24FBD48401    | 1
| MC14490PG |   ON Semiconductor    | 863-MC14490DWG  | 2
| UX60SC-MB-5S8 |   Hirose  | 798-UX60-MB-5S8 | 1
| NTA4151P |    ON Semiconductor    | 863-NTA4151PT1G | 1
| RGB_LED | KingBright  | 604-WP154A4-RGB | 1
| 33R | Vishay/Dale | 71-CRCW020133R0FNED | 5
| 1M |  Vishay/Dale | 71-CRCW02011M00FKED | 1
| 1K |  Vishay/Dale | 71-CRCW02011K00FNED | 2
| 100K |    Vishay/Dale | 71-CRCW0201100KFKED | 1
| 20K | Vishay/Dale | 71-CRCW020120K0FKED | 1
| TL_6100 | E-Switch    | 612-TL6100AF130QP   | 2
| 12MHz |   AVX | 581-CX3225GB12000HE | 1

