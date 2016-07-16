# A program for sumping the EEPROM contents from an FT2232H chip

To use

```
make
./dump
```

If `dump` fails, it might be necessary to remove a couple of interfering kernel modules

```
sudo rmmod ftdi_sio && sudo rmmod usbserial
```
