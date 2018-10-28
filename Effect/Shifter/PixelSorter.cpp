#include "PixelSorter.h"



PixelSorter::PixelSorter() {}



void PixelSorter::storeBeginRowIters() {

  for (int j = lineCounter, i = 0; 
       j < (lineCounter + params[SORT_WIDTH_SLIDER].u.fd.value) &&
       (j < (pixelLines - 1)); ++j, ++i) {
    
    borderIters.push_back(
      make_pair(pixelMap[j].begin() + currPixDistance, vector<PixelStruct>::iterator{}));
  }
}






void PixelSorter::storeEndRowIters() {
  
  for (int j = lineCounter, i=0;
       j < (lineCounter + params[SORT_WIDTH_SLIDER].u.fd.value) &&
       (j < (pixelLines - 1)); ++j, ++i) {
    
    borderIters[i].second = pixelMap[j].begin() + currPixDistance;
  }
}





void PixelSorter::getSortLength() {

  sortLength = (PF_Fixed)params[SORT_VALUE_RANGE].u.fs_d.value;

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

  auto columnWidthSpan = lineCounter + params[SORT_WIDTH_SLIDER].u.fd.value;


  for (auto i = lineCounter; i < columnWidthSpan && i < pixelLines - 1; ++i) {
    columnAvg += pixelMap[i][pixelCounter].value;
  }
  columnAvg /= params[SORT_WIDTH_SLIDER].u.fd.value;

  if (currPixDistance != 0) { startingRGBValue = columnAvg; }

}




