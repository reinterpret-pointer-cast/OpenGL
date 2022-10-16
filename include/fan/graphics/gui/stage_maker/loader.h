#ifndef stage_loader_path
#define stage_loader_path
#endif

struct stage_loader_t {
	loco_t* get_loco() {
		pile_t* pile = OFFSETLESS(this, pile_t, stage_loader);
		return &pile->loco;
	}

	#include _FAN_PATH(CONCAT2(stage_loader_path, stages/stage.h))

	void open() {
		
	}
	void close() {

	}

	template <typename stage_t>
	void push_and_open_stage(const stage_common_t::open_properties_t& op) {
		stage_t stage;
		stage.lib_open(get_loco(), &stage.stage_common, op);
		stage.stage_common.open();
		auto& stages = pile_t::stage_loader_t::stage::stages;
		stages.push_back(new stage_common_t(stage.stage_common));
	}
	void erase_stage(uint32_t id) {
		auto& stages = pile_t::stage_loader_t::stage::stages;
		if (id >= stages.size()) {
			return;
		}

		auto loco = get_loco();

		auto it = stages[id]->instances.GetNodeFirst();
		while (it != stages[id]->instances.dst) {
			auto node = stages[id]->instances[it];
			loco->button.erase(&node.cid);
			it = it.Next(&stages[id]->instances);
		}
		stages[id]->instances.close();
		delete stages[id];
		stages.erase(stages.begin() + id);
	}
};

#undef stage_loader_path