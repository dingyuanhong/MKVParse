#pragma once

#include "MKVValue.h"

//https://blog.csdn.net/lxmnet123/article/details/10741935
//https://blog.csdn.net/finewind/article/details/41807863
//https://www.matroska.org/technical/specs/index.html#simpleblock_structure

#define EBML 0x1A45DFA3
#define SEGMENT 0x18538067
//识别文件的信息
#define SEGMENT_INFORMATION 0x1549a966
#define TRACKS 0x1654ae6b
//Track包含了音视频的基本信息
//音视频解码器类型、视频分辨率、音频采样率等
#define TRACK_ENTRY 0xae
#define CLUSTER 0x1f43b675

static MKVValue mkv_list[] = {
	{ EBML ,"EBML",0},
	{ 0x4286 ,"EBMLVersion",INT_TYPE },
	{ 0x42f7 ,"EBMLReadVersion",INT_TYPE },
	{ 0x42f2 ,"EBMLMaxIDLength",INT_TYPE },
	{ 0x42f3 ,"EBMLMaxSizeLength",INT_TYPE },
	{ 0x4282 ,"DocType",STRING_TYPE },
	{ 0x4287 ,"DocTypeVersion",INT_TYPE },
	{ 0x4285 ,"DocTypeReadVersion",INT_TYPE },

	/*{ 0xbf ,"CRC-32",HEXSTRING_TYPE },
	{ 0xec ,"Void",STRING_TYPE },*/

	{ SEGMENT ,"Segment",0 },
	{ SEGMENT_INFORMATION ,"Info",0 },
	{ 0x73a4 ,"SegmentUID",HEXSTRING_TYPE },
	{ 0x2ad7b1 ,"TimecodeScale",INT_TYPE },
	{ 0x7ba9 ,"Title",STRING_TYPE },
	{ 0x4d80 ,"MuxingApp",STRING_TYPE },
	{ 0x5741 ,"WritingApp",STRING_TYPE },
	//{ 0x4489 ,"Duration",-1 },

	{ 0x114d9b74 ,"SeekHead	",0 },
	{ 0x4dbb ,"Seek",0 },
	{ 0x53ab ,"SeekID",INT_TYPE },
	{ 0x53ac ,"SeekPosition",INT_TYPE },

	{ TRACKS ,"Tracks",0 },
	{ TRACK_ENTRY ,"TrackEntry",0 },
	{ 0xd7 ,"TrackNumber",INT_TYPE },
	{ 0x73c5 ,"TrackUID",HEXSTRING_TYPE },
	{ 0x83 ,"TrackType",INT_TYPE },
	{ 0x536e ,"Name",STRING_TYPE },
	{ 0x86 ,"CodecID",STRING_TYPE },
	{ 0xe0 ,"Video",HEXSTRING_TYPE },
	{ 0x63a2 ,"CodecPrivate",HEXSTRING_TYPE },
	{ 0x9c ,"FlagLacing",-1 },
	{ 0x22b59c ,"Language",INT_TYPE },
	{ 0x23e383 ,"DefaultDuration",INT_TYPE },
	{ 0xe1 ,"Audio",HEXSTRING_TYPE },

	{ 0x1254c367 ,"Tags",0 },
	{ 0x7373 ,"Tag",0 },
	{ 0x63c0 ,"Targets",0 },
	{ 0x63c5 ,"TrackUID",HEXSTRING_TYPE },
	{ 0x63c4 ,"ChapterUID",HEXSTRING_TYPE },
	{ 0x63c6 ,"AttachmentUID",HEXSTRING_TYPE },
	{ 0x67C8 ,"SimpleTag",0 },
	{ 0x45a3 ,"TagName",STRING_TYPE },
	{ 0x4487 ,"TagString",STRING_TYPE },
	{ 0x4485 ,"TagBinary",HEXSTRING_TYPE },

	{ CLUSTER ,"Cluster",0 },
	{ 0xe7 ,"Timecode",INT_TYPE },
	{ 0xa7 ,"Position",INT_TYPE },
	{ 0xa3 ,"SimpleBlock",BLOCK_TYPE },
	

	{ 0x1c53bb6b ,"Cues",0 },
	{ 0xbb ,"CuePoint",0 },
	{ 0xb3,"CueTime",INT_TYPE},
	{ 0xb7,"CueTrackPositions",0 },
	{ 0xf7,"CueTrack",INT_TYPE },
	{ 0xf1,"CueClusterPosition",INT_TYPE },
	{ 0xf0,"CueRelativePosition",INT_TYPE },
};

static inline MKVValue *getMKVValue_(VINT id)
{
	for (int i = 0; i < sizeof(mkv_list) / sizeof(mkv_list[0]); i++)
	{
		MKVValue & value = mkv_list[i];
		if (id == value.id) {
			value.free = 0;
			return &value;
		}
	}
	return NULL;
}