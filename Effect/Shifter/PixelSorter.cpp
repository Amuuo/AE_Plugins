#include "PixelSorter.h"



PixelSorter::PixelSorter() {
}






void PixelSorter::sortPixelMap() {


  for (lineCounter = 0; lineCounter < pixelLines;
       lineCounter += params[SORT_WIDTH_SLIDER].u.fd.value) {

    resetSortingVariables();
    storeBeginRowIters();

    for (pixelCounter = 0; pixelCounter < linePixels; ++pixelCounter, ++currPixDistance) {

      if (isEmpty) {
        resetSortingVariables();
        storeBeginRowIters();
        getAndStorePixelValue();
        isEmpty = false;
      }
      if (pixelDistanceIsLongEnoughToSort()) {
        segmentLength = currPixDistance;
        storeEndRowIters();
        sortPixelSegments();
        resetSortingVariables();
        continue;
      }
      else {
        getAndStorePixelValue();
      }
    }
  }
  resetSortingVariables();
}







void PixelSorter::storeBeginRowIters() {

  for (int j = lineCounter, i=0; 
       j < (lineCounter + params[SORT_WIDTH_SLIDER].u.fd.value) &&
       (j < (pixelLines - 1)); ++j, ++i) {
    
    
    borderIters[i].first = pixelMap[j].begin() + pixelCounter; 
  
  }
}






void PixelSorter::storeEndRowIters() {
  
  for (int j = lineCounter, i=0;
       j < (lineCounter + params[SORT_WIDTH_SLIDER].u.fd.value) &&
       (j < (pixelLines - 1)); ++j, ++i) {
    
    borderIters[i].second = pixelMap[j].begin() + pixelCounter;
  }
}





void PixelSorter::getSortLength() {

  sortLength = params[SORT_VALUE_RANGE].u.fd.value;

  if (params[VARIABLE_SORT_CHECKBOX].u.bd.value) {
    if (params[FAVOR_DARK_RANGES].u.bd.value) {
      sortLength = (PF_Fixed)(((PF_FpLong)sortLength / 4) *
        pow((abs(lowValue.value - MAX_RBG_VALUE) / MAX_RBG_VALUE + 1),
            2 + (params[VARIABLE_SLIDER].u.fs_d.value - 1)));
    }
    else {
      sortLength = (PF_Fixed)(((PF_FpLong)sortLength / 4) *
        pow((highValue.value / MAX_RBG_VALUE + 1),
            2 + (params[VARIABLE_SLIDER].u.fs_d.value - 1)));
    }
  }
}






void PixelSorter::getLineWidthPixelAverage() {
  
  columnAvg = 0;

  auto columnWidthSpan = lineCounter < pixelLines ? 
    lineCounter + params[SORT_WIDTH_SLIDER].u.fd.value : 0;


  for (auto i = lineCounter; i < columnWidthSpan && i < pixelLines - 1; ++i) {
    columnAvg += pixelMap[i][pixelCounter].value;
  }
  columnAvg /= params[SORT_WIDTH_SLIDER].u.fd.value;

  if (currPixDistance != 0) { startingRGBValue = columnAvg; }

}










void PixelSorter::resetSortingVariables() {

  reset();
  startingRGBValue = 0;
  currPixDistance = 0;
  pixValueAverage = 0;
  columnAvg = 0;
  lengthIsShortEnoughForFlip = false;
  minLength = 0;
  currentPixelValueDistance = 0;
  highValue = PF_Pixel{};
  lowValue = PF_Pixel{};

}






void PixelSorter::reverseSortIfTrue(PF_Boolean needToReverse, PF_Fixed index) {
  
  if (needToReverse) {
    reverse(borderIters[index].first, borderIters[index].second);
  }
}





bool PixelSorter::pixelDistanceIsLongEnoughToSort() {

  getSortLength();
  getUserSetMinLength();

  switch (params[SORT_METHOD_DROPDOWN].u.pd.value) {
    case MANUAL_SORT:

      if (highValue.value >= params[HIGH_RANGE_SORT_LIMIT].u.fs_d.value &&
          lowValue.value <= params[LOW_RANGE_SORT_LIMIT].u.fs_d.value) {
        if (currentPixelValueDistance >= sortLength || pixelCounter == linePixels - 1) {
          return true;
        }
      }
      break;

    case BASIC_SORT:

      if (((currentPixelValueDistance >= sortLength) &&
        (currPixDistance > params[MIN_SORT_LENGTH_SLIDER].u.fd.value)) ||
           (pixelCounter == linePixels - 1)) {
        lengthIsShortEnoughForFlip =
          currPixDistance < params[MIN_REVERSE_DIST_SLIDER].u.fs_d.value ? true : false;
        return true;
      }
      break;

    default: break;

  }
  return false;
}







void PixelSorter::sortPixelSegments() {

  for (auto h = 0; h < borderIters.size(); ++h) {

    sort(borderIters[h].first,
         borderIters[h].second, [](const PixelStruct& pix1, const PixelStruct& pix2) {
                                      return pix1.value < pix2.value ? true : false; });
    reverseSortIfTrue(params[REVERSE_SORT_CHECKBOX].u.bd.value, h);
  }

}






void PixelSorter::getAndStorePixelValue() {

  
  getLineWidthPixelAverage();

  if (highValue.value < columnAvg) {
    highValue.value = columnAvg;
  }
  else if (lowValue.value > columnAvg || lowValue.value == 0) {
    lowValue.value = columnAvg;
  }  
  
  currentPixelValueDistance = highValue.value - lowValue.value;
}






void PixelSorter::getUserSetMinLength() {
  
  minLength = params[MIN_SORT_LENGTH_SLIDER].u.fd.value;
  
  if (params[MIN_SORT_RAND_SLIDER].u.fs_d.value >= 2) {
    minLength += params[MIN_SORT_LENGTH_SLIDER].u.fd.value;

    minLength +=
      ((random() % (params[MIN_SORT_RAND_SLIDER].u.fd.value / 2)) - 
      params[MIN_SORT_RAND_SLIDER].u.fd.value);

    minLength *= ((MAX_RBG_VALUE - columnAvg) / MAX_RBG_VALUE + 1);
  }
}












