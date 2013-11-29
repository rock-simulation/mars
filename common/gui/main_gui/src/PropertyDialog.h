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
 * \file PropertyDialog.h
 * \author Vladimir Komsiyski
 * \brief PropertyDialog provides the QtProperty functionality.
 * \details PropertyDialog allows creation and managemenet of
 *          properties of various types using the QtProperty extension.
 */

#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include "qtpropertymanager.h"
#include "qttreepropertybrowser.h"
#include "qtbuttonpropertybrowser.h"
#include "qtvariantproperty.h"
#include "variantmanager.h"
#include "variantfactory.h"

#include "PropertyCallback.h"

#include <map>
#include <string>

class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QDialogButtonBox;
class QTabWidget;
class QScrollArea;

#include <QDialog>

namespace mars {
  namespace main_gui {

    /**
     * \brief The view modes of the property dialog - tree and button.
     */
    enum ViewMode {
      TreeViewMode,
      ButtonViewMode
    };

    /**
     * \brief The PropertyDialog provides the QtProperty functionality.
     * \details PropertyDialog allows creation and managemenet of
     *          properties of various types using the QtProperty extension.
     */
    class PropertyDialog : public QDialog {

      Q_OBJECT

      public:

      /**
       * \brief A contructor.
       */
      PropertyDialog(QWidget *parent);

      /**
       * \brief A destructor.
       */
      virtual ~PropertyDialog();

      /**
       * \brief Creates a property
       * \param path If path begins with "../"
       *        then the property is created in this dialog
       *        else a tab widget is created.
       * \param type A QVariant::Type.
       * \param value The initial value.
       * \param options Items for enumTypeId() and flagTypeId()
       */
      QtVariantProperty* addGenericProperty(const std::string &path,
                                            int type,
                                            const QVariant &value,
                                            std::map<QString, QVariant> *attributes = NULL,
                                            QStringList *options = NULL);

      /**
       * \brief Adds the property to the parent's subproperties.
       */
      QtVariantProperty* addGenericProperty(QtVariantProperty *parent,
                                            QtVariantProperty *property);

      /**
       * \brief Creates a PropertyDialog tab widget and adds the generic
       *        property to it.
       */
      QtVariantProperty* addTabbedProperty(const std::string &path,
                                           int type,
                                           const QVariant &value,
                                           std::map<QString, QVariant> *attributes = NULL,
                                           QStringList *options = NULL);

      /**
       * \brief Inserts the property to the parent's subproperties after the after property.
       */
      QtVariantProperty* insertGenericProperty(QtVariantProperty *parent,
                                               QtVariantProperty *property,
                                               QtVariantProperty *after);

      /**
       * \brief Removes the property.
       */
      void removeGenericProperty(QtVariantProperty *property);

      /**
       * \brief Removes all subproperties of the parent property.
       */
      void removeAllSubProperties(QtVariantProperty *parent);

      /**
       * \brief Detaches and destroys all subproperties of the parent.
       */
      void destroyAllSubProperties(QtProperty *parent);

      /**
       * \brief Adds all properties from the list to the parent's subproperties.
       */
      void addAllSubProperties(QtVariantProperty *parent,
                               QList<QtVariantProperty*> list);

      /**
       * \brief Sets the background color of a property in the
       *        Tree Property Browser.
       */
      void setPropertyColor(QtVariantProperty *property, QColor color);

      /**
       * \brief Returns the background color of the property in the
       *        Tree Property Browser.
       */
      QColor getPropertyColor(QtVariantProperty *property) const;

      /**
       * \brief Returns the top level item that is currently focused on.
       */
      QtProperty* currentItem(void);

      /**
       * \brief Sets the focus on the top level item property in the
       * Tree Property Browser.
       */
      void setCurrentItem(QtProperty *property);

      /**
       * \brief Creates a QPushButton with the given name and connects
       *        its \c clicked() slot with the \c method of the receiver \c recv.
       */
      QPushButton* addGenericButton(const char *name, const QObject *recv,
                                    const char *method) ;

      /**
       * \brief Sets the visibility of the view button.
       */
      void setViewButtonVisibility(bool visible);

      /**
       * \brief Sets the visibility of the button box.
       */
      void setButtonBoxVisibility(bool visible);

      /**
       * \brief Removes all buttons from the button box.
       */
      void clearButtonBox(void);

