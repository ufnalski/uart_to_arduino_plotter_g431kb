# Yet another UART to USB sniffer [STM32G431KB]
Why another UART to USB converter when we have [CH343G](https://www.waveshare.com/ch343-usb-uart-board.htm), [FT232](https://www.waveshare.com/ft232-usb-uart-board-type-c.htm), [CP2102](https://www.waveshare.com/cp2102-usb-uart-board-type-c.htm), and many other similar converters? The short answer is flexibility. The longer one is sniffing communication between two wiper actuators developed by Bosch. I was playing with Renault Scenic IV wiper actuators. The part numbers are [288A54125R](https://allegro.pl/listing?string=288A54125R) and [288157018R](https://allegro.pl/listing?string=288157018R). The left one (at least for the European version) communicates with the BCM over the LIN bus [[19200 bps]](https://github.com/ufnalski/renault_scenic_wiper_actuators_g431kb), whereas the right one takes commands from the left one using standard UART communication (no break and sync fields, bitrate much higher than allowed by the LIN bus standard). Standard UART to USB converters support some predefined set of bitrates. It was clear from the logic analyzer that the wipers use something close to 64 kbps, which is a non-standard baud rate for UART. The closest standard 57600 bps didn't work. Therefore, an STM32 was used as a converter. The beauty of such a solution is its versatility. You can play with any baud rates, any data types, any checksums/CRCs, any serial plotters, and many more. A couple of lines and the world is your oyster :grey_exclamation:

![UART to USB sniffer in action](/Assets/Images/uart_to_usb_sniffer_in_action.jpg)
![Logic analyzer UART decoder](/Assets/Images/logic_analyzer_uart_64_kps.jpg)
![Web serial plotter wiper position](/Assets/Images/web_serial_plotter_wiper_position.jpg)

> [!NOTE]
> The measured baud rate is 1000000/(156.2/10) kbps which is 64 kbps. The division by 10 comes from an 8-bit payload plus one start bit and one stop bit.

> [!TIP]
> The half-duplex UART line between the drives implements a 12 V logic levels. Check your drawer for a ready-to-use (RTU) solution. You can use a LIN transceiver to sense the line (as in my example). A single-wire CAN transceiver should also do the job. [Optocouplers](https://www.dfrobot.com/product-2408.html) are another way to tackle that. What I need here is the listen only (LO) mode which makes things much simpler.

> [!IMPORTANT]
> It is highly recommended to use USB isolators when playing with automotive sensors and actuators, i.e. higher than 5V devices. That is why sniffers such as [CANable](https://makerbase3d.com/product/makerbase-canable-v2/) or [LUC (LIN)](https://ucandevices.github.io/ulc.html) implement the idea of galvanic isolation. I almost always place universal USB isolators between the notebook and all other devices if there is a risk of going something wrong on the "high" voltage side - breadboards are living organisms and everything is possible :slightly_smiling_face: My favorite HS USB isolator is [EVAL-CN0550-EBZ](https://wiki.analog.com/resources/eval/user-guides/circuits-from-the-lab/cn0550) from Analog Devices.

![EVAL-CN0550-EBZ USB isolator](/Assets/Images/analog_devices_hs_usb_isolator.jpg)

# Pinout

| Pin # | Left drive (master) | Right drive (slave) |
|:-----:|:-------------------:|:-------------------:|
| 1     | 12 V                | 12 V                |
| 2     | GND                 | GND                 |
| 3     | NC                  | NC                  |
| 4     | UART (64 kbps)      | UART (64 kbps)      |
| 5     | LIN bus (19200 bps) | NC                  |

# Missing files?
Don't worry :slightly_smiling_face: Just log in to MyST and hit Alt-K to generate /Drivers/CMCIS/ and /Drivers/STM32G4xx_HAL_Driver/ based on the .ioc file. After a couple of seconds your project will be ready for building.

# Exemplary free serial plotters (see #define USE_*** in main.c)
* [Arduino IDE Serial plotter](https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-serial-plotter/) (Arduino IDE)
* [Serial Port Plotter](https://github.com/CieNTi/serial_port_plotter) (CieNTi)
* [Web Serial Plotter](https://www.atomic14.com/2025/09/03/web-serial-plotter) (atomic14)
* [Better Serial Plotter](https://github.com/nathandunk/BetterSerialPlotter) (nathandunk)
* [Web Serial Plotter](https://github.com/sekigon-gonnoc/web-serial-plotter) (sekigon-gonnoc)

# Call to action
Create your own [home laboratory/workshop/garage](http://ufnalski.edu.pl/control_engineering_for_hobbyists/2025_dzien_popularyzacji_matematyki/Dzien_Popularyzacji_Matematyki_2025.pdf)! Get inspired by [ControllersTech](https://www.youtube.com/@ControllersTech), [DroneBot Workshop](https://www.youtube.com/@Dronebotworkshop), [Andreas Spiess](https://www.youtube.com/@AndreasSpiess), [GreatScott!](https://www.youtube.com/@greatscottlab), [bitluni's lab](https://www.youtube.com/@bitluni), [ElectroBOOM](https://www.youtube.com/@ElectroBOOM), [Phil's Lab](https://www.youtube.com/@PhilsLab), [atomic14](https://www.youtube.com/@atomic14), [That Project](https://www.youtube.com/@ThatProject), [Paul McWhorter](https://www.youtube.com/@paulmcwhorter), [Max Imagination](https://www.youtube.com/@MaxImagination), [Nikodem Bartnik](https://www.youtube.com/@nikodembartnik), [Stuff Made Here](https://www.youtube.com/@StuffMadeHere), [Mario's Ideas](https://www.youtube.com/@marios_ideas), [Aaed Musa](https://www.aaedmusa.com/), [Haase Industries](https://www.youtube.com/@h1tec), and many other professional hobbyists sharing their awesome projects and tutorials! Shout-out/kudos to all of them! Promote [README-driven learning](http://ufnalski.edu.pl/proceedings/sene2025/Ufnalski_PE_formatted_SENE_2025.pdf) :sunglasses:

> [!WARNING]
> Automotive actuators - do try them at home :grey_exclamation:

220+ challenges to start from: [Control Engineering for Hobbyists at the Warsaw University of Technology](http://ufnalski.edu.pl/control_engineering_for_hobbyists/Control_Engineering_for_Hobbyists_list_of_challenges.pdf).

Stay tuned!
