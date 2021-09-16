/*
 * BlynkMgr
 *    Manage the connection to Blynk and writers who wish to publish data
 *
 */

#ifndef BlynkMgr_h
#define BlynkMgr_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------

using BlynkPinRange = struct {
  uint8_t firstPin;
  uint8_t lastPin;
};

class BlynkPublisher {
public:
  virtual bool publish() = 0;
  virtual BlynkPinRange getPinRange() = 0;
};

namespace BlynkMgr {
  // Before anything can be written to Blynk, a successful call to init
  // must be made. This requires a valid Blynk application key 
  extern void init(String& blynkKey);

  // Register a publisher which owns a range of pins and is responsible
  // for publishing data to blynk via the PublishCallback.
  // Returns:
  // true  -> The publisher was registered
  // false -> No space for more publishers
  extern bool registerPublisher(BlynkPublisher* p);

  // Ask all registered publishers to send their data to Blynk
  // They may or may not publish anything depending on whether they
  // have any updated information to report.
  // Returns:
  // true  -> All publishers were given an opportunity to write their data
  // false -> We don't have a connection to Blynk
  extern bool publish();

  // Must be called periodically to give the Blynk subsystem a chance
  // to run. Typically called from the sketch's loop() function
  extern void loop();

  // Must be called to flush any pending data to the Blynk service. This might
  // be called before a sesnor device goes to sleep.
  extern void disconnect();

  extern void writeUnsigned(int pin, uint32_t value);
  extern void writeSigned(int pin, int32_t value);
  extern void writeFloat(int pin, float value);
  extern void writeString(int pin, String value);
  extern void writeBool(int pin, bool value);
}

#endif // BlynkMgr_h
