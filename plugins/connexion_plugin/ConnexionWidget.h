/*
 *  Copyright 2011, 2012, DFKI GmbH Robotics Innovation Center
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
 * \file ConnexionWidget.h
 * \author Malte Roemmermann
 * \brief "ConnexionWidget" is a window to setup the connexion 3d mouse
 **/

#ifndef MARS_PLUGINS_CONNEXION_WIDGET_H
#define MARS_PLUGINS_CONNEXION_WIDGET_H

#ifdef _PRINT_HEADER_
#warning "ConnexionWidget.h"
#endif

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/main_gui/BaseWidget.h>

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
    namespace connexion_plugin {

      class ConnexionWidget : public main_gui::BaseWidget {
        Q_OBJECT;

        public:
        ConnexionWidget(mars::interfaces::ControlCenter* c);
        ~ConnexionWidget();
  
        void setWindowID(unsigned long id);
        void setObjectID(unsigned long id);
        void setLockAxis(bool lock[6]);
        void setSensitivity(double sens[6]);

      public slots:
        void windowSelected(int index);
        void objectSelected(int index);
        void modeSelected(int index);
        void checkX(bool val);
        void checkY(bool val);
        void checkZ(bool val);
        void checkRX(bool val);
        void checkRY(bool val);
        void checkRZ(bool val);
        void checkFilter(bool val);
        void checkSyncWithFrames(bool val);
        void sensXChanged(double val);
        void sensYChanged(double val);
        void sensZChanged(double val);
        void sensRXChanged(double val);
        void sensRYChanged(double val);
        void sensRZChanged(double val);
        void filterValueChanged(double val);

      signals:
        void windowSelected(unsigned long);
        void objectSelected(unsigned long);
        void setObjectMode(int);
        void setLockAxis(int, bool);
        void sigSensitivity(int, double);
        void setUseFilter(bool);
        void setFilterValue(double);
        void setSyncWithFrames(bool);

      private:
        mars::interfaces::ControlCenter *control;
        QFont standardFont;
        QGridLayout *mainLayout;
        QComboBox *winIDCombo;
        QComboBox *objectIDCombo;
        QComboBox *objectCombo;
        QCheckBox *checkLockX, *checkLockY, *checkLockZ,
          *checkLockRX, *checkLockRY, *checkLockRZ,
          *checkFilter_;
        QDoubleSpinBox *sensitivityX, *sensitivityY, *sensitivityZ,
          *sensitivityRX, *sensitivityRY, *sensitivityRZ, *filterValue_;
        unsigned int win_id, object_id;
        bool take_events;

        std::vector<unsigned long> windowIDs, objectIDs;
        void updateGUI(void);
        QLayout* generateDoubleBox(QDoubleSpinBox *&theBox, QString objectName,
                                   QString text);
        void generateDoF(QGridLayout *mainLayout, 
                         QCheckBox *&checkBox, QDoubleSpinBox *&spinBox,
                         const char *checkSlot,const char *spinSlot,
                         const QString &label, int row);
      }; // end of class ConnexionWidget

    } // end of namespace connexion_plugin
  } // end of namespace plugins
} // end of namespace mars

#endif /* MARS_PLUGINS_CONNEXION_WIDGET_H */
