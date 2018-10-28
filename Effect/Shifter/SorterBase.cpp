

#include "SorterBase.h"


SorterBase::SorterBase() {}

SorterBase::~SorterBase() {
  in_data.pica_basicP->ReleaseSuite(kPFColorCallbacksSuite, kPFColorCallbacksSuiteVersion1);
}


void SortSegment::reset() {
  highValue = PixelStruct{};
  lowValue = PixelStruct{};
  segmentLength = 0;
  isEmpty = true;
  columnAvg = 0;
  borderIters.clear();
}



void SorterBase::setupParams() {

  pixelLines = in_data.width;
  linePixels = in_data.height;

  pixelMap = vector<vector<PixelStruct>>(pixelLines, vector<PixelStruct>(linePixels, PixelStruct{}));

  in_data.pica_basicP->AcquireSuite(kPFColorCallbacksSuite,
                                    kPFColorCallbacksSuiteVersion1,
                                    &colorSuite);

}