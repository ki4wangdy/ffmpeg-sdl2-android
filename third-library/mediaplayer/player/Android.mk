#
# Copyright (c) 2013 Bilibili
# Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
#
# This file is part of ijkPlayer.
#
# ijkPlayer is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# ijkPlayer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with ijkPlayer; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# -mfloat-abi=soft is a workaround for FP register corruption on Exynos 4210
# http://www.spinics.net/lists/arm-kernel/msg368417.html
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_CFLAGS += -mfloat-abi=soft
endif

# LOCAL_CFLAGS += -std=c99

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH)/..)

LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH)/../ijkj4a)

LOCAL_SRC_FILES += avutil/dict.c
LOCAL_SRC_FILES += avutil/fifo.c
LOCAL_SRC_FILES += avutil/stl.cpp
LOCAL_SRC_FILES += avutil/threadpool.c
LOCAL_SRC_FILES += avutil/tree.c
LOCAL_SRC_FILES += avutil/utils.c

LOCAL_SRC_FILES += src/test.c

# LOCAL_SHARED_LIBRARIES := ijkffmpeg ijksdl
# LOCAL_STATIC_LIBRARIES := android-ndk-profiler ijksoundtouch

LOCAL_STATIC_LIBRARIES := ijkj4a x264

LOCAL_LDLIBS += -llog -lc -lm -lz -ldl

LOCAL_MODULE := mediaplayer

# VERSION_SH  = $(LOCAL_PATH)/version.sh
# VERSION_H   = ijkversion.h

include $(BUILD_SHARED_LIBRARY)
