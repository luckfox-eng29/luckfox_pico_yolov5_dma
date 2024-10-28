# NOTE
**本工程适用于 Kernel-5.10.110 版本的 luckfox-pico SDK，目前本仓库不再维护迁移到 https://github.com/LuckfoxTECH/luckfox_pico_rknn_example**

**This project is suitable for the luckfox-pico SDK based on Kernel version 5.10.110. The current repository is no longer maintained. Please migrate to https://github.com/LuckfoxTECH/luckfox_pico_rknn_example**

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
