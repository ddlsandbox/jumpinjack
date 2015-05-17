/*
 * Enemy.h
 *
 *  Created on: May 17, 2015
 *      Author: diego
 */

#ifndef CHARACTERS_ENEMY_H_
#define CHARACTERS_ENEMY_H_

#include "../sdl/ActiveDrawable.h"

namespace jumpinjack
{
  typedef enum
  {
    ENEMY_GROUND = 1, ENEMY_FALL = 2, ENEMY_FOLLOW = 4
  } t_enemy_behavior;

  class Enemy : public ActiveDrawable
  {
    public:
      Enemy (SDL_Renderer * renderer, std::string sprite_file,
             int sprite_length, int sprite_start_line, int sprite_frequency);
      virtual ~Enemy ();

      virtual void onCreate (void);
      virtual void onDestroy (void);
      virtual t_collision onCollision (Drawable * item, t_direction dir,
                                       t_itemtype type, t_point & point,
                                       t_point & delta);
      virtual void update (SDL_Point & next_point);
    private:
      int status_count;
  };

} /* namespace jumpinjack */

#endif /* CHARACTERS_ENEMY_H_ */
