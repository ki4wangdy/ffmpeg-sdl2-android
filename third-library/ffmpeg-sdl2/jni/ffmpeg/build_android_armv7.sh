#!/bin/bash

#export NDK=/ndk/android-ndk-r10e
export NDK=/Users/wangdy/work/ndk/android-ndk-r10e
export PLATFORM=$NDK/platforms/android-18/arch-arm/
export PREBUILT=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64

./configure	\
	--target-os=linux \
	--prefix=./ffmpeg_android	\
	--enable-small	\
	--disable-asm	\
	--arch=arm	\
	--cpu=cortex-a8	\
	--disable-linux-perf	\
	--cc=$PREBUILT/bin/arm-linux-androideabi-gcc	\
 	--cross-prefix=$PREBUILT/bin/arm-linux-androideabi- \
	--nm=$PREBUILT/bin/arm-linux-androideabi-nm	\
	--sysroot=$PLATFORM	\
  	--disable-shared \
  	--enable-static \
  	--enable-zlib \
	--enable-gpl	\
	--enable-libx264	\
  	--disable-doc \
	--enable-neon \
	--disable-ffprobe	\
	--disable-ffplay	\
	--disable-ffmpeg	\
	--disable-ffserver	\
	--disable-debug 	\
	--extra-cflags="-I../x264_android/android/arm/include -O3 -pie -fPIE -fpic -fasm -Wno-psabi -fno-short-enums -fno-strict-aliasing -finline-limit=300 -mfloat-abi=softfp -mfpu=vfp -marm -march=armv7-a -mcpu=cortex-a8 -mfpu=vfpv3-d16 -mfloat-abi=softfp -mthumb"	\
	--extra-ldflags="-L../x264_android/android/arm/lib"

make clean
make 
# make V=1

make install

export ANDROID_LD=$PREBUILT/bin/arm-linux-androideabi-ld

$ANDROID_LD	\
	-rpath-link=$PLATFORM/usr/lib	\
	-L$PLATFORM/usr/lib	\
	-L.	\
	-soname libffmpeg.so	\
	-shared -nostdlib	-Bsymbolic	--whole-archive	--no-undefined	\
	-o libffmpeg.so	./ffmpeg_android/lib/*.a	\
	../x264_android/lib/*.a	\
	-lc -lm -lz -ldl -llog	\
	--dynamic-linker=/system/bin/linker	\
	$PREBUILT/lib/gcc/arm-linux-androideabi/4.9/libgcc.a

	


