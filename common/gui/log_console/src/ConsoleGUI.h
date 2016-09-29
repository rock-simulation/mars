/*
 *  Copyright 2011, 2012, 2016 DFKI GmbH Robotics Innovation Center
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
 * \file ConsoleGUI.h
 * \author Malte Langosz
 * \brief "ConsoleGUI" is a template for the widget interface of the MARS GUI
 **/

#ifndef CONSOLE_GUI_H
#define CONSOLE_GUI_H

#ifdef _PRINT_HEADER_
#warning "ConsoleGUI.h"
#endif

#include <mars/main_gui/BaseWidget.h>
#include <QTextEdit>
#include <QCloseEvent>
#include <QPaintEvent>
#include <QHBoxLayout>
#include <QCheckBox>

namespace mars {

  namespace log_console {

    class ConsoleGUI : public main_gui::BaseWidget {
      Q_OBJECT;

    public:
      ConsoleGUI(QWidget *parent, cfg_manager::CFGManagerInterface *cfg);
      ~ConsoleGUI();
    
      void setTextColor(const QColor &color) {
        myTextEdit->setTextColor(color);
      }

      void setMaxLines(int maxLines);
    public slots:
      void append(const QString &text) {
        myTextEdit->append(text);
      }
      void onCheckBoxToggled(int state);

    protected:
      void paintEvent(QPaintEvent *event);
    
    signals:
      void messageTypeChanged(int, bool);

    private:
      QCheckBox *buttons[5];
      QTextEdit *myTextEdit;
      QTextDocument *myText;
      int maxLines;

    }; // end of class ConsoleGUI

  } // end of namespace log_console

} // end of namespace mars

#endif // CONSOLE_GUI_H
