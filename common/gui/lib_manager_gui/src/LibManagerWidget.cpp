#include "LibManagerWidget.h"

#include <QHeaderView>
#include <QFileDialog>
#include <cstdio>

namespace mars {
  namespace plugins {
    namespace lib_manager_gui {

      using namespace std;
      
      enum Columns {
        COL_NAME = 0,
        COL_PATH,
        COL_VERSION,
        COL_SRC,
        COL_REVISION,
        COL_REFCOUNT,

        NUM_OF_COLS
      };

      LibManagerWidget::LibManagerWidget(QWidget *parent,
                                         cfg_manager::CFGManagerInterface *cfg)
        : mars::main_gui::BaseWidget(parent, cfg, "LibManagerWidget") {
        table = new QTableWidget(0, NUM_OF_COLS, this);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setContextMenuPolicy(Qt::NoContextMenu);
        table->sortItems(COL_REFCOUNT, Qt::DescendingOrder);
        table->setSortingEnabled(true);
        table->setHorizontalHeaderItem(COL_NAME, new QTableWidgetItem("Name"));
        table->setHorizontalHeaderItem(COL_PATH, new QTableWidgetItem("Path"));
        table->setHorizontalHeaderItem(COL_VERSION, new QTableWidgetItem("Version"));
        table->setHorizontalHeaderItem(COL_SRC, new QTableWidgetItem("Source"));
        table->setHorizontalHeaderItem(COL_REVISION, new QTableWidgetItem("Revision"));
        table->setHorizontalHeaderItem(COL_REFCOUNT, new QTableWidgetItem("References"));
        table->horizontalHeader()->setStretchLastSection(true);
        
        loadButton = new QPushButton("Load Library...");
        unloadButton = new QPushButton("Unload Library");
        dumpButton = new QPushButton("Dump Info...");
        QHBoxLayout *buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(loadButton);
        buttonLayout->addWidget(unloadButton);
        buttonLayout->addWidget(dumpButton);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(table);
        mainLayout->addLayout(buttonLayout);
        
        setLayout(mainLayout);
        
        connect(loadButton, SIGNAL(clicked(bool)), this, SLOT(onLoad()));
        connect(unloadButton, SIGNAL(clicked(bool)), this, SLOT(onUnload()));
        connect(dumpButton, SIGNAL(clicked(bool)), this, SLOT(onDump()));
      }
        
      LibManagerWidget::~LibManagerWidget() {
        delete table;
      }

      void LibManagerWidget::clear() {
        while(table->rowCount()) {
          table->removeRow(0);
        }
      }

      void LibManagerWidget::setDefaultLibPath(const string &path) {
        defaultLibPath = QString::fromStdString(path);
      }

      void LibManagerWidget::onLoad() {
        QString filter;
#ifdef WIN32
        filter = "Shared Object (*.dll)";
#else
#  ifdef __APPLE__
        filter = "Shared Object (*.dylib)";
#  else
        filter = "Shared Object (*.so)";
#  endif
#endif
        QString path = QFileDialog::getOpenFileName(this, "Load Library...",
                                                    defaultLibPath, filter);
        if(!path.isNull())
          emit load(path.toStdString());
      }

      void LibManagerWidget::onUnload() {
        std::list<std::string> libs;
        std::list<std::string>::iterator libsIt;
        QList<QTableWidgetItem*> items = table->selectedItems();
        for(QList<QTableWidgetItem*>::iterator it = items.begin(); 
            it != items.end(); ++it) {
          if((*it)->column() == 0) {
            libs.push_back((*it)->text().toStdString());
          }
        }
        for(libsIt=libs.begin(); libsIt!=libs.end(); ++libsIt) {
          emit unload(*libsIt);
        }
      }

      void LibManagerWidget::onDump() {
        QString path = QFileDialog::getSaveFileName(this, "Dump Info to...",
                                                    "", "All Files (*.*);;XML files (*.xml)");
        if(!path.isNull())
          emit dump(path.toStdString());
      }

      void LibManagerWidget::updateLibInfo(const lib_manager::LibInfo &info) {
        // disable sorting temporarily
        table->setSortingEnabled(false);
        int row; // <-- declare before loop because we use it after the loop!
        for(row = 0; row < table->rowCount(); ++row) {
          string rowName = table->item(row, COL_NAME)->text().toStdString();
          if(info.name.compare(rowName) == 0) {
            break;
          }
        }
        if(row == table->rowCount()) {
          // the lib is new. extend the table
          table->setRowCount(row + 1);
        }
        // add or update row
        QTableWidgetItem *item;
        item = new QTableWidgetItem(QString::fromStdString(info.name));
        table->setItem(row, COL_NAME, item);
        item = new QTableWidgetItem(QString::fromStdString(info.path));
        table->setItem(row, COL_PATH, item);
        item = new QTableWidgetItem();
        item->setData(Qt::DisplayRole, info.version);
        table->setItem(row, COL_VERSION, item);
        item = new QTableWidgetItem(QString::fromStdString(info.src));
        table->setItem(row, COL_SRC, item);
        item = new QTableWidgetItem(QString::fromStdString(info.revision));
        table->setItem(row, COL_REVISION, item);
        item = new QTableWidgetItem();
        item->setData(Qt::DisplayRole, info.references);
        table->setItem(row, COL_REFCOUNT, item);
        // enable sorting again
        table->setSortingEnabled(true);
      }

    } // end of namespace lib_manager_gui
  } // end of namespace plugins
} // end of namespace mars
