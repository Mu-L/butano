#include "bf_game_hero.h"

#include "btn_camera.h"
#include "btn_keypad.h"
#include "btn_colors.h"
#include "btn_fixed_rect.h"
#include "btn_sound_items.h"
#include "btn_sprite_builder.h"
#include "btn_sprite_items_hero_body.h"
#include "btn_sprite_items_hero_death.h"
#include "btn_sprite_items_hero_weapons.h"
#include "bf_scene_type.h"
#include "bf_game_enemies.h"
#include "bf_game_objects.h"
#include "bf_game_hero_bomb.h"
#include "bf_game_background.h"
#include "bf_butano_background.h"
#include "bf_game_enemy_bullets.h"
#include "bf_game_hero_bullet_level.h"

namespace bf::game
{

namespace
{
    constexpr const int body_delta_y = 40;
    constexpr const int weapon_delta_x = 2;
    constexpr const int weapon_delta_y = -13;
    constexpr const int scale_weapon_frames = 30;
    constexpr const int scale_weapon_half_frames = scale_weapon_frames / 2;
    constexpr const int body_shadows_multiplier = 4;
    constexpr const btn::fixed_size dimensions(14, 14);

    btn::vector<btn::sprite_ptr, 3> _create_body_shadows()
    {
        btn::vector<btn::sprite_ptr, 3> result;

        for(int index = 0; index < 3; ++index)
        {
            btn::sprite_builder builder(btn::sprite_items::hero_body);
            builder.set_z_order(constants::hero_shadows_z_order);
            result.push_back(builder.release_build());
        }

        return result;
    }

    btn::sprite_cached_animate_action<2> _create_body_sprite_animate_action()
    {
        btn::sprite_ptr body_sprite = btn::sprite_items::hero_body.create_sprite(0, body_delta_y);
        return btn::create_sprite_cached_animate_action_forever(btn::move(body_sprite), 16,
                                                                btn::sprite_items::hero_body, 0, 2);
    }

