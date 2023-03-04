TH_mutex_t PlayInfoListMutex;
_PlayInfoList_t PlayInfoList;

uint32_t GroupAmount;
struct _Group_t {
  _PlayInfoList_NodeReference_t FirstReference;
  _PlayInfoList_NodeReference_t LastReference;
}*GroupList;

struct _Play_t {
  _PlayInfoList_NodeReference_t Reference;
};
VEC_t PlayList;

TH_mutex_t MessageQueueListMutex;
VEC_t MessageQueueList;

_DecoderList_t DecoderList[_constants::Opus::SupportedChannels];

_CacheList_t CacheList;

sint32_t Open(uint32_t GroupAmount) {
  TH_mutex_init(&this->PlayInfoListMutex);
  this->PlayInfoList.Open();

  this->GroupAmount = GroupAmount;
  this->GroupList = (_Group_t *)A_resize(0, sizeof(_Group_t) * this->GroupAmount);
  for (uint32_t i = 0; i < this->GroupAmount; i++) {
    this->GroupList[i].FirstReference = this->PlayInfoList.NewNodeLast_alloc();
    this->GroupList[i].LastReference = this->PlayInfoList.NewNodeLast_alloc();
  }

  VEC_init(&this->PlayList, sizeof(_Play_t), A_resize);

  TH_mutex_init(&this->MessageQueueListMutex);
  VEC_init(&this->MessageQueueList, sizeof(_Message_t), A_resize);

  for(uint32_t Channel = 0; Channel < _constants::Opus::SupportedChannels; Channel++){
    uint32_t size = sizeof(_DecoderHead_t);
    size += opus_decoder_get_size(Channel + 1);
    _DecoderList_t *dl = &this->DecoderList[Channel];
    dl->Open(size);
    for(uint32_t i = 0; i < _constants::Opus::CacheDecoderPerChannel; i++){
      _DecoderList_NodeReference_t r = dl->NewNodeLast_alloc();
      uint8_t *dld = (uint8_t *)(*dl)[r];

      _DecoderHead_t *DecoderHead = (_DecoderHead_t *)dld;
      DecoderHead->CacheID = _CacheList_gnric();

      OpusDecoder *od = (OpusDecoder *)&dld[sizeof(_DecoderHead_t)];
      int re = opus_decoder_init(od, 48000, Channel + 1);
      if(re != OPUS_OK){
        fan::throw_error("opus_decoder_init");
      }
    }
  }

  this->CacheList.Open();
  for(uint32_t i = 0; i < _constants::Opus::CacheSegmentAmount; i++){
    _CacheList_NodeReference_t r = this->CacheList.NewNodeLast_alloc();
    auto Data = &this->CacheList[r];
    Data->SegmentID = (_SegmentID_t)-1;
    Data->DecoderID = _DecoderList_gnric();
  }

  return 0;
}
void Close() {
  this->PlayInfoList.Close();
  A_resize(this->GroupList, 0);
  VEC_free(&this->PlayList);
  VEC_free(&this->MessageQueueList);
}

void _DecodeCopy(
  uint8_t ChannelAmount,
  f32_t *F2_20,
  f32_t *Output
){
  switch(ChannelAmount){
    case 1:{
      /* need manuel interleave */
      for(uint32_t i = 0; i < _constants::Opus::SegmentFrameAmount20; i++){
        Output[i * 2 + 0] = F2_20[i];
        Output[i * 2 + 1] = F2_20[i];
      }
      break;
    }
    case 2:{
      MEM_copy(
        F2_20,
        Output,
        _constants::Opus::SegmentFrameAmount20 * 2 * sizeof(f32_t));
      break;
    }
  }
}

