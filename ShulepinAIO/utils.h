#pragma once
#include "sdk.hpp"

namespace utils
{
	extern float get_ping();
	extern float get_percent_health(game_object* object);
	extern float get_percent_mana(game_object* object = g_sdk->object_manager->get_local_player());
	extern float get_real_health(game_object* object, const std::string& damage_type = "ALL", const bool use_pred = false, const float t = 0.f);
	extern float get_real_attack_range(game_object* target, game_object* source = g_sdk->object_manager->get_local_player());
	extern game_object* get_target(const float range = FLT_MAX);
	extern bool is_me(game_object* object);
	extern bool is_enough_mana(const float value);
	extern bool is_auto_attack(spell_cast* cast);
	extern bool is_enemy(game_object* object);
	extern bool is_spell_ready(const int spell_slot, const float extra_time = 0);
	extern bool is_valid_target(game_object* hero, const float range = FLT_MAX);
	extern bool is_on_cc(game_object* hero, const float time = 0.f, const float extra_time = 0.f);
	extern bool is_on_harass(const bool allow_to_use_in_other_modes = true, const bool global_var_state = true);
	extern bool is_position_inside_turret_range(const math::vector3 position, const float extra_range = 0.f);
	extern game_object* get_target(const float range);
	extern game_object* get_target_with_list(const float range, std::unordered_map<std::string, bool> list);
	extern uint32_t interpolate_color(uint32_t color1, uint32_t color2, float t);
}
