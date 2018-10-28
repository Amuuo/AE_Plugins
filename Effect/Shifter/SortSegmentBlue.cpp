
#include "SortSegmentBlue.h"

void SortSegmentBlue::getRGBInterpolatedVectors() {

  const PF_Fixed blue_range = highValue.pixel.blue - highValue.pixel.blue;
  const PF_Fixed blue_interpolation_slope = (PF_Fixed)((PF_FpLong)blue_range / segmentLength);

  PF_Fixed blue_start = lowValue.pixel.blue;

  for (auto x = borderIters.begin(); x != borderIters.end(); ++x) {

    x->first->pixel.blue = blue_start;

    blue_start = (blue_start + blue_interpolation_slope <= 255) ?
      (blue_start += blue_interpolation_slope) : blue_start;
  }

}