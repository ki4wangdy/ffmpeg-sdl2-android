SRCPATH=.
prefix=./android/arm
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
libdir=${exec_prefix}/lib
includedir=${prefix}/include
SYS_ARCH=ARM
SYS=LINUX
CC=/Users/wangdy/work/ndk/android-ndk-r10e/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-gcc
CFLAGS=-Wno-maybe-uninitialized -Wshadow -O3 -ffast-math  -Wall -I. -I$(SRCPATH) --sysroot=/Users/wangdy/work/ndk/android-ndk-r10e/platforms/android-18/arch-arm/ -std=gnu99 -D_GNU_SOURCE -fPIC -fomit-frame-pointer -fno-tree-vectorize -fvisibility=hidden
CFLAGSSO=
CFLAGSCLI=
COMPILER=GNU
COMPILER_STYLE=GNU
DEPMM=-MM -g0
DEPMT=-MT
LD=/Users/wangdy/work/ndk/android-ndk-r10e/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-gcc -o 
LDFLAGS= --sysroot=/Users/wangdy/work/ndk/android-ndk-r10e/platforms/android-18/arch-arm/ -lm  -ldl
LDFLAGSCLI=-ldl 
LIBX264=libx264.a
CLI_LIBX264=$(LIBX264)
AR=/Users/wangdy/work/ndk/android-ndk-r10e/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-gcc-ar rc 
RANLIB=/Users/wangdy/work/ndk/android-ndk-r10e/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-gcc-ranlib
STRIP=/Users/wangdy/work/ndk/android-ndk-r10e/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-strip
INSTALL=install
AS=
ASFLAGS= -I. -I$(SRCPATH) -DSTACK_ALIGNMENT=4 -DPIC
RC=
RCFLAGS=
EXE=
HAVE_GETOPT_LONG=1
DEVNULL=/dev/null
PROF_GEN_CC=-fprofile-generate
PROF_GEN_LD=-fprofile-generate
PROF_USE_CC=-fprofile-use
PROF_USE_LD=-fprofile-use
HAVE_OPENCL=yes
CC_O=-o $@
default: cli
install: install-cli
default: lib-static
install: install-lib-static
