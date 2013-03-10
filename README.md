#loststone
=========

The loststone project is a fully open source trackball. loststone was designed
from ground up. It is not based on any existing designs. It does however take
inspiration of existing opensourse projects such as [mbed](http://mbed.org) and
[USBug](http://squonk42.github.com/USBug/)

##Fetures
=========

loststone is being designed to perform as well as any trackballs/mice
on the market today.

   * No drivers:<br>
     Drivers are annoying. **ESPECIALLY** when there is absolutely no need for
     them. Lost stone is being designed to use the OS level HID drivers. This
     includes all configurations.
   * Firmware cursor acceleration:<br>
     Cursor acceleration is implemented within the firmware.
   * Fully programmable buttons:<br>
     You want the left button to be on the right and the right button on the
     left? Sure.
   * Profiles:<br>
     There are 5 selectable profiles. Each profile has the ability to configure
     any and all programmable features. This is good for multiptle users are a
     single users that like different settings for different situations.

##Specifications

####Hardware

   * **Sensor**([ADNS-9500](http://www.avagotech.com/pages/en/navigation_interface_devices/navigation_sensors/laserstream/adns-9500/)):
     * High speed motion: 150ips and 30G
     * Frame rate: Up to 11,750fps
     * Selectable resolution: Up to 5040cpi with 90cpi step size
     * independent X and Y resolutions
   * **Main Processing * Unit**([LPC11U24(http://www.nxp.com/products/microcontrollers/cortex_m0_m0/LPC11U24FBD48.html)):
     * Processing Power: Cortex-M0, which is, to say the least complete overkill.
     * GPIO: with 40 GPIO pins there are more than enough pins for buttons and
       LED's and anything else that may come up.

## System Requirements

   * Any OS released in the past decade.
     Ok, to be a bit more specific an OS that supports USB HID