_CacheID_t _GetCacheID(piece_t *piece, _SegmentID_t SegmentID){
  _CacheID_t clid = this->CacheList.GetNodeLast();
  auto Cache = &this->CacheList[clid];
  if(Cache->SegmentID != (_SegmentID_t)-1){
    ((_SACSegment_t *)Cache->piece->SACData)[Cache->SegmentID].CacheID = _CacheList_gnric();
    _DecoderID_t dlid = Cache->DecoderID;
    if(_DecoderList_inric(dlid) == false){
      _DecoderList_t *dl = &this->DecoderList[Cache->piece->ChannelAmount - 1];
      _DecoderHead_t *DecoderHead = (_DecoderHead_t *)(*dl)[dlid];
      DecoderHead->CacheID = _CacheList_gnric();
      dl->ReLinkAsLast(dlid);
    }
  }
  Cache->piece = piece;
  Cache->SegmentID = SegmentID;
  ((_SACSegment_t *)piece->SACData)[SegmentID].CacheID = clid;
  return clid;
}

void _WarmUpDecoder(piece_t *piece, _SegmentID_t SegmentID, OpusDecoder *od){
  opus_decoder_ctl(od, OPUS_RESET_STATE);

  _SegmentID_t fsid = SegmentID - _constants::Opus::DecoderWarmUpAmount;
  if(SegmentID < _constants::Opus::DecoderWarmUpAmount){
    fsid = 0;
  }

  while(fsid != SegmentID){
    _SACSegment_t *SACSegment = &((_SACSegment_t *)piece->SACData)[fsid];

    f32_t F2_20[_constants::Opus::SegmentFrameAmount20 * 5 * 2];
    sint32_t oerr = opus_decode_float(
      od,
      &piece->SACData[SACSegment->Offset],
      SACSegment->Size,
      F2_20,
      _constants::Opus::SegmentFrameAmount20 * 5,
      0);

    if(oerr != _constants::Opus::SegmentFrameAmount20){
      fan::print("help", oerr);
      fan::throw_error("a");
    }

    fsid++;
  }
}

_DecoderID_t _GetDecoderID(piece_t *piece, _SegmentID_t SegmentID){
  _DecoderList_t *DecoderList = &this->DecoderList[piece->ChannelAmount - 1];

  if(SegmentID != 0){
    _CacheID_t pCacheID = ((_SACSegment_t *)piece->SACData)[SegmentID - 1].CacheID;
    if(_CacheList_inric(pCacheID) == false){
      auto pCache = &this->CacheList[pCacheID];
      _DecoderID_t DecoderID = pCache->DecoderID;
      if(_DecoderList_inric(DecoderID) == false){
        pCache->DecoderID = _DecoderList_gnric();
        return DecoderID;
      }
    }
  }

  _DecoderID_t DecoderID = DecoderList->GetNodeLast();

  _DecoderHead_t *DecoderHead = (_DecoderHead_t *)(*DecoderList)[DecoderID];

  _CacheID_t dclid = DecoderHead->CacheID;
  if(_CacheList_inric(dclid) == false){
    auto dCache = &this->CacheList[dclid];
    dCache->DecoderID = _DecoderList_gnric();
  }

  OpusDecoder *od = (OpusDecoder *)&DecoderHead[1];

  this->_WarmUpDecoder(piece, SegmentID, od);

  return DecoderID;
}

void
_LinkDecoderCache(
  _DecoderList_t *DecoderList,
  _DecoderID_t DecoderID,
  _CacheID_t CacheID
){
  _DecoderHead_t *DecoderHead = (_DecoderHead_t *)(*DecoderList)[DecoderID];
  DecoderHead->CacheID = CacheID;
  auto Cache = &this->CacheList[CacheID];
  Cache->DecoderID = DecoderID;
}

