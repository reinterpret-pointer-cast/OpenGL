typedef struct{
  f32_t Sp0Y;
  f32_t Sp0X;
  uint8_t Stage;
  union{
    struct{
      f32_t CircleOffsetY;
      f32_t CircleOffsetX;
    }Corner;
    struct{
      f32_t SPp0Y;
      f32_t SPp0X;
    }Side;
  }StageData;
}__ETC_BCOL_PP(CPCU_Rectangle_Circle_t);

bool
__ETC_BCOL_PP(CPCU_Rectangle_Circle_IsThereCollision)
(
  __ETC_BCOL_PP(CPCU_Rectangle_Circle_t) *Data
){
  return Data->Stage != (uint8_t)-1;
}

void
__ETC_BCOL_PP(CPCU_Rectangle_Circle_Pre)
(
  f32_t p0Y,
  f32_t p0X,
  f32_t p0SizeY,
  f32_t p0SizeX,
  f32_t p1Y,
  f32_t p1X,
  f32_t p1Size,
  __ETC_BCOL_PP(CPCU_Rectangle_Circle_t) *Data
){
  Data->Sp0Y = p0Y - p1Y;
  Data->Sp0X = p0X - p1X;
  f32_t SPp0Y = __absf(Data->Sp0Y);
  f32_t SPp0X = __absf(Data->Sp0X);
  if(SPp0X > p0SizeX && SPp0Y > p0SizeY){
    f32_t CornerY = SPp0Y - p0SizeY;
    f32_t CornerX = SPp0X - p0SizeX;
    f32_t Divider = __hypotenuse(CornerY, CornerX);
    Data->StageData.Corner.CircleOffsetY = CornerY / Divider;
    Data->StageData.Corner.CircleOffsetX = CornerX / Divider;
    f32_t DDY = CornerY - Data->StageData.Corner.CircleOffsetY * p1Size;
    f32_t DDX = CornerX - Data->StageData.Corner.CircleOffsetX * p1Size;
    if(DDY > 0 || DDX > 0){
      Data->Stage = (uint8_t)-1;
    }
    else{
      Data->Stage = 0;
    }
  }
  else{
    Data->StageData.Side.SPp0Y = SPp0Y;
    Data->StageData.Side.SPp0X = SPp0X;
    f32_t CombinedSizeY = p0SizeY + p1Size;
    f32_t CombinedSizeX = p0SizeX + p1Size;
    if(SPp0Y > CombinedSizeY || SPp0X > CombinedSizeX){
      Data->Stage = (uint8_t)-1;
    }
    else{
      Data->Stage = 1;
    }
  }
}

void
__ETC_BCOL_PP(CPCU_Rectangle_Circle_Solve)
(
  f32_t p0Y,
  f32_t p0X,
  f32_t p0SizeY,
  f32_t p0SizeX,
  f32_t p1Y,
  f32_t p1X,
  f32_t p1Size,
  __ETC_BCOL_PP(CPCU_Rectangle_Circle_t) *Data,
  f32_t *op0Y,
  f32_t *op0X,
  f32_t *oDirectionY,
  f32_t *oDirectionX
){
  switch(Data->Stage){
    case 0:{
      *op0Y = p1Y + __copysignf(p0SizeY + Data->StageData.Corner.CircleOffsetY * p1Size, Data->Sp0Y);
      *op0X = p1X + __copysignf(p0SizeX + Data->StageData.Corner.CircleOffsetX * p1Size, Data->Sp0X);
      *oDirectionY = __copysignf(Data->StageData.Corner.CircleOffsetY, Data->Sp0Y);
      *oDirectionX = __copysignf(Data->StageData.Corner.CircleOffsetX, Data->Sp0X);
      break;
    }
    case 1:{
      if(Data->StageData.Side.SPp0X <= p0SizeX){
        *op0Y = p1Y + __copysignf(p0SizeY + p1Size, Data->Sp0Y);
        *op0X = p0X;
        *oDirectionY = __copysignf(1, Data->Sp0Y);
        *oDirectionX = 0;
      }
      else if(Data->StageData.Side.SPp0Y <= p0SizeY){
        *op0Y = p0Y;
        *op0X = p1X + __copysignf(p0SizeX + p1Size, Data->Sp0X);
        *oDirectionY = 0;
        *oDirectionX = __copysignf(1, Data->Sp0X);
      }
      break;
    }
  }
}