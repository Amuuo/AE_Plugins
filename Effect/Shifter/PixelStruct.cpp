
#include"PixelStruct.h"


PixelStruct::PixelStruct() {
}

PixelStruct::PixelStruct(PF_Pixel pix) {
  pixel = pix;
}

void PixelStruct::operator()(PF_Pixel _pixel) {
  pixel = _pixel;
}
