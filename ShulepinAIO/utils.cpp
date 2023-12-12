#include "sdk.hpp"
#include "utils.h"
#include <cstdint>

namespace utils
{
	float get_ping()
	{
		return static_cast<float>(g_sdk->net_client->get_ping()) / 1000.f;
	}

	float get_percent_health(game_object* object)
	{
		return 100.f * object->get_hp() / object->get_max_hp();
	}

	float get_percent_mana(game_object* object)
	{
		return 100.f * object->get_par() / object->get_max_par();
	}

	float get_real_health(game_object* object, const std::string& damage_type, const bool use_pred, const float t)
	{
		const auto game_time = g_sdk->clock_facade->get_game_time();
		const auto hp = use_pred ? sdk::health_prediction->get_predicted_health(object, game_time + t) : object->get_hp();

		if (damage_type == "AD")
		{
			return hp + object->get_physical_shield();
		}
		if (damage_type == "AP")
		{
			return hp + object->get_magical_shield();
		}
		if (damage_type == "ALL")
		{
			return hp + object->get_all_shield();
		}
		return hp + object->get_all_shield();
	}

	float get_real_attack_range(game_object* target, game_object* source)
	{
		return source->get_attack_range() + target->get_bounding_radius() + source->get_bounding_radius();
	}

	bool is_enough_mana(const float value)
	{
		return get_percent_mana() > value;
	}

	bool is_me(game_object* object)
	{
		const auto& player = g_sdk->object_manager->get_local_player();
		return object->get_id() == player->get_id();
	}

	bool is_enemy(game_object* object)
	{
		const auto& player = g_sdk->object_manager->get_local_player();
		return object->get_team_id() != player->get_team_id();
	}

	bool is_auto_attack(spell_cast* cast)
	{
		return cast->is_basic_attack() || cast->is_special_attack();
	}

	bool is_spell_ready(const int spell_slot, const float extra_time)
	{
		const auto& player = g_sdk->object_manager->get_local_player();
		const auto& spell_state = player->get_spell_cast_state(spell_slot);

		if (spell_state == 0)
		{
			return true;
		}

		const auto cooldown = player->get_spell(spell_slot)->get_cooldown();
		return cooldown < extra_time && (spell_state & (1 << 5)) != 0;
	}

	bool is_on_cc(game_object* hero, const float time, const float extra_time)
	{
		for (auto&& buff : hero->get_buffs())
		{
			if (!buff)
			{
				continue;
			}

			// ? Airborne, Sleep, Stasis, Suspension ? //
			if (buff->get_type() == buff_type::snare ||
				buff->get_type() == buff_type::stun ||
				buff->get_type() == buff_type::suppression ||
				buff->get_type() == buff_type::charm ||
				buff->get_type() == buff_type::flee ||
				buff->get_type() == buff_type::taunt ||
				buff->get_type() == buff_type::berserk)
			{
				const auto end_time = buff->get_end_time();

				if (time + extra_time > end_time - g_sdk->clock_facade->get_game_time())
				{
					continue;
				}
				
				return true;
			}
		}

		return false;
	}
	
	bool is_valid_target(game_object* hero, const float range)
	{
		const auto& player = g_sdk->object_manager->get_local_player();

		return
			hero &&
			hero->is_valid() && 
			hero->is_visible() && 
			hero->is_targetable() &&
			hero->get_position().distance(player->get_position()) <= range;
	}

	bool is_on_harass(const bool allow_to_use_in_other_modes, const bool global_var_state)
	{
		if (!global_var_state) return false;

		if (allow_to_use_in_other_modes)
		{
			return sdk::orbwalker->harass() || sdk::orbwalker->clear() || sdk::orbwalker->lasthit();
		}

		return sdk::orbwalker->harass();
	}

	bool is_position_inside_turret_range(const math::vector3 position, const float extra_range)
	{
		for (const auto& turret : g_sdk->object_manager->get_turrets())
		{
			if (!turret || !turret->is_valid() || turret->is_dead()) continue;

			const auto total_range = 750.f + turret->get_bounding_radius() +
				g_sdk->object_manager->get_local_player()->get_bounding_radius() + extra_range;

			if (position.distance(turret->get_position()) < total_range)
			{
				return true;
			}
		}

		return false;
	}

	game_object* get_target(const float range)
	{
		return sdk::target_selector->get_hero_target([&range](game_object* hero)
		{
			return is_valid_target(hero, range);
		});
	}

	game_object* get_target_with_list(const float range, std::unordered_map<std::string, bool> list)
	{
		return sdk::target_selector->get_hero_target([&range, &list](game_object* hero)
		{
			return is_valid_target(hero, range) && !list[hero->get_char_name()];
		});
	}

	uint32_t interpolate_color(uint32_t color1, uint32_t color2, float t)
	{
		uint8_t r1 = (color1 >> 16) & 0xFF;
		uint8_t g1 = (color1 >> 8) & 0xFF;
		uint8_t b1 = color1 & 0xFF;
		
		uint8_t r2 = (color2 >> 16) & 0xFF;
		uint8_t g2 = (color2 >> 8) & 0xFF;
		uint8_t b2 = color2 & 0xFF;
		
		uint8_t r = static_cast<uint8_t>(r1 + t * (r2 - r1));
		uint8_t g = static_cast<uint8_t>(g1 + t * (g2 - g1));
		uint8_t b = static_cast<uint8_t>(b1 + t * (b2 - b1));
		
		return (0xFF << 24) | (r << 16) | (g << 8) | b;
	}
}
