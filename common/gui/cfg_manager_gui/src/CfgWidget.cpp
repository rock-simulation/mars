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

#include "CfgWidget.h"
#include "MainCfgGui.h"

#include <string>
#include <utility>
#include <map>

#include <QVBoxLayout>

namespace mars {
  namespace cfg_manager_gui {

    using namespace cfg_manager;
    using namespace std;

    CfgWidget::CfgWidget(MainCfgGui *m, CFGManagerInterface *_cfg,
                         QWidget *parent) :
      BaseWidget(parent, _cfg, "cfg"),
      mainCfgGui(m),
      pDialog(new main_gui::PropertyDialog(NULL)),
      ignore_change(0) {
  
      startTimer(200);
      pDialog->setButtonBoxVisibility(false);
      pDialog->setPropCallback(dynamic_cast<PropertyCallback*>(this));
      QVBoxLayout *layout = new QVBoxLayout();
      layout->addWidget(pDialog);
      this->setLayout(layout);
    }

    CfgWidget::~CfgWidget(void) {
      mainCfgGui->unsetWidget();
    }

    void CfgWidget::addParam(const cfgParamInfo &newParam) {
      addMutex.lock();
      addList.push_back(newParam);
      addMutex.unlock();
    }

    void CfgWidget::changeParam(const cfgParamId _id) {
      changeMutex.lock();
      changeList.push_back(_id);
      changeMutex.unlock();
    }

    void CfgWidget::removeParam(const cfgParamId _id) {
      removeMutex.lock();
      removeList.push_back(_id);
      removeMutex.unlock();
    }

    void CfgWidget::timerEvent(QTimerEvent* event) {
  
      (void)event;

      cfgParamInfo *theParam;
      paramWrapper theWrapper, *theWrapper_p;
      string path;
      cfgPropertyStruct prop;

      // go throud the add list
      addMutex.lock();
      ignore_change = true;
      while(addList.size() > 0) {
        std::map<QString, QVariant> attr;
        attr["singleStep"] = 0.01;
        attr["decimals"] = 12;
        theParam = &(addList[0]);
    
        // get the type and add the param to the gui
        path = string("");
        //path = string("../");
        path.append(theParam->group);
        path.append(string("/"));
        path.append(theParam->name);

        prop.propertyIndex = 0;
        prop.paramId = theParam->id;
    
        theWrapper.theParam = *theParam;
        switch(theParam->type) {
        case doubleParam:
          prop.propertyType = doubleProperty;
          cfg->getProperty(&prop);
          theWrapper.guiElem = pDialog->addGenericProperty(path,
                                                           QVariant::Double,
                                                           prop.dValue, &attr);
          theWrapper.theProp = prop;
          break;
        case intParam:
          prop.propertyType = intProperty;
          cfg->getProperty(&prop);
          theWrapper.guiElem = pDialog->addGenericProperty(path, QVariant::Int, prop.iValue);
          theWrapper.theProp = prop;
          break;
        case boolParam:
          prop.propertyType = boolProperty;
          cfg->getProperty(&prop);
          theWrapper.guiElem = pDialog->addGenericProperty(path, QVariant::Bool,
                                                           prop.bValue);
          theWrapper.theProp = prop;
          break;
        case stringParam:
          prop.propertyType = stringProperty;
          cfg->getProperty(&prop);
          theWrapper.guiElem = pDialog->addGenericProperty(path, QVariant::String,
                                                           QString::fromStdString(prop.sValue));
          theWrapper.theProp = prop;
          break;
        default:
          theWrapper.guiElem = 0;
        }
        if(theWrapper.guiElem) paramList.push_back(theWrapper);
        addList.erase(addList.begin());
      }
      ignore_change = false;
      addMutex.unlock();

      // check for updates
      //changeMutex.lock();
      ignore_change = true;
      while(changeList.size() > 0) {
        theWrapper_p = getWrapperById(changeList[0]);
        if(theWrapper_p && theWrapper_p->guiElem) {
          cfg->getProperty(&theWrapper_p->theProp);
          switch(theWrapper_p->theParam.type) {
          case doubleParam:
            theWrapper_p->guiElem->setValue(QVariant(theWrapper_p->theProp.dValue));
            break;
          case intParam:
            theWrapper_p->guiElem->setValue(QVariant(theWrapper_p->theProp.iValue));
            break;
          case boolParam:
            theWrapper_p->guiElem->setValue(QVariant(theWrapper_p->theProp.bValue));
            break;
          case stringParam:
            theWrapper_p->guiElem->setValue(QVariant(QString::fromStdString(theWrapper_p->theProp.sValue)));
            break;
          case noParam:
          case dstNrOfParamTypes:
            break;
          // don't supply a default case so that the compiler might warn
          // us if we forget to handle a new enum value.
          }
        }
        changeList.erase(changeList.begin());
      }
      ignore_change = false;
      //changeMutex.unlock();

      // and go through the remove list ^^
      removeMutex.lock();
      ignore_change = true;
      while (removeList.size() > 0) {
        theWrapper_p = getWrapperById(removeList[0]);
        pDialog->removeGenericProperty(theWrapper_p->guiElem);
        removeList.erase(removeList.begin());
      }
      ignore_change = false;
      removeMutex.unlock();
    }

    paramWrapper* CfgWidget::getWrapperById(cfgParamId _id) {
      vector<paramWrapper>::iterator iter;

      for(iter=paramList.begin(); iter!=paramList.end(); ++iter) {
        if(iter->theParam.id == _id)
          return &(*iter);
      }
      return 0;
    }

    void CfgWidget::valueChanged(QtProperty *property, const QVariant &value) {
      if(ignore_change) return;
      vector<paramWrapper>::iterator iter;
      double dValue;
      int iValue;
      bool bValue;
      string sValue;
  
      for(iter=paramList.begin(); iter!=paramList.end(); ++iter) {
        if(iter->guiElem == property) {
          switch(iter->theParam.type) {
          case doubleParam:
            dValue = value.toDouble();
            if(dValue != iter->theProp.dValue) {
              iter->theProp.dValue = dValue;
              cfg->setProperty(iter->theProp);
            }
            break;
          case intParam:
            iValue = value.toInt();
            if(iValue != iter->theProp.iValue) {
              iter->theProp.iValue = iValue;
              cfg->setProperty(iter->theProp);
            }
            break;
          case boolParam:
            bValue = value.toBool();
            if(bValue != iter->theProp.bValue) {
              iter->theProp.bValue = bValue;
              cfg->setProperty(iter->theProp);
            }
            break;
          case stringParam:
            sValue = value.toString().toStdString();
            if(sValue != iter->theProp.sValue) {
              iter->theProp.sValue = sValue;
              cfg->setProperty(iter->theProp);
            }

            break;
          default:
            break;
          } // switch
          break;
        } // if
      } // for
    }


    void CfgWidget::accept() {}
    void CfgWidget::reject() {}

  } // end of namespace cfg_manager_gui
} // end of namespace mars
