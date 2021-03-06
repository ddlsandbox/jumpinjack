/*
 * Checkpoint.h
 *
 *  Created on: July 29, 2016
 *      Author: diego
 */

#ifndef ITEMS_CHECKPOINT_H_
#define ITEMS_CHECKPOINT_H_

#include "../sdl/ActiveDrawable.h"

namespace jumpinjack
{

  class Checkpoint : public ActiveDrawable
  {
    public:
      Checkpoint (SDL_Renderer * renderer, std::string sprite_file);
      virtual ~Checkpoint ();

      virtual void onCreate (void);
      virtual void onDestroy (void);
      virtual t_collision onCollision (Drawable * item, t_direction dir,
                                       t_itemtype type, t_point & point,
                                       t_point & delta,
                                       t_point * otherpoint = 0,
                                       t_point * otherdelta = 0);
      virtual void update (t_point & next_point);

    protected:
      enum ckp_state{ CKP_INIT, CKP_HIT, CKP_END};
      ckp_state state;       /* rotation speed */
      bool taken;
  };

} /* namespace jumpinjack */

#endif /* ITEMS_CHECKPOINT_H_ */
