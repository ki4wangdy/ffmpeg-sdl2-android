

#include <libavformat/avformat.h>

#include <stdint.h>
#include "x264.h"

void test(){
	avformat_alloc_context();
	x264_picture_t t;
	x264_picture_init(&t);
}