//
//  vin_common.cpp
//  VINOCR
//
//  Created by Dai on 16/6/29.
//  Copyright © 2016年 Dai. All rights reserved.
//

#include "vin_common.hpp"

namespace VINOCRSPACE {
    bool Sobel(const Mat &image,Mat& result,int TYPE) {
        if(image.channels()!=1)
            return false;
        // 系数设置
        int kx(0);
        int ky(0);
        if( TYPE==0 ){//水平方向
            kx=0;ky=1;
        }
        else if( TYPE==1 ){//竖直方向
            kx=1;ky=0;
        }
        else if( TYPE==2 ){//both
            kx=1;ky=1;
        }
        else
            return false;
        
        // 设置mask
        float mask[3][3]={{1,2,1},{0,0,0},{-1,-2,-1}};
        Mat y_mask=Mat(3,3,CV_32F,mask)/8;
        Mat x_mask=y_mask.t(); // 转置
        
        // 计算x方向和y方向上的滤波
        Mat sobelX,sobelY;
        filter2D(image,sobelX,CV_32F,x_mask);
        filter2D(image,sobelY,CV_32F,y_mask);
        sobelX=abs(sobelX);
        sobelY=abs(sobelY);
        // 梯度图
        Mat gradient=kx*sobelX.mul(sobelX)+ky*sobelY.mul(sobelY);
        
        // 计算阈值
        int scale=4;
        double cutoff=scale*mean(gradient)[0];
        
        result.create(image.size(),image.type());
        result.setTo(0);
        for(int i=1;i<image.rows-1;i++)
        {
            float* sbxPtr=sobelX.ptr<float>(i);
            float* sbyPtr=sobelY.ptr<float>(i);
            float* prePtr=gradient.ptr<float>(i-1);
            float* curPtr=gradient.ptr<float>(i);
            float* lstPtr=gradient.ptr<float>(i+1);
            uchar* rstPtr=result.ptr<uchar>(i);
            // 阈值化和极大值抑制
            for(int j=1;j<image.cols-1;j++)
            {
                if( curPtr[j]>cutoff && (
                                         (sbxPtr[j]>kx*sbyPtr[j] && curPtr[j]>curPtr[j-1] && curPtr[j]>curPtr[j+1]) ||
                                         (sbyPtr[j]>ky*sbxPtr[j] && curPtr[j]>prePtr[j] && curPtr[j]>lstPtr[j]) ))
                    rstPtr[j]=255;
            }
        }
        
        return true;
    }
}