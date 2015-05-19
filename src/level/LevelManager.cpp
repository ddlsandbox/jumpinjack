/*
 * LevelManager.cpp
 *
 *  Created on: May 16, 2015
 *      Author: diego
 */

#include "LevelManager.h"
#include "../characters/Enemy.h"
#include "../items/Proyectile.h"
#include <sstream>
#include <fstream>

using namespace std;

namespace jumpinjack
{

  LevelManager::LevelManager (SDL_Renderer * renderer, int level_id,
                              std::vector<Player *> & players) :
          renderer (renderer), level_id (level_id),
          num_players (players.size ())
  {
    stringstream ss;
    string level_data;
    ss << "level" << level_id << ".dat";
    level_data = GlobalDefs::getResource (RESOURCE_DATA, ss.str ().c_str ());
    string line;
    ifstream myfile (level_data);
    assert(myfile.is_open ());

    items.reserve (MAX_LEVEL_ITEMS);
    for (Player * player : players)
      {
        t_point position, delta;
        getline (myfile, line);
        sscanf (line.c_str (), "%d %d %d %d", &position.x, &position.y,
                &delta.x, &delta.y);
        items.push_back (
          { player, ITEM_PLAYER, position, delta });
      }

    for (int i = num_players; i < MAX_PLAYERS; i++)
      getline (myfile, line);

    bg_layers.reserve (PARALLAX_LAYERS);
    for (int i = 0; i < PARALLAX_LAYERS; i++)
      {
        getline (myfile, line);
        char bg_file_cstr[300];
        string bg_file;
        int parallax_level;
        int repeat_x;
        int auto_speed;
        sscanf (line.c_str (), "%s %d %d %d", bg_file_cstr, &parallax_level,
                &repeat_x, &auto_speed);
        bg_file = GlobalDefs::getResource (RESOURCE_IMAGE, bg_file_cstr);
        bg_layers.push_back (
            new BackgroundDrawable (renderer, bg_file, parallax_level,
                                    repeat_x == 1, auto_speed));
      }

    getline (myfile, line);
    level_surface = new Surface (
        GlobalDefs::getResource (RESOURCE_IMAGE, line.c_str ()));

    items.push_back (
          { new Enemy (renderer,
                       GlobalDefs::getResource (RESOURCE_IMAGE, "enemies.png"),
                       8, 0, 2), ITEM_ENEMY,
            { 500, 300 },
            { 0, 0 } });

    level_width = 4000;
    myfile.close ();
  }

  LevelManager::~LevelManager ()
  {
    for (itemInfo & it : items)
      if (it.type != ITEM_PLAYER)
        delete it.item;
    for (BackgroundDrawable * bg : bg_layers)
      delete bg;
    delete level_surface;
  }

  void LevelManager::applyAction (int player_id, t_action action)
  {
    assert(player_id < num_players);
    itemInfo & player_info = items[player_id];
    Player * player = (Player *) player_info.item;

    if (!player->getStatus (STATUS_LISTENING))
      return;

    bool run = false;
    int max_speed =
        (action & ACTION_SPRINT) ?
            1.5 * player->getSpeed () : player->getSpeed ();
    if (action & ACTION_RIGHT)
      {
        player->setState (PLAYER_RUN);
        player->setDirection (DIRECTION_RIGHT);
        player_info.delta.x += player->getAccel ();
        if (player_info.delta.x > max_speed)
          player_info.delta.x = max_speed;
        run = true;
      }
    if (action & ACTION_LEFT)
      {
        player->setState (PLAYER_RUN);
        player->setDirection (DIRECTION_LEFT);
        player_info.delta.x -= player->getAccel ();
        if (player_info.delta.x < -max_speed)
          player_info.delta.x = -max_speed;
        run = true;
      }
    if (action & ACTION_SHOOT)
      {
        t_point point = player_info.point;
        if (player->getDirection() == DIRECTION_RIGHT)
          point.x += 50;
        else
          point.x -= 50;
        t_point delta;
        Proyectile * proyectile = new Proyectile (
            renderer,
            GlobalDefs::getResource (RESOURCE_IMAGE, "proyectile.png"),
            player->getDirection (), delta, 60, 30);
        itemInfo shoot_info =
          { proyectile, ITEM_PROYECTILE, point, delta };
        items.push_back (shoot_info);
      }
    if (!run && !player_info.delta.x)
      player->setState (PLAYER_STAND);
    if (action & ACTION_UP)
      {
        if (player->onJump < GlobalDefs::jump_sensitivity)
          {
            /* ignore gravity when jumping */
            player_info.delta.y -= GlobalDefs::base_gravity;
            int jump_power = player->getJump () + abs (player_info.delta.x) / 5;
            if (!player->onJump)
              {
                player_info.delta.y -= jump_power;
              }
            player->onJump++;
          }
      }
    if (player_info.delta.y < 0)
      {
        player->setState (PLAYER_JUMP);
      }
    else if (player_info.delta.y > 0)
      {
        player->setState (PLAYER_FALL);
      }

    if (action & ACTION_UP_REL)
      {
        /* prevent jump while on air */
        if (player->onJump > 0 && player->onJump <= JUMPING_TRIGGER)
          player->onJump = JUMPING_TRIGGER;
        else
          player->onJump = JUMPING_RESET;
      }
  }

