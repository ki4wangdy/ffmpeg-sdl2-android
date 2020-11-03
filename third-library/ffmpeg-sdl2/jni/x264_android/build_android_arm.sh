#!/bin/bash
#NDK=/sers/yesimroy/Library/Android/sdk/ndk-bundle
NDK=/Users/wangdy/work/ndk/android-ndk-r10e
PLATFORM=$NDK/platforms/android-18/arch-arm/
TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64
PREFIX=./android/arm

function build_one
{
  ./configure \
  --prefix=$PREFIX \
  --enable-static \
  --enable-pic \
  --host=arm-linux \
  --cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
  --sysroot=$PLATFORM	\
  --disable-asm

  make clean
  make
  make install
}

build_one

echo Android ARM builds finished
