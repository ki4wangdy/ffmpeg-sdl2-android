

#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavcodec/avcodec.h>

#include <libavutil/opt.h>
#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>

int thread_exit=0;

#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

int run_thread(void *data){
    while (thread_exit==0) {
        SDL_Event event;
        event.type = SFM_REFRESH_EVENT;
        SDL_PushEvent(&event);
        //Wait 40 ms
        SDL_Delay(10);
    }
    return 0;
}

SDL_Rect m_DrawRect;

int main(){
    
    // ffmpeg related
    av_register_all();
    avformat_network_init();
    
    AVFormatContext *pFormatContext = NULL;
    AVCodecContext *pCodeContext = NULL;
    
    AVCodec *pCodec = NULL;
    
    const char* filePath = "/Users/wangdy/work/ffmpeg/flvparser/FlvParser2/FlvParser/test.flv";
    
    // init avformat context
    pFormatContext = avformat_alloc_context();
    
    // open flv file
    int isOk = 0;
    isOk = avformat_open_input(&pFormatContext, filePath, NULL, NULL);
    if(isOk != 0){
        goto over;
    }
    
    fprintf(stdout, "open file successfully\n");
    
    isOk = avformat_find_stream_info(pFormatContext,NULL);
    if(isOk != 0){
        goto over;
    }
    
    fprintf(stdout, "find stream info successfully\n");
    
    int videoindex = 0 ;
    for(int i=0; i<pFormatContext->nb_streams; i++){
        if(pFormatContext->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){
            videoindex=i;
            break;
        }
    }
    
    if(videoindex == -1){
        fprintf(stdout,"Didn't find a video stream.\n");
        return 0;
    }
        
    pCodec = avcodec_find_decoder(pFormatContext->streams[videoindex]->codecpar->codec_id);
    if(!pCodec){
        fprintf(stdout, "codec not find\n");
        return 0;
    }
    
    pCodeContext = pFormatContext->streams[videoindex]->codec;
    int result = avcodec_open2(pCodeContext, pCodec, NULL);
    if (result != 0){
        return 0;
    }
    
    AVFrame *pFrame;
    AVFrame *pFrameYUV;
    
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    
    // sdl init
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stdout, "Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
    
    int screenWidth = pCodeContext->width;
    int screenHeight = pCodeContext->height;
    
    SDL_Window *screen = SDL_CreateWindow("title",0, 0, screenWidth, screenHeight, 0);
    if(!screen){
        fprintf(stdout, "SDL_SetVideoMode create failed\n");
        return 0;
    }
    
    SDL_Renderer* render = SDL_CreateRenderer(screen,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!render){
        fprintf(stdout, "SDL_GetRenderer is error\n");
        return 0;
    }
    
    uint64_t t = pFormatContext->streams[videoindex]->start_time;
    if(t > 0){
        fprintf(stdout, "start time %f\n",t * av_q2d(pFormatContext->streams[videoindex]->time_base) / 1000000);
    }
    
    AVPacket* packet = av_packet_alloc();
    
    SDL_Thread* thread = SDL_CreateThread(run_thread, "ki-thread", NULL);
    
    SDL_Texture* pTexture = NULL;
    pTexture = SDL_CreateTexture(render,SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, screenWidth, screenHeight);
    
    SDL_Rect rect;
    SDL_Event event;
    while(1) {
        SDL_WaitEvent(&event);
        if(event.type == SFM_REFRESH_EVENT){
            
            isOk = av_read_frame(pFormatContext, packet);
            if(isOk >= 0){
                
                if(packet->stream_index == videoindex){
                    
                    isOk = avcodec_send_packet(pCodeContext,packet);
                    av_packet_unref(packet);
                    if(isOk != 0){
                        fprintf(stdout, "avcodec_send_packet is error\n");
                        return 0;
                    }
                    
                    isOk = avcodec_receive_frame(pCodeContext, pFrameYUV);
                    if(isOk != 0){
                        continue;
                    }
                    fprintf(stdout, "success w:%d h:%d pts:%f dts:%f \n",pFrameYUV->width, pFrameYUV->height,packet->pts, packet->dts);
                    
                    SDL_UpdateYUVTexture(pTexture, NULL,
                                         pFrameYUV->data[0], pFrameYUV->linesize[0],
                                         pFrameYUV->data[1], pFrameYUV->linesize[1],
                                         pFrameYUV->data[2], pFrameYUV->linesize[2]);
                    
                    
                    // Set Size of Window
                    rect.x = 0;
                    rect.y = 0;
                    rect.w = screenWidth;
                    rect.h = screenHeight;
                    
                    //展示
                    SDL_RenderClear(render);
                    SDL_RenderCopy(render, pTexture, NULL, &rect);
                    SDL_RenderPresent(render);
                    
                    av_frame_unref(pFrameYUV);
                    
                }
            }
            av_packet_unref(packet);
        }
    }
    
    SDL_Quit();
    
    avcodec_close(pCodeContext);
    avformat_close_input(&pFormatContext);
    
over:
    avformat_free_context(pFormatContext);
    
    return 0;
}
