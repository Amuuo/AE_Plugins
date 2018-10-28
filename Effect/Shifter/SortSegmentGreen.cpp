
#include "SortSegmentGreen.h"


void SortSegmentGreen::getRGBInterpolatedVectors() {

  const PF_Fixed green_range = highValue.pixel.green - highValue.pixel.green;
  const PF_Fixed green_interpolation_slope = (PF_Fixed)((PF_FpLong)green_range / segmentLength);

  PF_Fixed green_start = lowValue.pixel.green;

  for (auto x = borderIters.begin(); x != borderIters.end(); ++x) {

    x->first->pixel.green = green_start;

    green_start = (green_start + green_interpolation_slope <= 255) ?
      (green_start += green_interpolation_slope) : green_start;
  }
}