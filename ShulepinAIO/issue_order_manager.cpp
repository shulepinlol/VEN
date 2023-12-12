#include "issue_order_manager.h"

namespace issue_order_manager
{
	std::unordered_map<int, float> last_issued_order;
	float last_issued_move_order = 0;

	float get_last_issued_order_time_by_slot(const int spell_slot)
	{
		return last_issued_order[spell_slot];
	}

	bool is_issue_order_passed(const int spell_slot, const float value)
	{
		return get_last_issued_order_time_by_slot(spell_slot) + value < g_sdk->clock_facade->get_game_time();
	}

	bool move_to(const math::vector3 position, const float t)
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const float time = g_sdk->clock_facade->get_game_time();
		if (last_issued_move_order + t < time)
		{
			player->issue_order(game_object_order::move_to, position);
			last_issued_move_order = time;
			return true;
		}
		return false;
	}

	bool cast_spell(const int spell_slot, const float t)
	{
		auto const player = g_sdk->object_manager->get_local_player();
		const auto time = g_sdk->clock_facade->get_game_time();
		if (last_issued_order[spell_slot] + t < time)
		{
			player->cast_spell(spell_slot);
			last_issued_order[spell_slot] = time;
			return true;
		}
		return false;
	}

	bool cast_spell(const int spell_slot, game_object* target, const float t)
	{
		auto const player = g_sdk->object_manager->get_local_player();
		const auto time = g_sdk->clock_facade->get_game_time();
		if (last_issued_order[spell_slot] + t < time)
		{
			player->cast_spell(spell_slot, target);
			last_issued_order[spell_slot] = time;
			return true;
		}
		return false;
	}

	bool cast_spell(const int spell_slot, const math::vector3 cast_position, const float t)
	{
		auto const player = g_sdk->object_manager->get_local_player();
		const auto time = g_sdk->clock_facade->get_game_time();
		if (last_issued_order[spell_slot] + t < time)
		{
			player->cast_spell(spell_slot, cast_position);
			last_issued_order[spell_slot] = time;
			return true;
		}
		return false;
	}

	bool cast_spell(const int spell_slot, const math::vector3 start_position, const math::vector3 end_position, const float t)
	{
		auto const player = g_sdk->object_manager->get_local_player();
		const auto time = g_sdk->clock_facade->get_game_time();
		if (last_issued_order[spell_slot] + t < time)
		{
			player->cast_spell(spell_slot, start_position, end_position);
			last_issued_order[spell_slot] = time;
			return true;
		}
		return false;
	}

	bool release(const int spell_slot, const math::vector3 cast_position, const bool release_cast, const float t)
	{
		auto const player = g_sdk->object_manager->get_local_player();
		const auto time = g_sdk->clock_facade->get_game_time();
		if (last_issued_order[spell_slot] + t < time)
		{
			player->update_chargeable_spell(spell_slot, cast_position, release_cast);
			last_issued_order[spell_slot] = time;
			return true;
		}
		return false;
	}
}
