/*
 * SdlManager.h
 *
 *  Created on: May 16, 2015
 *      Author: diego
 */

#ifndef SDL_SDLMANAGER_H_
#define SDL_SDLMANAGER_H_

#include "../GlobalDefs.h"
#include "../characters/Player.h"
#include "../level/LevelManager.h"

#include <SDL2/SDL_image.h>

#include <vector>
#include <queue>
#include <string>

struct queued_event
{
    t_event user_event;
    t_point point;
};

namespace jumpinjack
{
  class SdlManager
  {
    public:
      SdlManager ();
      virtual ~SdlManager ();

      int addPlayer(std::string sprite_file, int sprite_length,
                    int sprite_start_line, int sprite_frequency);

      int startLevel(int level_id);

      void mapEvent (
          t_event_type type, int event_id, t_event user_event,
          t_trigger trigger);
      void pollEvents ();
      t_event pollSingleEvent (
          t_point * point);

      void applyAction (t_action action);

      void startLoop ();
      void endLoop ();

      void update ();

      void render ();
    private:
      bool init();

      SDL_Window * window;
      SDL_Renderer * renderer;

      Uint32 start_ticks;

      std::vector<t_event_record> mapped_events;
      std::queue<queued_event> events_queue;

      std::vector<Player *> players;
      LevelManager * level;
  };

} /* namespace sdlfw */

#endif /* SDL_SDLMANAGER_H_ */
