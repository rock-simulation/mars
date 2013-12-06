/*
 *  Copyright 2013, DFKI GmbH Robotics Innovation Center
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

/**
 * \file MotorPlotConfig.h
 * \author Malte Langosz
 * \brief
 **/

#ifndef MARS_PLUGINS_MOTOR_PLOT_CONFIG_H
#define MARS_PLUGINS_MOTOR_PLOT_CONFIG_H

#ifdef _PRINT_HEADER_
#warning "MotorPlotConfig.h"
#endif

#include <mars/interfaces/sim/ControlCenter.h>

#include <vector>

#include <QWidget>
#include <QString>

class QComboBox;
class QCheckBox;
class QDoubleSpinBox;
class QLayout;
class QGridLayout;

namespace mars {
  namespace plugins {
    namespace Plot3D {

      class MotorPlotConfig : public QWidget {
        Q_OBJECT;

        public:
        MotorPlotConfig(mars::interfaces::ControlCenter* c);
        ~MotorPlotConfig();
  
      public slots:
        void motorSelected(int index);
        void addPlot(bool b);
        void removePlot(bool b);

      signals:
        void hideSignal(void);
        void closeSignal(void);
        void motorSelected(unsigned long);
        void addPlot();
        void removePlot();

      private:
        mars::interfaces::ControlCenter *control;
        QFont standardFont;
        QGridLayout *mainLayout;
        QComboBox *motorIDCombo;
        QComboBox *objectCombo;
        bool take_events;

        std::vector<unsigned long> motorIDs;
        void updateGUI(void);

      protected slots:
        void hideEvent(QHideEvent* event);
        void closeEvent(QCloseEvent* event);

      }; // end of class MotorPlotConfig

    } // end of namespace Plot3D
  } // end of namespace plugins
} // end of namespace mars

#endif /* MARS_PLUGINS_MOTOR_PLOT_CONFIG_H */
