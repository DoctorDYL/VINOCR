//
//  vin_common.h
//  VINOCR
//
//  Created by Dai on 16/6/24.
//  Copyright © 2016年 Dai. All rights reserved.
//

#ifndef vin_common_h
#define vin_common_h

#include <stdio.h>

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <time.h>
#include <math.h>
#include <sys/types.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <string>
#include <list>
#include <vector>
#include <map>


using namespace std;
using namespace cv;

namespace VINOCRSPACE {
    bool Sobel(const Mat &image,Mat& result,int TYPE);
}

#endif /* vin_common_h */
