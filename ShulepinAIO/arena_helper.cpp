#include "sdk.hpp"
#include "arena_helper.h"
#include <format>
#include <sstream>
#include <iomanip>
#include <unordered_set>

#include "spell.h"
#include "utils.h"

namespace arena_helper
{
	script::spell* trinket;
	
	void __fastcall game_update();
	void __fastcall present();
	bool __fastcall before_attack(orb_sdk::event_data* data);
	void __fastcall execute_cast(game_object* object, spell_cast* cast);
	void __fastcall create_object(game_object* object);
	void __fastcall delete_object(game_object* object);

	struct plant_info
	{
		math::vector3 position;
		float respawn_time;
	};
	struct hero_info
	{
		math::vector3 position;
		float time;
	};
	std::unordered_set<game_object*> cherry_plant_powerups;
	std::unordered_map<game_object*, plant_info> untargetable_plants;
	std::unordered_map<game_object*, hero_info> missing_heroes;
	using on_plant_untargetable_callback_t = std::function<void(game_object*)>;
	on_plant_untargetable_callback_t on_plant_untargetable_callback;
	std::unordered_map<game_object*, bool> plant_states;
	std::unordered_map<game_object*, bool> hero_states;
	bool should_block_attack = false;
	
	namespace config::plants
	{
		bool enabled;
		bool key;
		std::string key_string;
	}

	namespace config::trinket
	{
		bool enabled;
		bool last_seen_t;
	}
	
	void create_menu()
	{
		const auto& player = g_sdk->object_manager->get_local_player();
		const auto config = g_sdk->menu_manager->add_category("shulepin_aio_arena_helper", "ShulepinAIO - Arena Helper");
		{
			const auto trinket_config = config->add_sub_category("shulepin_aio_arena_helper_trinket", "Trinket Helper");
			{
				trinket_config->add_checkbox("shulepin_aio_arena_helper_trinket_enabled", "Cast trinket on invisible enemies", true, [](const bool value) { config::trinket::enabled = value; });
				trinket_config->add_slider_int("shulepin_aio_arena_helper_trinket_last_seen_time", "Last seen time < X seconds", 1, 10, 1, 3, [](const int value){ config::trinket::last_seen_t = value; });
			}
			const auto plant_config = config->add_sub_category("shulepin_aio_arena_helper_plants", "Plants Helper");
			{
				plant_config->add_checkbox("shulepin_aio_arena_helper_plants_enabled", "Track plants", true, [](const bool value) { config::plants::enabled = value; });
				plant_config->add_hotkey("shulepin_aio_arena_helper_plants_key", "Additional key", 0x01, false, false, [](std::string*, bool value)
					{
						config::plants::key = value;
					});
				plant_config->add_label("In order to attack plant you need to hold Combo key + Additional key");
			}
		}
	}

	void load_events()
	{
		g_sdk->event_manager->register_callback(event_manager::event::game_update, reinterpret_cast<void*>(game_update));
		g_sdk->event_manager->register_callback(event_manager::event::present, reinterpret_cast<void*>(present));
		g_sdk->event_manager->register_callback(event_manager::event::execute_cast, reinterpret_cast<void*>(execute_cast));
		sdk::orbwalker->register_callback(orb_sdk::before_attack, reinterpret_cast<void*>(before_attack));
		g_sdk->event_manager->register_callback(event_manager::event::create_object, reinterpret_cast<void*>(create_object));
		g_sdk->event_manager->register_callback(event_manager::event::delete_object, reinterpret_cast<void*>(delete_object));
	}

	void unload_events()
	{
		g_sdk->event_manager->unregister_callback(event_manager::event::game_update, reinterpret_cast<void*>(game_update));
		g_sdk->event_manager->unregister_callback(event_manager::event::present, reinterpret_cast<void*>(present));
		g_sdk->event_manager->unregister_callback(event_manager::event::execute_cast, reinterpret_cast<void*>(execute_cast));
		sdk::orbwalker->unregister_callback(orb_sdk::before_attack, reinterpret_cast<void*>(before_attack));
		g_sdk->event_manager->unregister_callback(event_manager::event::create_object, reinterpret_cast<void*>(create_object));
		g_sdk->event_manager->unregister_callback(event_manager::event::delete_object, reinterpret_cast<void*>(delete_object));
	}

