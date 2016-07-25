//
//  vin_locate.cpp
//  VINOCR
//
//  Created by Dai on 16/6/24.
//  Copyright © 2016年 Dai. All rights reserved.
//

#include "vin_locate.hpp"
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/core_c.h>

namespace VINOCRSPACE {
    
    //最大类间方差阈值法
    int otsu(Mat src) {
        if (src.cols == 0 || src.rows == 0) {
            return 0;
        }
        
        int width = src.cols;
        int height = src.rows;
        int hist[256] = {0};//直方图
        
        //统计灰度级中每个像素在整幅图像中的个数
        for (auto it = src.begin<uchar>(); it != src.end<uchar>(); it++) {
            hist[*it]++;
        }
        
        int pixelSum = width * height, threshold = 0;
        
        //经典ostu算法,得到前景和背景的分割
        //遍历灰度级[0,255],计算出方差最大的灰度值,为最佳阈值
        float w0, w1, u0, u1, u, delta = 0, deltaMax = 0;
        for (int i = 0; i < 256; i++) {
            w0 = w1 = u0 = u1 = 0;
            
            for (int j = 0; j < 256; j++) {
                float k = (float)hist[j]/(float)pixelSum;
                if (j <= i) {//背景部分
                    //以i为阈值分类，第一类总的概率
                    w0 += k;
                    u0 += j*k;
                } else {//前景部分
                    //以i为阈值分类，第二类总的概率
                    w1 += k;
                    u1 += j*k;
                }
            }
            
            u = u0 + u1; //整幅图像的平均灰度
            u0 /= w0;   //第一类的平均灰度
            u1 /= w1;   //第二类的平均灰度
            
            //计算类间方差
            delta = w0 * (u0 - u) * (u0 - u) + w1 * (u1 - u) * (u1 - u);
            //找出最大类间方差以及对应的阈值
            if (delta > deltaMax) {
                deltaMax = delta;
                threshold = i;
            }
        }
        //返回最佳阈值;
        return threshold;
    }
    
    //分段二值化 避免局部光照影响
    void VINLocate::thresholdSegment(const Mat &src, Mat &dst, int segment) {
        Mat bin = src.clone();
        
        int w = bin.cols;
        int h = bin.rows;
        int n = w/segment;
        
        for (int i = 0; i < segment; i++) {
            Mat child;
            
            if (i == segment - 1) {//最后一部分
                child = bin(Rect(i*n, 0, w - i*n, h));
            }else {
                child = bin(Rect(i*n, 0, n, h));
            }
            
            Mat cpChild = child.clone();
            int thr = otsu(cpChild);//最大方差过滤
            threshold(cpChild, cpChild, thr, 255, CV_THRESH_BINARY);
            cpChild.convertTo(child, child.type());
        }
        
        dst = bin.clone();
    }
    