void _DecodeSegment(piece_t *piece, _SegmentID_t SegmentID){
  _DecoderList_t *DecoderList = &this->DecoderList[piece->ChannelAmount - 1];

  _DecoderID_t DecoderID = this->_GetDecoderID(piece, SegmentID);
  DecoderList->ReLinkAsFirst(DecoderID);

  _CacheID_t CacheID = this->_GetCacheID(piece, SegmentID);

  this->_LinkDecoderCache(DecoderList, DecoderID, CacheID);

  OpusDecoder *od = (OpusDecoder *)&((_DecoderHead_t *)(*DecoderList)[DecoderID])[1];

  _SACSegment_t *SACSegment = &((_SACSegment_t *)piece->SACData)[SegmentID];

  f32_t F2_20[_constants::Opus::SegmentFrameAmount20 * 5 * 2];
  sint32_t oerr = opus_decode_float(
    od,
    &piece->SACData[SACSegment->Offset],
    SACSegment->Size,
    F2_20,
    _constants::Opus::SegmentFrameAmount20 * 5,
    0);

  if(oerr != _constants::Opus::SegmentFrameAmount20){
    fan::print("help", oerr);
    fan::throw_error("a");
  }

  auto Cache = &this->CacheList[CacheID];
  _DecodeCopy(piece->ChannelAmount, F2_20, Cache->Samples);
}

void _GetFrames(piece_t *piece, uint64_t Offset, f32_t **FramePointer, uint32_t *FrameAmount) {
  Offset += piece->BeginCut;
  _SegmentID_t SegmentID = Offset / _constants::FrameCacheAmount;
  uint32_t PieceCacheMod = Offset % _constants::FrameCacheAmount;
  _CacheID_t *CacheID = &((_SACSegment_t *)piece->SACData)[SegmentID].CacheID;
  if(_CacheList_inric(*CacheID) == true){
    _DecodeSegment(piece, SegmentID);
  }
  this->CacheList.ReLinkAsFirst(*CacheID);
  auto Cache = &this->CacheList[*CacheID];
  *FramePointer = &Cache->Samples[PieceCacheMod * _constants::ChannelAmount];
  *FrameAmount = _constants::FrameCacheAmount - PieceCacheMod;
}

void _AddSoundToPlay(_PlayInfoList_NodeReference_t PlayInfoReference) {
  VEC_handle0(&this->PlayList, 1);
  uint32_t PlayID = this->PlayList.Current - 1;
  _Play_t *Play = &((_Play_t *)this->PlayList.ptr)[PlayID];
  Play->Reference = PlayInfoReference;
  auto PlayInfo = &this->PlayInfoList[PlayInfoReference];
  #if fan_debug >= 0
    if (PlayInfo->PlayID != (uint32_t)-1) {
      /* trying play sound that already playing */
      fan::throw_error("fan_debug");
    }
  #endif
  PlayInfo->PlayID = PlayID;
}
void
_RemoveFromPlayList
(
  uint32_t PlayID
){
  /* super fast remove */
  ((_Play_t *)this->PlayList.ptr)[PlayID] = ((_Play_t *)this->PlayList.ptr)[--this->PlayList.Current];

  /* moved one needs update */
  _PlayInfoList_NodeReference_t PlayInfoReference = ((_Play_t *)this->PlayList.ptr)[PlayID].Reference;
  auto PlayInfo = &this->PlayInfoList[PlayInfoReference];
  PlayInfo->PlayID = PlayID;
}
void
_RemoveFromPlayInfoList
(
  _PlayInfoList_NodeReference_t PlayInfoReference,
  const PropertiesSoundStop_t *Properties
){
  auto PlayInfo = &this->PlayInfoList[PlayInfoReference];
  if (PlayInfo->PlayID == (uint32_t)-1) {
    /* properties are ignored */
    TH_lock(&this->PlayInfoListMutex);
    this->PlayInfoList.unlrec(PlayInfoReference);
    TH_unlock(&this->PlayInfoListMutex);
  }
  else {
    auto PlayInfo = &this->PlayInfoList[PlayInfoReference];
    if (Properties->FadeOutTo != 0) {
      PropertiesSoundPlay_t* PropertiesPlay = &PlayInfo->properties;
      if (PropertiesPlay->Flags.FadeIn) {
        PropertiesPlay->Flags.FadeIn = false;
        PropertiesPlay->Flags.FadeOut = true;
        f32_t CurrentVolume = PropertiesPlay->FadeFrom / PropertiesPlay->FadeTo;
        PropertiesPlay->FadeTo = Properties->FadeOutTo;
        PropertiesPlay->FadeFrom = ((f32_t)1 - CurrentVolume) * PropertiesPlay->FadeTo;
      }
      else {
        PropertiesPlay->FadeFrom = 0;
        PropertiesPlay->FadeTo = Properties->FadeOutTo;
        PropertiesPlay->Flags.FadeOut = true;
      }
    }
    else {
      this->_RemoveFromPlayList(PlayInfo->PlayID);
      TH_lock(&this->PlayInfoListMutex);
      this->PlayInfoList.unlrec(PlayInfoReference);
      TH_unlock(&this->PlayInfoListMutex);
    }
  }
}

