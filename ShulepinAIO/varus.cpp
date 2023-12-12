#include "sdk.hpp"
#include "varus.h"
#include "utils.h"
#include "spell.h"

namespace varus
{
	script::spell* q;
	script::spell* w;
	script::spell* e;
	script::spell* r;
	
	constexpr uint32_t w_debuff = -1823502239;
	constexpr uint32_t r_debuff = 1155701612;
	
	bool should_force_q;
	bool executing_kill_logic;
	bool e_combo_global_check;
	bool e_harass_global_check;
	int executing_kill_logic_t;
	game_object* force_q_target;
	
	std::unordered_map<uint32_t, int> hero_debuff_stacks;
	std::unordered_map<uint32_t, float> q_damage_cache;
	std::unordered_map<uint32_t, float> w_damage_cache;
	std::unordered_map<uint32_t, float> e_damage_cache;
	std::unordered_map<uint32_t, float> r_damage_cache;
	std::unordered_map<uint32_t, float> q_full_damage_cache;
	std::unordered_map<uint32_t, float> w_full_damage_cache;
	std::unordered_map<uint32_t, float> stack_damage_cache;
	
	void __fastcall game_update();
	void __fastcall present();
	void __fastcall buff_gain(game_object* object, buff_instance* buff);
	void __fastcall buff_loss(game_object* object, buff_instance* buff);
	void __fastcall execute_cast(game_object* object, spell_cast* cast);
	bool __fastcall before_attack(orb_sdk::event_data* data);
	
	namespace config::combo
	{
		bool use_q;
		bool use_w;
		bool use_e;
		bool use_r;
		int priority;
	}

	namespace config::combo::q
	{
		int min_stacks;
		int full_charge_mode;
		bool ignore_full_charge;
		float ignore_full_charge_range;
		bool force_q_charge;
		bool force_q_charge_with_w;
	}

	namespace config::combo::q::prediction
	{
		bool fast;
		float max_range;
		int hitchance;
		float extra_charge_range;
	}

	namespace config::combo::q::ignore
	{
		std::unordered_map<std::string, bool> list;
	}

	namespace config::combo::q::mana_manager
	{
		float value;
	}

	namespace config::combo::w
	{
		int mode;
		float min_hp;
	}

	namespace config::combo::e
	{
		int min_stacks;
		bool block_e_as;
		float block_e_as_slider;
	}

	namespace config::combo::e::prediction
	{
		bool fast;
		float max_range;
		int hitchance;
	}

	namespace config::combo::e::ignore
	{
		std::unordered_map<std::string, bool> list;
	}

	namespace config::combo::e::mana_manager
	{
		float value;
	}

	namespace config::combo::r::cast_conditions
	{
		bool use_on_cc;
		bool use_on_killable;
		int x_aa;
	}
	
	namespace config::combo::r::prediction
	{
		float max_range;
		int hitchance;
	}

	namespace config::combo::r::ignore
	{
		std::unordered_map<std::string, bool> list;
	}

	namespace config::combo::r::mana_manager
	{
		float value;
	}

	namespace config::harass
	{
		bool use_q;
		bool use_w;
		bool use_e;
	}

	namespace config::harass::q
	{
		int min_stacks;
		int full_charge_mode;
		bool ignore_full_charge;
		float ignore_full_charge_range;
	}

	namespace config::harass::q::prediction
	{
		bool fast;
		float max_range;
		int hitchance;
		float extra_charge_range;
	}

	namespace config::harass::q::ignore
	{
		std::unordered_map<std::string, bool> list;
	}

	namespace config::harass::q::mana_manager
	{
		float value;
	}

	namespace config::harass::w
	{
		int mode;
		float min_hp;
	}

	namespace config::harass::e
	{
		int min_stacks;
	}

	namespace config::harass::e::prediction
	{
		bool fast;
		float max_range;
		int hitchance;
	}

	namespace config::harass::e::ignore
	{
		std::unordered_map<std::string, bool> list;
	}

	namespace config::harass::e::mana_manager
	{
		float value;
	}

	namespace config::auto_kill
	{
		bool enabled;
		bool health_pred;
		bool use_q;
		bool use_w;
		bool use_e;
	}

	namespace config::auto_kill::q::prediction
	{
		float max_range;
		int hitchance;
		float extra_charge_range;
	}

	namespace config::auto_kill::q::ignore
	{
		std::unordered_map<std::string, bool> list;
	}

	namespace config::auto_kill::q::mana_manager
	{
		float value;
	}

	namespace config::auto_kill::e::prediction
	{
		float max_range;
		int hitchance;
		float extra_charge_range;
	}

	namespace config::auto_kill::e::ignore
	{
		std::unordered_map<std::string, bool> list;
	}

	namespace config::auto_kill::e::mana_manager
	{
		float value;
	}

	namespace config::semi_manual::q
	{
		bool full_charge;
		bool value;
		std::string key;
		uint32_t info;
	}

	namespace config::semi_manual::q::prediction
	{
		bool fast;
		float max_range;
		int hitchance;
		float extra_charge_range;
	}

	namespace config::semi_manual::q::ignore
	{
		std::unordered_map<std::string, bool> list;
	}

	namespace config::semi_manual::q::mana_manager
	{
		float value;
	}

	namespace config::semi_manual::r
	{
		bool value;
		std::string key;
		uint32_t info;
	}

	namespace config::semi_manual::r::prediction
	{
		bool fast;
		float max_range;
		int hitchance;
		float extra_charge_range;
	}

	namespace config::semi_manual::r::ignore
	{
		std::unordered_map<std::string, bool> list;
	}

	namespace config::semi_manual::r::mana_manager
	{
		float value;
	}

	namespace config::visuals
	{
		bool draw_range;
		bool draw_range_ready;
		bool draw_q;
		bool draw_e;
		bool draw_r;
		uint32_t draw_q_color;
		uint32_t draw_e_color;
		uint32_t draw_r_color;
		bool draw_q_damage;
		bool draw_w_damage;
		bool draw_e_damage;
		bool draw_r_damage;
		bool draw_aa_damage;
	}
	
	void register_spells()
	{
		// Spells
		q = new script::spell(0, 1595.f);
		w = new script::spell(1, 0.f);
		e = new script::spell(2, 925.f);
		r = new script::spell(3, 1370.f);

		// Set Q data
		q->set_charged(true);
		q->set_min_range(825.f);
		q->set_max_range(1595.f);
		q->add_charged_buff_mapping("Varus", -1173990314);
		q->set_skillshot(pred_sdk::spell_type::linear, pred_sdk::targetting_type::edge_to_edge, 0.f, 70.f, 1900.f, { pred_sdk::collision_type::yasuo_wall });

		// Set E data
		e->set_skillshot(pred_sdk::spell_type::circular, pred_sdk::targetting_type::center, 0.2419f, 300.f, FLT_MAX, {});

		// Set R data
		r->set_skillshot(pred_sdk::spell_type::linear, pred_sdk::targetting_type:: edge_to_edge, 0.2419f, 120.f, 1500.f, { pred_sdk::collision_type::yasuo_wall });
	}

