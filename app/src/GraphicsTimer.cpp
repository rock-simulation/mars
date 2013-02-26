/*
 *  Copyright 2011, 2012 DFKI GmbH Robotics Innovation Center
 *
 *  This file is part of the MARS simulation framework.
 *
 *  MARS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3
 *  of the License, or (at your option) any later version.
 *
 *  MARS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with MARS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "GraphicsTimer.h"
#include <mars/utils/misc.h>

namespace mars {
  namespace app {

    using mars::interfaces::GraphicsManagerInterface;
    using mars::interfaces::SimulatorInterface;

    GraphicsTimer::GraphicsTimer(GraphicsManagerInterface *graphics_,
                                 SimulatorInterface *sim_)
      : graphics(graphics_), sim(sim_) {
      graphicsTimer = new QTimer();
      connect(graphicsTimer, SIGNAL(timeout()), this, SLOT(timerEvent()));
      connect(this, SIGNAL(internalRun()), this, SLOT(runOnceInternal()),
              Qt::QueuedConnection);
    }

    void GraphicsTimer::run() {
      graphicsTimer->start(10);
    }

    void GraphicsTimer::stop() {
      graphicsTimer->stop();
    }


    void GraphicsTimer::runOnce(){
      runFinished=false;
      emit internalRun();
      while(!runFinished){
        mars::utils::msleep(1);
      }
    }

    void GraphicsTimer::runOnceInternal(){
      timerEvent();
      runFinished=true;
    }

    void GraphicsTimer::timerEvent(void) {
      if(sim->getAllowDraw() || !sim->getSyncGraphics()) {
        if(graphics) {
          graphics->draw();
        }
        else {
          sim->finishedDraw();
        }
      }
    }
 
  } // end of namespace app
} // end of namespace mars
