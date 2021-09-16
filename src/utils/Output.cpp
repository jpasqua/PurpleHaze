#include <WebThing.h>
#include "Output.h"

namespace Output {
  // ----- Units
  static bool MetricDefault = false;
  static bool* useMetric = &MetricDefault;
  static const char* MetricTempUnits = "°C";
  static const char* ImperialTempUnits = "°F";
  static const char* MetricBaroUnits = "hPa";
  static const char* ImperialBaroUnits = "inHg";

  void setUnits(bool* metric) { useMetric = metric; }

  float temp(float temp) { return (useMetric) ? temp : WTBasics::c_to_f(temp); }
  float tempSpread(float spread) { return (useMetric) ? spread : WTBasics::delta_c_to_f(spread); }
  float baro(float baro) { return (useMetric) ? baro : WTBasics::hpa_to_inhg(baro); }
  const char* tempUnits() { return (useMetric) ? MetricTempUnits : ImperialTempUnits; }
  const char* baroUnits() { return (useMetric) ? MetricBaroUnits : ImperialBaroUnits; }


  // ----- Time
  // TO DO: We should use the proper value of use24Hour, not default to not use it!
  String dateTime(time_t theTime) { return WebThing::formattedTime(theTime, false, true); }
};
