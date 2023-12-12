#pragma once
#include "sdk.hpp"

namespace issue_order_manager
{
	extern float get_last_issued_order_time_by_slot(const int spell_slot);
	extern bool is_issue_order_passed(const int spell_slot, const float value);

	extern bool move_to(const math::vector3 position, const float t = 0.1f);
	extern bool cast_spell(const int spell_slot, const float t = 0.1f);
	extern bool cast_spell(const int spell_slot, game_object* target, const float t = 0.1f);
	extern bool cast_spell(const int spell_slot, math::vector3 cast_position, const float t = 0.1f);
	extern bool cast_spell(const int spell_slot, const math::vector3 start_position, const math::vector3 end_position, const float t = 0.1f);
	extern bool release(const int spell_slot, const math::vector3 cast_position, const bool release_cast = true, const float t = 0.1f);
}
