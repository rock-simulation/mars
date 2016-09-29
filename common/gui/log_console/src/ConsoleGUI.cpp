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

#include "ConsoleGUI.h"

#ifdef WIN32
#include <windows.h>
#endif

namespace mars {

  namespace log_console {

    ConsoleGUI::ConsoleGUI(QWidget *parent,
                           cfg_manager::CFGManagerInterface *cfg)
      : main_gui::BaseWidget(parent, cfg, "Console") {
      maxLines = -1;
      QHBoxLayout *buttonLayout = new QHBoxLayout();
      QVBoxLayout *mainLayout = new QVBoxLayout();
      myTextEdit = new QTextEdit();
      myText = new QTextDocument();

      //setAttribute(Qt::WA_DeleteOnClose);
      QPalette palette;
      QBrush brush(QColor(0, 0, 0, 255));
      brush.setStyle(Qt::SolidPattern);
      palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
      QBrush brush1(QColor(255, 255, 255, 255));
      brush1.setStyle(Qt::SolidPattern);
      palette.setBrush(QPalette::Active, QPalette::Button, brush1);
      palette.setBrush(QPalette::Active, QPalette::Light, brush1);
      palette.setBrush(QPalette::Active, QPalette::Midlight, brush1);
      QBrush brush2(QColor(127, 127, 127, 255));
      brush2.setStyle(Qt::SolidPattern);
      palette.setBrush(QPalette::Active, QPalette::Dark, brush2);
      QBrush brush3(QColor(170, 170, 170, 255));
      brush3.setStyle(Qt::SolidPattern);
      palette.setBrush(QPalette::Active, QPalette::Mid, brush3);
      QBrush brush4(QColor(212, 148, 90, 255));
      brush4.setStyle(Qt::SolidPattern);
      palette.setBrush(QPalette::Active, QPalette::Text, brush4);
      QBrush brush5(QColor(254, 255, 202, 255));
      brush5.setStyle(Qt::SolidPattern);
      palette.setBrush(QPalette::Active, QPalette::BrightText, brush5);
      palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
      QBrush brush6(QColor(54, 54, 54, 255));
      brush6.setStyle(Qt::SolidPattern);
      palette.setBrush(QPalette::Active, QPalette::Base, brush6);
      palette.setBrush(QPalette::Active, QPalette::Window, brush1);
      palette.setBrush(QPalette::Active, QPalette::Shadow, brush);
      palette.setBrush(QPalette::Active, QPalette::AlternateBase, brush1);
      palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
      palette.setBrush(QPalette::Inactive, QPalette::Button, brush1);
      palette.setBrush(QPalette::Inactive, QPalette::Light, brush1);
      palette.setBrush(QPalette::Inactive, QPalette::Midlight, brush1);
      palette.setBrush(QPalette::Inactive, QPalette::Dark, brush2);
      palette.setBrush(QPalette::Inactive, QPalette::Mid, brush3);
      palette.setBrush(QPalette::Inactive, QPalette::Text, brush4);
      palette.setBrush(QPalette::Inactive, QPalette::BrightText, brush5);
      palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
      palette.setBrush(QPalette::Inactive, QPalette::Base, brush6);
      palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);
      palette.setBrush(QPalette::Inactive, QPalette::Shadow, brush);
      palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush1);
      palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush2);
      palette.setBrush(QPalette::Disabled, QPalette::Button, brush1);
      palette.setBrush(QPalette::Disabled, QPalette::Light, brush1);
      palette.setBrush(QPalette::Disabled, QPalette::Midlight, brush1);
      palette.setBrush(QPalette::Disabled, QPalette::Dark, brush2);
      palette.setBrush(QPalette::Disabled, QPalette::Mid, brush3);
      palette.setBrush(QPalette::Disabled, QPalette::Text, brush2);
      palette.setBrush(QPalette::Disabled, QPalette::BrightText, brush5);
      palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush2);
      palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
      palette.setBrush(QPalette::Disabled, QPalette::Window, brush1);
      palette.setBrush(QPalette::Disabled, QPalette::Shadow, brush);
      palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush1);
      setPalette(palette);
      QFont font;
      font.setFamily(QString::fromUtf8("Tahoma"));
      font.setPointSize(9);
      setFont(font);
      setAutoFillBackground(false);

      for(int i=0; i<5; ++i) {
        buttons[i] = new QCheckBox();
      }
      buttons[0]->setText("Fatals");
      buttons[0]->setObjectName("Fatal checkbox");
      buttons[1]->setText("Errors");
      buttons[1]->setObjectName("Error checkbox");
      buttons[2]->setText("Warnings");
      buttons[2]->setObjectName("Warning checkbox");
      buttons[3]->setText("Info");
      buttons[3]->setObjectName("Info checkbox");
      buttons[4]->setText("Debug");
      buttons[4]->setObjectName("Debug checkbox");
      for(int i = 0; i < 5; ++i) {
        buttons[i]->setChecked(true);
        connect(buttons[i], SIGNAL(stateChanged(int)),
                this, SLOT(onCheckBoxToggled(int)));
        buttonLayout->addWidget(buttons[i]);
      }

      mainLayout->addLayout(buttonLayout);
      mainLayout->addWidget(myTextEdit);
      setLayout(mainLayout);
      myText->setParent(myTextEdit);
      myTextEdit->setDocument(myText);
    }

    ConsoleGUI::~ConsoleGUI(void) {
    }


    void ConsoleGUI::setMaxLines(int maxLines) {
      this->maxLines = maxLines;
      myText->setMaximumBlockCount(maxLines);
    }

    void ConsoleGUI::paintEvent(QPaintEvent *event) {
      QWidget::paintEvent(event);
      //emit geometryChanged();
    }


    void ConsoleGUI::onCheckBoxToggled(int state) {
      for(int i = 0; i < 5; ++i) {
        if(sender() == buttons[i]) {
          emit messageTypeChanged(i, bool(state==2));
        }
      }
    }

  } // end of namespace log_console

} // end of namespace mars