  bool LevelManager::canMoveTo (t_point p, ActiveDrawable * character,
                                t_direction dir)
  {
    pixelType pixel;
    switch (dir & 0xF)
      {
      case DIRECTION_DOWN:
        p.y++;
        pixel = level_surface->testPixel (p);
        return (pixel != PIXELTYPE_SOLID && pixel != PIXELTYPE_DOWN_ONLY);
        break;
      case DIRECTION_UP:
        p.y -= character->getHeight ();
        pixel = level_surface->testPixel (p);
        return (pixel != PIXELTYPE_SOLID && pixel != PIXELTYPE_UP_ONLY);
        break;
      case DIRECTION_LEFT:
        p.x -= character->getWidth () / 4;
        pixel = level_surface->testPixel (p);
        return (pixel != PIXELTYPE_SOLID);
        break;
      case DIRECTION_RIGHT:
        p.x += character->getWidth () / 4;
        pixel = level_surface->testPixel (p);
        return (pixel != PIXELTYPE_SOLID);
        break;
      default:
        return false;
      }
    return false;
  }

  template<typename T>
    int sgn (T val)
    {
      return (T (0) < val) - (val < T (0));
    }

  bool detectCollision (itemInfo & it1, itemInfo & it2,
                        t_direction * collision_direction)
  {
    t_point min1 =
      { it1.point.x - it1.item->getWidth () / 4, it1.point.y
          - it1.item->getHeight () };
    t_point max1 =
      { it1.point.x + it1.item->getWidth () / 4, it1.point.y };
    t_point min2 =
      { it2.point.x - it2.item->getWidth () / 4, it2.point.y
          - it2.item->getHeight () };
    t_point max2 =
      { it2.point.x + it2.item->getWidth () / 4, it2.point.y };
    if (min2.x > max1.x || min1.x > max2.x || min2.y > max1.y
        || min1.y > max2.y)
      return false;
    else
      {
        t_direction hdir =
            (it1.point.x < it2.point.x) ? DIRECTION_RIGHT : DIRECTION_LEFT;
        *collision_direction = (t_direction) (DIRECTION_HORIZONTAL | hdir);
        if (it1.point.y < (it2.point.y - it2.item->getHeight () / 2))
          {
            t_direction vdir = DIRECTION_DOWN;
            *collision_direction = (t_direction) (*collision_direction
                | DIRECTION_VERTICAL | vdir);
          }
        else if (it2.point.y < (it1.point.y - it1.item->getHeight () / 2))
          {
            t_direction vdir = DIRECTION_UP;
            *collision_direction = (t_direction) (*collision_direction
                | DIRECTION_VERTICAL | vdir);
          }
        return true;
      }
  }

  void LevelManager::updatePosition (itemInfo & it)
  {
    int friction = GlobalDefs::base_friction;
    int gravity = GlobalDefs::base_gravity;

    if (it.type != ITEM_PASSIVE)
      {
        ActiveDrawable * character = (ActiveDrawable *) it.item;
        character->update (it.delta);

        /* move horizontal */
        if (it.delta.x)
          {
            if (it.delta.x > 0)
              it.delta.x = max (it.delta.x - friction, 0);
            else
              it.delta.x = min (it.delta.x + friction, 0);

            int inc = sgn (it.delta.x);
            t_direction dir = (inc > 0) ? DIRECTION_RIGHT : DIRECTION_LEFT;
            for (int i = 0; i < abs (it.delta.x); i++)
              {
                if (canMoveTo (it.point, character, dir))
                  {
                    it.point.x += inc;
                  }
                else
                  {
                    character->onCollision (0, DIRECTION_HORIZONTAL,
                                            ITEM_PASSIVE, it.point, it.delta);
                    it.delta.x = 0;
                  }
              }

            if (it.type == ITEM_PLAYER)
              {
                if (it.point.x < 0)
                  it.point.x = 0;
                else if (it.point.x > level_width)
                  it.point.x = level_width;
              }
          }
        /* move horizontal */

        /* gravity */
        if (canMoveTo (it.point, character, DIRECTION_DOWN))
          it.delta.y = min (GlobalDefs::max_falling_speed,
                            it.delta.y + gravity);

        /* move vertical */
        if (it.delta.y)
          {
            int inc = sgn (it.delta.y);
            t_direction dir = (inc > 0) ? DIRECTION_DOWN : DIRECTION_UP;
            for (int i = 0; i < abs (it.delta.y); i++)
              {
                if (canMoveTo (it.point, character, dir))
                  {
                    it.point.y += inc;
                    if (dir == DIRECTION_DOWN)
                      {
                        if (!character->onJump)
                          character->onJump = JUMPING_TRIGGER;
                        else if (character->onJump
                            < GlobalDefs::jump_sensitivity)
                          character->onJump = GlobalDefs::jump_sensitivity;
                      }

                  }
                else
                  {
                    if (dir == DIRECTION_DOWN)
                      {
                        if (character->onJump == JUMPING_TRIGGER)
                          character->onJump = JUMPING_RESET;
                        else if (character->onJump)
                          character->onJump = (JUMPING_TRIGGER + 1);
                      }
                    if (character->onCollision (
                        0, (t_direction) (DIRECTION_VERTICAL | DIRECTION_DOWN),
                        ITEM_PASSIVE, it.point, it.delta) == COLLISION_IGNORE)
                      it.delta.y = 0;
                    break;
                  }
              }
          }
        /* move vertical */
      }
  }

