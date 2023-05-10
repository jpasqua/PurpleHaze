# Defining your Hardware

Some of the hardware components in your device are known to the WebThing/WebThingApp infrastructure and some are not. For example, if you're building a WebThingApp, then you have a display and WebThingApp needs to know about it. You may also have a temperature sensor that you want handled by one of the built-in WebThing classes. Alternatively, you may have motor controllers or other devices that are completely unknown to WebThing/WebThingApp.

Whatever the case may be, you need to configure and initialize all of your hardware as well as the ESP pins that talk to them. That is, you may need to set certain pins as INPUT or OUTPUT, configure the I2C interface, etc.

## Displays

Configuring your display is a multistep process and depends what sort of display you're using. At the moment (2021/10/21) WebThingApp supports two general classes of display:

* OLED:
	* A small 128*64 monochrome display with no input capability of its own.
	* Driven by the "ThingPulse OLED SSD1306" [library](https://github.com/ThingPulse/esp8266-oled-ssd1306).
	* Supports several variations of display and display driver, including NONE.
* Touch:
	* A 320x240 color display with touch input capability
	* Driven by the ["TFT_eSPI"](https://github.com/Bodmer/TFT_eSPI) library from Bodmer.
	* Supports many variations of display and display driver.

### Selecting a display

Selecting your display requires you to specify both the general class of display you're using and the specific display/driver.

**Choose the general display type**:

Inside the WebThingApp library, edit the file: `/src/gui/devices/DeviceSelect.h`. In it you will see these two lines:

````
	// #define DEVICE_TYPE DEVICE_TYPE_TOUCH
	#define DEVICE_TYPE DEVICE_TYPE_OLED
````
Uncomment the one associated with your display, and comment out the other. For example, to select the OLED display type you should have:

````
	// #define DEVICE_TYPE DEVICE_TYPE_TOUCH
	#define DEVICE_TYPE DEVICE_TYPE_OLED
````

**Choose the specific display/driver**:

For ***Touch*** displays:

Refer to the section: [Configuring the graphics library: TFT_eSPI](https://github.com/jpasqua/WebThingApp#configuring-the-graphics-library-tft_espi). It explains how to configure `User_Setup_Select.h` to select the specific display and driver type in use. This includes which pins are used to communicate with the display.

For ***OLED*** displays:

In your app you need to call `Display.setDeviceOptions` with a `DisplayDeviceOptions` object. It defines which driver to use, which pins are conencted to it, and its address.


## Buttons

*WebThing* provides a button manager to make it easier to integrate physical momentary buttons into your device - whether it has a touch display or an OLED display. You can do this in your app's `app_configureHW` function. For each physical button you have, call `WebThing::buttonMgr.addButton()` passing the pin to which the button is connected.

`ScreenMgr` has the notion of a screen sequence which is most relevant for devices with non-touch displays. You may designate an *advance* button and optionally a *previous* button which the user can press to move forward or backward through the screen sequence. These buttons are specified using `ScreenMgr.setSequenceButtons`.

The following snippet is taken from *CurrencyMon_OLED* example in *WebThingApp*. 

````
void CurrencyMonApp::app_configureHW() {
  // CUSTOM: Register any physical buttons that are connected
  for (int i = 0; i < hwConfig.nPhysicalButtons; i++) {
    uint8_t pin = hwConfig.physicalButtons[i];
    if (pin != Basics::UnusedPin) {
      WebThing::buttonMgr.addButton(pin);
    }
  }

  ScreenMgr.setSequenceButtons(hwConfig.advanceButton, hwConfig.previousButton);
}
````

Beyond the use as *advance* and *previous* buttons, buttons may be used by Screens to capture screen-specific user input. For example, the built-in RebootScreen employs two buttons as *confirm* and *cancel* buttons.

The *CurrencyMon_OLED* example overloads the *advance* and *previous* buttons for use by the RebootScreen:

````
  screens.rebootScreen->setButtons(hwConfig.advanceButton, hwConfig.previousButton);
````

For examples of how to use buttons within a screen implementation (for Touch or OLED), refer to:

````
CurrencyMon_Touch/src/screens/HomeScreen.cpp
WebThingApp/src/screens/oled/RebootScreen.cpp
````

In both cases you will find that the general process is to:

1. Create and register one or more `ButtonMapping` objects. For each button you wish to respond to, the mapping relates the physical pin (known by WebThing::buttonMgr) to an app-specific ID. For example, it might map pin D3 to `Cancel`.
2. Within the screen's button handler, compare the button id paramter to each of the IDs you defined (e.g. `Cancel`) and take the appropriate action.

In Touch devices you may have virtual button presses (taps on the display) mixed with physical button presses.

## Sensors

Your device may have sensors of various kinds attached. Neither WebThing nor WebThing app needs to know anything about them. However, there are a small number of sensors that WebThing supplies for use by apps:

* **Weather**
	* Sensors
		* BME280: Temperature, Humidity, Barometric Pressure
		* DHT22: Temperature, Humidity
		* DS18B20: Temperature
	* Infrastructure
		* There are a number of headers and implementations that support the use of these sensors individually or in groups. For example, you may pair a DS18B20 for accurate temperature readings with a BME280 for the other readings.
		* WeatherMgr coordinates the use of one or more weather sensors and provides a history of readings based on `BPAUtils/HistoryBuffers`. 
* **Air Quality Sensors**
	* Sensors
		* PMS5003: Particulate readings
		* Other sensors, like CO2, may be added in the future.
	* Infrastructure:
		* AQIMgr coordinates the operation of the underlying air quality sensor(s) and provides a history of readings based on `BPAUtils/HistoryBuffers`.

Whether you use built-in sensors or your own, WebThingApp provides a couple of related functions:

1. Your app's `app_configureHW()` function will be called during initialization. This provides a good spot to initialize your sensors.
2. Your app's `app_conditionalUpdate()` function will be called frequently. This provides an opporunity to read data from your sensors. Of course you may want read new data at a much slower pace (e.g. every 5 minutes). It is up to you to only read new data when it is appropriate. `WeatherMgr` has that functionality built into its `takeReadings()` function.
3. Your app's `app_loop()` function is called each time around the arduino main loop. If your sensor needs periodic care and feeding, you can do it from here.