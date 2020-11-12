
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
//Important!
#pragma pack(1)
 
 
#define TAG_TYPE_SCRIPT 18
#define TAG_TYPE_AUDIO  8
#define TAG_TYPE_VIDEO  9
 
typedef unsigned char byte;
typedef unsigned int uint;
 
typedef struct {
	byte Signature[3];
	byte Version;
	byte Flags;
	uint DataOffset;
} FLV_HEADER;
 
typedef struct {
	byte TagType;
	byte DataSize[3];
	byte Timestamp[3];
	uint Reserved;
} TAG_HEADER;
 
 
//reverse_bytes - turn a BigEndian byte array into a LittleEndian integer
uint reverse_bytes(byte *p, char c) {
	int r = 0;
	int i;
	for (i=0; i<c; i++) 
		r |= ( *(p+i) << (((c-1)*8)-8*i));
	return r;
}

double hexStr2double(const unsigned char* hex, const unsigned int length) {

    double ret = 0;
    char hexstr[length * 2];
    memset(hexstr, 0, sizeof(hexstr));

    for(unsigned int i = 0; i < length; i++) {
        sprintf(hexstr + i * 2, "%02x", hex[i]);
    }

    sscanf(hexstr, "%llx", (unsigned long long*)&ret);

    return ret;
}

void parseMeta(char* m_meta){
	unsigned int arrayLen = 0;
    unsigned int offset = 13;

    unsigned int nameLen = 0;
    double numValue = 0;
    bool boolValue = false;

    if(m_meta[offset++] == 0x08) {

        arrayLen |= m_meta[offset++];
        arrayLen = arrayLen << 8;
        arrayLen |= m_meta[offset++];
        arrayLen = arrayLen << 8;
        arrayLen |= m_meta[offset++];
        arrayLen = arrayLen << 8;
        arrayLen |= m_meta[offset++];

        //cerr << "ArrayLen = " << arrayLen << endl;
    } else {
        //TODO:
        // cerr << "metadata format error!!!" << endl;
        return ;
    }

    double m_duration;
	double m_width;
	double m_height;
	double m_framerate;
	double m_videodatarate;
	double m_audiodatarate;
	double m_videocodecid;
	double m_audiosamplerate;
	double m_audiosamplesize;
	double m_audiocodecid;
	double m_stereo;
	double m_fileSize;

    for(unsigned int i = 0; i < arrayLen; i++) {

        numValue = 0;
        boolValue = false;

        nameLen = 0;
        nameLen |= m_meta[offset++];
        nameLen = nameLen << 8;
        nameLen |= m_meta[offset++];
        //cerr << "name length=" << nameLen << " ";

        char name[nameLen + 1];

        memset(name, 0, sizeof(name));
        memcpy(name, &m_meta[offset], nameLen);
        name[nameLen + 1] = '\0';
        offset += nameLen;

        switch(m_meta[offset++]) {
	        case 0x0: //Number type

	            numValue = hexStr2double((const unsigned char *)&m_meta[offset], 8);
	            offset += 8;
	            break;

	        case 0x1: //Boolean type
	            if(offset++ != 0x00) {
	                boolValue = true;
	            }
	            break;

	        case 0x2: //String type
	            nameLen = 0;
	            nameLen |= m_meta[offset++];
	            nameLen = nameLen << 8;
	            nameLen |= m_meta[offset++];
	            offset += nameLen;
	            break;

	        case 0x12: //Long string type
	            nameLen = 0;
	            nameLen |= m_meta[offset++];
	            nameLen = nameLen << 8;
	            nameLen |= m_meta[offset++];
	            nameLen = nameLen << 8;
	            nameLen |= m_meta[offset++];
	            nameLen = nameLen << 8;
	            nameLen |= m_meta[offset++];
	            offset += nameLen;
	            break;
	        //FIXME:
	        default:
	            break;
        }


        

        if(strncmp(name, "duration", 8)	== 0) {
            m_duration = numValue;
        } else if(strncmp(name, "width", 5)	== 0) {
            m_width = numValue;
        } else if(strncmp(name, "height", 6) == 0) {
            m_height = numValue;
        } else if(strncmp(name, "framerate", 9) == 0) {
            m_framerate = numValue;
        } else if(strncmp(name, "videodatarate", 13) == 0) {
            m_videodatarate = numValue;
        } else if(strncmp(name, "audiodatarate", 13) == 0) {
            m_audiodatarate = numValue;
        } else if(strncmp(name, "videocodecid", 12) == 0) {
            m_videocodecid = numValue;
        } else if(strncmp(name, "audiosamplerate", 15) == 0) {
            m_audiosamplerate = numValue;
        } else if(strncmp(name, "audiosamplesize", 15) == 0) {
            m_audiosamplesize = numValue;
        } else if(strncmp(name, "audiocodecid", 12) == 0) {
            m_audiocodecid = numValue;
        } else if(strncmp(name, "stereo", 6) == 0) {
            m_stereo = boolValue;
        } else if(strncmp(name, "filesize",8) == 0){
        	m_fileSize = numValue;
        }

    }

	fprintf(stdout, "\nm_duration:%f\nm_width:%f\nm_height:%f\nm_framerate:%f\nm_videodatarate:%f\nm_audiodatarate:%f\nm_videocodecid:%f\nm_audiosamplerate:%f\nm_audiosamplesize:%f\nm_audiocodecid:%f\nm_stereo:%f\nm_fileSize:%f\n", m_duration,m_width,m_height,m_framerate,m_videodatarate,m_audiodatarate,m_videocodecid,m_audiosamplerate,m_audiosamplesize,m_audiocodecid,m_stereo,m_fileSize);

}
 
