#include "lucian.h"
#include "utils.h"
#include "issue_order_manager.h"

namespace lucian
{
	static constexpr float q_range = 500.f;
	static constexpr float q_extended_range = 1000.f;
	static constexpr float q_radius = 60.f;
	static constexpr float w_range = 900.f;
	static constexpr float w_delay = 0.25f;
	static constexpr float w_speed = 1600.f;
	static constexpr float w_radius = 55.f;
	static constexpr float r_range = 1200.f;
	static constexpr float r_radius = 110.f;
	static constexpr float r_delay = 0.f;
	static constexpr float r_speed = 2000.f;

	bool should_block_movement = false;
	math::vector3 r_cast_position;

	void __fastcall game_update();
	void __fastcall present();
	void __fastcall execute_cast(game_object* object, spell_cast* cast);
	void __fastcall process_cast(game_object* object, spell_cast* cast);
	void __fastcall buff_loss(game_object* object, buff_instance* buff);
	bool __fastcall before_attack(orb_sdk::event_data* data);
	bool __fastcall before_move(orb_sdk::event_data* data);

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
		bool use_extended_q;
		bool dont_use_extended_q_inside_aa;
		float max_q_range;
		int hitchance;
		std::unordered_map<std::string, bool> white_list;
	}

	namespace config::combo::w
	{
		bool fast_prediction;
		bool ignore_collision;
		float max_w_range;
		int hitchance;
	}

	namespace config::combo::e
	{
		int cast_position;
		int dash_type;
		float dash_fixed_range;
	}

	namespace config::combo::e::safety_checks
	{
		bool enabled; 
		bool turret_dive;
	}

	namespace config::combo::e::safety_checks::anti_melee
	{
		bool enabled;
		float extra_range;
		float my_hp;
		float enemy_hp;
		std::unordered_map<std::string, bool> white_list;
	}

	namespace config::combo::r
	{
		bool magnet;
		bool use_e;
	}

	namespace config::harass
	{
		bool use_q;
		bool use_w;
	}

	namespace config::harass::extra
	{
		bool allow_to_use_in_other_modes;
	}

	namespace config::harass::q
	{
		bool use_extended_q;
		bool dont_use_extended_q_inside_aa;
		float max_q_range;
		int hitchance;
		float min_mana;
		std::unordered_map<std::string, bool> white_list;
	}

	namespace config::harass::w
	{
		float max_w_range;
		int hitchance;
		float min_mana;
		std::unordered_map<std::string, bool> white_list;
	}

	namespace config::visuals
	{
		bool draw_range;
		bool draw_range_ready;
		bool draw_q;
		bool draw_q2;
		bool draw_w;
		bool draw_e; 
		bool draw_r;
		uint32_t draw_q_color;
		uint32_t draw_q2_color;
		uint32_t draw_w_color;
		uint32_t draw_e_color;
		uint32_t draw_r_color;
		bool draw_q_damage;
		bool draw_w_damage;
		bool draw_e_damage;
		bool draw_r_damage;
		bool draw_aa_damage;
	}

	namespace config::keybinds
	{
		bool burst_toggle;
		std::string burst_toggle_key;
		uint32_t burst_toggle_info;
		bool harass_toggle;
		std::string harass_toggle_key;
		uint32_t harass_toggle_info;
		bool block_e_toggle;
		std::string block_e_toggle_key;
		uint32_t block_e_toggle_info;
		bool semi_r_toggle;
		std::string semi_r_toggle_key;
		uint32_t semi_r_toggle_info;
		bool magnet_r_toggle;
		std::string magnet_r_toggle_key;
		uint32_t magnet_r_toggle_info;
	}

	void create_menu()
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const auto config = g_sdk->menu_manager->add_category("shulepin_aio_lucian", "ShulepinAIO - " + player->get_char_name());
		{
			const auto combo = config->add_sub_category("shulepin_aio_lucian_combo", "Combo Settings");
			{
				combo->add_checkbox("shulepin_aio_lucian_combo_use_q", "Use Q", true, [](const bool value) { config::combo::use_q = value; });
				combo->add_checkbox("shulepin_aio_lucian_combo_use_w", "Use W", true, [](const bool value) { config::combo::use_w = value; });
				combo->add_checkbox("shulepin_aio_lucian_combo_use_e", "Use E", true, [](const bool value) { config::combo::use_e = value; });
				combo->add_checkbox("shulepin_aio_lucian_combo_use_r", "Use R", true, [](const bool value) { config::combo::use_r = value; });
				combo->add_separator();
				const auto combo_q = combo->add_sub_category("shulepin_aio_lucian_combo_q_settings", "Q Settings");
				{
					combo_q->add_checkbox("shulepin_aio_lucian_combo_q_settings_extended", "Use Extended Q", false, [](const bool value) { config::combo::q::use_extended_q = value; });
					combo_q->add_checkbox("shulepin_aio_lucian_combo_q_settings_extended_aa", "^- Don't Use Inside AA Range", true, [](const bool value) { config::combo::q::dont_use_extended_q_inside_aa = value; });
					combo_q->add_separator();
					combo_q->add_slider_float("shulepin_aio_lucian_combo_q_settings_max_range", "Max Range", 500.f, 1000.f, 10.f, 950.f, [](const float value) { config::combo::q::max_q_range = value; });
					combo_q->add_slider_int("shulepin_aio_lucian_combo_q_settings_hitchance", "Hitchance", 0, 100, 1, 40, [](const int value) { config::combo::q::hitchance = value; });
					combo_q->add_separator();
					const auto combo_q_while_list = combo_q->add_sub_category("shulepin_aio_lucian_combo_q_white_list", "Extended Q White List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							auto const char_name = hero->get_char_name();
							combo_q_while_list->add_checkbox("shulepin_aio_lucian_combo_q_white_list_" + char_name, char_name, true, [char_name](const bool value) { config::combo::q::white_list[char_name] = value; });
						}
					}
				}
				const auto combo_w = combo->add_sub_category("shulepin_aio_lucian_combo_w_settings", "W Settings");
				{
					combo_w->add_checkbox("shulepin_aio_lucian_combo_w_settings_fast_prediction", "Fast Prediction", true, [](const bool value) { config::combo::w::fast_prediction = value; });
					combo_w->add_label(" ^- Will ignore max range and hitchance settings");
					combo_w->add_checkbox("shulepin_aio_lucian_combo_w_settings_ignore_collision", "Ignore Collision", true, [](const bool value) { config::combo::w::ignore_collision = value; });
					combo_w->add_slider_float("shulepin_aio_lucian_combo_w_settings_max_range", "Max Range", 500.f, 900.f, 10.f, 900.f, [](const float value) { config::combo::w::max_w_range = value; });
					combo_w->add_slider_int("shulepin_aio_lucian_combo_w_settings_hitchance", "Hitchance", 0, 100, 1, 0, [](const int value) { config::combo::w::hitchance = value; });
				}
				const auto combo_e = combo->add_sub_category("shulepin_aio_lucian_combo_e_settings", "E Settings");
				{
					combo_e->add_combo("shulepin_aio_lucian_combo_e_settings_cast_position", "Cast Position", { "Cursor", "Enemy", "Side [WIP]", "Smart [WIP]" }, 0, [](const int value) { config::combo::e::cast_position = value; });
					combo_e->add_combo("shulepin_aio_lucian_combo_e_settings_dash", "Dash Range Logic", { "Fixed Range", "Dynamic Range [Enemy Path]", "Dynamic Range [Cursor Distance]" }, 0, [](const int value) { config::combo::e::dash_type = value; });
					combo_e->add_slider_float("shulepin_aio_lucian_combo_e_settings_dash_fixed_range", "Fixed Dash Range", 200.f, 425.f, 25.f, 200.f, [](const float value) { config::combo::e::dash_fixed_range = value; });
					combo_e->add_separator();
					const auto combo_e_safety_checks = combo_e->add_sub_category("shulepin_aio_lucian_combo_e_settings_s", "Safety Checks");
					{
						combo_e_safety_checks->add_checkbox("shulepin_aio_lucian_combo_e_settings_safety_checks_enabled", "Enabled", true, [](const bool value) { config::combo::e::safety_checks::enabled = value; });
						combo_e_safety_checks->add_checkbox("shulepin_aio_lucian_combo_e_settings_safety_checks_turret", "Don't Dash Into Turret Range", true, [](const bool value) { config::combo::e::safety_checks::turret_dive = value; });
						const auto combo_e_safety_checks_anti_melee = combo_e_safety_checks->add_sub_category("shulepin_aio_lucian_combo_e_settings_s_m", "Anti-melee");
						{
							combo_e_safety_checks_anti_melee->add_checkbox("shulepin_aio_lucian_combo_e_settings_safety_checks_anti_melee_enabled", "Enabled", true, [](const bool value) { config::combo::e::safety_checks::anti_melee::enabled = value; });
							combo_e_safety_checks_anti_melee->add_separator();
							combo_e_safety_checks_anti_melee->add_slider_float("shulepin_aio_lucian_combo_e_settings_safety_checks_anti_melee_my_hp", "My HP < x%", 0.f, 100.f, 1.f, 75.f, [](const float value) { config::combo::e::safety_checks::anti_melee::my_hp = value; });
							combo_e_safety_checks_anti_melee->add_slider_float("shulepin_aio_lucian_combo_e_settings_safety_checks_anti_melee_enemy_hp", "Enemy HP > x%", 0.f, 100.f, 1.f, 20.f, [](const float value) { config::combo::e::safety_checks::anti_melee::enemy_hp = value; });
							combo_e_safety_checks_anti_melee->add_slider_float("shulepin_aio_lucian_combo_e_settings_safety_checks_anti_melee_extra_range", "Extra Range", 0.f, 300.f, 10.f, 0.f, [](const float value) { config::combo::e::safety_checks::anti_melee::extra_range = value; });
							combo_e_safety_checks_anti_melee->add_separator();
							for (const auto& hero : g_sdk->object_manager->get_heroes())
							{
								if (!utils::is_enemy(hero)) continue;
								auto const char_name = hero->get_char_name();
								combo_e_safety_checks_anti_melee->add_checkbox("shulepin_aio_lucian_combo_e_settings_safety_checks_anti_melee_white_list_" + char_name, char_name, hero->is_melee(), [char_name](const bool value) { config::combo::e::safety_checks::anti_melee::white_list[char_name] = value; });
							}
						}
					}
				}
				const auto combo_r = combo->add_sub_category("shulepin_aio_lucian_combo_r_settings", "R Settings");
				{
					combo_r->add_checkbox("shulepin_aio_lucian_combo_r_magnet", "Magnet R", true, [](const bool value) { config::combo::r::magnet = value; });
					combo_r->add_checkbox("shulepin_aio_lucian_combo_r_use_e", "Use E While R [WIP]", false, [](const bool value) { config::combo::r::use_e = value; });
				}
				combo->add_separator();
				combo->add_combo("shulepin_aio_lucian_combo_priority", "Combo Priority", { "Q", "W", "E", "Smart [WIP]" }, 2, [](const int value) { config::combo::priority = value; });
			}
			const auto harass = config->add_sub_category("shulepin_aio_lucian_harass", "Harass Settings");
			{
				harass->add_checkbox("shulepin_aio_lucian_harass_use_q", "Use Q", true, [](const bool value) { config::harass::use_q = value; });
				harass->add_checkbox("shulepin_aio_lucian_harass_use_w", "Use W", true, [](const bool value) { config::harass::use_w = value; });
				harass->add_separator();
				const auto harass_q = harass->add_sub_category("shulepin_aio_lucian_harass_q_settings", "Q Settings");
				{
					harass_q->add_checkbox("shulepin_aio_lucian_harass_q_settings_extended", "Use Extended Q", false, [](const bool value) { config::harass::q::use_extended_q = value; });
					harass_q->add_checkbox("shulepin_aio_lucian_harass_q_settings_extended_aa", "^- Don't Use Inside AA Range", true, [](const bool value) { config::harass::q::dont_use_extended_q_inside_aa = value; });
					harass_q->add_separator();
					harass_q->add_slider_float("shulepin_aio_lucian_harass_q_settings_max_range", "Max Range", 500.f, 1000.f, 10.f, 950.f, [](const float value) { config::harass::q::max_q_range = value; });
					harass_q->add_slider_int("shulepin_aio_lucian_harass_q_settings_hitchance", "Hitchance", 0, 100, 1, 40, [](const int value) { config::harass::q::hitchance = value; });
					harass_q->add_separator();
					harass_q->add_slider_float("shulepin_aio_lucian_harass_q_settings_mana", "Min. Mana (%)", 0.f, 100.f, 1.f, 0.f, [](const float value) { config::harass::q::min_mana = value; });
					harass_q->add_separator();
					const auto harass_q_while_list = harass_q->add_sub_category("shulepin_aio_lucian_harass_q_white_list", "White List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							auto const char_name = hero->get_char_name();
							harass_q_while_list->add_checkbox("shulepin_aio_lucian_harass_q_white_list_" + char_name, char_name, true, [char_name](const bool value) { config::harass::q::white_list[char_name] = value; });
						}
					}
				}
				const auto harass_w = harass->add_sub_category("shulepin_aio_lucian_harass_w_settings", "W Settings");
				{
					harass_w->add_slider_float("shulepin_aio_lucian_harass_w_settings_max_range", "Max Range", 500.f, 900.f, 10.f, 900.f, [](const float value) { config::harass::w::max_w_range = value; });
					harass_w->add_slider_int("shulepin_aio_lucian_harass_w_settings_hitchance", "Hitchance", 0, 100, 1, 0, [](const int value) { config::harass::w::hitchance = value; });
					harass_w->add_separator();
					harass_w->add_slider_float("shulepin_aio_lucian_harass_w_settings_mana", "Min. Mana (%)", 0.f, 100.f, 1.f, 75.f, [](const float value) { config::harass::q::min_mana = value; });
					harass_w->add_separator();
					const auto harass_w_while_list = harass_w->add_sub_category("shulepin_aio_lucian_harass_w_white_list", "White List");
					{
						for (const auto& hero : g_sdk->object_manager->get_heroes())
						{
							if (!utils::is_enemy(hero)) continue;
							auto const char_name = hero->get_char_name();
							harass_w_while_list->add_checkbox("shulepin_aio_lucian_harass_w_white_list_" + char_name, char_name, true, [char_name](const bool value) { config::harass::w::white_list[char_name] = value; });
						}
					}
				}
				harass->add_separator();
				const auto harass_extra = harass->add_sub_category("shulepin_aio_lucian_harass_global", "Extra Settings");
				{
					harass_extra->add_checkbox("shulepin_aio_lucian_harass_global_modes", "Execute Harass In Clear/LastHit Modes", true, [](const bool value) { config::harass::extra::allow_to_use_in_other_modes = value; });
				}
			}
			const auto visuals = config->add_sub_category("shulepin_aio_lucian_visuals", "Visuals");
			{
				const auto spell_ranges = visuals->add_sub_category("shulepin_aio_lucian_visuals_spell_ranges", "Spell Ranges");
				{
					spell_ranges->add_checkbox("shulepin_aio_lucian_visuals_spell_ranges_enabled", "Enabled", true, [](const bool value) { config::visuals::draw_range = value; });
					spell_ranges->add_checkbox("shulepin_aio_lucian_visuals_spell_ranges_ready", " ^- Only When Spell Is Ready", true, [](const bool value) { config::visuals::draw_range_ready = value; });
					spell_ranges->add_separator();
					spell_ranges->add_checkbox("shulepin_aio_lucian_visuals_spell_ranges_q", "Draw Q Range", false, [](const bool value) { config::visuals::draw_q = value; });
					spell_ranges->add_colorpicker("shulepin_aio_lucian_visuals_spell_ranges_q_color", " ^ Q Color", 0x80FFFFFF, [](const uint32_t value) { config::visuals::draw_q_color = value; });
					spell_ranges->add_separator();
					spell_ranges->add_checkbox("shulepin_aio_lucian_visuals_spell_ranges_q2", "Draw Q Extended Range", true, [](const bool value) { config::visuals::draw_q2 = value; });
					spell_ranges->add_colorpicker("shulepin_aio_lucian_visuals_spell_ranges_q2_color", " ^ Q Extended Color", 0x80FFFFFF, [](const uint32_t value) { config::visuals::draw_q2_color = value; });
					spell_ranges->add_separator();
					spell_ranges->add_checkbox("shulepin_aio_lucian_visuals_spell_ranges_w", "Draw W Range", false, [](const bool value) { config::visuals::draw_w = value; });
					spell_ranges->add_colorpicker("shulepin_aio_lucian_visuals_spell_ranges_w_color", " ^ W Color", 0x80FFFFFF, [](const uint32_t value) { config::visuals::draw_w_color = value; });
					spell_ranges->add_separator();
					spell_ranges->add_checkbox("shulepin_aio_lucian_visuals_spell_ranges_e", "Draw E Range", false, [](const bool value) { config::visuals::draw_e = value; });
					spell_ranges->add_colorpicker("shulepin_aio_lucian_visuals_spell_ranges_e_color", " ^ E Color", 0x80FFFFFF, [](const uint32_t value) { config::visuals::draw_e_color = value; });
					spell_ranges->add_separator();
					spell_ranges->add_checkbox("shulepin_aio_lucian_visuals_spell_ranges_r", "Draw R Range", false, [](const bool value) { config::visuals::draw_r = value; });
					spell_ranges->add_colorpicker("shulepin_aio_lucian_visuals_spell_ranges_r_color", " ^ R Color", 0x80FFFFFF, [](const uint32_t value) { config::visuals::draw_r_color = value; });
				}
				const auto damage_indicator = visuals->add_sub_category("shulepin_aio_lucian_visuals_damage_indicator", "Damage Indicator");
				{
					damage_indicator->add_checkbox("shulepin_aio_lucian_visuals_damage_indicator_q", "Draw Q Damage", true, [](const bool value) { config::visuals::draw_q_damage = value; });
					damage_indicator->add_checkbox("shulepin_aio_lucian_visuals_damage_indicator_w", "Draw W Damage", true, [](const bool value) { config::visuals::draw_w_damage = value; });
					damage_indicator->add_checkbox("shulepin_aio_lucian_visuals_damage_indicator_e", "Draw E Damage", true, [](const bool value) { config::visuals::draw_e_damage = value; });
					damage_indicator->add_checkbox("shulepin_aio_lucian_visuals_damage_indicator_r", "Draw R Damage", true, [](const bool value) { config::visuals::draw_r_damage = value; });
					damage_indicator->add_checkbox("shulepin_aio_lucian_visuals_damage_indicator_aa", "Draw AA Damage", true, [](const bool value) { config::visuals::draw_aa_damage = value; });
				}
			}
			const auto keybindings = config->add_sub_category("shulepin_aio_lucian_keys", "Keybindings");
			{
				keybindings->add_hotkey("shulepin_aio_lucian_burst_toggle", "Burst Key", 0x01, false, false, [](const std::string* key, const bool value) { config::keybinds::burst_toggle_key = *key; config::keybinds::burst_toggle = value; });
				config::keybinds::burst_toggle_info = sdk::infotab->add_text({ "Burst Key [" + config::keybinds::burst_toggle_key + "]" }, []() -> infotab_sdk::text_entry
					{
						infotab_sdk::text_entry entry{};

						if (config::keybinds::burst_toggle)
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

				keybindings->add_hotkey("shulepin_aio_lucian_harass_toggle", "Harass Toggle Key", 0x47, true, true, [](const std::string* key, const bool value) { config::keybinds::harass_toggle_key = *key; config::keybinds::harass_toggle = value; });
				config::keybinds::harass_toggle_info = sdk::infotab->add_text({ "Harass Toggle [" + config::keybinds::harass_toggle_key + "]" }, []() -> infotab_sdk::text_entry
					{
						infotab_sdk::text_entry entry{};

						if (config::keybinds::harass_toggle)
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

				keybindings->add_hotkey("shulepin_aio_lucian_block_e_toggle", "Block E", 0x4D, false, true, [](const std::string* key, const bool value) { config::keybinds::block_e_toggle_key = *key; config::keybinds::block_e_toggle = value; });
				config::keybinds::block_e_toggle_info = sdk::infotab->add_text({ "Block E [" + config::keybinds::block_e_toggle_key + "]" }, []() -> infotab_sdk::text_entry
					{
						infotab_sdk::text_entry entry{};

						if (config::keybinds::block_e_toggle)
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

				keybindings->add_hotkey("shulepin_aio_lucian_semi_r_toggle", "Semi R", 0x54, false, false, [](const std::string* key, const bool value) { config::keybinds::semi_r_toggle_key = *key; config::keybinds::semi_r_toggle = value; });
				config::keybinds::semi_r_toggle_info = sdk::infotab->add_text({ "Semi R [" + config::keybinds::semi_r_toggle_key + "]" }, []() -> infotab_sdk::text_entry
					{
						infotab_sdk::text_entry entry{};

						if (config::keybinds::semi_r_toggle)
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

				keybindings->add_hotkey("shulepin_aio_lucian_magnet_r_toggle", "Magnet R", 0x47, false, true, [](const std::string* key, const bool value) { config::keybinds::magnet_r_toggle_key = *key; config::keybinds::magnet_r_toggle = value; });
				config::keybinds::magnet_r_toggle_info = sdk::infotab->add_text({ "Magnet R [" + config::keybinds::magnet_r_toggle_key + "]" }, []() -> infotab_sdk::text_entry
					{
						infotab_sdk::text_entry entry{};

						if (config::keybinds::magnet_r_toggle)
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
			}
		}
	}

	void load_events()
	{
		g_sdk->event_manager->register_callback(event_manager::event::game_update, reinterpret_cast<void*>(game_update));
		g_sdk->event_manager->register_callback(event_manager::event::present, reinterpret_cast<void*>(present));
		g_sdk->event_manager->register_callback(event_manager::event::execute_cast, reinterpret_cast<void*>(execute_cast));
		g_sdk->event_manager->register_callback(event_manager::event::process_cast, reinterpret_cast<void*>(process_cast));
		g_sdk->event_manager->register_callback(event_manager::event::buff_loss, reinterpret_cast<void*>(buff_loss));
		sdk::orbwalker->register_callback(orb_sdk::before_attack, reinterpret_cast<void*>(before_attack));
		sdk::orbwalker->register_callback(orb_sdk::before_move, reinterpret_cast<void*>(before_move));
	}

	void unload_events()
	{
		g_sdk->event_manager->unregister_callback(event_manager::event::game_update, reinterpret_cast<void*>(game_update));
		g_sdk->event_manager->unregister_callback(event_manager::event::present, reinterpret_cast<void*>(present));
		g_sdk->event_manager->unregister_callback(event_manager::event::execute_cast, reinterpret_cast<void*>(execute_cast));
		g_sdk->event_manager->unregister_callback(event_manager::event::process_cast, reinterpret_cast<void*>(process_cast));
		g_sdk->event_manager->unregister_callback(event_manager::event::buff_loss, reinterpret_cast<void*>(buff_loss));
		sdk::orbwalker->unregister_callback(orb_sdk::before_attack, reinterpret_cast<void*>(before_attack));
		sdk::orbwalker->unregister_callback(orb_sdk::before_move, reinterpret_cast<void*>(before_move));

		sdk::infotab->remove_text(config::keybinds::burst_toggle_info);
		sdk::infotab->remove_text(config::keybinds::harass_toggle_info);
		sdk::infotab->remove_text(config::keybinds::block_e_toggle_info);
		sdk::infotab->remove_text(config::keybinds::semi_r_toggle_info);
		sdk::infotab->remove_text(config::keybinds::magnet_r_toggle_info);
	}

	void load()
	{
		create_menu();
		load_events();
	}

	void unload()
	{
		unload_events();
	}

	bool is_on_burst_mode()
	{
		return config::keybinds::burst_toggle;
	}

	bool is_r_active()
	{
		auto const player = g_sdk->object_manager->get_local_player();

		if (player->get_buff_by_hash(2869241321))
		{
			return true;
		}

		return false;
	}

	bool should_block_e()
	{
		if (config::keybinds::block_e_toggle && !is_on_burst_mode())
		{
			return true;
		}

		return false;
	}

	bool is_dash_position_safe(const math::vector3 dash_position)
	{
		if (!config::combo::e::safety_checks::enabled || 
			!config::combo::e::safety_checks::turret_dive || 
			is_on_burst_mode()) return true;

		if (utils::is_position_inside_turret_range(dash_position))
		{
			return false;
		}

		return true;
	}

	pred_sdk::pred_data get_q_prediction(game_object* target, const int hitchance = 50, const float range = q_extended_range)
	{
		const auto player = g_sdk->object_manager->get_local_player();
		pred_sdk::spell_data pred_input{};
		pred_input.spell_type = pred_sdk::spell_type::linear;
		pred_input.targetting_type = pred_sdk::targetting_type::edge_to_edge;
		pred_input.expected_hitchance = hitchance;
		pred_input.spell_slot = 0;
		pred_input.range = range;
		pred_input.radius = q_radius;
		pred_input.delay = 0.4f - 0.15f / 17.f * (static_cast<float>(player->get_level()) - 1.f);
		pred_input.projectile_speed = FLT_MAX;
		auto pred_output = sdk::prediction->predict(target, pred_input);
		return pred_output;
	}

	pred_sdk::pred_data get_w_prediction(game_object* target, const int hitchance = 50, const float range = w_range, const bool ignore_collision = false)
	{
		pred_sdk::spell_data w_data{};
		w_data.spell_type = pred_sdk::spell_type::linear;
		w_data.targetting_type = pred_sdk::targetting_type::center;
		w_data.expected_hitchance = hitchance;
		w_data.spell_slot = 1;
		w_data.range = range;
		w_data.radius = w_radius;
		w_data.delay = w_delay;
		w_data.projectile_speed = w_speed;
		if (!ignore_collision) {
			w_data.forbidden_collisions =
			{
				pred_sdk::collision_type::unit,
				pred_sdk::collision_type::yasuo_wall,
			};
		}
		auto pred_output = sdk::prediction->predict(target, w_data);
		return pred_output;
	}

	math::vector3 get_w_fast_predicted_position(game_object* target)
	{
		auto const player = g_sdk->object_manager->get_local_player();
		const float travel_time = w_delay + (player->get_position().distance(target->get_position()) / w_speed);
		auto const predicted_position = sdk::prediction->predict_on_path(target, travel_time, true);
		return predicted_position;
	}

	pred_sdk::pred_data get_r_prediction(game_object* target)
	{
		pred_sdk::spell_data r_data{};
		r_data.spell_type = pred_sdk::spell_type::linear;
		r_data.targetting_type = pred_sdk::targetting_type::center;
		r_data.expected_hitchance = 0;
		r_data.spell_slot = 3;
		r_data.range = r_range;
		r_data.radius = r_radius;
		r_data.delay = r_delay;
		r_data.projectile_speed = r_speed;
		auto pred_output = sdk::prediction->predict(target,r_data);
		return pred_output;
	}

	math::vector3 get_r_fast_predicted_position(game_object* target)
	{
		auto const player = g_sdk->object_manager->get_local_player();
		const float travel_time = r_delay + (player->get_position().distance(target->get_position()) / r_speed);
		auto const predicted_position = sdk::prediction->predict_on_path(target, travel_time, true);
		return predicted_position;
	}

	float get_q_damage(game_object* target)
	{
		auto const player = g_sdk->object_manager->get_local_player();
		auto const q_damage = sdk::damage->get_spell_damage(player, 0);
		auto const total_damage = sdk::damage->calc_damage(dmg_sdk::damage_type::physical, player, target, q_damage);
		return total_damage;
	}

	float get_w_damage(game_object* target)
	{
		auto const player = g_sdk->object_manager->get_local_player();
		auto const w_damage = sdk::damage->get_spell_damage(player, 1);
		auto const total_damage = sdk::damage->calc_damage(dmg_sdk::damage_type::magical, player, target, w_damage);
		return total_damage;
	}

	float get_e_damage(game_object* target)
	{
		auto const player = g_sdk->object_manager->get_local_player();
		auto const aa_damage = sdk::damage->get_aa_damage(player, target);
		return aa_damage * 2;
	}

	float get_r_damage(game_object* target)
	{
		auto const player = g_sdk->object_manager->get_local_player();
		auto const r_damage = sdk::damage->get_spell_damage(player, 3);
		auto const total_damage = sdk::damage->calc_damage(dmg_sdk::damage_type::physical, player, target, r_damage);
		return total_damage;
	}

	float get_dash_range(game_object* target)
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const auto cursor_position = g_sdk->hud_manager->get_cursor_position();

		float dash_range = 200;
		const int dash_type = config::combo::e::dash_type;

		if (dash_type == 0)
		{
			dash_range = config::combo::e::dash_fixed_range;
		}
		else if (dash_type == 1)
		{
			auto const dist_to_target = player->get_position().distance(target->get_position());
			const float aa_delay = (dist_to_target / 2800.f);
			const float delay = aa_delay + (dist_to_target / 1350.f);
			const auto pred_position = sdk::prediction->predict_on_path(target, delay, true);
			const float dash_dynamic_range = pred_position.distance(player->get_position()) < (player->get_attack_range() + target->get_bounding_radius()) ? 200.f : 425.f;
			dash_range = dash_dynamic_range;
		}
		else if (dash_type == 2)
		{
			dash_range = player->get_position().distance(cursor_position) > 500.f ? 425.f : 200.f;
		}

		return dash_range;
	}

	math::vector3 get_dash_position(game_object* target)
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const int cast_type = config::combo::e::cast_position;
		const float dash_range = get_dash_range(target);
		auto cast_position = math::vector3();
		const auto cursor_position = g_sdk->hud_manager->get_cursor_position();
		const auto enemy_position = target->get_position();

		if (cast_type == 0)
		{
			cast_position = player->get_position().extended(cursor_position, dash_range);
		}
		else if (cast_type == 1)
		{
			cast_position = player->get_position().extended(enemy_position, dash_range);
		}
		else if (cast_type == 2)
		{
			cast_position = player->get_position().extended(cursor_position, dash_range);
		}
		else if (cast_type == 3)
		{
			cast_position = player->get_position().extended(cursor_position, dash_range);
		}

		return cast_position;
	}

	void cast_extended_q(game_object* target, const int hitchance, const float range, const bool check_aa_range = false)
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const auto attack_range = player->get_attack_range();
		const auto player_position = player->get_position();
		const auto target_position = target->get_position();

		const bool is_inside_aa_range = player_position.distance(target_position) < attack_range;
		const auto pred_output = get_q_prediction(target, hitchance, range);
		if (pred_output.is_valid && (check_aa_range && !is_inside_aa_range || !check_aa_range))
		{
			const auto extended_target_position = player_position.extended(pred_output.cast_position, range, true);
			for (const auto& minion : g_sdk->object_manager->get_minions())
			{
				if (!minion || !utils::is_valid_target(minion, q_range) || !utils::is_enemy(minion) || !minion->is_lane_minion()) continue;

				const auto minion_position = minion->get_position();
				const auto extended_minion_position = player_position.extended(minion_position, range, true);

				if (extended_target_position.distance(extended_minion_position) < target->get_bounding_radius())
				{
					issue_order_manager::cast_spell(0, minion);
				}
			}
		}
	}

	void cast_e(const math::vector3 dash_position, const bool safety_checks = true)
	{
		if (should_block_e()) return;

		if (safety_checks && is_dash_position_safe(dash_position) || !safety_checks)
		{
			issue_order_manager::cast_spell(2, dash_position);
		}
	}

	void q_extended_combat(game_object* target)
	{
		if (!target || !config::harass::q::white_list[target->get_char_name()] || !utils::is_enemy(target) || !utils::is_valid_target(target, q_extended_range) || !utils::is_spell_ready(0) || sdk::orbwalker->would_cancel_attack()) return;

		if (sdk::orbwalker->combo())
		{
			if (config::combo::use_q && config::combo::q::use_extended_q)
			{
				cast_extended_q(target, config::combo::q::hitchance, config::combo::q::max_q_range, config::combo::q::dont_use_extended_q_inside_aa);
			}
		}

		if (utils::is_on_harass(config::harass::extra::allow_to_use_in_other_modes, config::keybinds::harass_toggle))
		{
			if (config::harass::use_q && config::harass::q::use_extended_q && utils::is_enough_mana(config::harass::q::min_mana))
			{
				cast_extended_q(target, config::harass::q::hitchance, config::harass::q::max_q_range, config::harass::q::dont_use_extended_q_inside_aa);
			}
		}
	}

	void w_combat()
	{
		const auto target = sdk::target_selector->get_hero_target([](game_object* hero) {
			return utils::is_valid_target(hero, w_range);
		});

		if (!target || !config::harass::w::white_list[target->get_char_name()] || !utils::is_spell_ready(1) || sdk::orbwalker->would_cancel_attack()) return;

		if (utils::is_on_harass(config::harass::extra::allow_to_use_in_other_modes, config::keybinds::harass_toggle))
		{
			if (config::harass::use_w && utils::is_enough_mana(config::harass::q::min_mana))
			{
				const auto pred_output = get_w_prediction(target, config::harass::w::hitchance, config::harass::w::max_w_range, false);
				if (pred_output.is_valid)
				{
					issue_order_manager::cast_spell(1, pred_output.cast_position);
				}
			}
		}
	}

	void r_combat()
	{
		if (!config::combo::use_r) return;

		const auto player = g_sdk->object_manager->get_local_player();
		const auto target = sdk::target_selector->get_hero_target([](game_object* hero) {
			return utils::is_valid_target(hero, 1200.f);
		});

		if (target && config::keybinds::semi_r_toggle)
		{
			auto const pred_output = get_r_prediction(target);
			if (pred_output.is_valid)
			{
				issue_order_manager::cast_spell(3, pred_output.cast_position);
			}
		}

		if (config::combo::r::magnet && config::keybinds::magnet_r_toggle)
		{
			if (target && is_r_active())
			{
				auto const predicted_position = get_r_fast_predicted_position(target);
				const auto position = player->get_position().extended(r_cast_position, r_range, true);
				const auto dist_to_target = player->get_position().distance(predicted_position);
				const auto cursor_position = g_sdk->hud_manager->get_cursor_position();
				const auto v1_position = position.extended(predicted_position, position.distance(predicted_position) + dist_to_target, true);
				const auto v2_position = v1_position.extended(predicted_position, 200.f);
				const auto v3_position = v1_position.extended(predicted_position, -200.f);
				const auto v4_position = cursor_position.distance(v2_position) < cursor_position.distance(v3_position) ? v2_position : v3_position;

				issue_order_manager::move_to(v4_position);

				should_block_movement = true;
			}
			else
			{
				should_block_movement = false;
			}
		}

	}

	void combat(game_object* object, spell_cast* cast)
	{
		const auto target = cast->get_target();
		const auto combo_spell_priority = config::combo::priority;
		const auto block_e_state = should_block_e();

		if (!target || !target->is_hero() || !utils::is_valid_target(target, q_extended_range) || !utils::is_me(object) || !utils::is_auto_attack(cast)) return;

		if (sdk::orbwalker->combo())
		{
			if (config::combo::use_e && !block_e_state && (combo_spell_priority == 2 || !utils::is_spell_ready(combo_spell_priority)) && utils::is_spell_ready(2))
			{
				const auto cast_position = get_dash_position(target);
				cast_e(cast_position);
			}
			else if (config::combo::use_q && (combo_spell_priority == 0 || !utils::is_spell_ready(combo_spell_priority) || block_e_state) && utils::is_spell_ready(0))
			{
				issue_order_manager::cast_spell(0, target);
			}
			else if (config::combo::use_w && (combo_spell_priority == 1 || !utils::is_spell_ready(combo_spell_priority) || block_e_state) && utils::is_spell_ready(1))
			{
				if (config::combo::w::fast_prediction)
				{
					const auto cast_position = get_w_fast_predicted_position(target);
					issue_order_manager::cast_spell(1, cast_position);
				}
				else
				{
					const auto pred_output = get_w_prediction(target, config::combo::w::hitchance, config::combo::w::max_w_range, config::combo::w::ignore_collision);
					if (pred_output.is_valid)
					{
						issue_order_manager::cast_spell(1, pred_output.cast_position);
					}
				}
			}
		}

		if (utils::is_on_harass(config::harass::extra::allow_to_use_in_other_modes, config::keybinds::harass_toggle))
		{
			if (config::harass::use_q && utils::is_enough_mana(config::harass::q::min_mana))
			{
				issue_order_manager::cast_spell(0, target);
			}
		}
	}

	void anti_melee_logic(game_object* hero)
	{
		if (!utils::is_spell_ready(2) || !config::combo::e::safety_checks::anti_melee::enabled ||
			!config::combo::e::safety_checks::anti_melee::white_list[hero->get_char_name()] ||
			!hero || !utils::is_enemy(hero) || !utils::is_valid_target(hero, q_extended_range) ||
			sdk::orbwalker->would_cancel_attack()) return;

		auto const player = g_sdk->object_manager->get_local_player();
		auto const cursor_position = g_sdk->hud_manager->get_cursor_position();
		auto const danger_base_range = hero->is_melee() ? hero->get_attack_range() : 150.f;
		auto const danger_total_range = danger_base_range + hero->get_bounding_radius() + config::combo::e::safety_checks::anti_melee::extra_range;

		if (player->get_position().distance(hero->get_position()) < danger_total_range && 
			utils::get_percent_health(player) < config::combo::e::safety_checks::anti_melee::my_hp &&
			utils::get_percent_health(hero) > config::combo::e::safety_checks::anti_melee::enemy_hp)
		{
			auto const dash_position = player->get_position().extended(hero->get_position(), -425.f);
			cast_e(dash_position, false);
		}
	}

	void hero_loop()
	{
		for (const auto& hero : g_sdk->object_manager->get_heroes())
		{
			q_extended_combat(hero);
			anti_melee_logic(hero);
		}
	}

	void draw_spell_ranges()
	{
		if (!config::visuals::draw_range)
			return;

		auto const player = g_sdk->object_manager->get_local_player();

		if (utils::is_spell_ready(0) || !config::visuals::draw_range_ready)
		{
			if (config::visuals::draw_q)
				g_sdk->renderer->add_circle_3d(player->get_position(), q_range, 2.f, config::visuals::draw_q_color);
			if (config::visuals::draw_q2)
				g_sdk->renderer->add_circle_3d(player->get_position(), q_extended_range, 2.f, config::visuals::draw_q2_color);
		}

		if ((utils::is_spell_ready(1) || !config::visuals::draw_range_ready) && config::visuals::draw_w)
		{
			g_sdk->renderer->add_circle_3d(player->get_position(), w_range, 2.f, config::visuals::draw_w_color);
		}

		if ((utils::is_spell_ready(2) || !config::visuals::draw_range_ready) && config::visuals::draw_e)
		{
			g_sdk->renderer->add_circle_3d(player->get_position(), 425.f, 2.f, config::visuals::draw_e_color);
		}

		if ((utils::is_spell_ready(3) || !config::visuals::draw_range_ready) && config::visuals::draw_r)
		{
			g_sdk->renderer->add_circle_3d(player->get_position(), r_range, 2.f, config::visuals::draw_r_color);
		}
	}

	void draw_damage_indicator()
	{
		for (const auto& hero : g_sdk->object_manager->get_heroes())
		{
			float total_damage = 0.f;
			if (hero && utils::is_valid_target(hero) && utils::is_enemy(hero))
			{
				if (config::visuals::draw_q_damage && utils::is_spell_ready(0))
				{
					total_damage = total_damage + get_q_damage(hero);
				}
				if (config::visuals::draw_w_damage && utils::is_spell_ready(1))
				{
					total_damage = total_damage + get_w_damage(hero);
				}
				if (config::visuals::draw_e_damage && utils::is_spell_ready(2))
				{
					total_damage = total_damage + get_e_damage(hero);
				}
				if (config::visuals::draw_r_damage && utils::is_spell_ready(3))
				{
					total_damage = total_damage + get_r_damage(hero);
				}
				if (config::visuals::draw_aa_damage)
				{
					total_damage = total_damage + sdk::damage->get_aa_damage(g_sdk->object_manager->get_local_player(), hero);
				}
				g_sdk->renderer->add_damage_indicator(hero, total_damage);
			}
		}
	}

	void __fastcall game_update()
	{
		hero_loop();
		w_combat();
		r_combat();
	}

	void __fastcall present()
	{
		draw_spell_ranges();
		draw_damage_indicator();
	}

	void __fastcall execute_cast(game_object* object, spell_cast* cast)
	{
		combat(object, cast);
	}

	void __fastcall process_cast(game_object* object, spell_cast* cast)
	{
		auto const name = cast->get_spell_data()->get_static_data()->get_name();
		if (utils::is_me(object) && name == std::string("LucianR"))
		{
			r_cast_position = cast->get_end_pos();
		}
	}

	void __fastcall buff_loss(game_object* object, buff_instance* buff)
	{
		auto const buff_hash = buff->get_hash();
		if (utils::is_me(object) && buff_hash == 2869241321)
		{
			should_block_movement = false;
		}
	}

	bool __fastcall before_attack(orb_sdk::event_data* data)
	{
		if (is_r_active())
		{
			return false;
		}
		return true;
	}

	bool __fastcall before_move(orb_sdk::event_data* data)
	{
		if (config::combo::r::magnet && should_block_movement)
		{
			return false;
		}
		return true;
	}

}
