#pragma once
#include "sdk.hpp"

namespace script
{
    class spell {
    private:
        int spell_slot_;
        float spell_range_;
        pred_sdk::spell_type spell_type_;
        pred_sdk::targetting_type spell_targetting_type_;
        float spell_delay_;
        float spell_radius_;
        float spell_speed_;
        float spell_last_cast_t_ = 0.f;
        float min_range_;
        float max_range_;
        bool is_charged_;
        std::unordered_map<std::string, uint32_t> charged_buff_map_;
        std::vector<pred_sdk::collision_type> spell_collision_data_;

    public:
        // Constructor
        spell(int spell_slot, float range);

        // Spell state checks
        [[nodiscard]] bool is_ready(float extra_time = 0.f) const;
        [[nodiscard]] bool is_issue_order_passed(float value) const;
        [[nodiscard]] static bool is_enough_mana_pct(float value);
        [[nodiscard]] bool is_charging();

        // Setters for spell properties
        void set_skillshot(pred_sdk::spell_type spell_type, pred_sdk::targetting_type targetting_type, float delay, float radius, float speed, const std::vector<pred_sdk::collision_type>& collision_data);
        void set_spell_type(pred_sdk::spell_type spell_type);
        void set_targetting_type(pred_sdk::targetting_type spell_targetting_type);
        void set_range(float range);
        void set_delay(float delay);
        void set_radius(float radius);
        void set_speed(float speed);
        void set_collision(const std::vector<pred_sdk::collision_type>& spell_collision_data);
        void set_min_range(float min_range);
        void set_max_range(float max_range);
        void set_charged(bool value);

        // Getters for spell properties
        [[nodiscard]] pred_sdk::spell_type get_spell_type() const;
        [[nodiscard]] pred_sdk::targetting_type get_targetting_type() const;
        [[nodiscard]] float get_range() const;
        [[nodiscard]] float get_delay() const;
        [[nodiscard]] float get_radius() const;
        [[nodiscard]] float get_speed() const;
        [[nodiscard]] std::vector<pred_sdk::collision_type> get_collision() const;
        [[nodiscard]] float get_cooldown() const;
        [[nodiscard]] pred_sdk::pred_data get_prediction(game_object* target, int hitchance, float range) const;
        [[nodiscard]] uint32_t get_stacks() const;
        [[nodiscard]] uint8_t get_toggle_state() const;
        [[nodiscard]] int get_level() const;
        [[nodiscard]] float get_min_range() const;
        [[nodiscard]] float get_max_range() const;
        [[nodiscard]] float get_charged_time(float duration = 1.5f) const;
        [[nodiscard]] float get_charged_range(float max_range) const;
        [[nodiscard]] bool get_charged() const;
        [[nodiscard]] float get_travel_time(game_object* target) const;
        void add_charged_buff_mapping(const std::string& champion_name, uint32_t buff_hash);

        // Spell casting methods
        bool release(math::vector3 cast_position, bool release_cast = true, float t = 0.5f);
        bool cast_spell(float t = 0.5f);
        bool cast_spell(game_object* target, float t = 0.5f);
        bool cast_spell(math::vector3 cast_position, float t = 0.5f);
        bool cast_spell(const math::vector3 start_position, const math::vector3 end_position, const float t = 0.5f);
        bool cast_spell_on_hitchance(game_object* target, int hitchance, float range, float t = 0.5f);
        bool cast_mood(const float t = 0.5f);
    };
}