sint32_t piece_open(piece_t *piece, void *data, uintptr_t size) {
  #define tbs(p0) \
    if(DataIndex + (p0) > size){ \
      return -1; \
    }

  uint8_t *Data = (uint8_t *)data;
  uintptr_t DataIndex = 0;

  _SACHead_t *SACHead;
  tbs(sizeof(_SACHead_t));
  SACHead = (_SACHead_t *)&Data[DataIndex];
  DataIndex += sizeof(_SACHead_t);

  if(SACHead->Sign != 0xff){
    return -1;
  }

  piece->ChannelAmount = SACHead->ChannelAmount;

  piece->BeginCut = SACHead->BeginCut;

  tbs(SACHead->TotalSegments);
  uint8_t *SACSegmentSizes = &Data[DataIndex];

  piece->TotalSegments = 0;
  uint64_t TotalOfSACSegmentSizes = 0;
  for(uint32_t i = 0; i < SACHead->TotalSegments; i++){
    TotalOfSACSegmentSizes += SACSegmentSizes[i];
    if(SACSegmentSizes[i] == 0xff){
      continue;
    }
    piece->TotalSegments++;
  }

  uint64_t PieceSegmentSizes = piece->TotalSegments * sizeof(_SACSegment_t);
  {
    DataIndex += SACHead->TotalSegments;
    uint64_t LeftSize = size - DataIndex;
    if(TotalOfSACSegmentSizes != LeftSize){
      fan::throw_error("corrupted sac?");
    }
    uintptr_t ssize = PieceSegmentSizes + LeftSize;
    piece->SACData = A_resize(0, ssize);
    MEM_copy(&Data[DataIndex], &piece->SACData[PieceSegmentSizes], LeftSize);
  }

  {
    uint16_t BeforeSum = 0;
    uint32_t psi = 0;
    uint32_t DataOffset = PieceSegmentSizes;
    for(uint32_t i = 0; i < SACHead->TotalSegments; i++){
      BeforeSum += SACSegmentSizes[i];
      if(SACSegmentSizes[i] == 0xff){
        continue;
      }
      ((_SACSegment_t *)piece->SACData)[psi].Offset = DataOffset;
      ((_SACSegment_t *)piece->SACData)[psi].Size = BeforeSum;
      ((_SACSegment_t *)piece->SACData)[psi].CacheID = _CacheList_gnric();
      DataOffset += BeforeSum;
      BeforeSum = 0;
      psi++;
    }
  }

  #undef tbs

  piece->FrameAmount = (uint64_t)piece->TotalSegments * _constants::Opus::SegmentFrameAmount20;
  if(SACHead->EndCut >= piece->FrameAmount){
    return -1;
  }
  piece->FrameAmount -= SACHead->EndCut;

  return 0;
}
sint32_t piece_open(piece_t *piece, const fan::string &path) {
  sint32_t err;

  fan::string sd;
  err = fan::io::file::read(path, &sd);
  if(err != 0){
    return err;
  }

  err = piece_open(piece, sd.data(), sd.size());
  if(err != 0){
    return err;
  }

  return 0;
}

