#define BLL_set_BaseLibrary 1
//#define BLL_set_namespace
#define BLL_set_CPP_ConstructDestruct
#define BLL_set_prefix pipeline_list
#define BLL_set_type_node uint8_t
#define BLL_set_node_data \
  VkPipelineLayout pipeline_layout; \
	VkPipeline pipeline;

#define BLL_set_Link 1
#define BLL_set_AreWeInsideStruct 1