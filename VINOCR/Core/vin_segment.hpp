//
//  vin_segment.hpp
//  VINOCR
//
//  Created by Dai on 16/6/27.
//  Copyright © 2016年 Dai. All rights reserved.
//

#ifndef vin_segment_hpp
#define vin_segment_hpp

#include "vin_common.hpp"

namespace VINOCRSPACE {
    class VINSegment {
    private:
        static const int vinCount = 17;//vin码为17位
        void vin_cut(const Mat &src, vector<cv::Mat> &vinCharMat);
        bool charArea(const Mat &src, Mat &dst);
        
    public:
        bool charSegment(const Mat &src, vector<Mat> &vinCharMat, Mat &histgram);
        bool charSegment(const Mat &src, vector<Mat> &vinCharMat);
    };
};

#endif /* vin_segment_hpp */
