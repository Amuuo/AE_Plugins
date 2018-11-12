
#include"PixelStruct.h"


PixelStruct::PixelStruct() {
}

PixelStruct::PixelStruct(PF_Pixel pix) {
  pixel = pix;
}

void PixelStruct::set(PF_Pixel _pixel) {
  pixel = _pixel;
}

PF_Boolean PixelStruct::operator<(const PF_Fixed & val) {
  return value < val;
}


/*
PF_Boolean PixelStruct::operator<(const PixelStruct & pix) {
  return value < pix.value;
}*/

PF_Boolean PixelStruct::operator>(const PF_Fixed & val) {
  return value > val;
}


/*
PF_Boolean PixelStruct::operator>(const PixelStruct & pix) {
  return value > pix.value;
}*/

PF_Boolean PixelStruct::operator==(const PF_Fixed & val) {
  return value == val;
}


/*
PF_Boolean PixelStruct::operator==(const PixelStruct & pix) {
  return value == pix.value;
}*/

PF_Boolean PixelStruct::operator<=(const PF_Fixed & val) {
  return value <= val;
}

/*
PF_Boolean PixelStruct::operator<=(const PixelStruct & pix) {
  return value <= pix.value;
}*/

PF_Boolean PixelStruct::operator>=(const PF_Fixed & val) {
  return value >= val;
}

PF_Boolean PixelStruct::operator!=(const PF_Fixed & val) {
  return value != val;
}

/*
PF_Boolean PixelStruct::operator>=(const PixelStruct & pix) {
  return value >= pix.value;
}*/