void PauseGroup(uint32_t GroupID) {
  TH_lock(&this->MessageQueueListMutex);
  VEC_handle0(&this->MessageQueueList, 1);
  _Message_t *Message = &((_Message_t *)this->MessageQueueList.ptr)[this->MessageQueueList.Current - 1];
  Message->Type = _MessageType_t::PauseGroup;
  Message->Data.PauseGroup.GroupID = GroupID;
  TH_unlock(&this->MessageQueueListMutex);
}
void ResumeGroup(uint32_t GroupID) {
  TH_lock(&this->MessageQueueListMutex);
  VEC_handle0(&this->MessageQueueList, 1);
  _Message_t *Message = &((_Message_t*)this->MessageQueueList.ptr)[this->MessageQueueList.Current - 1];
  Message->Type = _MessageType_t::ResumeGroup;
  Message->Data.ResumeGroup.GroupID = GroupID;
  TH_unlock(&this->MessageQueueListMutex);
}

void StopGroup(uint32_t GroupID) {

}

SoundPlayID_t SoundPlay(piece_t *piece, uint32_t GroupID, const PropertiesSoundPlay_t *Properties) {
  #if fan_debug >= 0
    if (GroupID >= this->GroupAmount) {
      fan::throw_error("fan_debug");
    }
  #endif
  TH_lock(&this->PlayInfoListMutex);
  _PlayInfoList_NodeReference_t PlayInfoReference = this->PlayInfoList.NewNode();
  auto PlayInfo = &this->PlayInfoList[PlayInfoReference];
  PlayInfo->piece = piece;
  PlayInfo->GroupID = GroupID;
  PlayInfo->PlayID = (uint32_t)-1;
  PlayInfo->properties = *Properties;
  PlayInfo->offset = 0;
  this->PlayInfoList.linkPrev(this->GroupList[GroupID].LastReference, PlayInfoReference);
  TH_unlock(&this->PlayInfoListMutex);

  TH_lock(&this->MessageQueueListMutex);
  VEC_handle0(&this->MessageQueueList, 1);
  _Message_t* Message = &((_Message_t *)this->MessageQueueList.ptr)[this->MessageQueueList.Current - 1];
  Message->Type = _MessageType_t::SoundPlay;
  Message->Data.SoundPlay.PlayInfoReference = PlayInfoReference;
  TH_unlock(&this->MessageQueueListMutex);

  return PlayInfoReference;
}

void SoundStop(_PlayInfoList_NodeReference_t PlayInfoReference, const PropertiesSoundStop_t *Properties) {
  TH_lock(&this->MessageQueueListMutex);
  VEC_handle0(&this->MessageQueueList, 1);
  _Message_t* Message = &((_Message_t *)this->MessageQueueList.ptr)[this->MessageQueueList.Current - 1];
  Message->Type = _MessageType_t::SoundStop;
  Message->Data.SoundStop.PlayInfoReference = PlayInfoReference;
  Message->Data.SoundStop.Properties = *Properties;
  TH_unlock(&this->MessageQueueListMutex);
}

