//
//  vin_detect.cpp
//  VINOCR
//
//  Created by Dai on 16/6/24.
//  Copyright © 2016年 Dai. All rights reserved.
//

#include "vin_detect.hpp"

#include "vin_locate.hpp"
#include "vin_segment.hpp"

namespace VINOCRSPACE {
    
    //clahe处理
    bool VINDetect::claheClip(const Mat &src, Mat &dst) {
        if (src.channels() != 3) {
            return false;
        }
        
        //灰度处理
        Mat grayMat;
        cvtColor(src, grayMat, CV_RGB2GRAY);
        
        Ptr<CLAHE> clahe = createCLAHE();
        clahe->setClipLimit(Clahe_ClipLimit);
        clahe->apply(grayMat, dst);
        
        return true;
    }
    
    cv::Mat VINDetect::thinImage(const cv::Mat & src, const int maxIterations)
    {
        assert(src.type() == CV_8UC1);
        
        cv::Mat dst;
        threshold(src, dst, 128, 1, CV_THRESH_BINARY);
        
        int width  = src.cols;
        int height = src.rows;
        int count = 0;  //记录迭代次数
        while (true)
        {
            count++;
            if (maxIterations != -1 && count > maxIterations) //限制次数并且迭代次数到达
                break;
            std::vector<uchar *> mFlag; //用于标记需要删除的点
            //对点标记
            for (int i = 0; i < height ;++i)
            {
                uchar * p = dst.ptr<uchar>(i);
                for (int j = 0; j < width; ++j)
                {
                    //如果满足四个条件，进行标记
                    //  p9 p2 p3
                    //  p8 p1 p4
                    //  p7 p6 p5
                    uchar p1 = p[j];
                    if (p1 == 0) continue;
                    uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
                    uchar p8 = (j == 0) ? 0 : *(p + j - 1);
                    uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
                    uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
                    uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
                    uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
                    uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
                    uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);
                    if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
                    {
                        int ap = 0;
                        if (p2 == 0 && p3 == 1) ++ap;
                        if (p3 == 0 && p4 == 1) ++ap;
                        if (p4 == 0 && p5 == 1) ++ap;
                        if (p5 == 0 && p6 == 1) ++ap;
                        if (p6 == 0 && p7 == 1) ++ap;
                        if (p7 == 0 && p8 == 1) ++ap;
                        if (p8 == 0 && p9 == 1) ++ap;
                        if (p9 == 0 && p2 == 1) ++ap;
                        
                        if (ap == 1 && p2 * p4 * p6 == 0 && p4 * p6 * p8 == 0)
                        {
                            //标记
                            mFlag.push_back(p+j);
                        }
                    }
                }
            }
            
            //将标记的点删除
            for (std::vector<uchar *>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
            {
                **i = 0;
            }
            
            //直到没有点满足，算法结束
            if (mFlag.empty())
            {
                break;
            }
            else
            {
                mFlag.clear();//将mFlag清空
            }
            
            //对点标记
            for (int i = 0; i < height; ++i)
            {
                uchar * p = dst.ptr<uchar>(i);
                for (int j = 0; j < width; ++j)
                {
                    //如果满足四个条件，进行标记
                    //  p9 p2 p3
                    //  p8 p1 p4
                    //  p7 p6 p5
                    uchar p1 = p[j];
                    if (p1 == 0) continue;
                    uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
                    uchar p8 = (j == 0) ? 0 : *(p + j - 1);
                    uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
                    uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
                    uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
                    uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
                    uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
                    uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);
                    
                    if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
                    {
                        int ap = 0;
                        if (p2 == 0 && p3 == 1) ++ap;
                        if (p3 == 0 && p4 == 1) ++ap;
                        if (p4 == 0 && p5 == 1) ++ap;
                        if (p5 == 0 && p6 == 1) ++ap;
                        if (p6 == 0 && p7 == 1) ++ap;
                        if (p7 == 0 && p8 == 1) ++ap;
                        if (p8 == 0 && p9 == 1) ++ap;
                        if (p9 == 0 && p2 == 1) ++ap;
                        
                        if (ap == 1 && p2 * p4 * p8 == 0 && p2 * p6 * p8 == 0)
                        {
                            //标记
                            mFlag.push_back(p+j);
                        }
                    }
                }
            }
            
            //将标记的点删除
            for (std::vector<uchar *>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
            {
                **i = 0;
            }
            
            //直到没有点满足，算法结束
            if (mFlag.empty())
            {
                break;
            }
            else
            {
                mFlag.clear();//将mFlag清空
            }
        }
        
        threshold(dst, dst, 0, 255, CV_THRESH_BINARY);
        
        return dst;
    }
    
