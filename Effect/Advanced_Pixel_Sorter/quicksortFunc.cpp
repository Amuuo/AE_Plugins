
#include"quicksortFunc.h"


template<class T>
void swap(T* s1, T* s2) {
  T tmp = *s1;
  *s1 = *s2;
  *s2 = tmp;
}



template<class T>
void insertionSort(PF_Fixed sizeOfArray, T* toSort) {



  PF_Fixed j;
  for (PF_Fixed i = 1; i < sizeOfArray; ++i) {

    if (toSort[i].value < toSort[i - 1].value) {
      j = i;
      while (toSort[j].value < toSort[j - 1].value) {
        swap(&toSort[j - 1], &toSort[j]);
        j--;
      }
    }
  }
}


template<class T>
void quickSort(PF_Fixed sizeOfArray, T* toSort) {


  if (sizeOfArray < 100) {
    insertionSort<T>(sizeOfArray, toSort);
    return;
  }

  else {


    // swap pivot and assign
    swap(&toSort[sizeOfArray / 2], &toSort[sizeOfArray - 1]);
    PF_Fixed lessCount = 0, moreCount = sizeOfArray - 2;
    PF_Fixed pivot = toSort[sizeOfArray - 1].value;


    // swap array elements around piviot
    for (; lessCount <= moreCount;) {

      if (toSort[lessCount] > pivot && toSort[moreCount] <= pivot) {
        swap(&toSort[lessCount++], &toSort[moreCount--]);
      }
      else {
        lessCount = toSort[lessCount] <= pivot ? lessCount + 1 : lessCount;
        moreCount = toSort[moreCount] > pivot ? moreCount - 1 : moreCount;
      }
    }

    // swap pivot back and recursively call qp
    swap(&toSort[lessCount], &toSort[sizeOfArray - 1]);
    quickSort<T>(lessCount, toSort);
    quickSort<T>(sizeOfArray - moreCount - 1, &toSort[lessCount]);
  }
}