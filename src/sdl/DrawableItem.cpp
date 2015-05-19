/*
 * DrawableItem.cpp
 *
 *  Created on: May 17, 2015
 *      Author: diego
 */

#include "DrawableItem.h"

namespace jumpinjack
{

  DrawableItem::DrawableItem (SDL_Renderer * renderer, int zindex,
                              std::string sprite_file, int sprite_length,
                              int sprite_start_line, int sprite_frequency) :
          Drawable (renderer, zindex, true), sprite_length (sprite_length),
          sprite_start_line (sprite_start_line),
          sprite_frequency (sprite_frequency), sprite_freq_divisor (0),
          sprite_line (sprite_start_line), sprite_index (0)
  {
    status = (t_status) (STATUS_ALIVE | STATUS_LISTENING);
    loadFromFile (sprite_file);

    /* assuming square sprites */
    sprite_size =
      { image_size.x / sprite_length, image_size.x / sprite_length};
  }

  DrawableItem::~DrawableItem ()
  {
  }

  void DrawableItem::setStatus (t_status s)
  {
    status = (t_status) (status | s);
  }

  void DrawableItem::unsetStatus (t_status s)
  {
    status = (t_status) (status & ~s);
  }

  bool DrawableItem::getStatus (t_status s)
  {
    return status & s;
  }

  SDL_Rect DrawableItem::updateSprite ()
  {
    sprite_freq_divisor = (sprite_freq_divisor + 1) % sprite_frequency;
    if (!sprite_freq_divisor)
      sprite_index = (sprite_index + 1) % sprite_length;

    t_point sprite_offset =
      { sprite_index * sprite_size.x, sprite_line * sprite_size.y };

    SDL_Rect renderQuad =
      { sprite_offset.x, sprite_offset.y, sprite_size.x, sprite_size.y };

    return renderQuad;
  }
} /* namespace jumpinjack */
