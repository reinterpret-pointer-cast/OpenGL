#include fan_pch

int main(int argc, char** argv) {
  if (argc < 2) {
    fan::throw_error("usage: TexturePackCompiled");
  }
  loco_t loco;
  //
  model_maker_t mm;
  mm.open(argv[1]);
  //if(argc == 3){
    //mm.load(argv[2]);
  //}
  //else{
    mm.load("model.fmm");//
  //}//
  loco_t::vfi_t v;
  loco_t::vfi_t::shape_id_t nt;
  loco_t::cid_nt_t n = nt;

  constexpr bool x = fan_has_function(loco_t::vfi_t, push_back(n, {}));
  loco_t::shapes_t::sprite_t::properties_t p;
  //
  //
  loco.set_vsync(false);
  //pile->loco.window.set_max_fps(165);
  //pile->loco.window.set_max_fps(5);
  loco.loop([&] {
    loco.get_fps();
  });


  // pile->close();

  return 0;
}