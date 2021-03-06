/*
 * InGameMenu.h
 *
 *  Created on: May 24, 2015
 *      Author: diego
 */

#ifndef LEVEL_INGAMEMENU_H_
#define LEVEL_INGAMEMENU_H_

#include "AbstractMenu.h"

const t_point ingame_menu_size =
  { 400, 300 };

namespace jumpinjack
{
  class InGameMenu : public AbstractMenu
  {
    public:
      InGameMenu (SDL_Renderer * renderer);
      virtual ~InGameMenu ();

      virtual menu_action poll ();
      virtual void renderFixed (t_point point);
    private:
      int menu_y;
  };

} /* namespace jumpinjack */

#endif /* LEVEL_INGAMEMENU_H_ */
