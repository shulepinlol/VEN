#include "spell.h"
#include "utils.h"

namespace script
{
	spell::spell(const int spell_slot, const float range): spell_type_(), spell_targetting_type_(), spell_delay_(0),
	                                                       spell_radius_(0),
	                                                       spell_speed_(0), min_range_(0), max_range_(0), is_charged_(false)
	{
		spell_slot_ = spell_slot;
		spell_range_ = range;
	}

	void spell::set_skillshot(const pred_sdk::spell_type spell_type, const pred_sdk::targetting_type targetting_type,
	                          const float delay, const float radius, const float speed,
	                          const std::vector<pred_sdk::collision_type>& collision_data)
	{
		spell_type_ = spell_type;
		spell_targetting_type_ = targetting_type;
		spell_delay_ = delay;
		spell_radius_ = radius;
		spell_speed_ = speed;
		spell_collision_data_ = collision_data;
	}

	void spell::set_range(const float range)
	{
		spell_range_ = range;
	}

	void spell::set_spell_type(const pred_sdk::spell_type spell_type)
	{
		spell_type_ = spell_type;
	}

	void spell::set_targetting_type(const pred_sdk::targetting_type spell_targetting_type)
	{
		spell_targetting_type_ = spell_targetting_type;
	}

	void spell::set_delay(const float delay)
	{
		spell_delay_ = delay;
	}

	void spell::set_radius(const float radius)
	{
		spell_radius_ = radius;
	}

	void spell::set_speed(const float speed)
	{
		spell_speed_ = speed;
	}

	void spell::set_collision(const std::vector<pred_sdk::collision_type>& spell_collision_data)
	{
		spell_collision_data_ = spell_collision_data;
	}

	void spell::set_min_range(const float min_range)
	{
		min_range_ = min_range;
	}

	void spell::set_max_range(const float max_range)
	{
		max_range_ = max_range;
	}

	void spell::set_charged(const bool value)
	{
		is_charged_ = value;
	}

	float spell::get_min_range() const
	{
		return min_range_;
	}

	float spell::get_max_range() const
	{
		return max_range_;
	}

	void spell::add_charged_buff_mapping(const std::string& champion_name, uint32_t buff_hash)
	{
		charged_buff_map_[champion_name] = buff_hash;
	}

	int spell::get_level() const
	{
		return g_sdk->object_manager->get_local_player()->get_spell(spell_slot_)->get_level();
	}

	float spell::get_charged_time(const float duration) const
	{
		const auto& player = g_sdk->object_manager->get_local_player();
		const auto it = charged_buff_map_.find(player->get_char_name());

		if (it != charged_buff_map_.end())
		{
			const auto buff = player->get_buff_by_hash(it->second);
			const auto time = g_sdk->clock_facade->get_game_time();

			if (buff)
			{
				return fmaxf(0.f, fminf(1.f, (time - buff->get_start_time() + (g_sdk->net_client->get_ping() / 1000) * 0.033f) / duration));
			}
		}

		return 0.f;
	}

	float spell::get_charged_range(const float max_range) const
	{
		return min_range_ + fminf((max_range - min_range_), (max_range - min_range_) * get_charged_time());
	}

	pred_sdk::spell_type spell::get_spell_type() const
	{
		return spell_type_;
	}

	pred_sdk::targetting_type spell::get_targetting_type() const
	{
		return spell_targetting_type_;
	}

	float spell::get_range() const
	{
		return spell_range_;
	}

	float spell::get_delay() const
	{
		return spell_delay_;
	}

	float spell::get_radius() const
	{
		return spell_radius_;
	}

	float spell::get_speed() const
	{
		return spell_speed_;
	}

	std::vector<pred_sdk::collision_type> spell::get_collision() const
	{
		return spell_collision_data_;
	}

	bool spell::get_charged() const
	{
		return is_charged_;
	}


	float spell::get_cooldown() const
	{
		return g_sdk->object_manager->get_local_player()->get_spell(spell_slot_)->get_cooldown();
	}

	uint8_t spell::get_toggle_state() const
	{
		return g_sdk->object_manager->get_local_player()->get_spell(spell_slot_)->get_toggle_state();
	}

	bool spell::is_charging()
	{
		return get_charged_time() > 0.f;
	}

	bool spell::is_ready(const float extra_time) const
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const auto spell_state = player->get_spell_cast_state(spell_slot_);

		if (spell_state == 0)
		{
			return true;
		}