void PixelSorter::getLineWidthColorAverage() {
  
  auto columnWidthSpan = lineCounter + params[SORT_WIDTH_SLIDER].u.fd.value;

  PF_Fixed r_avg{ 0 };
  PF_Fixed g_avg{ 0 };
  PF_Fixed b_avg{ 0 };



  for (auto i = lineCounter; i < columnWidthSpan && i < pixelLines - 1; ++i) {
    r_avg += pixelMap[i][pixelCounter].pixel.red;
    g_avg += pixelMap[i][pixelCounter].pixel.green;
    b_avg += pixelMap[i][pixelCounter].pixel.blue;
  }
  r_avg /= params[SORT_WIDTH_SLIDER].u.fd.value;
  g_avg /= params[SORT_WIDTH_SLIDER].u.fd.value;
  b_avg /= params[SORT_WIDTH_SLIDER].u.fd.value;




  if (r_avg > highValue.pixel.red)   highValue.pixel.red = r_avg;
  if (g_avg > highValue.pixel.green) highValue.pixel.green = g_avg;
  if (b_avg > highValue.pixel.blue)  highValue.pixel.blue = b_avg;


  if (r_avg < lowValue.pixel.red || lowValue.pixel.red == 0)
    lowValue.pixel.red = r_avg;

  if (g_avg < lowValue.pixel.green || lowValue.pixel.green == 0)
    lowValue.pixel.green = g_avg;

  if (b_avg < lowValue.pixel.blue || lowValue.pixel.blue == 0)
    lowValue.pixel.blue = b_avg;


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
    case USER_MANUAL_SORT:

      if (highValue.value >= params[HIGH_RANGE_SORT_LIMIT].u.fs_d.value &&
          lowValue.value <= params[LOW_RANGE_SORT_LIMIT].u.fs_d.value) {
        if (currentPixelValueDistance >= sortLength || pixelCounter == linePixels - 1) {
          return true;
        }
      }
      break;

    case USER_MAIN_SORT:

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
  switch (params[SORT_BY_DROPDOWN].u.pd.value) {
    case SORT_BY_LUMINOSITY:

      for (auto h = 0; h < borderIters.size(); ++h) {

        sort(borderIters[h].first,
             borderIters[h].second, [](const PixelStruct& pix1, const PixelStruct& pix2) {
          return pix1.value < pix2.value ? true : false;
        });
      }
  }
        
        /*
        reverseSortIfTrue(params[REVERSE_SORT_CHECKBOX].u.bd.value || 
                          lengthIsShortEnoughForFlip, h);

        switch (params[SORT_BY_COLOR_DROPDOWN].u.pd.value) {


          case RED_SORT:
          {
            PF_FpLong rgbBeginVal = (borderIters[h].first)->pixel.red;
            PF_FpLong rgbEndVal = (borderIters[h].second)->pixel.red;
            PF_FpLong rgbStep = (rgbBeginVal - rgbEndVal) / currPixDistance;

            for (auto pix = borderIters[h].first;
                 pix != borderIters[h].second; ++pix) {
              rgbBeginVal -= rgbStep;
              pix->pixel.red = static_cast<A_u_char>(rgbBeginVal);
            }

            break;
          }


          case GREEN_SORT:
          {
            PF_FpLong rgbBeginVal = (borderIters[h].first)->pixel.green;
            PF_FpLong rgbEndVal = (borderIters[h].second)->pixel.green;
            PF_FpLong rgbStep = (rgbBeginVal - rgbEndVal) / currPixDistance;

            for (auto pix = borderIters[h].first;
                 pix != borderIters[h].second; ++pix) {
              rgbBeginVal -= rgbStep;
              pix->pixel.green = static_cast<A_u_char>(rgbBeginVal);
            }

            break;
          }


          case BLUE_SORT:
          {
            PF_FpLong rgbBeginVal = (borderIters[h].first)->pixel.blue;
            PF_FpLong rgbEndVal = (borderIters[h].second)->pixel.blue;
            PF_FpLong rgbStep = (rgbBeginVal - rgbEndVal) / currPixDistance;

            for (auto pix = borderIters[h].first;
                 pix != borderIters[h].second; ++pix) {
              rgbBeginVal -= rgbStep;
              pix->pixel.blue = static_cast<A_u_char>(rgbBeginVal);
            }

            break;
          }
          default:break;
        }
      }
      break;

    case SORT_BY_RGB:

      for (auto line = 0, pixel = 0; line < borderIters.size(); ++line) {
        if (params[PIXEL_INTERPOLATION_CHECKBOX].u.bd.value) {
          for (auto k = borderIters[line].first;
               k != borderIters[line].second; ++k, ++pixel) {
            k->pixel = pixelMap[line][pixel].pixel;
          }
          pixel = 0;
        }
      }
      break;

    default: break;
  }
  */
}






void PixelSorter::getAndStorePixelValue() {

  switch (params[SORT_BY_DROPDOWN].u.pd.value) {
    case SORT_BY_LUMINOSITY:
      getLineWidthPixelAverage();
      break;

    case SORT_BY_RGB:
      getLineWidthPixelAverage();
      getLineWidthColorAverage();
      break;

    default: break;
  }


  if (highValue.value < columnAvg) {
    highValue.value = columnAvg;
  }
  else if (lowValue.value > columnAvg || lowValue.value == 0) {
    lowValue.value = columnAvg;
  }  
  
  currentPixelValueDistance = highValue.value - lowValue.value;
}






void PixelSorter::getUserSetMinLength() {
  
  minLength = (PF_Fixed)params[MIN_SORT_LENGTH_SLIDER].u.fs_d.value;
  
  if (params[MIN_SORT_RAND_SLIDER].u.fs_d.value >= 2) {
    minLength += (PF_Fixed)params[MIN_SORT_LENGTH_SLIDER].u.fs_d.value;

    minLength +=
      (PF_Fixed)((random() % (int)(params[MIN_SORT_RAND_SLIDER].u.fs_d.value / 2)) - 
      params[MIN_SORT_RAND_SLIDER].u.fs_d.value);

    minLength *= (PF_Fixed)((MAX_RBG_VALUE - columnAvg) / MAX_RBG_VALUE + 1);
  }
}












