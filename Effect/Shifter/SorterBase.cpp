#include "SorterBase.h"



void SortSegment::reset() {
  highValue = PixelStruct{};
  lowValue = PixelStruct{};
  segmentLength = 0;
  isEmpty = true;
  columnAvg = 0;  
}



void SorterBase::init(PF_InData * in)
{
  using iterPair = pair<vector<PixelStruct>::iterator, vector<PixelStruct>::iterator>;
  using pixMatrix = vector<vector<PixelStruct>>;

  in_data = in;

  pixelLines = in->width;
  linePixels = in->height;

  //pixelMap.clear();
  pixelMap = pixMatrix(pixelLines,vector<PixelStruct>(linePixels, PixelStruct()));

  borderIters.resize(params[SORT_WIDTH_SLIDER].u.fd.value);
  
  //borderIters = vector<iterPair>(static_cast<size_t>(
  //  params[SORT_WIDTH_SLIDER].u.fd.value), iterPair{});
  
  

  //in_data.pica_basicP->AcquireSuite(kPFColorCallbacksSuite,
  //                                   kPFColorCallbacksSuiteVersion1, &colorSuite);    
}


