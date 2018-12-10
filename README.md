# mirror of https://gitlab.com/visti_how/nrfdfu_ng

Original README.md below.

## nrfdfu_ng

A Linux utility for performing Bluetooth DFU firmware upgrades for Nordic Semiconductor nRF52 and probably nRF51 using a regular BT interface. This i s rewrite of [http://git.panaceas.org/cgit.cgi/nRF51/nrfdfu/](http://git.panaceas.org/cgit.cgi/nRF51/nrfdfu/) to support newer SDKs.

This work has been "sponsored" by the company I currently work for [https://ztove.com/](https://ztove.com/) as we needed the command line tool. And is provided here in case it is useful for anyone else.

It has been written for a nRF5_SDK_14.2.0_17b948a based firmware, but should work for previous and later SDKs as long as they use the same scheme for DFU.

### License
I have asked the original author on [https://devzone.nordicsemi.com/f/nordic-q-a/1620/linux-dfu-client-for-nrf51822-over-the-air-bootloader](https://devzone.nordicsemi.com/f/nordic-q-a/1620/linux-dfu-client-for-nrf51822-over-the-air-bootloader) about the original license.  
As of writing this I have not received an answer but it is probably something along "GPL-2 or later" as most files in bluez is released under this license and the original code is supposed to be based on bluez code.
