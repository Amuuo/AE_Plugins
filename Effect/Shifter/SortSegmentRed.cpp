#include "SortSegmentRed.h"


void SortSegmentRed::getRGBInterpolatedVectors() {

  const PF_Fixed red_range = highValue.pixel.red - highValue.pixel.red;
  const PF_Fixed red_interpolation_slope = (PF_Fixed)((PF_FpLong)red_range / segmentLength);

  PF_Fixed red_start = lowValue.pixel.red;

  for (auto x = borderIters.begin(); x != borderIters.end(); ++x) {

    x->first->pixel.red = red_start;

    red_start = (red_start + red_interpolation_slope <= 255) ?
      (red_start += red_interpolation_slope) : red_start;
  }
}