    //截取VIN区域
    bool VINLocate::cutArea(const Mat &src, Mat &dst) {
        if (src.rows == 0 || src.cols == 0) {
            return false;
        }
        
        //垂直边界
        Mat sobelYMat;
        Sobel(src, sobelYMat, 1);
        IplImage sobelY_ipl = IplImage(sobelYMat);
        IplImage *sobelY = &sobelY_ipl;
        
        for (int j = 0; j < sobelY->height; j++) {
            cvSet2D(sobelY, j, 0, cvScalar(0));
            cvSet2D(sobelY, j, sobelY->width - 1, cvScalar(0));
        }
        for (int i = 0; i < sobelY->width; i++) {
            cvSet2D(sobelY, 0, i, cvScalar(0));
            cvSet2D(sobelY, sobelY->height - 1, i, cvScalar(0));
        }
        
        //求切割区域的上下坐标点
        CvSeq *contour = NULL;
        CvMemStorage *storage = cvCreateMemStorage(0);
        cvFindContours(sobelY, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL,
                       CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
        
        int maxX=0, minX=sobelY->width;
        int maxY=0, minY=sobelY->height;
        while (contour) {
            CvRect rect = ((CvContour *)contour)->rect;
            contour = contour->h_next;
            if (rect.height < 8) {
                continue;
            }
            
            int xw = rect.x + rect.width;
            minX = min(rect.x, minX);
            maxX = max(xw, maxX);
            
            int yh = rect.y + rect.height;
            minY = min(rect.y, minY);
            maxY = max(yh, maxY);
        }
        cvReleaseMemStorage(&storage);
        
        //求剪切高度宽度大小||downy<sobelY->height/3
        //预留边界
        if (minX >= 2) {
            minX -= 2;
        }
        if (minY >= 2) {
            minY -= 2;
        }
        if (maxX < sobelY->width-2) {
            maxX += 2;
        }
        if (maxY < sobelY->height-2) {
            maxY += 2;
        }
        
        int ht = maxY - minY;
        int wt = maxX - minX;
        if (ht < 24 || wt < 12) {
            return false;
        }
        
        //截取
        dst = src(Rect(minX, minY, wt, ht)).clone();
        
        return true;
    }
    
    bool VINLocate::recutArea(const Mat &src, Mat &dst) {
        if (src.rows == 0 || src.cols == 0) {
            return false;
        }
        
        //垂直边界
        Mat sobelYMat;
        Sobel(src, sobelYMat, 1);
        IplImage sobelY_ipl = IplImage(sobelYMat);
        IplImage *sobelY = &sobelY_ipl;
        
        //求切割区域的上下坐标点
        CvSeq *contour = NULL;
        CvMemStorage *storage = cvCreateMemStorage(0);
        cvFindContours(sobelY, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL,
                       CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
        
        int maxX=0, minX=(&sobelY_ipl)->width;
        int maxY=0, minY=(&sobelY_ipl)->height;
        while (contour) {
            CvRect rect = ((CvContour *)contour)->rect;
            contour = contour->h_next;
            
            int xw = rect.x + rect.width;
            minX = min(rect.x, minX);
            maxX = max(xw, maxX);
            
            int yh = rect.y + rect.height;
            minY = min(rect.y, minY);
            maxY = max(yh, maxY);
        }
        cvReleaseMemStorage(&storage);
        
        if (minX >= 2) {
            minX -= 2;
        }
        if (minY >= 2) {
            minY -= 2;
        }
        if (maxX < sobelY->width-2) {
            maxX += 2;
        }
        if (maxY < sobelY->height-2) {
            maxY += 2;
        }
        
        int ht = maxY - minY;
        int wt = maxX - minX;
        if (ht < 24 || wt < 12) {
            return false;
        }
        
        //截取
        dst = src(Rect(minX, minY, wt, ht)).clone();
        
        return true;
    }
    
    //旋转变换
    bool VINLocate::rotateArea(const Mat &src, Mat &dst) {
        if(src.channels()!=1)
            return false;
        
        IplImage img = IplImage(src);
        IplImage *img_ptr = &img;
        
        //垂直边缘
        Mat sobelMat;
        Sobel(src, sobelMat, 1);
        
        //二值化
        IplImage sobel = IplImage(sobelMat);
        IplImage *sobelImg = cvCreateImage(cvGetSize(img_ptr), 8, 1);
        cvThreshold(&sobel, sobelImg, 30, 255, CV_THRESH_OTSU | CV_THRESH_BINARY);
        
        //计算旋转角度
        int w = sobelImg->width;
        int h = sobelImg->height;
        int nLinebyte = sobelImg->widthStep;
        
        int count = 0;
        double a = 0.0;
        typedef unsigned char BYTE;
        BYTE pixel;
        double x1 = 0.0, y1 = 0.0;
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                pixel = (BYTE)sobelImg->imageData[i * nLinebyte + j];
                if (pixel == 255) {
                    x1 += j;
                    count++;
                    y1 += i;
                }
            }
        }
        
        x1 = x1 / (double)count;
        y1 = y1 / (double)count;
        int u1, v1;
        double sum = 0.0;
        double sum1 = 0.0;
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                pixel = (BYTE)sobelImg->imageData[i * nLinebyte + j];
                if (pixel == 255) {
                    u1 = j - x1;
                    v1 = i - y1;
                    sum += u1 * v1;
                    sum1 += (u1 * u1 - v1 * v1);
                }
            }
        }
        sum = sum * 2.0;
        a = atan(sum / sum1);
        a = a / 2.0;
        a = a / 3.1415926 * 180.0;
        
        CvPoint2D32f center;
        center.x = float(img_ptr->width / 2.0 + 0.5);
        center.y = float(img_ptr->height / 2.0 + 0.5);
        //计算二维旋转的仿射变换矩阵
        float m[6];
        CvMat M = cvMat(2, 3, CV_32F, m);
        cv2DRotationMatrix(center, a, 1, &M);
        
        //保存矫正后的图片
        IplImage *retImg = cvCreateImage(cvGetSize(img_ptr), 8, 1);
        cvWarpAffine(img_ptr, retImg, &M, CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
        dst = Mat(retImg, 1);
        
        cvReleaseImage(&sobelImg);
        cvReleaseImage(&retImg);
        
        return true;
    }
    
    bool VINLocate::regionFilter(const cv::Mat &src, cv::Mat &dst) {
        if (src.channels() != 1) {
            return false;
        }
        
        Mat tmp = src.clone();
        IplImage image = IplImage(tmp);
        IplImage *cloneImg = cvCloneImage(&image);
        
        double allArea = (&image)->width*(&image)->height;
        
        CvSeq *contour = NULL;
        CvMemStorage *storage = cvCreateMemStorage(0);
        
        cvFindContours(&image, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL);
        while (contour != NULL){
            CvRect rect = ((CvContour *)contour)->rect;
            
            double area = rect.height*rect.width;
            //根据连通器大小过滤
            if (area < allArea/150) {
                cvDrawContours(cloneImg, contour, CV_RGB(0, 0, 0), CV_RGB(0, 0, 0), 0,CV_FILLED); //绘制黑色CV_RGB(0, 0, 0)轮廓
            }
            
            contour = contour->h_next;
        }
        
        //腐蚀和膨胀
        Mat cloneMat = Mat(cloneImg, 1);
        Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
        morphologyEx(cloneMat, dst, CV_MOP_OPEN, element);
        
        return true;
    }
    
    bool VINLocate::locateArea(const cv::Mat &src, cv::Mat &dst) {
        if (src.channels() != 1) {
            return false;
        }
        
        //旋转变换
        if(!this->rotateArea(src, dst)) {
            return false;
        }
        
        //区域截取
        if(!this->cutArea(dst, dst)) {
            return false;
        }
        
        //二值化
        this->thresholdSegment(dst, dst, 17);
        this->regionFilter(dst, dst);
        
        //过滤后重新截取
        if(!this->recutArea(dst, dst)) {
            return false;
        }
        
        return true;
    }
}