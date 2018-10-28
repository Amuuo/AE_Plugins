

#include "SorterBase.h"


SorterBase::SorterBase() {}


void SortSegment::reset() {
  highValue = PixelStruct{};
  lowValue = PixelStruct{};
  segmentLength = 0;
  isEmpty = true;
  columnAvg = 0;  
}



void SorterBase::setupParams() {

  pixelLines = in_data-> width;
  linePixels = in_data->height;

  pixelMap = vector<vector<PixelStruct>>(pixelLines, vector<PixelStruct>(linePixels, PixelStruct{}));

  in_data->pica_basicP->AcquireSuite(kPFColorCallbacksSuite,
                                    kPFColorCallbacksSuiteVersion1,
                                    &colorSuite);
  setIterWidth();
}

void SorterBase::setIterWidth() {
  borderIters =
    vector<pair<vector<PixelStruct>::iterator, vector<PixelStruct>::iterator>>(
      params[SORT_WIDTH_SLIDER].u.fd.value, pair<vector<PixelStruct>::iterator,
      vector<PixelStruct>::iterator>{});
}