void _DataCallback(f32_t *Output) {
  if (this->MessageQueueList.Current) {
    TH_lock(&this->MessageQueueListMutex);
    for (uint32_t i = 0; i < this->MessageQueueList.Current; i++) {
      _Message_t* Message = &((_Message_t*)this->MessageQueueList.ptr)[i];
      switch (Message->Type) {
        case _MessageType_t::SoundPlay: {
          this->_AddSoundToPlay(Message->Data.SoundPlay.PlayInfoReference);
          break;
        }
        case _MessageType_t::SoundStop: {
          this->_RemoveFromPlayInfoList(Message->Data.SoundStop.PlayInfoReference, &Message->Data.SoundStop.Properties);
          break;
        }
        case _MessageType_t::PauseGroup: {
          uint32_t GroupID = Message->Data.PauseGroup.GroupID;
          _PlayInfoList_NodeReference_t LastPlayInfoReference = this->GroupList[GroupID].LastReference;
          _PlayInfoList_NodeReference_t PlayInfoReference = this->GroupList[GroupID].FirstReference;
          TH_lock(&this->PlayInfoListMutex);
          PlayInfoReference = PlayInfoReference.Next(&this->PlayInfoList);
          TH_unlock(&this->PlayInfoListMutex);
          while (PlayInfoReference != LastPlayInfoReference) {
            auto PlayInfoNode = this->PlayInfoList.GetNodeByReference(PlayInfoReference);
            if (PlayInfoNode->data.PlayID != (uint32_t)-1) {
              this->_RemoveFromPlayList(PlayInfoNode->data.PlayID);
              PlayInfoNode->data.PlayID = (uint32_t)-1;
            }
            PlayInfoReference = PlayInfoNode->NextNodeReference;
          }
          break;
        }
        case _MessageType_t::ResumeGroup: {
          uint32_t GroupID = Message->Data.PauseGroup.GroupID;
          _PlayInfoList_NodeReference_t LastPlayInfoReference = this->GroupList[GroupID].LastReference;
          _PlayInfoList_NodeReference_t PlayInfoReference = this->GroupList[GroupID].FirstReference;
          TH_lock(&this->PlayInfoListMutex);
          PlayInfoReference = PlayInfoReference.Next(&this->PlayInfoList);
          TH_unlock(&this->PlayInfoListMutex);
          while (PlayInfoReference != LastPlayInfoReference) {
            auto PlayInfoNode = this->PlayInfoList.GetNodeByReference(PlayInfoReference);
            if (PlayInfoNode->data.PlayID == (uint32_t)-1) {
              this->_AddSoundToPlay(PlayInfoReference);
            }
            PlayInfoReference = PlayInfoNode->NextNodeReference;
          }
          break;
        }
        case _MessageType_t::StopGroup: {
          uint32_t GroupID = Message->Data.PauseGroup.GroupID;
          _PlayInfoList_NodeReference_t LastPlayInfoReference = this->GroupList[GroupID].LastReference;
          _PlayInfoList_NodeReference_t PlayInfoReference = this->GroupList[GroupID].FirstReference;
          TH_lock(&this->PlayInfoListMutex);
          PlayInfoReference = PlayInfoReference.Next(&this->PlayInfoList);
          while (PlayInfoReference != LastPlayInfoReference) {
            auto PlayInfoNode = this->PlayInfoList.GetNodeByReference(PlayInfoReference);
            PropertiesSoundStop_t Properties;
            this->_RemoveFromPlayInfoList(PlayInfoReference, &Properties);
            PlayInfoReference = PlayInfoNode->NextNodeReference;
          }
          TH_unlock(&this->PlayInfoListMutex);
        }
      }
    }
    this->MessageQueueList.Current = 0;
    TH_unlock(&this->MessageQueueListMutex);
  }
  for (uint32_t PlayID = 0; PlayID < this->PlayList.Current;) {
    _Play_t *Play = &((_Play_t *)this->PlayList.ptr)[PlayID];
    _PlayInfoList_NodeReference_t PlayInfoReference = Play->Reference;
    auto PlayInfo = &this->PlayInfoList[PlayInfoReference];
    piece_t *piece = PlayInfo->piece;
    PropertiesSoundPlay_t *Properties = &PlayInfo->properties;
    uint32_t OutputIndex = 0;
    uint32_t CanBeReadFrameCount;
    struct {
      f32_t FadePerFrame;
    }CalculatedVariables;
    gt_ReOffset:
    CanBeReadFrameCount = piece->GetFrameAmount() - PlayInfo->offset;
    if (CanBeReadFrameCount > _constants::CallFrameCount - OutputIndex) {
      CanBeReadFrameCount = _constants::CallFrameCount - OutputIndex;
    }
    if (Properties->Flags.FadeIn || Properties->Flags.FadeOut) {
      f32_t TotalFade = Properties->FadeTo - Properties->FadeFrom;
      f32_t CurrentFadeTime = (f32_t)CanBeReadFrameCount / _constants::opus_decode_sample_rate;
      if (TotalFade < CurrentFadeTime) {
        CanBeReadFrameCount = TotalFade * _constants::opus_decode_sample_rate;
        if (CanBeReadFrameCount == 0) {
          if(Properties->Flags.FadeIn == true){
            Properties->Flags.FadeIn = false;
          }
          else if(Properties->Flags.FadeOut == true){
            PropertiesSoundStop_t PropertiesSoundStop;
            this->_RemoveFromPlayInfoList(PlayInfoReference, &PropertiesSoundStop);
            continue;
          }
        }
      }
      CalculatedVariables.FadePerFrame = (f32_t)1 / (Properties->FadeTo * _constants::opus_decode_sample_rate);
    }
    while (CanBeReadFrameCount != 0) {
      f32_t* FrameCachePointer;
      uint32_t FrameCacheAmount;
      this->_GetFrames(
        piece,
        PlayInfo->offset,
        &FrameCachePointer,
        &FrameCacheAmount);
      if (FrameCacheAmount > CanBeReadFrameCount) {
        FrameCacheAmount = CanBeReadFrameCount;
      }
      if (Properties->Flags.FadeIn) {
        f32_t CurrentVolume = Properties->FadeFrom / Properties->FadeTo;
        for (uint32_t i = 0; i < FrameCacheAmount; i++) {
          for (uint32_t iChannel = 0; iChannel < _constants::ChannelAmount; iChannel++) {
            ((f32_t*)Output)[(OutputIndex + i) * _constants::ChannelAmount + iChannel] += FrameCachePointer[i * _constants::ChannelAmount + iChannel] * CurrentVolume;
          }
          CurrentVolume += CalculatedVariables.FadePerFrame;
        }
        Properties->FadeFrom += (f32_t)FrameCacheAmount / _constants::opus_decode_sample_rate;
      }
      else if (Properties->Flags.FadeOut) {
        f32_t CurrentVolume = Properties->FadeFrom / Properties->FadeTo;
        CurrentVolume = (f32_t)1 - CurrentVolume;
        for (uint32_t i = 0; i < FrameCacheAmount; i++) {
          for (uint32_t iChannel = 0; iChannel < _constants::ChannelAmount; iChannel++) {
            ((f32_t*)Output)[(OutputIndex + i) * _constants::ChannelAmount + iChannel] += FrameCachePointer[i * _constants::ChannelAmount + iChannel] * CurrentVolume;
          }
          CurrentVolume -= CalculatedVariables.FadePerFrame;
        }
        Properties->FadeFrom += (f32_t)FrameCacheAmount / _constants::opus_decode_sample_rate;
      }
      else {
        std::transform(
          &FrameCachePointer[0],
          &FrameCachePointer[FrameCacheAmount * _constants::ChannelAmount],
          &((f32_t*)Output)[OutputIndex * _constants::ChannelAmount],
          &((f32_t*)Output)[OutputIndex * _constants::ChannelAmount],
          std::plus<f32_t>{});
      }
      PlayInfo->offset += FrameCacheAmount;
      OutputIndex += FrameCacheAmount;
      CanBeReadFrameCount -= FrameCacheAmount;
    }

    #define JumpIfNeeded() \
      if(OutputIndex != _constants::CallFrameCount) { goto gt_ReOffset; }

    if (PlayInfo->offset == piece->GetFrameAmount()) {
      if (Properties->Flags.Loop == true) {
        PlayInfo->offset = 0;
      }
    }
    if (Properties->Flags.FadeIn) {
      if (Properties->FadeFrom >= Properties->FadeTo) {
        Properties->Flags.FadeIn = false;
      }
      JumpIfNeeded();
    }
    else if (Properties->Flags.FadeOut) {
      if (Properties->FadeFrom >= Properties->FadeTo) {
        PropertiesSoundStop_t PropertiesSoundStop;
        this->_RemoveFromPlayInfoList(PlayInfoReference, &PropertiesSoundStop);
        continue;
      }
      JumpIfNeeded();
    }
    else{
      if (PlayInfo->offset == piece->GetFrameAmount()) {
        PropertiesSoundStop_t PropertiesSoundStop;
        this->_RemoveFromPlayInfoList(PlayInfoReference, &PropertiesSoundStop);
        continue;
      }
      JumpIfNeeded();
    }

    #undef JumpIfNeeded

    PlayID++;
  }
}