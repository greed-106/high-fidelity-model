#!/bin/bash

path=/mnt/sda1/codehub/hfm-dataset/
# data=pq_ori_seq_03_1920x1080_yub422_10bit_2.yuv
data=072_Outdoor_4096x2304_30fps_422.yuv
width=4096
height=2304

# path=/mnt/sda1/kjshao/avspro/dataset-chanpinxian
# data=00004-TE_900x900_422_10bits.yuv
# width=900
# height=900

mode=Release

rm -rf build install
bash build.sh -c clang -b ${mode}

echo "Encoding..."
./install/clang/${mode}/bin/EncoderApp -c cfg/main.cfg -f 2 --IntraPeriod 2
echo "Decoding..."
./install/clang/${mode}/bin/DecoderApp -b output/bits.bin -o output/dec_${data}
