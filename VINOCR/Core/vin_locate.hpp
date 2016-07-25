//
//  vin_locate.hpp
//  VINOCR
//
//  Created by Dai on 16/6/24.
//  Copyright © 2016年 Dai. All rights reserved.
//

#ifndef vin_locate_hpp
#define vin_locate_hpp

#include "vin_common.hpp"

namespace VINOCRSPACE {
    
    class VINLocate {
    private:
        bool cutArea(const Mat &src, Mat &dst);
        bool recutArea(const Mat &src, Mat &dst);
        bool rotateArea(const Mat &src, Mat &dst);
        void thresholdSegment(const Mat &src, Mat &dst, int segment);
        bool regionFilter(const Mat &src, Mat &dst);
        
    public:
        
        bool locateArea(const Mat &src, Mat &dst);
    };
}

#endif /* vin_locate_hpp */
