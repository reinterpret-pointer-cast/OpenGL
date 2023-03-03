struct _constants {
  static constexpr uint32_t opus_decode_sample_rate = 48000;
  struct Opus{
    static constexpr uint32_t SegmentFrameAmount20 = 960;
    static constexpr uint32_t SupportedChannels = 2;
    static constexpr uint32_t CacheDecoderPerChannel = 0x08;
    static constexpr uint32_t CacheSegmentAmount = 0x400;
    static constexpr uint32_t DecoderWarmUpAmount = 0x04;
  };

  static constexpr f32_t OneSampleTime = (f32_t)1 / opus_decode_sample_rate;

  static constexpr uint32_t CallFrameCount = 480;
  static constexpr uint32_t ChannelAmount = 2;
  static constexpr uint32_t FrameCacheAmount = Opus::SegmentFrameAmount20;
  static constexpr uint64_t FrameCacheTime = opus_decode_sample_rate / CallFrameCount * 1; // 1 second
};

typedef uint8_t _DecoderID_Size_t;
typedef uint16_t _CacheID_Size_t;

#define BLL_set_BaseLibrary 1
#define BLL_set_AreWeInsideStruct 1
#define BLL_set_prefix _DecoderList
#define BLL_set_type_node _DecoderID_Size_t
#define BLL_set_declare_NodeReference 1
#define BLL_set_declare_rest 0
#include _WITCH_PATH(BLL/BLL.h)

#define BLL_set_BaseLibrary 1
#define BLL_set_AreWeInsideStruct 1
#define BLL_set_prefix _CacheList
#define BLL_set_type_node _CacheID_Size_t
#define BLL_set_declare_NodeReference 1
#define BLL_set_declare_rest 0
#include _WITCH_PATH(BLL/BLL.h)

typedef _DecoderList_NodeReference_t _DecoderID_t;
typedef _CacheList_NodeReference_t _CacheID_t;

typedef uint32_t _SegmentID_t;

#define BLL_set_BaseLibrary 1
#define BLL_set_AreWeInsideStruct 1
#define BLL_set_prefix _DecoderList
#define BLL_set_type_node _DecoderID_Size_t
#define BLL_set_declare_NodeReference 0
#define BLL_set_declare_rest 1
#include _WITCH_PATH(BLL/BLL.h)

struct piece_t {
  uint8_t ChannelAmount;
  uint16_t BeginCut;
  uint32_t TotalSegments;
  uint8_t *SACData;
  uint64_t FrameAmount;

  uint64_t GetFrameAmount(){
    return FrameAmount - BeginCut;
  }
};

#define BLL_set_BaseLibrary 1
#define BLL_set_AreWeInsideStruct 1
#define BLL_set_prefix _CacheList
#define BLL_set_type_node _CacheID_Size_t
#define BLL_set_NodeData \
  f32_t Samples[_constants::FrameCacheAmount * _constants::ChannelAmount]; \
  _DecoderID_t DecoderID; \
  piece_t *piece; \
  _SegmentID_t SegmentID;
#define BLL_set_declare_NodeReference 0
#define BLL_set_declare_rest 1
#include _WITCH_PATH(BLL/BLL.h)

struct _DecoderHead_t{
  _CacheID_t CacheID;
};

#pragma pack(push, 1)

struct _SACHead_t{
  uint8_t Sign;
  uint16_t Checksum;
  uint8_t ChannelAmount;
  uint16_t BeginCut;
  uint16_t EndCut;
  uint32_t TotalSegments;
};

struct _SACSegment_t{
  uint32_t Offset;
  uint16_t Size;
  _CacheID_t CacheID;
};

#pragma pack(pop)

struct PropertiesSoundPlay_t {
  struct {
    uint32_t Loop : 1 = false;
    uint32_t FadeIn : 1 = false;
    uint32_t FadeOut : 1 = false;
  }Flags;
  f32_t FadeFrom;
  f32_t FadeTo;
};
struct PropertiesSoundStop_t {
  f32_t FadeOutTo = 0;
};

#define BLL_set_BaseLibrary 1
#define BLL_set_AreWeInsideStruct 1
#define BLL_set_StoreFormat 1
#define BLL_set_StoreFormat1_ElementPerBlock 0x100
#define BLL_set_prefix _PlayInfoList
#define BLL_set_type_node uint32_t
#define BLL_set_NodeData \
  piece_t *piece; \
  uint32_t GroupID; \
  uint32_t PlayID; \
  PropertiesSoundPlay_t properties; \
  uint64_t offset;
#define BLL_set_ResizeListAfterClear 1
#include _WITCH_PATH(BLL/BLL.h)
typedef _PlayInfoList_NodeReference_t SoundPlayID_t;

enum class _MessageType_t {
  SoundPlay,
  SoundStop,
  PauseGroup,
  ResumeGroup,
  StopGroup
};
struct _Message_t {
  _MessageType_t Type;
  union {
    struct {
      _PlayInfoList_NodeReference_t PlayInfoReference;
    }SoundPlay;
    struct {
      _PlayInfoList_NodeReference_t PlayInfoReference;
      PropertiesSoundStop_t Properties;
    }SoundStop;
    struct {
      uint32_t GroupID;
    }PauseGroup;
    struct {
      uint32_t GroupID;
    }ResumeGroup;
    struct {
      uint32_t GroupID;
    }StopGroup;
  }Data;
};

struct Process_t{
  #include "Process.h"
}Process;

struct Out_t{
  #if fan_audio_set_backend == 0
    #include "backend/uni/miniaudio/a.h"
  #elif fan_audio_set_backend == 1
    #include "backend/unix/linux/alsa/a.h"
  #else
    #error ?
  #endif
}Out;

audio_t(uint32_t GroupAmount) : Process(GroupAmount){
  
}
~audio_t(){

}

sint32_t piece_open(piece_t *piece, void *data, uintptr_t size) {
  return Process.piece_open(piece, data, size);
}
sint32_t piece_open(piece_t *piece, const fan::string &path) {
  return Process.piece_open(piece, path);
}

SoundPlayID_t SoundPlay(piece_t *piece, uint32_t GroupID, const PropertiesSoundPlay_t *Properties) {
  return Process.SoundPlay(piece, GroupID, Properties);
}
void SoundStop(_PlayInfoList_NodeReference_t PlayInfoReference, const PropertiesSoundStop_t *Properties) {
  Process.SoundStop(PlayInfoReference, Properties);
}

void PauseGroup(uint32_t GroupID) {
  Process.PauseGroup(GroupID);
}
void ResumeGroup(uint32_t GroupID) {
  Process.ResumeGroup(GroupID);
}

void StopGroup(uint32_t GroupID) {
  Process.StopGroup(GroupID);
}

void SetVolume(f32_t Volume){
  Out.SetVolume(Volume);
}