    void VINDetect::HOGFeature(const Mat &src, CvMat *data_mat, int index) {
        IplImage srcImg = IplImage(src);
        IplImage *tmpImg = cvCreateImage(cvGetSize(&srcImg), 8, 3);
        cvCvtColor(&srcImg, tmpImg, CV_GRAY2BGR);
        
        
        IplImage *trainImg = cvCreateImage(cvSize(HOG_ROW, HOG_COL), 8, 3);
        cvResize(tmpImg, trainImg);
        
        HOGDescriptor *hog = new HOGDescriptor(cvSize(HOG_ROW, HOG_COL), cvSize(4, 8), cvSize(2, 4), cvSize(2, 4), 9);
        vector<float>descriptors;
        hog->compute(trainImg, descriptors, Size(1, 1), Size(0, 0));
        
        int count = 0;
        for (vector<float>::iterator it = descriptors.begin(); it != descriptors.end(); it++) {
            cvmSet(data_mat, index, count, *it);
            count++;
        }
        
        cvReleaseImage(&trainImg);
        cvReleaseImage(&tmpImg);
    }
    
    bool VINDetect::vinRecognizeRegion(const Mat &locate_mat, string &vin,char *svmPath) {
        //尺度变换 归一化
        float ratio = (float)locate_mat.cols / locate_mat.rows;
        int height = templateHeight;
        int width = (int)(height * ratio);
        Mat resize_mat;
        resize(locate_mat, resize_mat, cv::Size(width, height));
        
        //字符分割
        VINSegment *segment = new VINSegment();
        vector<Mat> vec = vector<Mat>();
        if(!segment->charSegment(resize_mat, vec)) {
            return false;
        }
        
        //开始识别
        char classifier[128] = "0123456789ABCDEFGHJKLMNPRSVWXYTUZ ";
        CvSVM *svm = new CvSVM();
        svm->load(svmPath);
        CvMat *data_mat = cvCreateMat(1, 2916, CV_32FC1);
        for (vector<Mat>::iterator it = vec.begin(); it != vec.end(); it++) {
            Mat thinMat = this->thinImage(*it);
            HOGFeature(thinMat, data_mat, 0);
            
            int ret = svm->predict(data_mat);
            if (ret >= strlen(classifier)) {
                vin.append(1, ' ');
            }else {
                vin.append(1, classifier[ret]);
            }
        }
        
        return true;
    }
    
    bool VINDetect::vinRecognize(const Mat &src_mat, string &vin, char *svmPath) {
        //增强对比度
        Mat clahe_mat;
        if(!this->claheClip(src_mat, clahe_mat)) {
            return false;
        }
        
        //提取VIN区域
        VINLocate *locate = new VINLocate();
        Mat locate_mat;
        if (!locate->locateArea(clahe_mat, locate_mat)) {
            return false;
        }
        
        //尺度变换 归一化
        float ratio = (float)locate_mat.cols / locate_mat.rows;
        int height = templateHeight;
        int width = (int)(height * ratio);
        Mat resize_mat;
        resize(locate_mat, resize_mat, cv::Size(width, height));
        
        //字符分割
        VINSegment *segment = new VINSegment();
        vector<Mat> vec = vector<Mat>();
        if(!segment->charSegment(resize_mat, vec)) {
            return false;
        }
        
        //开始识别
        char classifier[128] = "0123456789ABCDEFGHJKLMNPRSVWXYTUZ ";
        CvSVM *svm = new CvSVM();
        svm->load(svmPath);
        CvMat *data_mat = cvCreateMat(1, 2916, CV_32FC1);
        for (vector<Mat>::iterator it = vec.begin(); it != vec.end(); it++) {
            Mat thinMat = this->thinImage(*it);
            HOGFeature(thinMat, data_mat, 0);
            
            int ret = svm->predict(data_mat);
            if (ret >= strlen(classifier)) {
                vin.append(1, ' ');
            }else {
                vin.append(1, classifier[ret]);
            }
        }
        
        return true;
    }
}
