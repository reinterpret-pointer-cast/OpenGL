#define BLL_set_CPP_ConstructDestruct
#define BLL_set_CPP_Node_ConstructDestruct
#include _FAN_PATH(fan_bll_present.h)
#define BLL_set_namespace fan::vulkan
#define BLL_set_prefix theme_list
#define BLL_set_type_node uint8_t
#define BLL_set_NodeData loco_t::theme_t* theme_id;
#define BLL_set_Link 0
#define BLL_set_AreWeInsideStruct 0
#define BLL_set_NodeReference_Overload_Declare \
  theme_list_NodeReference_t(loco_t::theme_t*);