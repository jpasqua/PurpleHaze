/*
 * MovingAverage
 *    A very simple class for tracking moving averages.
 *
 */

#ifndef MovingAverage_h
#define MovingAverage_h


class MovingAverage {
public:
  MovingAverage() {
    _samplesToConsider = UINT16_MAX;
    _count = 0;
    _avg = 0.0;
    _sum = 0.0;
  }

  void setSamplesToConsider(uint16_t samplesToConsider) {
    _samplesToConsider = samplesToConsider;
  }

  float addSample(float sample) {
    if (_count == _samplesToConsider) { _sum -= _avg; }
    else { _count++; }
    _sum += sample;
    return (_avg = _sum/_count);
  }

  float getAverage() { return _avg; }

private:
  uint16_t _samplesToConsider;
  uint16_t _count;
  float _avg;
  float _sum;
};

#endif  // MovingAverage_h