		const auto cooldown = player->get_spell(spell_slot_)->get_cooldown();
		return cooldown < extra_time && (spell_state & (1 << 5)) != 0;
	}

	bool spell::is_enough_mana_pct(const float value)
	{
		return utils::is_enough_mana(value);
	}

	bool spell::is_issue_order_passed(const float value) const
	{
		return spell_last_cast_t_ + value < g_sdk->clock_facade->get_game_time();
	}

	pred_sdk::pred_data spell::get_prediction(game_object* target, const int hitchance, const float range) const
	{
		pred_sdk::spell_data pred_input{};

		pred_input.spell_type = spell_type_;
		pred_input.targetting_type = spell_targetting_type_;
		pred_input.expected_hitchance = hitchance;
		pred_input.spell_slot = spell_slot_;
		pred_input.range = is_charged_ ? get_charged_range(range) : range;
		pred_input.radius = spell_radius_;
		pred_input.delay = spell_delay_;
		pred_input.projectile_speed = spell_speed_;
		pred_input.forbidden_collisions = spell_collision_data_;

		auto pred_output = sdk::prediction->predict(target, pred_input);
		return pred_output;
	}

	uint32_t spell::get_stacks() const
	{
		return g_sdk->object_manager->get_local_player()->get_spell(spell_slot_)->get_charges();
	}

	float spell::get_travel_time(game_object* target) const
	{
		return spell_delay_ + (g_sdk->object_manager->get_local_player()->get_position().distance(target->get_position()) / spell_speed_);
	}

	bool spell::release(const math::vector3 cast_position, const bool release_cast, const float t)
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const auto time = g_sdk->clock_facade->get_game_time();
		if (spell_last_cast_t_ + t < time)
		{
			player->update_chargeable_spell(spell_slot_, cast_position, release_cast);
			spell_last_cast_t_ = time;
			g_sdk->log_console("[+] spell order %d was issued at %f", spell_slot_, spell_last_cast_t_);
			return true;
		}
		return false;
	}

	bool spell::cast_spell(const float t)
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const auto time = g_sdk->clock_facade->get_game_time();
		if (spell_last_cast_t_ + t < time)
		{
			player->cast_spell(spell_slot_);
			spell_last_cast_t_ = time;
			g_sdk->log_console("[+] spell order %d was issued at %f", spell_slot_, spell_last_cast_t_);
			return true;
		}
		return false;
	}

	bool spell::cast_spell(const math::vector3 cast_position, const float t)
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const auto time = g_sdk->clock_facade->get_game_time();
		if (spell_last_cast_t_ + t < time)
		{
			player->cast_spell(spell_slot_, cast_position);
			spell_last_cast_t_ = time;
			g_sdk->log_console("[+] spell order %d was issued at %f", spell_slot_, spell_last_cast_t_);
			return true;
		}
		return false;
	}

	bool spell::cast_spell(game_object* target, const float t)
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const auto time = g_sdk->clock_facade->get_game_time();
		if (spell_last_cast_t_ + t < time)
		{
			player->cast_spell(spell_slot_, target);
			spell_last_cast_t_ = time;
			g_sdk->log_console("[+] spell order %d was issued at %f", spell_slot_, spell_last_cast_t_);
			return true;
		}
		return false;
	}

	bool spell::cast_spell(const math::vector3 start_position, const math::vector3 end_position, const float t)
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const auto time = g_sdk->clock_facade->get_game_time();
		if (spell_last_cast_t_ + t < time)
		{
			player->cast_spell(spell_slot_, start_position, end_position);
			spell_last_cast_t_ = time;
			g_sdk->log_console("[+] spell order %d was issued at %f", spell_slot_, spell_last_cast_t_);
			return true;
		}
		return false;
	}

	bool spell::cast_spell_on_hitchance(game_object* target, const int hitchance, const float range, const float t)
	{
		const auto pred_output = this->get_prediction(target, hitchance, range);
		if (pred_output.is_valid)
		{
			return this->cast_spell(pred_output.cast_position);
		}
		return false;
	}

	bool spell::cast_mood(const float t)
	{
		const auto player = g_sdk->object_manager->get_local_player();
		const auto time = g_sdk->clock_facade->get_game_time();
		if (spell_last_cast_t_ + t < time)
		{
			player->cast_hwei_mood(spell_slot_);
			spell_last_cast_t_ = time;
			g_sdk->log_console("[+] spell order %d was issued at %f", spell_slot_, spell_last_cast_t_);
			return true;
		}
		return false;
	}
}
