//
//  VINOCR.h
//  VINOCR
//
//  Created by Dai on 16/7/6.
//  Copyright © 2016年 Dai. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@protocol VINOCRDelegate <NSObject>

- (void)vinOCRValue:(NSString *)value;

@end

@interface VINOCR : NSObject

//初始化delegate
+ (VINOCR *)instanceWithDelegate:(id<VINOCRDelegate>) delegate;
//使用debug模式 返回处理过程图片
+ (NSArray *)debugProcessImage:(UIImage *)image;

//识别图片
+ (BOOL)vinRecognition:(UIImage *)image;

@end
