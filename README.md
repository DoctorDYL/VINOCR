# VINOCR
汽车挡风玻璃VIN码识别

应用目前使用范围有限，对拍摄的VIN码图像限制较高。需要图像恰包含VIN码区域，光照等噪声抗性较低。

识别流程：
1.图像预处理［clahe处理］
2.定位VIN区域［图像旋转，有效区域提取］
3.识别［HOG＋SVM］
注：由于训练集与测试集数量有限，为保证识别的正确率，字符识别阶段引入了Google TesseractOCR进行字符识别(https://github.com/gali8/Tesseract-OCR-iOS)



使用方式：

1.将工程生成的VINOCR.framework引入目标工程

2.目标工程引入libc++.1.tbd

3.VINOCR不支持arm64架构，因此目标工程BuildSettings->Architectures->Valid Architectures中需删除

4.将VINOCR.framework中的tessdata文件夹拷贝到目标工程目录，并引入目标工程以确保TesseractOCR使用
