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


#include "PropertyDialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QTabWidget>

using namespace std;

namespace mars {
  namespace main_gui {

    TabPropertyDialog::TabPropertyDialog(QWidget *parent) : PropertyDialog(parent) {
      buttonBox->hide();
    }


    PropertyDialog::PropertyDialog(QWidget *parent)
      : QDialog(parent), propertyCallback(0) {
      tabView = false;
      variantFactory = new VariantFactory();
      variantManager = new VariantManager();
      variantEditorTree  = new QtTreePropertyBrowser();
      variantEditorButton = new QtButtonPropertyBrowser();
      hBoxLayout = new QHBoxLayout();
      vBoxLayout = new QVBoxLayout();
      tabWidget = NULL;
      viewMode = ButtonViewMode;

      viewButton = new QPushButton("Change View");
      viewButton->setFixedSize(95, 20);
      QObject::connect(viewButton, SIGNAL(clicked()), this, SLOT(toggleView()));

      variantEditorTree->setFactoryForManager(variantManager, variantFactory);
      variantEditorButton->setFactoryForManager(variantManager, variantFactory);

      connect(variantManager, SIGNAL(valueChanged(QtProperty*, const QVariant&)),
              this, SLOT(valueChanged(QtProperty*, const QVariant&)),
              Qt::DirectConnection);
      connect(variantEditorTree, SIGNAL(currentItemChanged(QtBrowserItem*)),
              this, SLOT(currentItemChanged(QtBrowserItem*)),
              Qt::DirectConnection);
      connect(variantEditorButton, SIGNAL(currentItemChanged(QtBrowserItem*)),
              this, SLOT(currentItemChanged(QtBrowserItem*)),
              Qt::DirectConnection);


      buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,
                                       Qt::Horizontal, this);

      connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
      connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));


      hBoxLayout->addWidget(variantEditorTree);
      hBoxLayout->addWidget(variantEditorButton);
      QFrame *frame = new QFrame(this);
      frame->setLayout(hBoxLayout);
      scrollArea = new QScrollArea(0);
      scrollArea->setWidget(frame);
      scrollArea->setWidgetResizable(true);

      vBoxLayout->addWidget(viewButton);
      vBoxLayout->addWidget(scrollArea);
      vBoxLayout->addWidget(buttonBox);

      setLayout(vBoxLayout);
      adjustSize();

      variantEditorButton->setVisible(true);
      variantEditorTree->setVisible(false);
    }


    PropertyDialog::~PropertyDialog() {
    }



    QtVariantProperty* PropertyDialog::addTabbedProperty(const std::string &path,
                                                         int type,
                                                         const QVariant& value,
                                                         std::map<QString, QVariant> *attributes,
                                                         QStringList *options) {
      if(tabView == false) {
        tabWidget = new QTabWidget(this);
        viewButton->hide();
        scrollArea->hide();
        vBoxLayout->insertWidget(0, tabWidget);
        tabWidget->show();
        tabView = true;
      }

      int tabExists = false;
      QString label = QString::fromStdString(path.substr(0, path.find_first_of('/')));
      string rest = ".." + path.substr(path.find_first_of('/'));
      map<QString, PropertyDialog*>::iterator it;

      for(it = myTabs.begin(); it != myTabs.end(); it++) {
        if(it->first == label) {
          tabExists = true;
          break;
        }
      }

      if(tabExists) {
        return it->second->addGenericProperty(rest, type, value, attributes, options);
      }

      PropertyDialog *page = new TabPropertyDialog(this);
      tabWidget->addTab(page, label);
      myTabs.insert(pair<QString, PropertyDialog*>(label, page));
      connect(page, SIGNAL(tabValueChanged(QtProperty*, const QVariant&)),
              this, SLOT(valueChanged(QtProperty*, const QVariant&)));

      return page->addGenericProperty(rest, type, value, attributes, options);
    }


    QtVariantProperty* PropertyDialog::addGenericProperty(const string &path,
                                                          int type,
                                                          const QVariant &value,
                                                          map<QString, QVariant> *attributes,
                                                          QStringList *options) {
      QList<string> propertyPath;
      string propertyName;
      bool propertyExists = false;
      bool parentExists = false;
      QtVariantProperty *parent = NULL;
      QtVariantProperty *item = NULL;
      QList<QtProperty*>::iterator it;
      QList<QtProperty*> current = myProperties;
      // tokenize the path
      unsigned int j = 0;
      for(unsigned int i = 0; i < path.length()+1; i++) {
        if(path[i] == '/') {
          propertyPath.push_back(path.substr(j, i-j));
          j = i + 1;
        }
        if (i == path.length() && propertyPath.size() > 0) {
          propertyPath.push_back(path.substr(j, i-j));
        }
      }

      if(propertyPath.size() <= 0) {  // empty or invalid path
        return NULL;
      }
      // if the path does not begin with "../" then insert a tab
      if(propertyPath[0] != "..") {
        return addTabbedProperty(path, type, value, attributes, options);
      }

      propertyPath.pop_front(); // get rid of the "../"
      propertyName = propertyPath.back(); // save the name of the actual property
      propertyPath.pop_back(); // leave only the path
      for(int i = 0; i < propertyPath.size(); i++) {
        parentExists = false;
        // check if this part of the path exists
        for(it = current.begin(); it != current.end(); it++) {
          if((*it)->propertyName().toStdString() == propertyPath[i]) {
            item = (QtVariantProperty*)(*it);
            parentExists = true;
            break;
          }
        }
        if(parentExists) { // start iterating over its subproperties
          current = item->subProperties();
        } else { // create it as a group property
          item = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),
                                             QString::fromStdString(propertyPath[i]));
          if(i == 0) { // add it to the views and myProperties
            variantEditorButton->addProperty(item);
            variantEditorTree->addProperty(item);
            collapseTree(variantEditorTree->items(item));
            myProperties.push_back(item);
          } else { // add it to the parent's subproperties
            parent->addSubProperty(item);
          }
        }
        parent = item;
      }

      if(parent != NULL) {  // now parent points to the direct parent of the new property
        current = parent->subProperties();
      } else {  // no parent means that it is created directly in the browsers
        current = myProperties;
      }

      propertyExists = false;
      for(it = current.begin(); it != current.end(); it++) {
        if (propertyName == (*it)->propertyName().toStdString()) { // it already exists;
          propertyExists = true;
          item = (QtVariantProperty*)(*it);
          break;
        }
      }
      if(!propertyExists) { // add the new property to the manager
        item = variantManager->addProperty(type, QString::fromStdString(propertyName));
      }

      if(parent != NULL) { // add the property to its parent's subproperties
        parent->addSubProperty(item);
        collapseTree(variantEditorTree->items(parent));
      } else { // add the property directly to the browsers
        myProperties.push_back(item);
        variantEditorTree->addProperty(item);
        variantEditorButton->addProperty(item);
        collapseTree(variantEditorTree->items(item));
      }

      customizeItem(item, type, value, attributes, options);

      return item;
    }


    void PropertyDialog::collapseTree(QList<QtBrowserItem*> list) {
      QList<QtBrowserItem*>::iterator it;
      for (it = list.begin(); it != list.end(); it++) {
        collapseTree((*it)->children());
        variantEditorTree->setExpanded(*it, false);
      }
    }

    void PropertyDialog::expandTree(QList<QtBrowserItem*> list) {
      QList<QtBrowserItem*>::iterator it;
      for(it = list.begin(); it != list.end(); it++) {
        collapseTree((*it)->children());
        variantEditorTree->setExpanded(*it, true);
      }
    }

    void PropertyDialog::expandTree(QtProperty *parent) {
      expandTree(variantEditorTree->items(parent));
    }

    void PropertyDialog::collapseTree(QtProperty *parent) {
      collapseTree(variantEditorTree->items(parent));
    }


    QtVariantProperty* PropertyDialog::addGenericProperty(QtVariantProperty *parent,
                                                          QtVariantProperty *property) {
      if(parent) { // a parent was found, add the property to its subproperties
        parent->addSubProperty(property);
        collapseTree(variantEditorTree->items(parent));
      } else { // no parent, add directly to the browsers
        variantEditorButton->addProperty(property);
        variantEditorTree->addProperty(property);
        collapseTree(variantEditorTree->items(property));
      }
      return property;
    }

    QtVariantProperty* PropertyDialog::insertGenericProperty(QtVariantProperty *parent,
                                                             QtVariantProperty *property,
                                                             QtVariantProperty *after) {
      if(parent) { // a parent was found, add the property to its subproperties
        parent->insertSubProperty(property, after);
      } else { // no parent, add directly to the browsers
        variantEditorButton->insertProperty(property, after);
        variantEditorTree->insertProperty(property, after);
      }
      return property;
    }

    void PropertyDialog::removeGenericProperty(QtVariantProperty *property) {
      QtProperty *parent = getParent(NULL, property);
      if(parent) { // a parent was found, remove the property from its subproperties
        parent->removeSubProperty(property);
        // remove all upward properties if they become empty after removing <property>
        if(parent->subProperties().size() == 0) {
          this->removeGenericProperty((QtVariantProperty*)parent);
        }
      } else { // no parent, remove the property directly from the browsers and <myProperties>
        variantEditorButton->removeProperty(property);
        variantEditorTree->removeProperty(property);
        QList<QtProperty*>::iterator it;
        for(it = myProperties.begin(); it != myProperties.end(); ) {
          if(*it == property) {
            //        delete *it;
            // the property should not be deleted, only removed (hidden)
            it = myProperties.erase(it);
          } else {
            ++it;
          }
        }
      }
      // propagate the remove to the underlying TabPropertyDialogs, if there are any
      map<QString, PropertyDialog*>::iterator it;
      for(it = myTabs.begin(); it != myTabs.end(); ) {
        it->second->removeGenericProperty(property);
        if(it->second->myProperties.size() == 0) {
          tabWidget->removeTab(tabWidget->indexOf(it->second));
          //delete it->second;
          myTabs.erase(it++);
        } else {
          ++it;
        }
      }
    }

    void PropertyDialog::removeAllSubProperties(QtVariantProperty *parent) {
      QList<QtBrowserItem*> pb = variantEditorButton->items(parent);
      for(int i = 0; i < pb.size(); i++) {
        variantEditorButton->setExpanded(pb[i], false);
      }
      QList<QtProperty*> list = parent->subProperties();
      for(int i = 0; i < list.size(); i++) {
        removeGenericProperty((QtVariantProperty*)list[i]);
      }
    }

    void PropertyDialog::addAllSubProperties(QtVariantProperty *parent,
                                             QList<QtVariantProperty*> list) {
      for(int i = 0; i< list.size(); i++) {
        (void)addGenericProperty(parent, list[i]);
      }
    }


    void PropertyDialog::destroyAllSubProperties(QtProperty *parent) {
      QList<QtProperty*> current;
      if(parent) { // a parent is passed, iterate over its subproperties
        current = parent->subProperties();
      } else { // no parent means initial iteration
        current = myProperties;
      }

      for(int i = 0; i < current.size(); i++) {
        destroyAllSubProperties(current[i]);
        delete current[i];
      }
    }

    QtProperty* PropertyDialog::getParent(QtProperty *parent,
                                          QtVariantProperty *property) const {
      QList<QtProperty*> current;
      if(parent) { // a parent is passed, iterate over its subproperties
        current = parent->subProperties();
      } else { // no parent means initial iteration
        current = myProperties;
      }

      for(int i = 0; i < current.size(); i++) {
        if(current[i] == property) { // the property is found
          return parent;
        }
        QtProperty *tmp = getParent(current[i], property);
        if(tmp) { // the property is found
          return tmp;
        }
      }
      return NULL; // no such property in this branch
    }

    void PropertyDialog::setPropertyColor(QtVariantProperty *property,
                                          QColor color) {
      QList<QtBrowserItem*>pb = variantEditorTree->items(property);
      for(int i=0; i < pb.size(); i++) {
        variantEditorTree->setBackgroundColor(pb[i], color);
      }
    }

    QColor PropertyDialog::getPropertyColor(QtVariantProperty *property) const {
      QList<QtBrowserItem*>pb = variantEditorTree->items(property);
      if(pb.size() > 0) {
        return variantEditorTree->backgroundColor(pb[0]);
      } else {
        return QColor(0, 0, 0, 0);
      }
    }

    QtProperty* PropertyDialog::getTopLevelParent(QtProperty *property) const {
      for(int i = 0; i < myProperties.size(); i++) {
        if(myProperties[i] == property) {
          return myProperties[i];
        }
        if(getParent(myProperties[i], (QtVariantProperty*)property)) {
          return myProperties[i];
        }
      }
      return NULL;
    }

    void PropertyDialog::customizeItem(QtVariantProperty *item, int type,
                                       const QVariant &value,
                                       map<QString, QVariant> *attributes,
                                       QStringList *options) {
      if(type == QtVariantPropertyManager::enumTypeId() && options) {
        item->setAttribute("enumNames", *options);
      }

      if(type == QtVariantPropertyManager::flagTypeId() && options) {
        item->setAttribute("flagNames", *options);
      }

      item->setValue(value);

      if(attributes) {
        map<QString,QVariant>::iterator it;
        for(it = attributes->begin(); it != attributes->end(); it++) {
          item->setAttribute((*it).first, (*it).second);
        }
      }
    }

    void PropertyDialog::valueChanged(QtProperty *property,
                                      const QVariant &value) {
      emit tabValueChanged(property, value);
      if(propertyCallback) {
        propertyCallback->valueChanged(property, value);
      }
    }

    void PropertyDialog::setCurrentItem(QtProperty *property) {
      QList<QtBrowserItem*>pb = variantEditorTree->items(property);
      if(pb.size() > 0) {
        variantEditorTree->setCurrentItem(pb[0]);
      }
    }

    void PropertyDialog::currentItemChanged(QtBrowserItem *current) {
      if(propertyCallback) {
        if(current) {
          propertyCallback->topLevelItemChanged(
                                                getTopLevelParent(
                                                                  (QtVariantProperty*)(current->property())));
        }
      }
    }

    QtProperty* PropertyDialog::currentItem(void) {
      switch (viewMode) {
      case TreeViewMode:
        if (variantEditorTree->currentItem()) {
          return getTopLevelParent(variantEditorTree->currentItem()->property());
        }
        break;
      case ButtonViewMode:
        if (variantEditorButton->currentItem()) {
          return getTopLevelParent(variantEditorButton->currentItem()->property());
        }
        break;
      default:
        break;
      }
      return NULL;
    }

    void PropertyDialog::accept(void) {
      if(propertyCallback) {
        propertyCallback->accept();
      }
    }

    void PropertyDialog::reject(void) {
      if(propertyCallback) {
        propertyCallback->reject();
      }
    }

    void PropertyDialog::closeEvent(QCloseEvent *event) {
      (void)event;
      /*
        if(tabView == true) {
        map<QString, PropertyDialog*>::iterator it
        for(it = myTabs.begin(); it != myTabs.end(); it++) {
        it->second->close();
        }
        }
      */
      emit closeSignal();
    }

    void PropertyDialog::toggleView() {
      switch (viewMode) {
      case TreeViewMode:
        viewMode = ButtonViewMode;
        variantEditorTree->hide();
        variantEditorButton->show();
        break;
      case ButtonViewMode:
        viewMode = TreeViewMode;
        variantEditorButton->hide();
        variantEditorTree->show();
        break;
      default:
        break;
      }
      updateGeometry();
    }


    void PropertyDialog::resizeEvent(QResizeEvent *event) {
      QDialog::resizeEvent(event);
      emit geometryChanged();
    }

    void PropertyDialog::moveEvent(QMoveEvent *event) {
      QDialog::moveEvent(event);
      emit geometryChanged();
    }

    QPushButton* PropertyDialog::addGenericButton(const char *name,
                                                  const QObject *recv,
                                                  const char *method) {
      QPushButton *temp = new QPushButton(tr(name));
      connect(temp, SIGNAL(clicked()), recv, method);
      buttonBox->addButton(temp, QDialogButtonBox::ActionRole);
      return temp;
    }

    void PropertyDialog::setButtonBoxVisibility(bool visible) {
      buttonBox->setVisible(visible);
    }

    void PropertyDialog::setViewButtonVisibility(bool visible) {
      viewButton->setVisible(visible);
    }

    void PropertyDialog::hideAllButtons(void) {
      buttonBox->hide();
      viewButton->hide();
    }

    void PropertyDialog::clearButtonBox(void) {
      buttonBox->clear();
    }

    void PropertyDialog::setViewMode(const ViewMode &mode) {
      if(viewMode != mode) {
        toggleView();
      }
    }

    ViewMode PropertyDialog::getViewMode(void) const {
      return viewMode;
    }


    void PropertyDialog::setPropCallback(PropertyCallback *pc) {
      propertyCallback = pc;
    }

  } // end namespace main_gui
} // end namespace mars
