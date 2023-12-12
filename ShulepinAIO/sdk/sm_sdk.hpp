#pragma once

#ifdef USE_CGAL
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Circle_2.h>
#include <CGAL/Bbox_2.h>
#endif

#include <sdk.hpp>
#include <color.hpp>

#ifdef USE_CGAL
typedef CGAL::Simple_cartesian< double > K;
typedef K::Point_2 Point;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Circle_2<K> Circle_2;
typedef CGAL::Bbox_2 Bbox_2;
#endif

class sm_sdk
{
public:
    enum class spell_iteration: uint8_t {
        all = 0,
        ally = 1,
        enemy = 2,
    };

    enum class spell_type: uint8_t {
        unsupported = 0,
        linear = 1,
        circular = 2,
    };

    class static_data
    {
    public:
        spell_type type{};
        game_object* hero{};
        bool valid{};
        bool has_projectile{};
        bool strict_missile_name{};
        bool is_cc{};
        int slot = -1;
        uint32_t missile_effect_key{};
        std::string missile_name{};
        float projectile_speed{};
        float travel_time{};
        float range[ 7 ]{};
        float radius[ 7 ]{};
        std::vector< char* > tags{};

        virtual float get_radius() const
        {
            if( !this->valid )
                return 0.f;

            const auto spell_entry = this->hero->get_spell( this->slot );
            if( !spell_entry )
                return 0.f;

            return this->radius[ spell_entry->get_level() ];
        }

        virtual float get_range() const
        {
            if( !this->valid )
                return 0.f;

            const auto spell_entry = this->hero->get_spell( this->slot );
            if( !spell_entry )
                return 0.f;

            return this->range[ spell_entry->get_level() ];
        }

        virtual ~static_data() = default;
    };

    class spell {
    public:
        bool operator==( const spell& a )
        {
            return a.owner == this->owner && a.slot == this->slot;
        }

        game_object* owner{};
        game_object* target{}; // will only be set for targetted spells
        game_object* missile{};
        game_object* particle{};
        math::vector3 start_pos{};
        math::vector3 end_pos{};
        int slot{};
        spell_type type{};
        int team_id{};
        bool is_drawing_only{};
        bool is_cc{};
        bool is_particle_on_ground{};
        bool has_projectile{};
        bool missile_created{};
        uint32_t missile_effect_key{};
        float radius{};
        float projectile_speed{};
        float travel_time{};
        float cast_delay{};
        float cast_end_time{};
        float deletion_time{};
        std::string missile_name{};
        std::string trap_name{};
        spell* parent_spell{};
        std::vector< spell* > additional_spells{};

        bool delete_on_missile_deletion = true;

    #ifdef USE_CGAL
        Polygon_2 polygon{};
        Circle_2 circle{};
        color color{};
        uint8_t previous_alpha = 0;
        bool allow_missile_positions = true;

        // used to prevent race conditions
        bool pending_deletion{};
    #endif
    };

    virtual void iterate_spells( sm_sdk::spell_iteration iter, const std::function< bool( sm_sdk::spell* spell ) >& fn ) = 0; // fn can return true; to break; and stop iterating spells further
    virtual std::array< sm_sdk::static_data, 64 > get_spells_static_data( game_object* hero ) = 0; /* /!\ only use this on script initialization, not inside an event /!\ */
    virtual math::vector3 get_missile_position( sm_sdk::spell* spell ) = 0;
    virtual float get_deletion_time( sm_sdk::spell* spell ) = 0;
    virtual bool is_casted( sm_sdk::spell* spell ) = 0;
};

namespace sdk
{
    inline sm_sdk* spell_manager = nullptr;
}

namespace sdk_init
{
    inline bool spell_manager()
    {
        if( sdk::spell_manager )
            return true;

        const std::string module_name = "VEN.SpellManager";
        if( !g_sdk->add_dependency( "Core/" + module_name ) )
            return false;

        sdk::spell_manager = reinterpret_cast< sm_sdk* >( g_sdk->get_custom_sdk( module_name ) );

        return true;
    }
}