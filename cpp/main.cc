// Copyright (c) 2023 by Rockchip Electronics Co., Ltd. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*-------------------------------------------
                Includes
-------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yolov5.h"
#include "image_utils.h"
#include "file_utils.h"
#include "image_drawing.h"

//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <unistd.h>   
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include <time.h>

#include "dma_alloc.cpp"

#define USE_DMA 1


/*-------------------------------------------
                  Main Function
-------------------------------------------*/
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("%s <model_path>\n", argv[0]);
        return -1;
    }

    const char *model_path = argv[1];
   
    clock_t start_t, end_t;
    double total_t;


    char text[8];
    int width    = 640;
    int height   = 640;
    int channels = 3;
    int ret;
    rknn_app_context_t rknn_app_ctx;
    object_detect_result_list od_results;
    memset(&rknn_app_ctx, 0, sizeof(rknn_app_context_t));

    
    
    init_yolov5_model(model_path, &rknn_app_ctx);
    init_post_process();

#if USE_DMA 
    // dma init
    //RV1106 rga requires that input and output bufs are memory allocated by dma
    rknn_app_ctx.img_dma_buf.size = width * height * channels; 
    dma_buf_alloc(RV1106_CMA_HEAP_PATH,
                  rknn_app_ctx.img_dma_buf.size, 
                  &rknn_app_ctx.img_dma_buf.dma_buf_fd, 
                  (void **) & (rknn_app_ctx.img_dma_buf.dma_buf_virt_addr));                            
    unsigned char *src_image = (unsigned char *)rknn_app_ctx.img_dma_buf.dma_buf_virt_addr;
#else
    unsigned char *src_image = (unsigned char *)malloc(sizeof(unsigned char)*width*height*channels);
    //unsigned char *src_image = (unsigned char *)rknn_app_ctx.input_mems[0]->virt_addr;
#endif 
    cv::VideoCapture cap;
    cv::Mat bgr;
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  240);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240);
    cap.open(0); 

    printf("init success !\n");

    while(1)
    {
        start_t = clock();
        cap >> bgr;
        cv::resize(bgr, bgr, cv::Size(width,height), 0, 0, cv::INTER_LINEAR);
        //bgr = cv::imread("./bus.jpg");
        for (int y = 0; y < height; ++y) {
          for (int x = 0; x < width; ++x) {
              cv::Vec3b pixel = bgr.at<cv::Vec3b>(y, x);
              src_image[(y * width + x) * channels + 0] = pixel[2]; // Red
              src_image[(y * width + x) * channels + 1] = pixel[1]; // Green
              src_image[(y * width + x) * channels + 2] = pixel[0]; // Blue
            }
        }


#if USE_DMA  
        memcpy(rknn_app_ctx.input_mems[0]->virt_addr, src_image, width*height*channels);
        dma_sync_cpu_to_device(rknn_app_ctx.img_dma_buf.dma_buf_fd);
#else
        memcpy(rknn_app_ctx.input_mems[0]->virt_addr, src_image,640*640*3);
#endif

        inference_yolov5_model(&rknn_app_ctx, &od_results);

        // Add rectangle and probability

        for (int i = 0; i < od_results.count; i++)
        {
            object_detect_result *det_result = &(od_results.results[i]);
            printf("%s @ (%d %d %d %d) %.3f\n", coco_cls_to_name(det_result->cls_id),
                     det_result->box.left, det_result->box.top,
                     det_result->box.right, det_result->box.bottom,
                     det_result->prop);

            cv::rectangle(bgr,cv::Point(det_result->box.left ,det_result->box.top),
                              cv::Point(det_result->box.right,det_result->box.bottom),cv::Scalar(0,255,0),3);

            sprintf(text, "%s %.1f%%", coco_cls_to_name(det_result->cls_id), det_result->prop * 100);
            cv::putText(bgr,text,cv::Point(det_result->box.left, det_result->box.top - 8),
                                         cv::FONT_HERSHEY_SIMPLEX,1,
                                         cv::Scalar(0,255,0),2);
        
        }
        memset(text,0,8); 
        end_t = clock();
        total_t = (double)CLOCKS_PER_SEC / (end_t - start_t);
        printf("fps = %.2f\n",total_t);
    }


    deinit_post_process();

    ret = release_yolov5_model(&rknn_app_ctx);
    if (ret != 0)
    {
        printf("release_yolov5_model fail! ret=%d\n", ret);
    }
#if USE_DMA
    dma_buf_free(rknn_app_ctx.img_dma_buf.size, &rknn_app_ctx.img_dma_buf.dma_buf_fd, 
                rknn_app_ctx.img_dma_buf.dma_buf_virt_addr);
#else 
    free(src_image);
#endif
    
    return 0;
}
