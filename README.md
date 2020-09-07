# AQM - Air Quality Monitor

This is a simple Air Quality Monitor based on the [Plantower PMS5003](https://www.adafruit.com/product/3686). You build some electronics, 3D-print a case, load this software, configure an app on your phone, and you're all set. What could be simpler? This software relies on the [Blynk](https://blynk.io) service to collect data in the cloud and make it available to the app on your phone. In the future, other services may be supported.

The weather station is configured using a Web User Interface where the user will specify information such as:

* The location of the air quality monitor
* API keys for the underlying services (like Blynk)
* etc.

Some of this configuration information is specific to AQM while other information is common to any web-connected device. You can learn more about such `WebThing`'s [here](https://github.com/jpasqua/WebThing).

## Dependencies

### Libraries
The following third party libraries are used within this project:

* [Arduino-Log](https://github.com/thijse/Arduino-Log)
* [ArduinoJson (v6)](https://github.com/bblanchon/ArduinoJson)
* [BlynkSimpleEsp8266](https://github.com/blynkkk/blynk-library)
* [ESPTemplateProcessor](https://github.com/jpasqua/ESPTemplateProcessor)
* [TimeLib](https://github.com/PaulStoffregen/Time.git)
* [WebThing](https://github.com/jpasqua/WebThing)

### Services
The following services play a role in providing parts of the functionality:

 - [Blynk](https://blynk.io): Provides a repository of weather data and the ability to view using a mobile app. Without this you will only be able to see current values via the we interface.
 - Services used by WebThing
	 - [Google Maps](https://developers.google.com/maps/documentation): Used for geocoding and reverse geocoding. Though not absolutely necessary, it does make using the system a bit more convenient.
	 - [TimeZoneDB](https://timezonedb.com): Used to get local time and time zone data. This is used to timestamp data.

### Hardware
The primary components of this project are a [Plantower PMS5003](https://www.adafruit.com/product/3686) sensor and an ESP8266 such as a Wemos D1 Mini (but virtually any ESP8266 will work). You can optionally add a few WS2812 LEDs (commonly known as NeoPixels) for status readouts if you'd like.

[To Do: Discuss PCB]

### 3D Model
You can find a housing for this sensor here: [TO DO: Create a housing]

 - [Original](https://www.thingiverse.com/thing:XYZ)

## First Use

Once you have assembled your hardware and loaded the firmware, `AQM` will boot and create its own WiFi access point. Connect to the new access point you will see on your phone or computer's wifi connection list. It will have an SSID of the form `aqmnnnnnn`. Once you do, you will enter a "captive portal" where you can configure the device for your actual WiFi network. When you've done that, you can power-cycle the unit and it will connect to your wifi network.

At this point it will make a web interface available. You can connect using either an IP address or a hostname:

- **IP Access**: You need to determine the IP Address used by the device. You have a couple of choices here:
    - If you are still connected via USB, you will see the IP address in the Arduino Serial Monitor
    - You can use a scanning tool or your router to look for the device on your network

## Settings

Once connected, you can use the web interface to view readings or change settings. There are a number of base settings that are common to all [WebThing](https://github.com/jpasqua/WebThing)'s.
The available settings are:

- Description: A description that is useful to the user. For example, the location of the weather station
- Blynk API Key: An API key established by the user when configuring the Blynk app.


## Operation

**AQM** lets you view your air quality data using the Blynk App for [iPhone](https://itunes.apple.com/us/app/blynk-control-arduino-raspberry/id808760481?ls=1&mt=8) or [Android](https://play.google.com/store/apps/details?id=cc.blynk). Follow these steps to get going:

1. Download the appropriate app using the links above.
2. Open the Blynk app and create a new Blynk account if you don't have one already.
3. Touch the QR code icon at the top of the screen.
4. Scan the QR code below. This will load the Solar Weather Station app which you may customize if you wish using the Blynk app (no coding required).
5. Blynk will send you an "Auth Token" via email. Enter this in the settings screen of the web interface in the field labeled "Blynk API Key". Hit save and restart the weather station.
6. Use the Blynk app to see current and historical values.

[To Do: provide correct QR code]
Blynk QR Code:<br><img src="images/BlynkQRCode.jpg" width="250" /></img><br>




