#  Having fun with Fermax CityMax ADS 2447 video door entry B/W monitor.

There are a few small projects you can undertake for fun using your old CityMax ADS 2447 monitor.

Here are some photos of the `CityMax ADS 2447` along with an overview of the internal components:

- [Front](./img/CityMax_ADS_2447_front.png)
- [Opened screen](./img/CityMax_ADS_2447_front_screen.png)
- [Opened left side](./img/CityMax_ADS_2447_front_left.png)

It has flat CRT tube `4''(10 cm) DDY-D0100`, similar to the one used in the Sony Watchman https://en.wikipedia.org/wiki/Sony_Watchman
The brain of the device is microcontroller `PIC16F627` (1024 x 14 words Flash, 224 x 8 bit RAM и 128 x 8 bit EEPROM)

- [PIC16F627 with label](./img/PIC16F627_1.png)
- [PIC16F627 without label](./img/PIC16F627_2.png)

Unfortunately, we are unable to read the contents of the flash memory because it was programmed with the [CONFIGURATION WORD 0x0337](./img/PIC16F627_flags.png),
where 13-10 (CP1:CP0) which sets the code protection fuses for 1K/2K flash memory.
However, we can read the EEPROM, which contain only one used byte. This byte is most likely the address of the monitor.

The monitor features a function called  `Auto-Start`. When you press the camera button for 2 seconds, it powers on screen and
attempts to initiate a call to the outdoor panel.
If communication is successful, the outdoor panel powers on the camera, and you will see video stream, communicate via audio, and open the door.

If we connect oscilloscope to the line `L (audio + data)` and use the Auto-Start function, we will observe the following signal:

- [Auto-Start signal on oscilloscope](./img/Auto_Start_signal_on_oscilloscope.png)

The signal is standard serial communication with speed 1200 bps, 8 data bits, odd parity, one stop bit. But the signal level is about 18-20 volts.
To adjust the voltage to the required level, a Zener diode can be used [link](https://electronics.stackexchange.com/questions/84286/looking-to-build-a-signal-level-converter-to-reduce-20v-input-to-3-3v-out-am-i):

- [Zener diode level shifter](./img/Zener_diode_level_shifter.png)

We can now build a sniffer device to monitor communication between the monitor and the outdoor panel.
I have used a 5-volt Zener diode and connected `Vout` signal to Arduino Nano pin 8.
To support odd parity, we need to use the AltSoftSerial library, available at https://github.com/neilh10/AltSoftSerial/releases/tag/v1.5.0.3.
An example Arduino sketch for this setup is provided [here](./sniffer/Fermax_serial_sniffer.ino)

Communication between the monitor and the outdoor panel involves two bytes:
- The first byte represents the function.
- The second byte is the monitor address. The special address `0xF0 (240)` is used for `OK/ACK` function.

Here is a list of functions I have identified with the sniffer in my environment:

| Function Code | Direction | Codename and Description                                |
|---------------|-----------|---------------------------------------------------------|
| 0x59          | Received  | OK/ACK - Confirmation from the outdoor panel            |
| 0xA9          | Received  | Ring - Call from the outdoor panel                      |
| 0xAF          | Sent      | Open audio channel - Start of audio communication       |
| 0x53          | Sent      | Auto-Start - Call from the monitor to the outdoor panel |
| 0x2B          | Sent      | Open the Door                                           |

Examples of sniffer logs:

A camera button (Auto-Start) was pressed on a monitor with address 5:
```
08.05.2024 21:06:49.570 5305 >> Auto-Start ID:05
08.05.2024 21:07:23.084 59F0 << OK ID:F0
```

A call was made from the outdoor panel to a monitor with address 12; the door was opened 6 seconds later:
```
08.05.2024 18:57:58.715 59F0 << OK ID:F0
08.05.2024 18:57:59.069 A90C << Ring ID:0C
08.05.2024 18:58:05.004 2B0C >> Open door ID:0C
08.05.2024 18:58:11.206 59F0 << OK ID:F0
```

A call was made from the outdoor panel to a monitor with address 1; 20 seconds later, the "Open Door" button was pressed three times:
```
08.05.2024 16:31:57.505 A901 << Ring ID:01
08.05.2024 16:32:17.457 2B01 >> Open door ID:01
08.05.2024 16:32:17.493 2B01 >> Open door ID:01
08.05.2024 16:32:17.524 2B01 >> Open door ID:01
08.05.2024 16:32:20.789 AF01 >> Open channel ID:01
08.05.2024 16:32:20.809 59F0 << OK ID:F0
```

Next, we will attempt to play Doom on the monitor. Although the microcontroller cannot output a video signal to the screen, 
the monitor has a standard composite signal input. Therefore, we can connect a Raspberry Pi to it and run Doom.
I have Raspberry Pi 1 model B with an RCA connector, so we can activate support of composite mode as explained [here](https://magpi.raspberrypi.com/articles/rca-pi-zero).
Add the following configuration to the `/boot/config.txt` file:

```
sdtv_mode=2
# hdmi_force_hotplug=1
display_auto_detect=1
dtoverlay=vc4-kms-v3d,cma-384,composite=1
```

Install and start Doom as outlined in the guide available [here](https://pimylifeup.com/raspberry-pi-doom/):

```bash
sudo apt update && sudo apt upgrade && sudo apt install chocolate-doom libgl1-mesa-dri xinit x11-xserver-utils  -y
mkdir ~/doom-wad/ && cd ~/doom-wad/
wget https://files.pimylifeup.com/doom/shareware_doom_iwad.zip && unzip shareware_doom_iwad.zip && rm shareware_doom_iwad.zip

chocolate-doom -iwad ~/doom-wad/DOOM1.WAD
```

To power on monitor, we will use the Auto-Start function, which will allow us to play the game for 30 seconds. After this period, you will need to press the camera button again.

- ![Doom on Fermax CityMax ADS 2447](./img/Doom_on_CityMax_ADS_2447.gif)