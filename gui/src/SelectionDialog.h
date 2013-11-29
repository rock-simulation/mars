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
 * \file SelectionDialog.h
 * \author Vladimir Komsiyski
 * \brief The SelectionDialog class is used to select a sorted list of objects
 *
 * Version 0.1
 */

#ifndef SELECTION_DIALOG_H
#define SELECTION_DIALOG_H

#ifdef _PRINT_HEADER_
#warning "SelectionDialog.h"
#endif

#include <QtGui>
#include <QListWidget>
#include <QPushButton>

namespace mars {
  namespace gui {

    class SelectionDialog : public QWidget {
      Q_OBJECT
      public:
      SelectionDialog(QWidget* parent);
      ~SelectionDialog();

      void reset(QStringList freeList);
      void reset(QStringList freeList, QStringList chosenList);

    private:
      QListWidget *free, *chosen;
      QPushButton *button_up, *button_down, *button_add, *button_remove;
      QString getChosen();

    signals:
      void modified(QString);

    private slots:
      void entry_up();
      void entry_down();
      void entry_add();
      void entry_remove();

    };

  } // end of namespace gui
} // end of namespace mars

#endif // SELECTION_DIALOG_H
