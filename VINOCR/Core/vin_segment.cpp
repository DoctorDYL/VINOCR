//
//  vin_segment.cpp
//  VINOCR
//
//  Created by Dai on 16/6/27.
//  Copyright © 2016年 Dai. All rights reserved.
//

#include "vin_segment.hpp"

namespace VINOCRSPACE {
    
    //计算二值图垂直投影。
    void verProjection_calculate(const Mat &mat, int *vArr) {
        int height = mat.rows;
        int width = mat.cols;
        
        //记录垂直投影点的个数
        for (int j = 0; j < width; j++) {
            int count = 0;
            for (int i = 0; i < height; i++) {
                if (mat.at<uchar>(i, j) > 0) {
                    count++;
                }
            }
            
            vArr[j] = count;
        }
    }
    
    void verProjection_precut(const int *vArr, const int width, int &count, int charArray[][2]) {
        int threshold = 0;
        
        int flag = 0;
        count = 0;
        // vArr[]：垂直投影点的个数
        for (int i = 0; i < width; i++) {
            if (i == 0 && vArr[0] > threshold && !flag) {//起始位置
                charArray[count][0] = i;
                flag = 1;
            }else if (vArr[i] <= threshold && vArr[i+1] > threshold && !flag) {//起始位置
                charArray[count][0] = i;
                flag = 1;
            }else if (vArr[i] > threshold && vArr[i+1] <= threshold && flag) {
                charArray[count][1] = i;
                
                count++;
                flag = 0;
            }
        }
        
        if (flag == 1) {
            charArray[count][1] = width - 1;
            flag = 0;
            count++;
        }
    }
    
    void vin_recut(const Mat &src, int threshold, const int maxW, vector<cv::Mat> &vinCharMat) {
        int *vArr = new int[src.cols];
        verProjection_calculate(src, vArr);
        
        int count = 0;//分割个数
        int flag = 0;
        int charArray[10][2] = {0};
        
        for (int i = 0; i < src.cols; i++) {
            if (i == 0 && vArr[0] > threshold && !flag) {//起始位置
                charArray[count][0] = i;
                flag = 1;
            }else if (vArr[i] <= threshold && vArr[i+1] > threshold && !flag) {//起始位置
                charArray[count][0] = i;
                flag = 1;
            }else if (vArr[i] > threshold && vArr[i+1] <= threshold && flag) {
                charArray[count][1] = i;
                flag = 0;
                
                count++;
            }
        }
        
        if (flag == 1) {
            charArray[count][1] = src.cols - 1;
            flag = 0;
            count++;
        }
        
        for (int i = 0; i < count; i++) {//遍历
            int start = charArray[i][0];
            int end = charArray[i][1];
            
            if (end - start < 3) {
                continue;
            }
            
            if (end < src.cols-1) {
                end += 1;
            }
            
            Mat resultMat = src(Rect(start, 0, end-start, src.rows)).clone();
            if (end - start > maxW) {//过宽,再分割
                vin_recut(resultMat, (threshold+1), maxW, vinCharMat);
                
            }else {
                //添加空隙
                vinCharMat.push_back(resultMat);
            }
        }
    }
    
    //vArr 垂直直方图
    void VINSegment::vin_cut(const Mat &src, vector<cv::Mat> &vinCharMat) {
        //垂直投影
        int *vArr = new int[src.cols];
        verProjection_calculate(src, vArr);
        
        int count = 0;//分割个数
        int charArray[100][2] = {0};
        verProjection_precut(vArr, src.cols, count, charArray);
        
        for (int i = 0; i < count; i++) {
            int start = charArray[i][0];
            int end = charArray[i][1];
            
            if (end - start < 3) {
                continue;
            }
            
            if (end < src.cols-1) {
                end += 1;
            }
            
            Mat resultMat = src(Rect(start, 0, end-start, src.rows)).clone();
            if ((end - start) > src.cols/12) {//过宽,再分割
                vin_recut(resultMat, 1, src.cols/12, vinCharMat);
            }else {
                //添加空隙
                Mat dstMat;
                this->charArea(resultMat, dstMat);
                vinCharMat.push_back(dstMat);
            }
        }
    }
    
    bool VINSegment::charArea(const Mat &src, Mat &dst) {
        assert(src.type() == CV_8UC1);
        
        //外围处理
        Mat mat = src.clone();
        
        int minH = mat.rows;
        int maxH = 0;
        int minW = mat.cols;
        int maxW = 0;
        
        for (int i = 0; i < mat.rows; i++) {
            for (int j = 0; j < mat.cols; j++) {
                if (mat.at<uchar>(i, j) != 0) {
                    minH = min(i, minH);
                    minW = min(j, minW);
                    
                    maxH = max(i, maxH);
                    maxW = max(j, maxW);
                }
            }
        }
        
        if (minH >= 2) {
            minH -= 2;
        }
        
        if (maxH < mat.rows - 2) {
            maxH += 2;
        }
        
        if (minW >= 2) {
            minW -= 2;
        }
        
        if (maxW < mat.cols - 2) {
            maxW += 2;
        }
        
        int wt = maxW - minW;
        int ht = maxH - minH;
        
        //截取
        dst = src(Rect(minW, minH, wt, ht)).clone();
        
        return true;
    }
    
    bool VINSegment::charSegment(const cv::Mat &src, vector<cv::Mat> &vinCharMat, cv::Mat &histgram) {
        //垂直投影
        int *vArr = new int[src.cols];
        verProjection_calculate(src, vArr);
        
        //直方图信息
        int width = src.cols;
        cv::Mat projImg(50, width,  CV_8U, cv::Scalar(255));
        for (int col = 0; col < width; ++col)
        {
            cv::line(projImg, cv::Point(col, 50 - vArr[col]), cv::Point(col, 50 - 1), cv::Scalar::all(0));
        }
        histgram  = projImg.clone();
        
        vin_cut(src, vinCharMat);
        
        return true;
    }
    
    bool VINSegment::charSegment(const cv::Mat &src, vector<cv::Mat> &vinCharMat) {
        //垂直投影
        int *vArr = new int[src.cols];
        verProjection_calculate(src, vArr);
        
        vin_cut(src, vinCharMat);
        
        return true;
    }
}