	void create_menu()
	{
		const auto& player = g_sdk->object_manager->get_local_player();
		const auto config = g_sdk->menu_manager->add_category("shulepin_aio_varus","ShulepinAIO - " + player->get_char_name());
		{
			const auto combo = config->add_sub_category("shulepin_aio_varus_combo", "Combo Settings");
			{
				combo->add_separator();
				combo->add_label("                          |> Spell Manager <|                          ");
				combo->add_separator();
				combo->add_checkbox("shulepin_aio_varus_combo_use_q", "Use [Q]", true, [](const bool value) { config::combo::use_q = value; });
				combo->add_checkbox("shulepin_aio_varus_combo_use_w", "Use [W]", true, [](const bool value) { config::combo::use_w = value; });
				combo->add_checkbox("shulepin_aio_varus_combo_use_e", "Use [E]", true, [](const bool value) { config::combo::use_e = value; });
				combo->add_checkbox("shulepin_aio_varus_combo_use_r", "Use [R]", true, [](const bool value) { config::combo::use_r = value; });
				combo->add_separator();
				combo->add_label("                          |> Spell Settings <|                          ");
				combo->add_separator();
				const auto combo_q = combo->add_sub_category("shulepin_aio_varus_combo_q", "[Q] Settings");
				{
					combo_q->add_checkbox("shulepin_aio_varus_combo_q_force_q_charge", "Force Q charge in root", false,
						[](const bool value) { config::combo::q::force_q_charge = value; });
					combo_q->add_checkbox("shulepin_aio_varus_combo_q_force_q_charge_with_w", " ^- Include W", false,
						[](const bool value) { config::combo::q::force_q_charge_with_w = value; });
					combo_q->add_separator();
					combo_q->add_slider_int("shulepin_aio_varus_combo_q_min_stacks", "Min. stacks", 0, 3, 1, 3,
						[](const int value) { config::combo::q::min_stacks = value; });
					combo_q->add_combo("shulepin_aio_varus_combo_q_full_charge", "Full charge",
						{ "If killable", "Always", "Never" }, 0, [](const int value)
						{
							config::combo::q::full_charge_mode = value;
						});
					combo_q->add_checkbox("shulepin_aio_varus_combo_q_ignore_full_charge",
						" v- Ignore full charge in melee range", true, [](const bool value)
						{
							config::combo::q::ignore_full_charge = value;
						});
					combo_q->add_slider_float("shulepin_aio_varus_combo_q_ignore_full_charge_range", "Melee range", 0.f,
						800.f, 10.f, 400.f, [](const float value)
						{
							config::combo::q::ignore_full_charge_range = value;
						});
					combo_q->add_separator();
					const auto combo_q_prediction = combo_q->add_sub_category("shulepin_aio_varus_combo_q_prediction", "Prediction Settings");
					{
						combo_q_prediction->add_checkbox("shulepin_aio_varus_combo_q_fast_prediction",
							"Use fast prediction in AA range", true, [](const bool value)
							{
								config::combo::q::prediction::fast = value;
							});
						combo_q_prediction->add_slider_float("shulepin_aio_varus_combo_q_max_range", "Max range", q->get_max_range() / 2, q->get_max_range(), 10.f, q->get_max_range() - 50.f, [](const float value) {config::combo::q::prediction::max_range = value; });
						combo_q_prediction->add_slider_int("shulepin_aio_varus_combo_q_hit_chance", "Hitchance", 0, 100, 1, 0, [](const int value) { config::combo::q::prediction::hitchance = value; });
						combo_q_prediction->add_slider_float("shulepin_aio_varus_combo_q_extra_charge_range", "Extra charge range",
							0.f, 200.f, 10.f, 0.f, [](const float value)
							{
								config::combo::q::prediction::extra_charge_range = value;
							});
					}
					const auto combo_q_black_list = combo_q->add_sub_category("shulepin_aio_varus_combo_q_black_list", "Ignore List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							const auto& char_name = hero->get_char_name();
							combo_q_black_list->add_checkbox("shulepin_aio_varus_combo_q_black_list_" + char_name, char_name, false, [char_name](const bool value) { config::combo::q::ignore::list[char_name] = value; });
						}
					}
					const auto combo_q_mana_manager = combo_q->add_sub_category("shulepin_aio_varus_combo_q_mana_manager", "Mana Manager");
					{
						combo_q_mana_manager->add_slider_float("shulepin_aio_varus_combo_q_mana_manager_value", "Min. mana (%)", 0.f, 100.f, 1.f, 0.f, [](const float value) { config::combo::q::mana_manager::value = value; });
					}
				}
				const auto combo_w = combo->add_sub_category("shulepin_aio_varus_combo_w", "[W] Settings");
				{
					combo_w->add_combo("shulepin_aio_varus_combo_w_mode", "Usage",
						{ "Min. HP < x%", "Never" }, 0, [](const int value)
						{
							config::combo::w::mode = value;
						});
					combo_w->add_slider_float("shulepin_aio_varus_combo_w_min_hp", "Min. HP < x%", 0.f, 100.f, 1, 50.f,
						[](const float value) { config::combo::w::min_hp = value; });
				}
				const auto combo_e = combo->add_sub_category("shulepin_aio_varus_combo_e", "[E] Settings");
				{
					combo_e->add_slider_int("shulepin_aio_varus_combo_e_min_stacks", "Min. stacks", 0, 3, 1, 3,
						[](const int value) { config::combo::e::min_stacks = value; });
					combo_e->add_checkbox("shulepin_aio_varus_combo_e_block_as", "Block E if Attack Speed > X%", false,
						[](const bool value) { config::combo::e::block_e_as = value; });
					combo_e->add_slider_float("shulepin_aio_varus_combo_e_block_as_slider",
						"Attack Speed (100 = 1.00 AS)", 100.f, 500.f, 50.f, 200.f,
						[](const float value) { config::combo::e::block_e_as_slider = value; });
					combo_e->add_separator();
					const auto combo_e_prediction = combo_e->add_sub_category("shulepin_aio_varus_combo_e_prediction", "Prediction Settings");
					{
						combo_e_prediction->add_checkbox("shulepin_aio_varus_combo_e_fast_prediction",
							"Use fast prediction in AA range", true, [](const bool value)
							{
								config::combo::e::prediction::fast = value;
							});
						combo_e_prediction->add_slider_float("shulepin_aio_varus_combo_e_max_range", "Max range", e->get_range() / 2.f, e->get_range(), 10.f, e->get_range(), [](const float value) {config::combo::e::prediction::max_range = value; });
						combo_e_prediction->add_slider_int("shulepin_aio_varus_combo_e_hit_chance", "Hitchance", 0, 100, 1, 0, [](const int value) { config::combo::e::prediction::hitchance = value; });
					}
					const auto combo_e_black_list = combo_e->add_sub_category("shulepin_aio_varus_combo_e_black_list", "Ignore List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							const auto& char_name = hero->get_char_name();
							combo_e_black_list->add_checkbox("shulepin_aio_varus_combo_e_black_list_" + char_name, char_name, false, [char_name](const bool value) { config::combo::e::ignore::list[char_name] = value; });
						}
					}
					const auto combo_e_mana_manager = combo_e->add_sub_category("shulepin_aio_varus_combo_e_mana_manager", "Mana Manager");
					{
						combo_e_mana_manager->add_slider_float("shulepin_aio_varus_combo_e_mana_manager_value", "Min. mana (%)", 0.f, 100.f, 1.f, 0.f, [](const float value) { config::combo::e::mana_manager::value = value; });
					}
				}
				const auto combo_r = combo->add_sub_category("shulepin_aio_varus_combo_r", "[R] Settings");
				{
					const auto combo_r_cast_conditions = combo_r->add_sub_category("shulepin_aio_varus_combo_r_cast_conditions", "Cast Conditions");
					{
						combo_r_cast_conditions->add_checkbox("shulepin_aio_varus_combo_r_cast_conditions_use_on_cc", "Use on CC", true, [](const bool value) { config::combo::r::cast_conditions::use_on_cc = value; });
						combo_r_cast_conditions->add_checkbox("shulepin_aio_varus_combo_r_cast_conditions_use_on_killable", "Use on killable target (Q + W + E + R damage)", true, [](const bool value) { config::combo::r::cast_conditions::use_on_killable = value; });
						combo_r_cast_conditions->add_slider_int("shulepin_aio_varus_combo_r_cast_conditions_include_x_aa", " ^- Include X autoattacks", 0, 10, 1, 3, [](const int value){ config::combo::r::cast_conditions::x_aa = value; });
					}
	
					const auto combo_r_prediction = combo_r->add_sub_category("shulepin_aio_varus_combo_r_prediction", "Prediction Settings");
					{
						combo_r_prediction->add_slider_float("shulepin_aio_varus_combo_r_max_range", "Max range", r->get_range() / 2.f, r->get_range(), 10.f, r->get_range() - 200.f, [](const float value) {config::combo::r::prediction::max_range = value; });
						combo_r_prediction->add_slider_int("shulepin_aio_varus_combo_r_hit_chance", "Hitchance", 0, 100, 1, 0, [](const int value) { config::combo::r::prediction::hitchance = value; });
					}
					const auto combo_r_black_list = combo_r->add_sub_category("shulepin_aio_varus_combo_r_black_list", "Ignore List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							const auto& char_name = hero->get_char_name();
							combo_r_black_list->add_checkbox("shulepin_aio_varus_combo_r_black_list_" + char_name, char_name, false, [char_name](const bool value) { config::combo::r::ignore::list[char_name] = value; });
						}
					}
					const auto combo_r_mana_manager = combo_r->add_sub_category("shulepin_aio_varus_combo_r_mana_manager", "Mana Manager");
					{
						combo_r_mana_manager->add_slider_float("shulepin_aio_varus_combo_r_mana_manager_value", "Min. mana (%)", 0.f, 100.f, 1.f, 0.f, [](const float value) { config::combo::r::mana_manager::value = value; });
					}
				}
				combo->add_separator();
				combo->add_label("                          |> Other Settings <|                          ");
				combo->add_separator();
				combo->add_combo("shulepin_aio_varus_combo_priority", "", { "Combo Priority -> Q", "Combo Priority -> E" }, 1,
					[](const int value) { config::combo::priority = value; });
			}
			const auto harass = config->add_sub_category("shulepin_aio_varus_harass", "Harass Settings");
			{
				harass->add_separator();
				harass->add_label("                          |> Spell Manager <|                          ");
				harass->add_separator();
				harass->add_checkbox("shulepin_aio_varus_harass_use_q", "Use [Q]", true, [](const bool value) { config::harass::use_q = value; });
				harass->add_checkbox("shulepin_aio_varus_harass_use_w", "Use [W]", true, [](const bool value) { config::harass::use_w = value; });
				harass->add_checkbox("shulepin_aio_varus_harass_use_e", "Use [E]", true, [](const bool value) { config::harass::use_e = value; });
				harass->add_separator();
				harass->add_label("                          |> Spell Settings <|                          ");
				harass->add_separator();
				const auto harass_q = harass->add_sub_category("shulepin_aio_varus_harass_q", "[Q] Settings");
				{
					harass_q->add_slider_int("shulepin_aio_varus_harass_q_min_stacks", "Min. stacks", 0, 3, 1, 3,
						[](const int value) { config::harass::q::min_stacks = value; });
					harass_q->add_combo("shulepin_aio_varus_harass_q_full_charge", "Full charge",
						{ "If killable", "Always", "Never" }, 0, [](const int value)
						{
							config::harass::q::full_charge_mode = value;
						});
					harass_q->add_checkbox("shulepin_aio_varus_harass_q_ignore_full_charge",
						" v- Ignore full charge in melee range", true, [](const bool value)
						{
							config::harass::q::ignore_full_charge = value;
						});
					harass_q->add_slider_float("shulepin_aio_varus_harass_q_ignore_full_charge_range", "Melee range", 0.f,
						800.f, 10.f, 400.f, [](const float value)
						{
							config::harass::q::ignore_full_charge_range = value;
						});
					harass_q->add_separator();
					const auto harass_q_prediction = harass_q->add_sub_category("shulepin_aio_varus_harass_q_prediction", "Prediction Settings");
					{
						harass_q_prediction->add_checkbox("shulepin_aio_varus_harass_q_fast_prediction",
							"Use fast prediction in AA range", true, [](const bool value)
							{
								config::harass::q::prediction::fast = value;
							});
						harass_q_prediction->add_slider_float("shulepin_aio_varus_harass_q_max_range", "Max range", q->get_max_range() / 2, q->get_max_range(), 10.f, q->get_max_range() - 50.f, [](const float value) {config::harass::q::prediction::max_range = value; });
						harass_q_prediction->add_slider_int("shulepin_aio_varus_harass_q_hit_chance", "Hitchance", 0, 100, 1, 0, [](const int value) { config::harass::q::prediction::hitchance = value; });
						harass_q_prediction->add_slider_float("shulepin_aio_varus_harass_q_extra_charge_range", "Extra charge range",
							0.f, 200.f, 10.f, 0.f, [](const float value)
							{
								config::harass::q::prediction::extra_charge_range = value;
							});
					}
					const auto harass_q_black_list = harass_q->add_sub_category("shulepin_aio_varus_harass_q_black_list", "Ignore List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							const auto& char_name = hero->get_char_name();
							harass_q_black_list->add_checkbox("shulepin_aio_varus_harass_q_black_list_" + char_name, char_name, false, [char_name](const bool value) { config::harass::q::ignore::list[char_name] = value; });
						}
					}
					const auto harass_q_mana_manager = harass_q->add_sub_category("shulepin_aio_varus_harass_q_mana_manager", "Mana Manager");
					{
						harass_q_mana_manager->add_slider_float("shulepin_aio_varus_harass_q_mana_manager_value", "Min. mana (%)", 0.f, 100.f, 1.f, 0.f, [](const float value) { config::harass::q::mana_manager::value = value; });
					}
				}
				const auto harass_w = harass->add_sub_category("shulepin_aio_varus_harass_w", "[W] Settings");
				{
					harass_w->add_combo("shulepin_aio_varus_harass_w_mode", "Usage",
						{ "Min. HP < x%", "Never" }, 0, [](const int value)
						{
							config::harass::w::mode = value;
						});
					harass_w->add_slider_float("shulepin_aio_varus_harass_w_min_hp", "Min. HP < x%", 0.f, 100.f, 1, 50.f,
						[](const float value) { config::harass::w::min_hp = value; });
				}
				const auto harass_e = harass->add_sub_category("shulepin_aio_varus_harass_e", "[E] Settings");
				{
					harass_e->add_slider_int("shulepin_aio_varus_harass_e_min_stacks", "Min. stacks", 0, 3, 1, 3,
						[](const int value) { config::harass::e::min_stacks = value; });
					harass_e->add_separator();
					const auto harass_e_prediction = harass_e->add_sub_category("shulepin_aio_varus_harass_e_prediction", "Prediction Settings");
					{
						harass_e_prediction->add_checkbox("shulepin_aio_varus_harass_e_fast_prediction",
							"Use fast prediction in AA range", true, [](const bool value)
							{
								config::harass::e::prediction::fast = value;
							});
						harass_e_prediction->add_slider_float("shulepin_aio_varus_harass_e_max_range", "Max range", 500.f, 1000.f, 10.f, 1000.f, [](const float value) {config::harass::e::prediction::max_range = value; });
						harass_e_prediction->add_slider_int("shulepin_aio_varus_harass_e_hit_chance", "Hitchance", 0, 100, 1, 0, [](const int value) { config::harass::e::prediction::hitchance = value; });
					}
					const auto harass_e_black_list = harass_e->add_sub_category("shulepin_aio_varus_harass_e_black_list", "Ignore List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							const auto& char_name = hero->get_char_name();
							harass_e_black_list->add_checkbox("shulepin_aio_varus_harass_e_black_list_" + char_name, char_name, false, [char_name](const bool value) { config::harass::e::ignore::list[char_name] = value; });
						}
					}
					const auto harass_e_mana_manager = harass_e->add_sub_category("shulepin_aio_varus_harass_e_mana_manager", "Mana Manager");
					{
						harass_e_mana_manager->add_slider_float("shulepin_aio_varus_harass_e_mana_manager_value", "Min. mana (%)", 0.f, 100.f, 1.f, 0.f, [](const float value) { config::harass::e::mana_manager::value = value; });
					}
				}
			}
			const auto auto_kill = config->add_sub_category("shulepin_aio_varus_auto_kill", "Auto Kill Settings");
			{
				auto_kill->add_separator();
				auto_kill->add_label("                            |> Auto Kill <|                            ");
				auto_kill->add_separator();
				auto_kill->add_checkbox("shulepin_aio_varus_auto_kill_enabled", "Enabled", true, [](const bool value) { config::auto_kill::enabled = value; });
				auto_kill->add_checkbox("shulepin_aio_varus_auto_kill_health_pred", "Use health prediction", true, [](const bool value) { config::auto_kill::health_pred = value; });
				auto_kill->add_separator();
				auto_kill->add_label("                          |> Spell Manager <|                          ");
				auto_kill->add_separator();
				auto_kill->add_checkbox("shulepin_aio_varus_auto_kill_use_q", "Use [Q]", true, [](const bool value) { config::auto_kill::use_q = value; });
				auto_kill->add_checkbox("shulepin_aio_varus_auto_kill_use_w", "Use [W]", true, [](const bool value) { config::auto_kill::use_w = value; });
				auto_kill->add_checkbox("shulepin_aio_varus_auto_kill_use_e", "Use [E]", true, [](const bool value) { config::auto_kill::use_e = value; });
				auto_kill->add_separator();
				auto_kill->add_label("                          |> Spell Settings <|                          ");
				auto_kill->add_separator();
				const auto auto_kill_q = auto_kill->add_sub_category("shulepin_aio_varus_auto_kill_q", "[Q] Settings");
				{
					const auto auto_kill_q_prediction = auto_kill_q->add_sub_category("shulepin_aio_varus_auto_kill_q_prediction", "Prediction Settings");
					{
						auto_kill_q_prediction->add_slider_float("shulepin_aio_varus_auto_kill_q_max_range", "Max range", q->get_max_range() / 2, q->get_max_range(), 10.f, q->get_max_range() - 50.f, [](const float value) {config::auto_kill::q::prediction::max_range = value; });
						auto_kill_q_prediction->add_slider_int("shulepin_aio_varus_auto_kill_q_hit_chance", "Hitchance", 0, 100, 1, 0, [](const int value) { config::auto_kill::q::prediction::hitchance = value; });
						auto_kill_q_prediction->add_slider_float("shulepin_aio_varus_auto_kill_q_extra_charge_range", "Extra charge range",
							0.f, 200.f, 10.f, 0.f, [](const float value)
							{
								config::auto_kill::q::prediction::extra_charge_range = value;
							});
					}
					const auto auto_kill_q_black_list = auto_kill_q->add_sub_category("shulepin_aio_varus_auto_kill_q_black_list", "Ignore List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							const auto& char_name = hero->get_char_name();
							auto_kill_q_black_list->add_checkbox("shulepin_aio_varus_auto_kill_q_black_list_" + char_name, char_name, false, [char_name](const bool value) { config::auto_kill::q::ignore::list[char_name] = value; });
						}
					}
					const auto auto_kill_q_mana_manager = auto_kill_q->add_sub_category("shulepin_aio_varus_auto_kill_q_mana_manager", "Mana Manager");
					{
						auto_kill_q_mana_manager->add_slider_float("shulepin_aio_varus_auto_kill_q_mana_manager_value", "Min. mana (%)", 0.f, 100.f, 1.f, 0.f, [](const float value) { config::auto_kill::q::mana_manager::value = value; });
					}
				}
				const auto auto_kill_e = auto_kill->add_sub_category("shulepin_aio_varus_auto_kill_e", "[E] Settings");
				{
					const auto auto_kill_e_prediction = auto_kill_e->add_sub_category("shulepin_aio_varus_auto_kill_e_prediction", "Prediction Settings");
					{
						auto_kill_e_prediction->add_slider_float("shulepin_aio_varus_auto_kill_e_max_range", "Max range", e->get_range() / 2, e->get_range(), 10.f, e->get_range(), [](const float value) {config::auto_kill::e::prediction::max_range = value; });
						auto_kill_e_prediction->add_slider_int("shulepin_aio_varus_auto_kill_e_hit_chance", "Hitchance", 0, 100, 1, 0, [](const int value) { config::auto_kill::e::prediction::hitchance = value; });
					}
					const auto auto_kill_e_black_list = auto_kill_e->add_sub_category("shulepin_aio_varus_auto_kill_e_black_list", "Ignore List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							const auto& char_name = hero->get_char_name();
							auto_kill_e_black_list->add_checkbox("shulepin_aio_varus_auto_kill_e_black_list_" + char_name, char_name, false, [char_name](const bool value) { config::auto_kill::e::ignore::list[char_name] = value; });
						}
					}
					const auto auto_kill_e_mana_manager = auto_kill_e->add_sub_category("shulepin_aio_varus_auto_kill_e_mana_manager", "Mana Manager");
					{
						auto_kill_e_mana_manager->add_slider_float("shulepin_aio_varus_auto_kill_e_mana_manager_value", "Min. mana (%)", 0.f, 100.f, 1.f, 0.f, [](const float value) { config::auto_kill::e::mana_manager::value = value; });
					}
				}
			}
			const auto semi_manual = config->add_sub_category("shulepin_aio_varus_semi_manual", "Semi Manual Settings");
			{
				semi_manual->add_separator();
				semi_manual->add_label("                          |> Spell Keybinds <|                          ");
				semi_manual->add_separator();
				semi_manual->add_hotkey("shulepin_aio_varus_semi_manual_q_key", "Semi Q", 0x51, false, false,
					[](std::string* key, bool value)
					{
						config::semi_manual::q::key = *key;
						config::semi_manual::q::value = value;
					});
				config::semi_manual::q::info = sdk::infotab->add_text(
					{"Semi Q [" + config::semi_manual::q::key + "]"}, []() -> infotab_sdk::text_entry
					{
						infotab_sdk::text_entry entry{};

						if (config::semi_manual::q::value)
						{
							entry.text = "ON";
							entry.color = 0xFF00FF00;
						}
						else
						{
							entry.text = "OFF";
							entry.color = 0xFFFF0000;
						}

						return entry;
					});
				semi_manual->add_hotkey("shulepin_aio_varus_semi_manual_r_key", "Semi R", 0x5A, false, false,
					[](std::string* key, bool value)
					{
						config::semi_manual::r::key = *key;
						config::semi_manual::r::value = value;
					});
				config::semi_manual::r::info = sdk::infotab->add_text(
					{"Semi R [" + config::semi_manual::r::key + "]"}, []() -> infotab_sdk::text_entry
					{
						infotab_sdk::text_entry entry{};

						if (config::semi_manual::r::value)
						{
							entry.text = "ON";
							entry.color = 0xFF00FF00;
						}
						else
						{
							entry.text = "OFF";
							entry.color = 0xFFFF0000;
						}

						return entry;
					});
				semi_manual->add_separator();
				semi_manual->add_label("                          |> Spell Settings <|                          ");
				semi_manual->add_separator();
				const auto semi_manual_q = semi_manual->add_sub_category("shulepin_aio_varus_semi_manual_q", "[Q] Settings");
				{
					semi_manual_q->add_checkbox("shulepin_aio_varus_semi_manual_q_full_charge", "Full charge", true, [](const bool value) { config::semi_manual::q::full_charge = value; });
					
					const auto semi_manual_q_prediction = semi_manual_q->add_sub_category("shulepin_aio_varus_semi_manual_q_prediction", "Prediction Settings");
					{
						semi_manual_q_prediction->add_checkbox("shulepin_aio_varus_semi_manual_q_fast_prediction",
							"Use fast prediction in AA range", true, [](const bool value)
							{
								config::semi_manual::q::prediction::fast = value;
							});
						semi_manual_q_prediction->add_slider_float("shulepin_aio_varus_semi_manual_q_max_range", "Max range", q->get_max_range() / 2, q->get_max_range(), 10.f, q->get_max_range() - 50.f, [](const float value) {config::semi_manual::q::prediction::max_range = value; });
						semi_manual_q_prediction->add_slider_int("shulepin_aio_varus_semi_manual_q_hit_chance", "Hitchance", 0, 100, 1, 0, [](const int value) { config::semi_manual::q::prediction::hitchance = value; });
						semi_manual_q_prediction->add_slider_float("shulepin_aio_varus_semi_manual_q_extra_charge_range", "Extra charge range",
							0.f, 200.f, 10.f, 0.f, [](const float value)
							{
								config::semi_manual::q::prediction::extra_charge_range = value;
							});
					}
					const auto semi_manual_q_black_list = semi_manual_q->add_sub_category("shulepin_aio_varus_semi_manual_q_black_list", "Ignore List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							const auto& char_name = hero->get_char_name();
							semi_manual_q_black_list->add_checkbox("shulepin_aio_varus_semi_manual_q_black_list_" + char_name, char_name, false, [char_name](const bool value) { config::semi_manual::q::ignore::list[char_name] = value; });
						}
					}
					const auto semi_manual_q_mana_manager = semi_manual_q->add_sub_category("shulepin_aio_varus_semi_manual_q_mana_manager", "Mana Manager");
					{
						semi_manual_q_mana_manager->add_slider_float("shulepin_aio_varus_semi_manual_q_mana_manager_value", "Min. mana (%)", 0.f, 100.f, 1.f, 0.f, [](const float value) { config::semi_manual::q::mana_manager::value = value; });
					}
				}
				const auto semi_manual_r = semi_manual->add_sub_category("shulepin_aio_varus_semi_manual_r", "[R] Settings");
				{
					const auto semi_manual_r_prediction = semi_manual_r->add_sub_category("shulepin_aio_varus_semi_manual_r_prediction", "Prediction Settings");
					{
						semi_manual_r_prediction->add_slider_float("shulepin_aio_varus_semi_manual_r_max_range", "Max range", r->get_range() / 2, r->get_range(), 10.f, r->get_range() - 200.f, [](const float value) {config::semi_manual::r::prediction::max_range = value; });
						semi_manual_r_prediction->add_slider_int("shulepin_aio_varus_semi_manual_r_hit_chance", "Hitchance", 0, 100, 1, 0, [](const int value) { config::semi_manual::r::prediction::hitchance = value; });
					}
					const auto semi_manual_r_black_list = semi_manual_r->add_sub_category("shulepin_aio_varus_semi_manual_r_black_list", "Ignore List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							const auto& char_name = hero->get_char_name();
							semi_manual_r_black_list->add_checkbox("shulepin_aio_varus_semi_manual_r_black_list_" + char_name, char_name, false, [char_name](const bool value) { config::semi_manual::r::ignore::list[char_name] = value; });
						}
					}
					const auto semi_manual_r_mana_manager = semi_manual_r->add_sub_category("shulepin_aio_varus_semi_manual_r_mana_manager", "Mana Manager");
					{
						semi_manual_r_mana_manager->add_slider_float("shulepin_aio_varus_semi_manual_r_mana_manager_value", "Min. mana (%)", 0.f, 100.f, 1.f, 0.f, [](const float value) { config::semi_manual::r::mana_manager::value = value; });
					}
				}
			}
			const auto visuals = config->add_sub_category("shulepin_aio_varus_visuals", "Visuals");
			{
				const auto spell_ranges = visuals->add_sub_category("shulepin_aio_varus_visuals_spell_ranges",
				                                                    "Spell Ranges");
				{
					spell_ranges->add_checkbox("shulepin_aio_varus_visuals_spell_ranges_enabled", "Enabled", true,
					                           [](const bool value) { config::visuals::draw_range = value; });
					spell_ranges->add_checkbox("shulepin_aio_varus_visuals_spell_ranges_ready",
					                           " ^- Only When Spell Is Ready", true, [](const bool value)
					                           {
						                           config::visuals::draw_range_ready = value;
					                           });
					spell_ranges->add_separator();
					spell_ranges->add_checkbox("shulepin_aio_varus_visuals_spell_ranges_q", "Draw Q Range", false,
					                           [](const bool value) { config::visuals::draw_q = value; });
					spell_ranges->add_colorpicker("shulepin_aio_varus_visuals_spell_ranges_q_color", " ^ Q Color",
					                              0x80FFFFFF, [](const uint32_t value)
					                              {
						                              config::visuals::draw_q_color = value;
					                              });
					spell_ranges->add_separator();
					spell_ranges->add_checkbox("shulepin_aio_varus_visuals_spell_ranges_e", "Draw E Range", false,
					                           [](const bool value) { config::visuals::draw_e = value; });
					spell_ranges->add_colorpicker("shulepin_aio_varus_visuals_spell_ranges_e_color", " ^ E Color",
					                              0x80FFFFFF, [](const uint32_t value)
					                              {
						                              config::visuals::draw_e_color = value;
					                              });
					spell_ranges->add_separator();
					spell_ranges->add_checkbox("shulepin_aio_varus_visuals_spell_ranges_r", "Draw R Range", false,
					                           [](const bool value) { config::visuals::draw_r = value; });
					spell_ranges->add_colorpicker("shulepin_aio_varus_visuals_spell_ranges_r_color", " ^ R Color",
					                              0x80FFFFFF, [](const uint32_t value)
					                              {
						                              config::visuals::draw_r_color = value;
					                              });
				}
				const auto damage_indicator = visuals->add_sub_category("shulepin_aio_varus_visuals_damage_indicator",
				                                                        "Damage Indicator");
				{
					damage_indicator->add_checkbox("shulepin_aio_varus_visuals_damage_indicator_q", "Draw Q Damage",
					                               true, [](const bool value)
					                               {
						                               config::visuals::draw_q_damage = value;
					                               });
					damage_indicator->add_checkbox("shulepin_aio_varus_visuals_damage_indicator_w", "Draw W Damage",
					                               true, [](const bool value)
					                               {
						                               config::visuals::draw_w_damage = value;
					                               });
					damage_indicator->add_checkbox("shulepin_aio_varus_visuals_damage_indicator_e", "Draw E Damage",
					                               true, [](const bool value)
					                               {
						                               config::visuals::draw_e_damage = value;
					                               });
					damage_indicator->add_checkbox("shulepin_aio_varus_visuals_damage_indicator_r", "Draw R Damage",
					                               true, [](const bool value)
					                               {
						                               config::visuals::draw_r_damage = value;
					                               });
				}
			}
		}
	}

	void load_events()
	{
		g_sdk->event_manager->register_callback(event_manager::event::game_update, reinterpret_cast<void*>(game_update));
		g_sdk->event_manager->register_callback(event_manager::event::present, reinterpret_cast<void*>(present));
		sdk::orbwalker->register_callback(orb_sdk::before_attack, reinterpret_cast<void*>(before_attack));
		g_sdk->event_manager->register_callback(event_manager::event::buff_gain, reinterpret_cast<void*>(buff_gain));
		g_sdk->event_manager->register_callback(event_manager::event::buff_loss, reinterpret_cast<void*>(buff_loss));
		g_sdk->event_manager->register_callback(event_manager::event::execute_cast, reinterpret_cast<void*>(execute_cast));
	}

	void unload_events()
	{
		g_sdk->event_manager->unregister_callback(event_manager::event::game_update, reinterpret_cast<void*>(game_update));
		g_sdk->event_manager->unregister_callback(event_manager::event::present, reinterpret_cast<void*>(present));
		sdk::orbwalker->unregister_callback(orb_sdk::before_attack, reinterpret_cast<void*>(before_attack));
		g_sdk->event_manager->unregister_callback(event_manager::event::buff_gain, reinterpret_cast<void*>(buff_gain));
		g_sdk->event_manager->unregister_callback(event_manager::event::buff_loss, reinterpret_cast<void*>(buff_loss));
		g_sdk->event_manager->unregister_callback(event_manager::event::execute_cast, reinterpret_cast<void*>(execute_cast));
	}

	void remove_info_tab()
	{
		sdk::infotab->remove_text(config::semi_manual::q::info);
		sdk::infotab->remove_text(config::semi_manual::r::info);
	}
	
	void load()
	{
		register_spells();
		create_menu();
		load_events();
	}

	void unload()
	{
		unload_events();
		remove_info_tab();
	}

	bool is_spell_locked()
	{
		auto result = false;

		if (!e->is_issue_order_passed(e->get_delay() + utils::get_ping() + 0.066f))
		{
			result = true;
		}

		return result;
	}

	int get_real_stacks(game_object* target)
	{
		return hero_debuff_stacks[target->get_id()];
	}

	void update_values()
	{
		q->set_min_range(825.f);
		
		if (executing_kill_logic && !q->is_charging())
		{
			executing_kill_logic = false;
		}
	}

	void update_buff(game_object* hero)
	{
		const auto& buff = hero->get_buff_by_hash(w_debuff);
		
		if (buff)
		{
			if (buff->get_stacks() > hero_debuff_stacks[hero->get_id()])
			{
				hero_debuff_stacks[hero->get_id()] = buff->get_stacks();
			}
		}
		else if (hero_debuff_stacks[hero->get_id()] > 1)
		{
			hero_debuff_stacks[hero->get_id()] = 0;
		}
	}

	float calculate_q_damage(game_object* target, const float charge_time)
	{
		const int spell_level = q->get_level();
		const auto& player = g_sdk->object_manager->get_local_player();
		const float attack_damage = player->get_attack_damage();

		constexpr float base_minimum_damage[] = { 10.f, 46.67f, 83.33f, 120.f, 156.67f };
		constexpr float additional_ad_scaling[] = { 0.8333f, 0.8667f, 0.9f, 0.9333f, 0.9667f };

		const float base_damage = base_minimum_damage[spell_level - 1];
		const float additional_ad_damage = attack_damage * additional_ad_scaling[spell_level - 1];

		float total_damage = base_damage + additional_ad_damage;
		total_damage += total_damage * (charge_time * 0.5f);

		return sdk::damage->calc_damage(dmg_sdk::damage_type::physical, player, target, total_damage);
	}

	float calculate_w_damage(game_object* target, const float charge_time, const int stacks)
	{
		const int spell_level = w->get_level();
		const auto& player = g_sdk->object_manager->get_local_player();
		const float ability_power = player->get_ability_power();

		constexpr float base_percentages[] = { 3.f, 3.5f, 4.f, 4.5f, 5.f };
		constexpr float ap_ratio_per_100_ap = 1.5f;

		const float total_percentage = base_percentages[spell_level - 1] + (ap_ratio_per_100_ap * (ability_power / 100.f));

		float total_damage = total_percentage * target->get_max_hp() / 100.f;
		total_damage *= static_cast<float>(stacks);
		total_damage += total_damage * (charge_time * 0.5f);

		return sdk::damage->calc_damage(dmg_sdk::damage_type::physical, player, target, total_damage);
	}

	float calculate_w_active_damage(game_object* target, const float charge_time, const int stacks, const float extra_damage = 0.f)
	{
		const auto& player = g_sdk->object_manager->get_local_player();
		const int level = player->get_level();

		constexpr float base_percentage = 6.f;
		constexpr float additional_percentage_per_level = 2.f;
		constexpr int level_cap = 13;

		const float total_percentage = base_percentage + std::min((static_cast<float>(level) - 1.f) / 3.f, static_cast<float>(level_cap - 1) / 3.f) * additional_percentage_per_level;

		float total_damage = total_percentage * (target->get_max_hp() - (target->get_hp() - extra_damage)) / 100.f;
		total_damage += total_damage * (charge_time * 0.5f);

		return sdk::damage->calc_damage(dmg_sdk::damage_type::magical, player, target, total_damage);
	}

	float calculate_e_damage(game_object* target)
	{
		const int spell_level = e->get_level();
		const auto& player = g_sdk->object_manager->get_local_player();
		const float bonus_attack_damage = player->get_attack_damage() - player->get_base_attack_damage();

		constexpr float base_minimum_damage[] = { 60.f, 100.f, 140.f, 180.f, 220.f };
		constexpr float additional_ad_scaling = 0.9f;

		const float base_damage = base_minimum_damage[spell_level - 1];
		const float additional_ad_damage = bonus_attack_damage * additional_ad_scaling;

		const float total_damage = base_damage + additional_ad_damage;

		return sdk::damage->calc_damage(dmg_sdk::damage_type::physical, player, target, total_damage);
	}

	float calculate_r_damage(game_object* target)
	{
		const int spell_level = r->get_level(); 
		const auto& player = g_sdk->object_manager->get_local_player();
		const float ability_power = player->get_ability_power();

		constexpr float base_magic_damage[] = { 150.f, 250.f, 350.f };
		constexpr float ap_scaling = 1.0f;

		const float base_damage = base_magic_damage[spell_level - 1];
		const float additional_ap_damage = ability_power * ap_scaling;

		const float total_magic_damage = base_damage + additional_ap_damage;

		return sdk::damage->calc_damage(dmg_sdk::damage_type::magical, player, target, total_magic_damage);
	}

	float calculate_total_damage(game_object* target, const bool full_charge = false, const bool include_q = true, const bool include_w = true, const bool include_e = true, const bool include_r = true)
	{
		auto stack_damage = 0.f;
		if (q->is_ready() || e->is_ready() || r->is_ready())
		{
			stack_damage = stack_damage_cache[target->get_id()];
		}
		
		const auto q_damage = include_q && q->is_ready() ? full_charge ? q_full_damage_cache[target->get_id()] : q_damage_cache[target->get_id()] : 0.f;
		const auto w_damage = include_w && w->is_ready() ? full_charge ? w_full_damage_cache[target->get_id()] : q_damage_cache[target->get_id()] : 0.f;
		const auto e_damage = include_e && e->is_ready() ? e_damage_cache[target->get_id()] : 0.f;
		const auto r_damage = include_r && r->is_ready() ? r_damage_cache[target->get_id()] : 0.f;
		const auto total_damage = q_damage + w_damage + e_damage + r_damage + stack_damage;

		return total_damage;
	}
	
	void q_combat_logic()
	{
		const auto& player = g_sdk->object_manager->get_local_player();

		if (!q->is_ready() ||
			sdk::orbwalker->would_cancel_attack() ||
			is_spell_locked() ||
			executing_kill_logic)
		{
			return;
		}

		if (sdk::orbwalker->combo())
		{
			if (!config::combo::use_q || 
				!utils::is_enough_mana(config::combo::q::mana_manager::value))
			{
				return;
			}

			auto target = utils::get_target_with_list(q->get_range(), config::combo::q::ignore::list);

			if (should_force_q)
			{
				target = force_q_target;
			}

			if (!target)
			{
				return;
			}

			const auto priority = config::combo::priority == 0 ? 0 : 2;
			const float attack_speed = player->get_percent_attack_speed_mod() * 100.f;
			const bool should_block_e = config::combo::e::block_e_as ? attack_speed > config::combo::e::block_e_as_slider : config::combo::e::block_e_as;
			const auto dist_to_target = player->get_position().distance(target->get_position());
			const auto hitchance = (config::combo::q::prediction::fast && dist_to_target < utils::get_real_attack_range(target)) ? 0 : config::combo::q::prediction::hitchance;
			const auto should_force_q_on_root = config::combo::q::force_q_charge ? should_force_q : config::combo::q::force_q_charge;
			const auto enough_w_stacks = get_real_stacks(target) >= config::combo::q::min_stacks;
			const auto should_ignore_full_charge = config::combo::q::ignore_full_charge ? player->get_position().distance(target->get_position()) > config::combo::q::ignore_full_charge_range : !config::combo::q::ignore_full_charge;
			const auto should_full_charge = config::combo::q::full_charge_mode == 0 ? false : config::combo::q::full_charge_mode == 1 ? true : false;
			const auto should_start_charge = enough_w_stacks && (priority == 0 || !utils::is_spell_ready(priority) || should_block_e || e_combo_global_check) && e->is_issue_order_passed(1.f) || should_force_q_on_root;
			const auto charge_time = should_full_charge && should_ignore_full_charge || should_force_q_on_root ? 0.9f : 0.f;

			if (q->is_charging())
			{
				if (q->get_charged_time() < charge_time)
				{
					return;
				}

				q->set_min_range(825.f - config::combo::q::prediction::extra_charge_range);

				const auto& pred_out = q->get_prediction(target, hitchance, config::combo::q::prediction::max_range);
				if (pred_out.is_valid)
				{
					if (q->release(pred_out.cast_position, true, 0.1f))
					{
						should_force_q = false;
					}
				}
			}
			else
			{
				if (!should_start_charge)
				{
					return;
				}

				if (w->is_ready() && 
					w->get_toggle_state() == 0 &&
					(should_force_q && config::combo::q::force_q_charge_with_w) ||
					(config::combo::w::mode == 0 && utils::get_percent_health(target) < config::combo::w::min_hp))
				{
					w->cast_spell();
				}
					
				q->cast_spell();
			}

		}

		if (sdk::orbwalker->harass())
		{
			if (!config::harass::use_q ||
				!utils::is_enough_mana(config::harass::q::mana_manager::value))
			{
				return;
			}

			const auto target = utils::get_target_with_list(q->get_range(), config::harass::q::ignore::list);

			if (!target)
			{
				return;
			}

			const auto dist_to_target = player->get_position().distance(target->get_position());
			const auto hitchance = (config::harass::q::prediction::fast && dist_to_target < utils::get_real_attack_range(target)) ? 0 : config::harass::q::prediction::hitchance;
			const auto enough_w_stacks = get_real_stacks(target) >= config::harass::q::min_stacks;
			const auto should_ignore_full_charge = config::harass::q::ignore_full_charge ? player->get_position().distance(target->get_position()) > config::harass::q::ignore_full_charge_range : !config::harass::q::ignore_full_charge;
			const auto should_full_charge = config::harass::q::full_charge_mode == 0 ? false : config::harass::q::full_charge_mode == 1 ? true : false;
			const auto should_start_charge = enough_w_stacks && e->is_issue_order_passed(1.f);
			const auto charge_time = should_full_charge && should_ignore_full_charge ? 0.9f : 0.f;

			if (q->is_charging())
			{
				if (q->get_charged_time() < charge_time)
				{
					return;
				}

				q->set_min_range(825.f - config::harass::q::prediction::extra_charge_range);

				const auto& pred_out = q->get_prediction(target, hitchance, config::harass::q::prediction::max_range);
				if (pred_out.is_valid)
				{
					if (q->release(pred_out.cast_position, true, 0.1f))
					{
						should_force_q = false;
					}
				}
			}
			else
			{
				if (!should_start_charge)
				{
					return;
				}

				if (w->is_ready() &&
					w->get_toggle_state() == 0 &&
					should_force_q ||
					(config::harass::w::mode == 0 && utils::get_percent_health(target) < config::harass::w::min_hp))
				{
					w->cast_spell();
				}

				q->cast_spell();
			}
		}
	}

	void e_combat_logic()
	{
		const auto& player = g_sdk->object_manager->get_local_player();

		if (!e->is_ready() ||
			sdk::orbwalker->would_cancel_attack() ||
			is_spell_locked() ||
			executing_kill_logic)
		{
			return;
		}

		if (sdk::orbwalker->combo())
		{
			if (!config::combo::use_e ||
				!utils::is_enough_mana(config::combo::e::mana_manager::value))
			{
				return;
			}

			const auto target = utils::get_target_with_list(e->get_range(), config::combo::e::ignore::list);

			if (!target)
			{
				return;
			}

			const auto priority = config::combo::priority == 0 ? 0 : 2;
			const auto stack_check = get_real_stacks(target) >= config::combo::e::min_stacks;
			const auto last_cast_check = q->is_issue_order_passed(1.f);
			const auto dist_to_target = player->get_position().distance(target->get_position());
			const auto hitchance = (config::combo::e::prediction::fast && dist_to_target < utils::get_real_attack_range(target)) ? 0 : config::combo::e::prediction::hitchance;
			const float attack_speed = player->get_percent_attack_speed_mod() * 100.f;
			const bool should_block_e = config::combo::e::block_e_as
				? attack_speed > config::combo::e::block_e_as_slider
				: config::combo::e::block_e_as;
			e_combo_global_check = stack_check && last_cast_check && !should_block_e && (priority == 2 || !utils::is_spell_ready(priority));

			if (e_combo_global_check)
			{
				e->cast_spell_on_hitchance(target, hitchance, e->get_range());
			}
		}

		if (sdk::orbwalker->harass())
		{
			if (!config::harass::use_e ||
				!utils::is_enough_mana(config::harass::e::mana_manager::value))
			{
				return;
			}

			const auto target = utils::get_target_with_list(e->get_range(), config::harass::e::ignore::list);

			if (!target)
			{
				return;
			}

			const auto stack_check = get_real_stacks(target) >= config::harass::e::min_stacks;
			const auto last_cast_check = q->is_issue_order_passed(1.f);
			const auto dist_to_target = player->get_position().distance(target->get_position());
			const auto hitchance = (config::harass::e::prediction::fast && dist_to_target < utils::get_real_attack_range(target)) ? 0 : config::harass::e::prediction::hitchance;
			e_harass_global_check = stack_check && last_cast_check;

			if (e_harass_global_check)
			{
				e->cast_spell_on_hitchance(target, hitchance, e->get_range());
			}
		}
	}

	void r_combat_logic()
	{
		const auto& player = g_sdk->object_manager->get_local_player();

		if (!r->is_ready() ||
			sdk::orbwalker->would_cancel_attack() ||
			is_spell_locked() ||
			executing_kill_logic)
		{
			return;
		}

		if (sdk::orbwalker->combo())
		{
			if (config::combo::r::cast_conditions::use_on_cc)
			{
				const auto target = sdk::target_selector->get_hero_target([](game_object* hero)
				{
					return utils::is_valid_target(hero, config::combo::r::prediction::max_range) &&
						!config::combo::r::ignore::list[hero->get_char_name()] &&
							utils::is_on_cc(hero, r->get_travel_time(hero));
				});

				if (target)
				{
					r->cast_spell_on_hitchance(target, config::combo::r::prediction::hitchance, config::combo::r::prediction::max_range);
				}
			}

			if (config::combo::r::cast_conditions::use_on_killable)
			{
				const auto target = sdk::target_selector->get_hero_target([](game_object* hero)
				{
					return utils::is_valid_target(hero, config::combo::r::prediction::max_range) &&
						!config::combo::r::ignore::list[hero->get_char_name()] &&
							(calculate_total_damage(hero) +
								sdk::damage->get_aa_damage(g_sdk->object_manager->get_local_player(), hero, true) *
								static_cast<float>(config::combo::r::cast_conditions::x_aa)) > utils::get_real_health(hero, "ALL");
				});
				
				if (target)
				{
					r->cast_spell_on_hitchance(target, config::combo::r::prediction::hitchance, config::combo::r::prediction::max_range);
				}
			}
		}
	}
	
	void q_auto_kill_logic(game_object* target)
	{
		if (!q->is_ready() ||
			!utils::is_enemy(target) ||
			!utils::is_valid_target(target, q->get_max_range()))
		{
			return;
		}

		const bool is_w_active_ready = w->is_ready() && w->get_toggle_state() == 0;
		const bool is_w_active_casted = w->get_toggle_state() == 1;

		const auto check_r_damage = calculate_r_damage(target);
		const auto check_w_damage = calculate_w_damage(target, 1.f, get_real_stacks(target));
		const auto check_q_damage = calculate_q_damage(target, 1.f) + check_w_damage;
		const auto check_w_active_damage = is_w_active_ready && config::auto_kill::use_w ? calculate_w_active_damage(target, 1.f, get_real_stacks(target), check_q_damage + check_w_damage) : 0.f;
		const auto check_w_active_combo_damage = calculate_w_active_damage(target, 1.f, get_real_stacks(target), check_q_damage + check_w_damage + check_r_damage);
		const auto check_total_damage = check_q_damage + check_w_active_damage;
		const auto check_is_killable = check_total_damage > utils::get_real_health(target, "AD");

		const auto real_w_damage = calculate_w_damage(target, q->get_charged_time(1.5f), get_real_stacks(target));
		const auto real_q_damage = calculate_q_damage(target, q->get_charged_time(1.25f)) + real_w_damage;
		const auto real_w_active_damage = is_w_active_casted ? calculate_w_active_damage(target, q->get_charged_time(2.f), get_real_stacks(target), real_q_damage + real_w_damage) : 0.f;
		const auto real_total_damage = real_q_damage + real_w_active_damage;
		const auto real_is_killable = 
			real_total_damage > utils::get_real_health(target, "AD", config::auto_kill::health_pred, q->get_travel_time(target));

		const auto is_fully_charged = q->get_charged_time(is_w_active_casted ? 2.f : 1.5f) > 0.9f;

		const bool should_start_charge = check_is_killable;
		const bool should_release = real_is_killable || is_fully_charged;

		q_damage_cache[target->get_id()] = real_q_damage - real_w_damage;
		w_damage_cache[target->get_id()] = real_w_active_damage;
		q_full_damage_cache[target->get_id()] = check_q_damage - check_w_damage;
		w_full_damage_cache[target->get_id()] = check_w_active_combo_damage;
		stack_damage_cache[target->get_id()] = real_w_damage;

		if (!config::auto_kill::enabled ||
			!config::auto_kill::use_q ||
			config::auto_kill::q::ignore::list[target->get_char_name()] ||
			!utils::is_enough_mana(config::auto_kill::q::mana_manager::value))
		{
			return;
		}

		if (config::auto_kill::use_w && should_start_charge && w->is_ready() && w->get_toggle_state() == 0)
		{
			w->cast_spell();
		}

		if (is_spell_locked())
		{
			return;
		}

		if (q->is_charging())
		{
			if (!should_release)
			{
				return;
			}

			q->set_min_range(825.f - config::auto_kill::q::prediction::extra_charge_range);

			const auto& pred_out = q->get_prediction(target, config::auto_kill::q::prediction::hitchance, config::auto_kill::q::prediction::max_range);
			if (pred_out.is_valid)
			{
				if (q->release(pred_out.cast_position, true, 0.1f))
				{
					executing_kill_logic = false;
				}
			}
		}
		else
		{
			if (!should_start_charge)
			{
				return;
			}

			executing_kill_logic = true;
			q->cast_spell();
		}
	}

	void e_auto_kill_logic(game_object* target)
	{
		if (!e->is_ready() ||
			!utils::is_enemy(target) ||
			!utils::is_valid_target(target, e->get_range()))
		{
			return;
		}

		const auto real_w_damage = calculate_w_damage(target, 0.f, get_real_stacks(target));
		const auto real_e_damage = calculate_e_damage(target);
		const auto real_total_damage = real_e_damage + real_w_damage;
		const auto real_is_killable = 
			real_total_damage > utils::get_real_health(target, "AD", config::auto_kill::health_pred, e->get_travel_time(target));

		e_damage_cache[target->get_id()] = real_e_damage - real_w_damage;
		stack_damage_cache[target->get_id()] = real_w_damage;

		if (!config::auto_kill::enabled ||
			!config::auto_kill::use_e ||
			q->is_charging() ||
			is_spell_locked() ||
			config::auto_kill::e::ignore::list[target->get_char_name()] ||
			!utils::is_enough_mana(config::auto_kill::e::mana_manager::value))
		{
			return;
		}

		if (real_is_killable)
		{
			e->cast_spell_on_hitchance(target, config::auto_kill::e::prediction::hitchance, config::auto_kill::e::prediction::max_range);
		}
	}

	void r_auto_kill_logic(game_object* target)
	{
		if (!r->is_ready() ||
			!utils::is_enemy(target) ||
			!utils::is_valid_target(target, r->get_range()))
		{
			return;
		}

		const auto real_w_damage = calculate_w_damage(target, 0.f, get_real_stacks(target));
		const auto real_r_damage = calculate_r_damage(target);
		const auto real_total_damage = real_r_damage + real_w_damage;

		r_damage_cache[target->get_id()] = real_r_damage - real_w_damage;
		stack_damage_cache[target->get_id()] = real_w_damage;
	}

	void q_semi_manual_logic()
	{
		const auto& player = g_sdk->object_manager->get_local_player();

		if (!q->is_ready() ||
			!config::semi_manual::q::value ||
			!utils::is_enough_mana(config::semi_manual::q::mana_manager::value) ||
			sdk::orbwalker->would_cancel_attack() ||
			is_spell_locked() ||
			executing_kill_logic)
		{
			return;
		}
		
		const auto target = utils::get_target_with_list(q->get_range(), config::semi_manual::q::ignore::list);

		if (!target)
		{
			return;
		}

		const auto dist_to_target = player->get_position().distance(target->get_position());
		const auto hitchance = (config::semi_manual::q::prediction::fast && dist_to_target < utils::get_real_attack_range(target)) ? 0 : config::semi_manual::q::prediction::hitchance;
		const auto charge_time = config::semi_manual::q::full_charge ? 0.9f : 0.f;

		if (q->is_charging())
		{
			if (q->get_charged_time() < charge_time)
			{
				return;
			}

			q->set_min_range(825.f - config::semi_manual::q::prediction::extra_charge_range);

			const auto& pred_out = q->get_prediction(target, hitchance, config::semi_manual::q::prediction::max_range);
			if (pred_out.is_valid)
			{
				if (q->release(pred_out.cast_position, true, 0.1f))
				{
					should_force_q = false;
				}
			}
		}
		else
		{
			q->cast_spell();
		}
	}

	void r_semi_manual_logic()
	{
		const auto& player = g_sdk->object_manager->get_local_player();

		if (!r->is_ready() ||
			!config::semi_manual::r::value ||
			!utils::is_enough_mana(config::semi_manual::r::mana_manager::value) ||
			sdk::orbwalker->would_cancel_attack() ||
			is_spell_locked() ||
			executing_kill_logic)
		{
			return;
		}
		
		const auto target = utils::get_target_with_list(config::semi_manual::r::prediction::max_range, config::semi_manual::r::ignore::list);

		if (!target)
		{
			return;
		}

		r->cast_spell_on_hitchance(target, config::semi_manual::r::prediction::hitchance, config::semi_manual::r::prediction::max_range);
	}

	void draw_spell_ranges()
	{
		if (!config::visuals::draw_range)
		{
			return;
		}

		const auto& player = g_sdk->object_manager->get_local_player();

		if ((q->is_ready() || !config::visuals::draw_range_ready) && config::visuals::draw_q)
		{
			g_sdk->renderer->add_circle_3d(player->get_position(), q->get_max_range(), 2.f, config::visuals::draw_q_color);
		}

		if ((e->is_ready() || !config::visuals::draw_range_ready) && config::visuals::draw_e)
		{
			g_sdk->renderer->add_circle_3d(player->get_position(), e->get_range(), 2.f, config::visuals::draw_e_color);
		}

		if ((r->is_ready() || !config::visuals::draw_range_ready) && config::visuals::draw_r)
		{
			g_sdk->renderer->add_circle_3d(player->get_position(), r->get_range(), 2.f, config::visuals::draw_r_color);
		}
	}
	
	void draw_damage_indicator(game_object* target)
	{
		const auto total_damage = calculate_total_damage(target, true,
			config::visuals::draw_q_damage,
			config::visuals::draw_w_damage,
			config::visuals::draw_e_damage,
			config::visuals::draw_r_damage
			);
		g_sdk->renderer->add_damage_indicator(target, total_damage);
	}

	void hero_game_update_loop()
	{
		for (const auto& hero : g_sdk->object_manager->get_heroes())
		{
			update_buff(hero);
			q_auto_kill_logic(hero);
			e_auto_kill_logic(hero);
			r_auto_kill_logic(hero);
		}
	}

	void hero_present_loop()
	{
		for (const auto& hero : g_sdk->object_manager->get_heroes())
		{
			if (!utils::is_enemy(hero) ||
				!utils::is_valid_target(hero))
			{
				continue;
			}

			draw_damage_indicator(hero);
		}
	}

	void combat_logic()
	{
		q_combat_logic();
		e_combat_logic();
		r_combat_logic();
	}

	void semi_manual_logic()
	{
		q_semi_manual_logic();
		r_semi_manual_logic();
	}
	
	void __fastcall game_update()
	{
		update_values();
		hero_game_update_loop();
		combat_logic();
		semi_manual_logic();
	}
	
	void __fastcall present()
	{
		hero_present_loop();
		draw_spell_ranges();
	}

	void __fastcall buff_gain(game_object* object, buff_instance* buff)
	{
		if (utils::is_enemy(object))
		{
			if (buff->get_hash() == w_debuff)
			{
				hero_debuff_stacks[object->get_id()] = buff->get_stacks();
			}
			if (buff->get_hash() == r_debuff)
			{
				should_force_q = true;
				force_q_target = object;
			}
		}
	}

	void __fastcall buff_loss(game_object* object, buff_instance* buff)
	{
		if (hero_debuff_stacks[object->get_id()] && buff->get_hash() == w_debuff)
		{
			hero_debuff_stacks[object->get_id()] = 0;
		}
		if (buff->get_hash() == r_debuff)
		{
			should_force_q = false;
		}
	}

	void __fastcall execute_cast(game_object* object, spell_cast* cast)
	{
		if (!utils::is_me(object) || 
			!utils::is_auto_attack(cast) || 
			object->has_buff_of_type(buff_type::blind))
		{
			return;
		}

		const auto& target = cast->get_target();
		if (target && utils::is_enemy(target) && utils::is_valid_target(target))
		{
			if (hero_debuff_stacks[target->get_id()] < 3)
			{
				hero_debuff_stacks[target->get_id()] = hero_debuff_stacks[target->get_id()] + 1;
			}
		}
	}

	bool __fastcall before_attack(orb_sdk::event_data* data)
	{
		if (q->is_charging())
		{
			return false;
		}
		return true;
	}

}