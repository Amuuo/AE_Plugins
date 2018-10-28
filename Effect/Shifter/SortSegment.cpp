
#include"SortSegment.h"



SortSegment::SortSegment() {
  
}


void SortSegment::getRGBInterpolatedVectors() {

  const PF_Fixed red_range = highValue.pixel.red - lowValue.pixel.red;
  const PF_Fixed red_interpolation_slope = (PF_Fixed)((PF_FpLong)red_range / segmentLength);

  const PF_Fixed green_range = highValue.pixel.green - lowValue.pixel.green;
  const PF_Fixed green_interpolation_slope = (PF_Fixed)((PF_FpLong)green_range / segmentLength);

  const PF_Fixed blue_range = highValue.pixel.blue - lowValue.pixel.blue;
  const PF_Fixed blue_interpolation_slope = (PF_Fixed)((PF_FpLong)blue_range / segmentLength);


  PF_Fixed red_start = lowValue.pixel.red;
  PF_Fixed green_start = lowValue.pixel.green;
  PF_Fixed blue_start = lowValue.pixel.blue;

  for (auto x = borderIters.begin(); x != borderIters.end(); ++x) {

    x->first->pixel.red = red_start;
    x->first->pixel.green = green_start;
    x->first->pixel.blue = blue_start;


    red_start = (red_start + red_interpolation_slope <= 255) ?
      (red_start += red_interpolation_slope) : red_start;

    green_start = (green_start + green_interpolation_slope <= 255) ?
      (green_start += green_interpolation_slope) : green_start;

    blue_start = (blue_start + blue_interpolation_slope <= 255) ?
      (blue_start += blue_interpolation_slope) : blue_start;
  }
}