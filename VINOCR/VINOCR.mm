//
//  VINOCR.m
//  VINOCR
//
//  Created by Dai on 16/7/6.
//  Copyright © 2016年 Dai. All rights reserved.
//

#import "VINOCR.h"

#import <TesseractOCR/TesseractOCR.h>
#import "UIImageCVMatConverter.h"

#include "vin_detect.hpp"
#include "vin_locate.hpp"
#include "vin_segment.hpp"

using namespace VINOCRSPACE;

@interface VINOCR()

@property (nonatomic, strong) NSOperationQueue *operationQueue;
@property (nonatomic, assign) id<VINOCRDelegate> delegate;

@end

@implementation VINOCR

static VINOCR *_instance;

+ (VINOCR *)instanceWithDelegate:(id<VINOCRDelegate>)delegate {
    static dispatch_once_t onceToken = 0;
    dispatch_once(&onceToken, ^{
        _instance = [[VINOCR alloc] init];
        _instance.delegate = delegate;
    });
    
    return _instance;
}

+ (BOOL)vinRecognition:(UIImage *)image {
    if (!_instance) {
        return NO;
    }
    
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        cv::Mat src = [UIImageCVMatConverter cvMatFromUIImage:image];
        
        VINDetect *detec = new VINDetect();
        VINLocate *locate = new VINLocate();
        
        cv::Mat claheMat;
        detec->claheClip(src, claheMat);
        
        cv::Mat locateMat;
        locate->locateArea(claheMat, locateMat);
        
        _instance.operationQueue = [[NSOperationQueue alloc] init];
        [_instance recognizeWithTesseract:locateMat];
    });
    
    return YES;
}

+ (NSArray *)debugProcessImage:(UIImage *)image {
#ifndef __OPTIMIZE__
    //debug模式
    NSMutableArray *array = [NSMutableArray array];
    
    cv::Mat src = [UIImageCVMatConverter cvMatFromUIImage:image];
    
    VINDetect *detec = new VINDetect();
    VINLocate *locate = new VINLocate();
    
    cv::Mat claheMat;
    detec->claheClip(src, claheMat);
    
    cv::Mat locateMat;
    locate->locateArea(claheMat, locateMat);
    [array addObject:[UIImageCVMatConverter UIImageFromCVMat:locateMat]];
    
    //尺度变换 归一化
    float ratio = (float)locateMat.cols / locateMat.rows;
    int height = 48;
    int width = (int)(height * ratio);
    Mat resize_mat;
    resize(locateMat, resize_mat, cv::Size(width, height));
    
    //字符分割
    VINSegment *segment = new VINSegment();
    vector<Mat> vec = vector<Mat>();
    Mat seg;
    segment->charSegment(resize_mat, vec, seg);
    [array addObject:[UIImageCVMatConverter UIImageFromCVMat:seg]];
    
    return array;
#else
    //release模式
    
    return nil;
#endif
}

- (void)recognizeWithTesseract:(Mat)src {
    UIImage *image = [UIImageCVMatConverter UIImageFromCVMat:src];
    
    [G8Tesseract clearCache];
    
    G8RecognitionOperation *operation = [[G8RecognitionOperation alloc] initWithLanguage:@"eng"];
    operation.tesseract.charWhitelist = @"0123456789ABCDEFGHJKLMNPQRSTUVWXYZ ";
    operation.tesseract.engineMode = G8OCREngineModeTesseractOnly;
    operation.tesseract.pageSegmentationMode = G8PageSegmentationModeAutoOnly;
    operation.tesseract.image = image;
    
    operation.recognitionCompleteBlock = ^(G8Tesseract *tesseract) {
        // Fetch the recognized text
        NSString *result = tesseract.recognizedText;
        
        if (result == nil || result.length == 0) {//使用其他结果
            NSString *path = [[NSBundle mainBundle] pathForResource:@"hog+svm/HOG_SVM_DATA" ofType:@"xml"];
            if (path == nil || path.length == 0) {
                if ([self.delegate respondsToSelector:@selector(vinOCRValue:)]) {
                    [self.delegate vinOCRValue:result];
                }
                
                return;
            }
            
            char svmPath[1024];
            [path getCString:svmPath maxLength:1024 encoding:NSUTF8StringEncoding];
            
            VINDetect *detec = new VINDetect();
            string value = string();
            detec->vinRecognizeRegion(src, value, svmPath);
            
            result = [NSString stringWithCString:value.c_str() encoding:NSUTF8StringEncoding];
        }
        
        if ([self.delegate respondsToSelector:@selector(vinOCRValue:)]) {
            [self.delegate vinOCRValue:result];
        }
    };
    
    [self.operationQueue addOperation:operation];
}

@end