      /**
       * \brief Hides the view button and the button box.
       */
      void hideAllButtons(void);

      /**
       * \brief Sets the view mode to either tree or button view mode.
       */
      void setViewMode(const ViewMode &mode);

      /**
       * \brief Sets the callback object.
       */
      void setPropCallback(PropertyCallback *pc);

      /**
       * \brief Returns the current view mode.
       */
      ViewMode getViewMode(void) const;

      //! Expands the branch of the property \c item.
      void expandTree(QtProperty *item);

      //! Collapses the branch of the property \c item.
      void collapseTree(QtProperty *item);

    protected:
      //! A vertical layout.
      QVBoxLayout *vBoxLayout;
      //! A horizontal layout.
      QHBoxLayout *hBoxLayout;
      //! A container for the buttons.
      QDialogButtonBox *buttonBox;
      //! A button that toggles the view mode.
      QPushButton *viewButton;
      //! A tab widget handling the tabbed view.
      QTabWidget *tabWidget;
      //! A scroll area for the property editors
      QScrollArea *scrollArea;

      //! The factory for the properties.
      QtVariantEditorFactory *variantFactory;
      //! The manager of the properties.
      QtVariantPropertyManager *variantManager;
      //! The tree property browser.
      QtTreePropertyBrowser *variantEditorTree;
      //! The button property browser.
      QtButtonPropertyBrowser *variantEditorButton;
      //! The callback instance associated with this instance.
      PropertyCallback *propertyCallback;

      //! Top level properties.
      QList<QtProperty*> myProperties;
      //! Tabs with their names.
      std::map<QString, PropertyDialog*> myTabs;
      //! An indicator for the tabbed view.
      int tabView;
      //! The view mode of the dialog.
      ViewMode viewMode;

      /**
       * \brief Adjusts the attributes and value of a property.
       */
      void customizeItem(QtVariantProperty *item,
                         int type,
                         const QVariant &value,
                         std::map<QString, QVariant> *attributes = NULL,
                         QStringList *options = NULL);

      /**
       * \brief Cleans up before closing.
       */
      virtual void closeEvent(QCloseEvent *event);

      /**
       * \brief Used for geometry handling.
       */
      void resizeEvent(QResizeEvent *event);

      /**
       * \brief Used for geometry handling.
       */
      void moveEvent(QMoveEvent *event);

    private:
      void expandTree(QList<QtBrowserItem*> list);
      void collapseTree(QList<QtBrowserItem*> list);

      // returns the parent of the property; must be called with parent=NULL
      QtProperty* getParent(QtProperty *parent, QtVariantProperty *property) const;
      QtProperty* getTopLevelParent(QtProperty *property) const;

    public slots:
      /**
       * \brief Associated with the \a OK button.
       */
      virtual void accept();

      /**
       * \brief Associated with the \a Cancel button.
       */
      virtual void reject();

    protected slots:
      /**
       * \brief Called every time a property has changed its value.
       * \param property The property with a new value.
       * \param value The new value.
       */
      virtual void valueChanged(QtProperty *property, const QVariant &value);

      /**
       * \brief Called every time another branch
       *        of a QtTreePropertyBrowser has been selected.
       * \param current The current top level item that is selected.
       */
      virtual void currentItemChanged(QtBrowserItem *current);

      /**
       * \brief Switches between tree an button view modes.
       */
      void toggleView();

    signals:
      //! Emitted when paintEvent is received
      void geometryChanged();

      /**
       * \brief Emitted when a \a QCloseEvent has been received.
       */
      void closeSignal();

      /**
       * \brief Emitted when a property in a tab has changed its value.
       *        Forwarded to the corresponding TabPropertyDialog.
       * */
      void tabValueChanged(QtProperty *property, const QVariant &value);

    }; // end class PropertyDialog


    /**
     * \brief A helper class with a tabbed view.
     */
    class TabPropertyDialog : public PropertyDialog {

      Q_OBJECT

      public:
      /**
       * \brief The constructor initializes the PropertyDialog and hides
       *        the buttons inside the tab.
       */
      TabPropertyDialog(QWidget *parent = 0);

    }; // end class TabPropertyDialog


  } // end namespace main_gui
} // end namespace mars


#endif /* PROPERTYDIALOG_H */