	void load()
	{
		trinket = new script::spell(12, 800.f);
		
		create_menu();
		load_events();
	}

	void unload()
	{
		unload_events();
	}

	void on_hero_vision_lost(game_object* hero)
	{
		hero_info info;
		info.position = hero->get_position();
		info.time = g_sdk->clock_facade->get_game_time();

		missing_heroes[hero] = info;

		g_sdk->log_console("[+] Hero %s has lost from vision at position (%f, %f, %f) at time %f seconds.",
			hero->get_char_name().c_str(),
			info.position.x,
			info.position.y,
			info.position.z,
			info.time);
	}
	
	void on_plant_untargetable(game_object* plant)
	{
		constexpr auto respawn_time = 15.f;

		plant_info info;
		info.position = plant->get_position();
		info.respawn_time = g_sdk->clock_facade->get_game_time() + respawn_time;

		untargetable_plants[plant] = info;

		g_sdk->log_console("[+] Plant %s has become untargetable at position (%f, %f, %f) at time %f seconds.",
			plant->get_name().c_str(),
			info.position.x,
			info.position.y,
			info.position.z,
			info.respawn_time);
	}

	void check_heroes()
	{
		const auto& heroes = g_sdk->object_manager->get_heroes();

		for (auto&& hero : heroes)
		{
			if (!hero ||
				!utils::is_enemy(hero) ||
				hero->is_dead() ||
				!hero->is_targetable())
			{
				continue;
			}

			bool is_visible = hero->is_visible();
			auto it = hero_states.find(hero);

			if (it == hero_states.end())
			{
				hero_states.emplace(hero, is_visible);
			}
			else
			{
				const bool was_visible = it->second;

				if (was_visible && !is_visible)
				{
					on_hero_vision_lost(hero);
				}

				it->second = is_visible;
			}
		}
	}
	
	void check_plants()
	{
		const auto& plants = cherry_plant_powerups;

		for (auto&& plant : plants)
		{
			if (!plant)
			{
				continue;
			}
			
			bool is_targetable = plant->is_targetable() || plant->is_dead();

			auto it = plant_states.find(plant);

			if (it == plant_states.end())
			{
				plant_states.emplace(plant, is_targetable);
			}
			else
			{
				const bool was_targetable = it->second;

				if (was_targetable && !is_targetable)
				{
					on_plant_untargetable(plant);
				}

				it->second = is_targetable;
			}

			if (utils::is_valid_target(plant) &&
				sdk::orbwalker->combo() &&
				config::plants::key &&
				sdk::orbwalker->is_in_auto_attack_range(g_sdk->object_manager->get_local_player(), plant))
			{
				should_block_attack = true;

				if (!sdk::orbwalker->would_cancel_attack() &&
					sdk::orbwalker->is_attack_ready_in(0.f))
				{
					sdk::orbwalker->attack(plant);
				}
			}
		}
	}

