void
__ETC_BCOL_PP(CPC_Rectangle_Square)
(
  f32_t p0Y,
  f32_t p0X,
  f32_t p0SizeY,
  f32_t p0SizeX,
  f32_t p1Y,
  f32_t p1X,
  f32_t p1Size,
  f32_t *op0Y,
  f32_t *op0X,
  f32_t *oDirectionY,
  f32_t *oDirectionX
){
  f32_t Sp0Y = p0Y - p1Y;
  f32_t Sp0X = p0X - p1X;
  f32_t SPp0Y = __absf(Sp0Y);
  f32_t SPp0X = __absf(Sp0X);
  f32_t DiffY = SPp0Y / (p1Size + p0SizeY);
  f32_t DiffX = SPp0X / (p1Size + p0SizeX);
  if(DiffY > DiffX){
    *op0Y = p1Y + __copysignf(p1Size + p0SizeY, Sp0Y);
    *op0X = p0X;
    *oDirectionY = __copysignf(1, Sp0Y);
    *oDirectionX = 0;
  }
  else{
    *op0Y = p0Y;
    *op0X = p1X + __copysignf(p1Size + p0SizeX, Sp0X);
    *oDirectionY = 0;
    *oDirectionX = __copysignf(1, Sp0X);
  }
}