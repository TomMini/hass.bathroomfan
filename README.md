# The smart bathroom fan #
### But why...? ###
My house has a loft-like attic where the walls are only high enough so you can’t see over them. This includes my bed- and bathroom, and in my bathroom I have a ceiling fan with humidity sensor to turn it on and off. During the fall this fan has a mind of it’s own and starts sucking air out in the middle of the night, keeping me awake. After one year of getting a ladder in the middle of the night to re-adjust the analog humidity sensor, I thought this could be done more easily.

### The solution... ###
My house already has some smart lights and appliances, all hooked up with Home-Assistant. My idea for the fan is to integrate it with Home-Assistant, making it remotely controllable. For this I order a bunch of stuff using the well-known Chinese online marketplace. The shopping cart looks like this:
*	Wemos D1 Mini with ESP8266 controller and Wifi
*	Wemos Mini Relay board
*	Wemos Mini DHT22 board with humidity and temperature sensor

Further I used some stuff laying around the house:
* The existing bathroom-fan
*	An old USB phone charger

### Hooking it all up... ###
The Wemos parts are wired up with the following schematics. De DHT22 sensor has some extra parts on the Wemos PCB that are not shown here but I think it’s nothing more than a pull-up resistor resistor on the data-line. The strange thing in my setup is that my DHT22 sensor is made by ASAIR and for some reason doesn't play nice on the D4-pin it's default wired to. That’s why in my setup it’s connected to D3 and working like a charm.
![Wiring](https://github.com/TomMini/hass.bathroomfan/blob/master/assets/Wiring.png)
The fan itself is stripped from it’s analogue humidity sensor and wired through the NO (normal open) switch of the relay. The old phone charger is hooked up to the mains (make sure it’s properly insulated!) and connected with the micro-USB connector of the Wemos D1 as a power supply.

### The code ###
The Arduino sourcecode can be found [here](https://github.com/TomMini/hass.bathroomfan/blob/master/src/hass.bathroomfan.ino)