	void trinket_logic()
	{
		const auto& player = g_sdk->object_manager->get_local_player();
		const auto& heroes = g_sdk->object_manager->get_heroes();
		const float current_time = g_sdk->clock_facade->get_game_time();

		for (const auto& hero : heroes)
		{
			if (!hero || !utils::is_enemy(hero))
				continue;

			const auto it = missing_heroes.find(hero);

			// If the hero is currently missing from our records, skip them
			if (it == missing_heroes.end())
				continue;

			const hero_info& info = it->second;

			// Calculate the time the hero has been invisible
			const float time_invisible = current_time - info.time;
			const auto cast_position = player->get_position().extended(info.position, 350.f / 2.f);
			
			// If the enemy has been invisible for less than X seconds, attempt to cast the trinket
			if (!hero->is_visible() &&
				time_invisible < static_cast<float>(config::trinket::last_seen_t) &&
				trinket->is_ready() &&
				cast_position.distance(player->get_position()) < trinket->get_range() + 350.f / 2.f)
			{
				// We can cast the trinket at the last known position
				if (trinket->cast_spell(cast_position))
				{
					g_sdk->log_console("[+] Casting trinket at %s's last known position: (%f, %f, %f)",
						hero->get_char_name().c_str(),
						info.position.x,
						info.position.y,
						info.position.z);
					break; 
				}
			}
		}
	}
	
	void __fastcall game_update()
	{
		if (config::plants::enabled)
		{
			check_plants();
		}
		
		if (config::trinket::enabled)
		{
			check_heroes();
			trinket_logic();
		}
	}

	void __fastcall present()
	{
		if (config::plants::enabled)
		{
			const auto player = g_sdk->object_manager->get_local_player();
			const auto player_pos = player->get_position();
			const auto& plants = cherry_plant_powerups; //g_sdk->object_manager->get_plants();

			for (auto&& plant : plants)
			{
				if (player_pos.distance(plant->get_position()) > 5000.f)
				{
					continue;
				}

				const bool is_targetable = plant->is_targetable();
				uint32_t color = 0xFF00FF00;

				if (!is_targetable)
				{
					auto it = untargetable_plants.find(plant);
					if (it != untargetable_plants.end())
					{
						const plant_info& info = it->second;
						const auto respawn_time = info.respawn_time - g_sdk->clock_facade->get_game_time();

						std::ostringstream stream;
						stream << std::fixed << std::setprecision(1) << respawn_time;
						std::string time_text = stream.str();
					
						//color = utils::interpolate_color(0xFF00FF00, 0xFFFFFF00, respawn_time / 15.f);
						color = 0xFFFF0000;
					
						if (respawn_time >= 0.f)
						{
							math::vector2 screen_position = g_sdk->renderer->world_to_screen(info.position);
							g_sdk->renderer->add_text(time_text, 20, screen_position, 1, 0xFFFFFFFF);
						}
					}
				}
			
				g_sdk->renderer->add_circle_3d(plant->get_position(), 75.f, 3.f, color);

				const auto v1 = math::vector3(player_pos.x, plant->get_position().y, player_pos.z);
				const auto v2 = v1.extended(plant->get_position(), player_pos.distance(plant->get_position()) - 75.f, true);
				const auto v1_screen = g_sdk->renderer->world_to_screen(v1);
				const auto v2_screen = g_sdk->renderer->world_to_screen(v2);

				g_sdk->renderer->add_line_2d(v1_screen, v2_screen, 3.f, color);
			}
		}
	}

	bool before_attack(orb_sdk::event_data* data)
	{
		if (data->target)
		{
			if (data->target->is_hero() && config::plants::key && should_block_attack)
			{
				return false;
			}
		}

		return true;
	}

	void __fastcall execute_cast(game_object* object, spell_cast* cast)
	{
		if (utils::is_me(object) && utils::is_auto_attack(cast))
		{
			should_block_attack = false;
		}
	}

	void __fastcall create_object(game_object* object)
	{
		if (object->get_name() == std::string("Honeyfruit") || object->get_name() == std::string("PowerupPlant"))
		{
			cherry_plant_powerups.insert(object);
			on_plant_untargetable(object);
			g_sdk->log_console("[+] Cherry Plant Powerup created");
		}
	}

	void __fastcall delete_object(game_object* object)
	{
		if (object->get_name() == std::string("Honeyfruit") || object->get_name() == std::string("PowerupPlant"))
		{
			cherry_plant_powerups.erase(object);
			on_plant_untargetable(object);
			g_sdk->log_console("[+] Cherry Plant Powerup deleted");
		}
	}
}