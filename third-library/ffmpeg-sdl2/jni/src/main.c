
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "SDL.h"

#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "libavutil/imgutils.h"  
#include "SDL.h"  

#include <jni.h>  
#include <android/log.h>  

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO , "ki_test_ffmpeg", __VA_ARGS__)  
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR , "ki_test_ffmpeg", __VA_ARGS__)  

//Output YUV420P data as a file   
#define OUTPUT_YUV420P 1  

typedef struct Sprite
{
	SDL_Texture* texture;
	Uint16 w;
	Uint16 h;
} Sprite;

/* Adapted from SDL's testspriteminimal.c */
Sprite LoadSprite(const char* file, SDL_Renderer* renderer)
{
	Sprite result;
	result.texture = NULL;
	result.w = 0;
	result.h = 0;
	
    SDL_Surface* temp;

    /* Load the sprite image */
    temp = SDL_LoadBMP(file);
    if (temp == NULL)
	{
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        return result;
    }
    result.w = temp->w;
    result.h = temp->h;

    /* Create texture from the image */
    result.texture = SDL_CreateTextureFromSurface(renderer, temp);
    if (!result.texture) {
        fprintf(stderr, "Couldn't create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(temp);
        return result;
    }
    SDL_FreeSurface(temp);

    return result;
}

void draw(SDL_Window* window, SDL_Renderer* renderer, const Sprite sprite)
{
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	SDL_Rect destRect = {w/2 - sprite.w/2, h/2 - sprite.h/2, sprite.w, sprite.h};
	/* Blit the sprite onto the screen */
	SDL_RenderCopy(renderer, sprite.texture, NULL, &destRect);
}

int main(int argc, char *argv[])
{
//     SDL_Window *window;
//     SDL_Renderer *renderer;
// 
//     if(SDL_CreateWindowAndRenderer(0, 0, 0, &window, &renderer) < 0)
//         exit(2);
// 
// 	Sprite sprite = LoadSprite("image.bmp", renderer);
//     if(sprite.texture == NULL)
//         exit(2);
// 
//     /* Main render loop */
//     Uint8 done = 0;
//     SDL_Event event;
//     while(!done)
// 	{
//         /* Check for events */
//         while(SDL_PollEvent(&event))
// 		{
//             if(event.type == SDL_QUIT || event.type == SDL_KEYDOWN || event.type == SDL_FINGERDOWN)
// 			{
//                 done = 1;
//             }
//         }
// 		
// 		
// 		/* Draw a gray background */
// 		SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
// 		SDL_RenderClear(renderer);
// 		
// 		draw(window, renderer, sprite);
// 	
// 		/* Update the screen! */
// 		SDL_RenderPresent(renderer);
// 		
// 		SDL_Delay(10);
//     }
// 
//     exit(0);

	LOGI("application start to .................. ki wangdy\n");

	AVFormatContext *pFormatCtx;
	int             i, videoindex;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;
	AVFrame *pFrame, *pFrameYUV;
	unsigned char *out_buffer;
	AVPacket *packet;
	int y_size;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;

	char filepath[] = "/storage/sdcard0/test.mp4";
	//SDL---------------------------  
	int screen_w = 0, screen_h = 0;
	SDL_Window *screen;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;

	FILE *fp_yuv;

	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0){
		LOGI("Couldn't open input stream.\n");
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0){
		LOGI("Couldn't find stream information.\n");
		return -1;
	}
	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
		videoindex = i;
		break;
	}
	if (videoindex == -1){
		LOGI("Didn't find a video stream.\n");
		return -1;
	}

	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL){
		LOGI("Codec not found.\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
		LOGI("Could not open codec.\n");
		return -1;
	}

	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();

	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
		AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Info-----------------------------  
	LOGI("--------------- File Information ----------------\n");
	av_dump_format(pFormatCtx, 0, filepath, 0);
	LOGI("-------------------------------------------------\n");
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

#if OUTPUT_YUV420P   
	fp_yuv = fopen("/storage/sdcard0/output.yuv", "wb+");
#endif    

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		LOGI("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;

	LOGI("SDL_CreateWindow(screen, &w, &h):%d %d", screen_w, screen_h);

	//SDL 2.0 Support for multiple windows  
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		480, 960,
		SDL_WINDOW_FULLSCREEN);

	if (!screen) {
		LOGI("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}

	int w = 0;
	int h = 0;
	SDL_GetWindowSize(screen, &w, &h);
	LOGI("SDL_GetWindowSize(screen, &w, &h):%d %d", w,h);

	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	//IYUV: Y + U + V  (3 planes)  
	//YV12: Y + V + U  (3 planes)  
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);

	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;

	//SDL End----------------------  
	while (av_read_frame(pFormatCtx, packet) >= 0){
		if (packet->stream_index == videoindex){
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (ret < 0){
				LOGI("Decode Error.\n");
				return -1;
			}
			if (got_picture){
				sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
					pFrameYUV->data, pFrameYUV->linesize);

#if OUTPUT_YUV420P  
				y_size = pCodecCtx->width*pCodecCtx->height;
				fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y   
				fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U  
				fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V  
#endif  
				//SDL---------------------------  
#if 0  
				SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
#else  
				SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
					pFrameYUV->data[0], pFrameYUV->linesize[0],
					pFrameYUV->data[1], pFrameYUV->linesize[1],
					pFrameYUV->data[2], pFrameYUV->linesize[2]);
#endif    

				SDL_RenderClear(sdlRenderer);
				SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
				SDL_RenderPresent(sdlRenderer);
				//SDL End-----------------------  
				//Delay 40ms  
				SDL_Delay(40);
			}
		}
		av_free_packet(packet);
	}
	//flush decoder  
	//FIX: Flush Frames remained in Codec  
	while (1) {
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
		if (ret < 0)
			break;
		if (!got_picture)
			break;
		sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
			pFrameYUV->data, pFrameYUV->linesize);
#if OUTPUT_YUV420P  
		int y_size = pCodecCtx->width*pCodecCtx->height;
		fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y   
		fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U  
		fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V  
#endif  
		//SDL---------------------------  
		SDL_UpdateTexture(sdlTexture, &sdlRect, pFrameYUV->data[0], pFrameYUV->linesize[0]);
		SDL_RenderClear(sdlRenderer);
		SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
		SDL_RenderPresent(sdlRenderer);
		//SDL End-----------------------  
		//Delay 40ms  
		SDL_Delay(40);
	}

	sws_freeContext(img_convert_ctx);

#if OUTPUT_YUV420P   
	fclose(fp_yuv);
#endif   

	SDL_Quit();

	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;

}
