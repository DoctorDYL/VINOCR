//
//  vin_detect.hpp
//  VINOCR
//
//  Created by Dai on 16/6/24.
//  Copyright © 2016年 Dai. All rights reserved.
//

#ifndef vin_detect_hpp
#define vin_detect_hpp

#include "vin_common.hpp"

namespace VINOCRSPACE {
    class VINDetect {
    private:
        // w:h == 10:1
        static const int templateHeight = 48;
        
        static const int Clahe_ClipLimit = 2;
        static const int Clahe_Col = 8;
        static const int Clahe_Row = 1;
        
        static const int HOG_ROW = 20;
        static const int HOG_COL = 40;
        
        void HOGFeature(const Mat &src, CvMat *data_mat, int index);
        Mat thinImage(const Mat & src, const int maxIterations = -1);
        
    public:
        bool claheClip(const Mat &src, Mat &dst);
        
        bool vinRecognizeRegion(const Mat &src_mat, string &vin,char *svmPath);
        bool vinRecognize(const Mat &src_mat, string &vin, char *svmPath);
    };
};



#endif /* vin_detect_hpp */