    btn::sprite_ptr _create_weapon_sprite(int level, const btn::fixed_point& position)
    {
        return btn::sprite_items::hero_weapons.create_sprite(position, level);
    }
}

hero::hero(status& status) :
    _status(status),
    _body_shadows(_create_body_shadows()),
    _body_sprite_animate_action(_create_body_sprite_animate_action()),
    _body_snapshots(body_snapshots_count, body_snapshot{ _body_sprite_animate_action.sprite().position(), 0 }),
    _weapon_position(weapon_delta_x, body_delta_y + weapon_delta_y),
    _weapon_sprite(_create_weapon_sprite(status.level(), _weapon_position))
{
}

void hero::show_shoot(btn::color fade_color)
{
    _show_shoot_counter = 5;

    btn::sprite_palette_ptr body_palette = _body_sprite_animate_action.sprite().palette();
    body_palette.set_fade(fade_color, 0.5);
    _body_palette_fade_action.emplace(btn::move(body_palette), _show_shoot_counter, 0);
}

btn::optional<scene_type> hero::update(const hero_bomb& hero_bomb, const enemies& enemies,
                                       enemy_bullets& enemy_bullets, objects& objects, background& background,
                                       butano_background& butano_background)
{
    btn::optional<scene_type> result;

    if(alive())
    {
        btn::sprite_ptr body_sprite = _body_sprite_animate_action.sprite();
        btn::fixed_point old_body_position = body_sprite.position();
        btn::fixed_point new_body_position = _move(old_body_position, body_sprite);
        btn::fixed_rect new_body_rect(new_body_position, dimensions);
        _animate_alive(old_body_position, new_body_position);

        if(objects.check_hero_weapon(new_body_rect))
        {
            [[maybe_unused]] bool level_added = _status.add_level();
            BTN_ASSERT(level_added, "Level add failed");

            _scale_weapon_counter = scale_weapon_frames;
            _weapon_sprite.set_item(btn::sprite_items::hero_weapons, _status.level());
            _weapon_sprite.set_scale(2);

            btn::sprite_palette_ptr weapon_palette = _weapon_sprite.palette();
            weapon_palette.set_fade(btn::colors::yellow, 0.5);
        }

        bool max_bombs_count = _status.bombs_count() == constants::max_hero_bombs;
        int level = _status.level();

        objects::bomb_check_result bomb_check_result = objects.check_hero_bomb(new_body_rect, max_bombs_count, level);
        int experience_to_add = bomb_check_result.experience_to_add;

        if(bomb_check_result.add_bomb)
        {
            [[maybe_unused]] bool bomb_added = _status.add_bomb();
            BTN_ASSERT(bomb_added, "Bomb add failed");
        }

        experience_to_add += objects.check_gem(new_body_rect, level);

        if(experience_to_add && add_experience(experience_to_add))
        {
            objects.spawn_hero_weapon(btn::fixed_point(0, -constants::view_height), level + 1);
        }

        if(! hero_bomb.active())
        {
            if(enemies.check_hero(new_body_rect) || enemy_bullets.check_hero(new_body_rect))
            {
                ++_death_counter;
            }
        }
    }
    else
    {
        result = _animate_dead(background, butano_background);
    }

    return result;
}

btn::fixed_point hero::_move(const btn::fixed_point& body_position, btn::sprite_ptr& body_sprite)
{
    btn::fixed_point new_body_position = body_position;
    btn::fixed speed = _shooting ? 1 : 2;

    if(btn::keypad::held(btn::keypad::button_type::LEFT))
    {
        btn::fixed sprite_x = btn::max(body_position.x() - speed, btn::fixed(-constants::play_width));
        body_sprite.set_x(sprite_x);
        new_body_position.set_x(sprite_x);

        if(sprite_x < constants::camera_width)
        {
            btn::camera::set_x(btn::max(btn::camera::x() - speed, btn::fixed(-constants::camera_width)));
        }
    }
    else if(btn::keypad::held(btn::keypad::button_type::RIGHT))
    {
        btn::fixed sprite_x = btn::min(body_position.x() + speed, btn::fixed(constants::play_width));
        body_sprite.set_x(sprite_x);
        new_body_position.set_x(sprite_x);

        if(sprite_x > -constants::camera_width)
        {
            btn::camera::set_x(btn::min(btn::camera::x() + speed, btn::fixed(constants::camera_width)));
        }
    }

    if(btn::keypad::held(btn::keypad::button_type::UP))
    {
        btn::fixed sprite_y = btn::max(body_position.y() - speed, btn::fixed(-constants::play_height));
        body_sprite.set_y(sprite_y);
        new_body_position.set_y(sprite_y);
    }
    else if(btn::keypad::held(btn::keypad::button_type::DOWN))
    {
        btn::fixed sprite_y = btn::min(body_position.y() + speed, btn::fixed(constants::play_height));
        body_sprite.set_y(sprite_y);
        new_body_position.set_y(sprite_y);
    }

    return new_body_position;
}

void hero::_animate_alive(const btn::fixed_point& old_body_position, const btn::fixed_point& new_body_position)
{
    int shoot_shift_y;

    if(_show_shoot_counter)
    {
        shoot_shift_y = -1;
        --_show_shoot_counter;
        _body_palette_fade_action->update();
    }
    else
    {
        shoot_shift_y = 0;
    }

    _weapon_position = new_body_position + btn::fixed_point(weapon_delta_x, weapon_delta_y);
    _weapon_sprite.set_position(_weapon_position + btn::fixed_point(0, shoot_shift_y));

    if(! _shooting && old_body_position != new_body_position)
    {
        _body_sprite_animate_action.update();
    }

    _body_sprite_animate_action.update();

    if(_scale_weapon_counter)
    {
        btn::sprite_palette_ptr weapon_palette = _weapon_sprite.palette();
        --_scale_weapon_counter;

        if(_scale_weapon_counter)
        {
            if(_scale_weapon_counter <= scale_weapon_half_frames)
            {
                _weapon_sprite.set_scale(1 + (btn::fixed(_scale_weapon_counter) / scale_weapon_half_frames));

                if(_scale_weapon_counter == scale_weapon_half_frames)
                {
                    weapon_palette.set_fade_intensity(0);
                }
            }
        }
        else
        {
            _weapon_sprite.set_scale(1);
        }
    }

    int shadows_count = _body_shadows.size();
    _body_snapshots.pop_back();
    _body_snapshots.push_front(body_snapshot{ new_body_position, _body_sprite_animate_action.current_index() });

    if(_shooting)
    {
        _body_shadows_counter = btn::max(_body_shadows_counter - 1, 0);
    }
    else
    {
        _body_shadows_counter = btn::min(_body_shadows_counter + 1, shadows_count * body_shadows_multiplier);
    }

    int visible_shadows_count = _body_shadows_counter / body_shadows_multiplier;

    for(int index = 0, limit = _body_shadows.size(); index < limit; ++index)
    {
        const body_snapshot& body_snapshot = _body_snapshots[(index + 1) * body_shadows_multiplier];
        btn::sprite_ptr& body_shadow = _body_shadows[shadows_count - index - 1];

        if(index >= visible_shadows_count || body_snapshot.position == new_body_position)
        {
            if(body_shadow.visible())
            {
                body_shadow.set_visible(false);
            }
        }
        else
        {
            int graphics_index = ((index + 1) * 4) + body_snapshot.graphics_index;
            body_shadow.set_position(body_snapshot.position);
            body_shadow.set_tiles(btn::sprite_items::hero_body.tiles_item().create_tiles(graphics_index));

            if(! body_shadow.visible())
            {
                body_shadow.set_visible(true);
            }
        }
    }
}

btn::optional<scene_type> hero::_animate_dead(background& background, butano_background& butano_background)
{
    btn::sprite_ptr body_sprite = _body_sprite_animate_action.sprite();
    btn::optional<scene_type> result;

    if(_death_counter == 1)
    {
        btn::sprite_palette_ptr body_palette = body_sprite.palette();
        body_palette.set_fade(btn::colors::yellow, 0.75);
        _body_palette_fade_action.emplace(btn::move(body_palette), 30, 0);
        _body_rotate_action.emplace(body_sprite, 0.5);

        btn::sprite_palette_ptr weapon_palette = _weapon_sprite.palette();
        weapon_palette.set_fade(btn::colors::yellow, 0.75);
        _weapon_palette_fade_action.emplace(btn::move(weapon_palette), 30, 0);
        _weapon_sprite.set_scale(1);
        _weapon_move_action.emplace(_weapon_sprite, 70, _weapon_sprite.position() + btn::fixed_point(10, -10));
        _weapon_rotate_action.emplace(_weapon_sprite, -10);

        _body_shadows.clear();
        background.show_hero_dying();
        _music_volume_action.emplace(50, 0);
        btn::sound_items::boss_shoot.play();
    }
    else if(_death_counter == 70)
    {
        btn::fixed_point explosion_position = body_sprite.position() + btn::fixed_point(2, 4);
        _death_explosion.emplace(btn::sprite_items::hero_death, explosion_position, 4, 0);
        _weapon_move_action.emplace(_weapon_sprite, 70, _weapon_sprite.position() + btn::fixed_point(5, -5));
        _weapon_rotate_action.emplace(_weapon_sprite, -5);
        background.show_hero_dead();
        btn::sound_items::death.play();
    }
    else if(_death_counter == 220)
    {
        butano_background.show(_weapon_sprite.position());
    }
    else if(_death_counter > 220 && ! butano_background.silhouette_visible())
    {
        btn::camera::set_x(0);
        _status.update_high_experience();
        _status = status();
        result = scene_type::TITLE;
    }

    ++_death_counter;

    if(_death_explosion && ! _death_explosion->done())
    {
        _death_explosion->update();
        body_sprite.set_visible(_death_explosion->show_target_sprite());
    }

    if(body_sprite.visible())
    {
        _body_rotate_action->update();
        body_sprite.set_y(body_sprite.y() + constants::background_speed);

        if(_death_counter % 8 == 0)
        {
            body_sprite.set_x(body_sprite.x() + 2);
        }
        else if(_death_counter % 4 == 0)
        {
            body_sprite.set_x(body_sprite.x() - 2);
        }
    }

    if(_weapon_move_action)
    {
        _weapon_move_action->update();
        _weapon_rotate_action->update();

        if(_weapon_move_action->done())
        {
            _weapon_move_action.reset();
            _weapon_rotate_action.reset();
        }
    }
    else
    {
        btn::fixed weapon_y = _weapon_sprite.y();

        if(weapon_y < constants::view_height)
        {
            _weapon_sprite.set_y(weapon_y + constants::background_speed);
        }
    }

    if(_body_palette_fade_action)
    {
        _body_palette_fade_action->update();
        _weapon_palette_fade_action->update();

        if(_body_palette_fade_action->done())
        {
            _body_palette_fade_action.reset();
            _weapon_palette_fade_action.reset();
        }
    }

    if(_music_volume_action)
    {
        _music_volume_action->update();

        if(_music_volume_action->done())
        {
            _music_volume_action.reset();
        }
    }

    return result;
}

}
