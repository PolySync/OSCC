<img src="https://github.com/PolySync/OSCC/blob/master/assets/oscc_logo_title.png">


The Open Source Car Control Project is a hardware and software project detailing the conversion of a late model vehicle into an autonomous driving research and development vehicle.

See the [Wiki](https://github.com/PolySync/OSCC/wiki) for full documentation, details, and other information.


Repository Contents
-------------------
* **/3d_models** - Technical drawings and 3D files for board enclosures and other useful parts
* **/assets** - Diagrams and images used in the wiki
* **/boards** - PCB schematics and board designs for control modules
* **/control** - Utilities for controlling a vehicle
* **/firmware** - Arduino code for the various modules
* **/vehicle_info** - Information on specific vehicles, such as sensor outputs and wiring diagrams


Boards
-----

Once we finish testing and validating the board designs, we will be releasing the schematics and design files along with the board test plans and testing firmware. Check back soon for complete designs.
Once we have validated the boards, we will be shipping the boards as a kit.
Thanks to [Trey German](https://www.PolymorphicLabs.com) and [Macrofab](https://macrofab.com/) for help desiging the boards and getting them made.


Click [Here](http://oscc.io) to order your autonomy kit now.

Building and Installing Arduino Firmware
------------

The OSCC Project is built around a number of individual modules that interoperate to create a fully controllable vehicle. These modules are built from Arduinos and Arduino shields designed specifically for interfacing with various vehicle components. Once these modules have been programmed with the accompanying firmware and installed into the vehicle, the vehicle is ready to receive control commands sent over a CAN bus from a computer running a control program. 

**Pre-requisites:** You must have Arduino Core pre-installed on your machine.

`sudo apt-get install arduino-core`

OSCC uses Arduino makefiles to avoid some of the limitations of the Arduino IDE. Using this method you can build, upload, and monitor the code.
Check out [Arduino-Makefile](https://github.com/sudar/Arduino-Makefile) for more information.

**Building Steering Firmware**

Navigate to the directory for the steering firmware.

`cd $OSCC-PROJECT-DIR/firmware/steering/kia_soul_ps/`

Once you are in the home directory of the steering code, build it using the command below.

`make`

Once the firmware is successfully built, you can upload it to the corresponding Arduino module. Connect to the Arduino with a USB cable and then run the command. Sometimes it takes a little while for the Arduino to initialize once connected, so if there is an error thrown initially, wait a little bit and then retry the command. 

`make upload`

**Building Throttle Firmware**

Navigate to the directory for the throttle firmware.

`cd $OSCC-PROJECT-DIR/firmware/throttle/kia_soul_ps/`

Once you are in the home directory of the throttle code, build it using the command below.

`make`

Once the firmware is successfully built you can upload it to the corresponding Arduino module. Connect to the Arduino with a USB cable and then run the command below. Sometimes it takes a little while for the Arduino to initialize once connected, so if there is an error thrown initially, try waiting a little bit and then retrying the command. 

`make upload`

**Building Brake Firmware**

Navigate to the directory for the throttle firmware.

`cd $OSCC-PROJECT-DIR/firmware/brake/kia_soul_ps/`

Once you are in the home directory of the brake code, build it using the command below. 

`make`

Once the firmware is successfully built you can upload it to the corresponding Arduino module. Connect to the Arduino with a USB cable and then run the command below. Sometimes it takes a little while for the Arduino to initialize once connected, so if there is an error thrown initially, try waiting a little bit and then retrying the command. 

`make upload`

**Building CAN gateway Firmware**

Navigate to the directory for the CAN gateway firmware.

`cd $OSCC-PROJECT-DIR/firmware/can_gateway/kia_soul_ps/`

Once you are in the home directory of the CAN gateway code, build it using the command below.

`make`

Once it is successfully built, you can upload it to the corresponding Arduino module. Connect to the Arduino with a USB cable and then run the command below. Sometimes it takes a little while for the Arduino to initialize once connected, so if there is an error thrown initially, try waiting a little bit and then retrying the command. 

`make upload`

**Monitoring Arduino modules**

It is sometimes useful to monitor individual Arduino modules to check for proper operation and to debug. To do this, simply run the command below when connected via USB.

`make monitor`

The GNU utility `screen` is used by default to communicate with the Arduino via serial over USB. It can be used to both receive the output of any 'Serial.print' statements in your Arduino code, and to push commands over serial to the Arduino. If you don't already have it installed, you can get it with the command below. In order to exit screen use `C-a \`.

`sudo apt-get install screen`

To do more in-depth debugging you can use any of a number of serial monitoring applications. Processing can be used quite effectively to provide output plots of data incoming across a serial connection.

Controlling Your Vehicle
------------

Now that all your Arduino modules are properly setup, it is time to start sending control commands. There is an example application to do this that uses the Logitech F310 Gamepad. The example interfaces to the joystick gamepad via the SDL2 joystick library and sends CAN commands over the control CAN bus via the Kvaser CANlib SDK. These CAN control commands are interpreted by the respective Arduino modules and used to actuate the vehicle.

**Pre-requisites:** A Logitech F310 gamepad is required, and the SDL2 library and CANlib SDK need to be pre-installed. 
A CAN interface adapter, such as the [Kvaser Leaflight](https://www.kvaser.com), is also required.

[logitech-F310](http://a.co/3GoUlkN)

Install the SDL2 library with the command below.

`sudo apt-get install libsdl2-dev`

Install the CANlib SDK via the following procedure.

[CANlib-SDK](https://www.kvaser.com/linux-drivers-and-sdk/)

**Building Joystick Commander Code**

Navigate to the directory for the joystick commander code.

`cd $OSCC-PROJECT-DIR/control/joystick_commander`

Once you are in the home directory of the joystick commander, build the code with the included Makefile.

`make`

Now that the joystick commander is built it is ready to be run. However, we need to determine what CAN interface the control CAN bus is connected to on your computer. This interface will be passed to the joystick commander as an argument, and will be used to allow the joystick commander to communicate with the CAN bus. To figure out what CAN interface your control CAN bus is connected to, navigate to the examples directory in the CANlib install.

`cd /usr/src/linuxcan/canlib/`

Run make to ensure all the CANlib examples are built. 

`make`

Then navigate to the examples directory of the CANlib install.

`cd /usr/src/linuxcan/canlib/examples/`

You can use the "listChannels" and "canmonitor" examples to determine which CAN channel your control bus is connected to. CAN monitor will dump any data on a selected channel and list channels will tell you what channels are avaliable. You can use both to determine which channel you will need to use. Once you know the correct chanel number, you can run the joystick example with the command below.

`./joystick-commander <channel-number>`

**Controlling the Vehicle with the Joystick Gamepad**

Once the joystick commander is up and running you can use it to send commands to the Arduino modules. The controls are listed when the programs start up. Be sure the switch on the back of the controller is 
switched to the 'X' position, not 'D'. The vehicle will only response to commands if control is enabled with the start button. The back button disables control.


Additional Vehicles & Contributing
------------

OSCC currently has information regarding the Kia Soul PS (2014-2016), but we want to grow! The repository is structured to facilitate including more vehicles as more is learned about them.


Please see [CONTRIBUTING.md](CONTRIBUTING.md).

License Information
-------------------

Hardware source materials (e.g. schematics, board layouts, wiring diagrams, data sheets, physical installation documentation, 3D models, etc.) for the OSCC (Open Source Car Control) Project are licensed under Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0).

Firmware source for the OSCC (Open Source Car Control) Project is licensed under GNU General Public License (GPLv3) unless otherwise noted (e.g. 3rd party or permissive dependencies, etc.).

Software source for the OSCC (Open Source Car Control) Project is licensed under the MIT License (MIT) unless otherwise noted (e.g. 3rd party dependencies, etc.).

Please see [LICENSE.md](LICENSE.md) for more details.

Contact Information
-------------------

Please direct questions regarding OSCC and/or licensing to help@polysync.io.

*Distributed as-is; no warranty is given.*

Copyright (c) 2016 PolySync Technologies, Inc.  All Rights Reserved.
