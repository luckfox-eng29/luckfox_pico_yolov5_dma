# luckfox_pico_yolov5_dma
使用 luckfox-pico 借助 opencv-mobile 实现 yolov5 摄像头实时推理，作为 dma 使用的参考例程。

# 编译
```
cd luckfox_yolov5_dma/cpp
mkdir build
cd build
cmake ..
make && make install
```

# 运行
将 `cpp/luckfox_yolov5_dma_demo` 拷贝到 luckfox-pico。
```
cd luckfox_yolov5_dma_demo
./rknn_yolov5_dma_demo_test ./model/yolov5.rknn
```
**注意：** 测试前请先执行 `killall rkipc` 关闭摄像头应用占用。