  t_direction reverseDirection (t_direction dir)
  {
    t_direction newdir = dir;
    if (dir & DIRECTION_LEFT)
      {
        newdir = (t_direction) ((newdir & ~DIRECTION_LEFT) | DIRECTION_RIGHT);
      }
    else if (dir & DIRECTION_RIGHT)
      {
        newdir = (t_direction) ((newdir & ~DIRECTION_RIGHT) | DIRECTION_LEFT);
      }
    if (dir & DIRECTION_UP)
      {
        newdir = (t_direction) ((newdir & ~DIRECTION_UP) | DIRECTION_DOWN);
      }
    else if (dir & DIRECTION_DOWN)
      {
        newdir = (t_direction) ((newdir & ~DIRECTION_DOWN) | DIRECTION_UP);
      }
    return newdir;
  }

  void LevelManager::update ()
  {
    /* update positions */
    for (size_t i = 0; i < items.size (); i++)
      {
        if (items[i].item->getStatus (STATUS_ALIVE))
          {
            updatePosition (items[i]);
          }
        else
          {
            delete items[i].item;
            items.erase (items.begin () + i);
            i--;
          }
      }

    /* collision detection */
    t_direction collision_direction;
    for (size_t i = 0; i < items.size (); i++)
      {
        itemInfo & item1 = items[i];
        if (!item1.item->getStatus (STATUS_LISTENING))
          continue;
        for (size_t j = i + 1; j < items.size (); j++)
          {
            itemInfo & item2 = items[j];
            if (!item2.item->getStatus (STATUS_LISTENING))
              continue;
            if (detectCollision (item1, item2, &collision_direction))
              {
                t_collision collision_result;
                if (item1.type != ITEM_PASSIVE)
                  {
                    collision_result =
                        ((ActiveDrawable *) item1.item)->onCollision (
                            item2.item, collision_direction, item2.type,
                            item1.point, item1.delta, &item2.point,
                            &item2.delta);
                    if (collision_result == COLLISION_DIE)
                      ((ActiveDrawable *) item1.item)->onDestroy ();
                  }
                if (item2.type != ITEM_PASSIVE)
                  {
                    collision_result =
                        ((ActiveDrawable *) item2.item)->onCollision (
                            item1.item, reverseDirection (collision_direction),
                            item1.type, item2.point, item2.delta, &item1.point,
                            &item1.delta);
                    if (collision_result == COLLISION_DIE)
                      ((ActiveDrawable *) item2.item)->onDestroy ();
                  }
              }
          }
      }
  }

  void LevelManager::render ()
  {
    int xOffset =
        (items[0].point.x > GlobalDefs::window_size.x / 2) ?
            (items[0].point.x - GlobalDefs::window_size.x / 2) : 0;
    if (xOffset > (level_width - GlobalDefs::window_size.x))
      xOffset = (level_width - GlobalDefs::window_size.x);

    for (BackgroundDrawable * bg : bg_layers)
      {
        bg->renderFixed (
          { xOffset, 0 });
      }
    for (itemInfo & it : items)
      {
        int effectiveX = it.point.x - xOffset;

        if (effectiveX > -it.item->getWidth ()
            && effectiveX < (GlobalDefs::window_size.x + it.item->getWidth ()))
          {
            t_point render_point =
              { effectiveX, it.point.y };
            it.item->renderFixed (render_point);
          }
      }
  }
} /* namespace jumpinjack */