/**
 * Analysis FLV file
 * @param url    Location of input FLV file.
 */
 
int simplest_flv_parser(char *url){
 
	//whether output audio/video stream
	int output_a=1;
	int output_v=1;
	//-------------
	FILE *ifh=NULL,*vfh=NULL, *afh = NULL;
 
	//FILE *myout=fopen("output_log.txt","wb+");
	FILE *myout=stdout;
 
	FLV_HEADER flv;
	TAG_HEADER tagheader;
	uint previoustagsize, previoustagsize_z=0;
	uint ts=0, ts_new=0;
 
	ifh = fopen(url, "rb+");
	if ( ifh== NULL) {
		printf("Failed to open files!");
		return -1;
	}
 
	//FLV file header
	fread((char *)&flv,1,sizeof(FLV_HEADER),ifh);
 
	fprintf(myout,"============== FLV Header ==============\n");
	fprintf(myout,"Signature:  0x %c %c %c\n",flv.Signature[0],flv.Signature[1],flv.Signature[2]);
	fprintf(myout,"Version:    0x %X\n",flv.Version);
	fprintf(myout,"Flags  :    0x %X\n",flv.Flags);
	fprintf(myout,"HeaderSize: 0x %X\n",reverse_bytes((byte *)&flv.DataOffset, sizeof(flv.DataOffset)));
	fprintf(myout,"========================================\n");
 
	//move the file pointer to the end of the header
	fseek(ifh, reverse_bytes((byte *)&flv.DataOffset, sizeof(flv.DataOffset)), SEEK_SET);
 
	//process each tag
	do {
 
		previoustagsize = getw(ifh);
 
		fread((void *)&tagheader,sizeof(TAG_HEADER),1,ifh);
 
		//int temp_datasize1=reverse_bytes((byte *)&tagheader.DataSize, sizeof(tagheader.DataSize));
		int tagheader_datasize=tagheader.DataSize[0]*65536+tagheader.DataSize[1]*256+tagheader.DataSize[2];
		int tagheader_timestamp=tagheader.Timestamp[0]*65536+tagheader.Timestamp[1]*256+tagheader.Timestamp[2];
 
		char tagtype_str[10];
		switch(tagheader.TagType){
		case TAG_TYPE_AUDIO:sprintf(tagtype_str,"AUDIO");break;
		case TAG_TYPE_VIDEO:sprintf(tagtype_str,"VIDEO");break;
		case TAG_TYPE_SCRIPT:sprintf(tagtype_str,"SCRIPT");break;
		default:sprintf(tagtype_str,"UNKNOWN");break;
		}
		fprintf(myout,"[%6s] %6d %6d |",tagtype_str,tagheader_datasize,tagheader_timestamp);
 
		//if we are not past the end of file, process the tag
		if (feof(ifh)) {
			break;
		}
 
		//process tag by type
		switch (tagheader.TagType) {
 
		case TAG_TYPE_SCRIPT: {
			// int data_size=reverse_bytes((byte *)&tagheader.DataSize, sizeof(tagheader.DataSize))-1;
			// unsigned char *tag = new unsigned char[data_size + 11];
			// memset(tag,0,data_size+11);

			// memcpy(tag, &tagheader, 11);
			// int readBytes = fread(&tag[11], sizeof(unsigned char), data_size, ifh);
			// if(readBytes != data_size){
			// 	fprintf(stdout, "error \n");
			// 	return 0;
			// }

			// delete[] tag;
			// int data_size=reverse_bytes((byte *)&tagheader.DataSize, sizeof(tagheader.DataSize));
			// fseek(ifh,11 + data_size,SEEK_SET);
			// fseek(ifh, tagheader_datasize, SEEK_SET);
			int data_sizes = reverse_bytes((byte *)&tagheader.DataSize, sizeof(tagheader.DataSize));
			void* cs= calloc(data_sizes,1);
			int size = fread(cs,data_sizes,1,ifh);

			fprintf(myout,"\n");
			parseMeta((char*)cs);
			fprintf(myout,"\n");
			free(cs);
			break;
		}

		case TAG_TYPE_AUDIO:{ 
			char audiotag_str[100]={0};
			strcat(audiotag_str,"| ");
			char tagdata_first_byte;
			tagdata_first_byte=fgetc(ifh);
			int x=tagdata_first_byte&0xF0;
			x=x>>4;
			switch (x)
			{
			case 0:strcat(audiotag_str,"Linear PCM, platform endian");break;
			case 1:strcat(audiotag_str,"ADPCM");break;
			case 2:strcat(audiotag_str,"MP3");break;
			case 3:strcat(audiotag_str,"Linear PCM, little endian");break;
			case 4:strcat(audiotag_str,"Nellymoser 16-kHz mono");break;
			case 5:strcat(audiotag_str,"Nellymoser 8-kHz mono");break;
			case 6:strcat(audiotag_str,"Nellymoser");break;
			case 7:strcat(audiotag_str,"G.711 A-law logarithmic PCM");break;
			case 8:strcat(audiotag_str,"G.711 mu-law logarithmic PCM");break;
			case 9:strcat(audiotag_str,"reserved");break;
			case 10:strcat(audiotag_str,"AAC");break;
			case 11:strcat(audiotag_str,"Speex");break;
			case 14:strcat(audiotag_str,"MP3 8-Khz");break;
			case 15:strcat(audiotag_str,"Device-specific sound");break;
			default:strcat(audiotag_str,"UNKNOWN");break;
			}
			strcat(audiotag_str,"| ");
			x=tagdata_first_byte&0x0C;
			x=x>>2;
			switch (x)
			{
			case 0:strcat(audiotag_str,"5.5-kHz");break;
			case 1:strcat(audiotag_str,"1-kHz");break;
			case 2:strcat(audiotag_str,"22-kHz");break;
			case 3:strcat(audiotag_str,"44-kHz");break;
			default:strcat(audiotag_str,"UNKNOWN");break;
			}
			strcat(audiotag_str,"| ");
			x=tagdata_first_byte&0x02;
			x=x>>1;
			switch (x)
			{
			case 0:strcat(audiotag_str,"8Bit");break;
			case 1:strcat(audiotag_str,"16Bit");break;
			default:strcat(audiotag_str,"UNKNOWN");break;
			}
			strcat(audiotag_str,"| ");
			x=tagdata_first_byte&0x01;
			switch (x)
			{
			case 0:strcat(audiotag_str,"Mono");break;
			case 1:strcat(audiotag_str,"Stereo");break;
			default:strcat(audiotag_str,"UNKNOWN");break;
			}
			fprintf(myout,"%s",audiotag_str);
 
			//if the output file hasn't been opened, open it.
			if(output_a!=0&&afh == NULL){
				afh = fopen("output.mp3", "wb");
			}
 
			//TagData - First Byte Data
			int data_size=reverse_bytes((byte *)&tagheader.DataSize, sizeof(tagheader.DataSize))-1;
			if(output_a!=0){
				//TagData+1
				for (int i=0; i<data_size; i++)
					fputc(fgetc(ifh),afh);
 
			}else{
				for (int i=0; i<data_size; i++)
					fgetc(ifh);
			}
			break;
		}
		case TAG_TYPE_VIDEO:{
			char videotag_str[100]={0};
			strcat(videotag_str,"| ");
			char tagdata_first_byte;
			tagdata_first_byte=fgetc(ifh);
			int x=tagdata_first_byte&0xF0;
			x=x>>4;
			switch (x)
			{
			case 1:strcat(videotag_str,"key frame  ");break;
			case 2:strcat(videotag_str,"inter frame");break;
			case 3:strcat(videotag_str,"disposable inter frame");break;
			case 4:strcat(videotag_str,"generated keyframe");break;
			case 5:strcat(videotag_str,"video info/command frame");break;
			default:strcat(videotag_str,"UNKNOWN");break;
			}
			strcat(videotag_str,"| ");
			x=tagdata_first_byte&0x0F;
			switch (x)
			{
			case 1:strcat(videotag_str,"JPEG (currently unused)");break;
			case 2:strcat(videotag_str,"Sorenson H.263");break;
			case 3:strcat(videotag_str,"Screen video");break;
			case 4:strcat(videotag_str,"On2 VP6");break;
			case 5:strcat(videotag_str,"On2 VP6 with alpha channel");break;
			case 6:strcat(videotag_str,"Screen video version 2");break;
			case 7:strcat(videotag_str,"AVC");break;
			default:strcat(videotag_str,"UNKNOWN");break;
			}
			fprintf(myout,"%s",videotag_str);
 
			fseek(ifh, -1, SEEK_CUR);
			//if the output file hasn't been opened, open it.
			if (vfh == NULL&&output_v!=0) {
				//write the flv header (reuse the original file's hdr) and first previoustagsize
					vfh = fopen("output.flv", "wb");
					fwrite((char *)&flv,1, sizeof(flv),vfh);
					fwrite((char *)&previoustagsize_z,1,sizeof(previoustagsize_z),vfh);
			}
#if 0
			//Change Timestamp
			//Get Timestamp
			ts = reverse_bytes((byte *)&tagheader.Timestamp, sizeof(tagheader.Timestamp));
			ts=ts*2;
			//Writeback Timestamp
			ts_new = reverse_bytes((byte *)&ts, sizeof(ts));
			memcpy(&tagheader.Timestamp, ((char *)&ts_new) + 1, sizeof(tagheader.Timestamp));
#endif
 
 
			//TagData + Previous Tag Size
			int data_size=reverse_bytes((byte *)&tagheader.DataSize, sizeof(tagheader.DataSize))+4;
			if(output_v!=0){
				//TagHeader
				fwrite((char *)&tagheader,1, sizeof(tagheader),vfh);
				//TagData
				for (int i=0; i<data_size; i++)
					fputc(fgetc(ifh),vfh);
			}else{
				for (int i=0; i<data_size; i++)
					fgetc(ifh);
			}
			//rewind 4 bytes, because we need to read the previoustagsize again for the loop's sake
			fseek(ifh, -4, SEEK_CUR);
 
			break;
			}
		default:
		{
			fprintf(stdout, "error default\n");
						//skip the data of this tag
			fseek(ifh, reverse_bytes((byte *)&tagheader.DataSize, sizeof(tagheader.DataSize)), SEEK_CUR);

		} 
		}
 
		fprintf(myout,"\n");
 
	} while (!feof(ifh));
 
 
	//fcloseall();
 
	return 0;

}

int main(){
	simplest_flv_parser((char*)"/Users/wangdy/work/ffmpeg/flvparser/FlvParser2/FlvParser/test.flv");
	return 0;
}
