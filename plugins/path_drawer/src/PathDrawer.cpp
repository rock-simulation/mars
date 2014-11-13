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
 * \file PathDrawer.cpp
 * \author Alexander (alexander.dettmann@dfki.de)
 * \brief draws
 *
 * Version 0.1
 */


#include "PathDrawer.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <fstream>

namespace mars {
  namespace plugins {
    namespace path_drawer {

      using namespace mars::utils;
      using namespace mars::interfaces;
      using namespace std;

      PathDrawer::PathDrawer(lib_manager::LibManager *theManager)
        : MarsPluginTemplate(theManager, "PathDrawer") {
      }
  
      void PathDrawer::init() {

        // create the line handle
        osg_lines::LinesFactory lF;
        l = lF.createLines();

        // get or create the cfg property
        string def_name = "path/path.obj";
        obj_file_struct = control->cfg->getOrCreateProperty("PathDrawer", "obj_file", def_name, this);
        printf("[path drawer] obj: %s\n", obj_file_struct.sValue.c_str());

        if(obj_file_struct.sValue == ""){
          addVectorsFromObjFile(def_name);
        }else{
          addVectorsFromObjFile(obj_file_struct.sValue);
        }

        // get or create the cfg property
        def_name = "obstacle_course/config_obstacle_course.svg";
        svg_file_struct = control->cfg->getOrCreateProperty("PathDrawer", "svg_file", def_name, this);
        printf("[path drawer] svg: %s\n", svg_file_struct.sValue.c_str());

        if(svg_file_struct.sValue == ""){
          addVectorsFromSvgFile(def_name);
        }else{
          addVectorsFromSvgFile(svg_file_struct.sValue);
        }
      }

      void PathDrawer::reset() {
      }

      PathDrawer::~PathDrawer() {
      }


      void PathDrawer::update(sReal time_ms) {

        // control->motors->setMotorValue(id, value);
      }

      void PathDrawer::receiveData(const data_broker::DataInfo& info,
                                    const data_broker::DataPackage& package,
                                    int id) {
        // package.get("force1/x", force);
      }
  
      void PathDrawer::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {

        //printf("updated property: %s\n", (_property.sValue).c_str());
        if(_property.paramId == obj_file_struct.paramId) {
          obj_file_struct.sValue = _property.sValue;

          // read the points from an obj file and pass them to the OSG Line Interface
          addVectorsFromObjFile(obj_file_struct.sValue);
        }

        if(_property.paramId == svg_file_struct.paramId) {
          svg_file_struct.sValue = _property.sValue;

          // read the points from an obj file and pass them to the OSG Line Interface
          addVectorsFromSvgFile(svg_file_struct.sValue);
        }
      }

      void PathDrawer::addVectorsFromObjFile(string file_name){
        ifstream file;
        file.open (file_name.c_str());
        if (file.is_open()){
          string line;
          while ( getline (file, line) ) {
            //printf("%s\n", line.c_str());
            if(int start = line.find("v ", 0, 2) != string::npos){
              double v[3];
              start += 1;
              int end_first = line.find(" ", start) + 1;
              int end_second = line.find(" ", end_first) + 1;
              v[0] = (sReal) atof((line.substr(start, end_first-start)).c_str());
              v[1] = (sReal) atof((line.substr(end_first, end_second-end_first)).c_str());
              v[2] = (sReal) atof((line.substr(end_second).c_str()));
              l->appendData(osg_lines::Vector(v[0], v[1], v[2]));
            }
          }
          file.close();
        }else{
          fprintf(stderr, "[PathDrawer] Error: Unable to open file '%s'\n", file_name.c_str());
        }

        l->setColor(osg_lines::Color(0.0, 1.0, 0.0, 1.0));
        l->setLineWidth(4);
        control->graphics->addOSGNode(l->getOSGNode());
      }

      void PathDrawer::addVectorsFromSvgFile(string file_name){
        printf("[PathDrawer] parsing svg file '%s'\n", file_name.c_str());
        sReal m_above_ground = 0.1;
        ifstream file;
        file.open (file_name.c_str());
        if (file.is_open()){
          string line, line_to_parse;
          unsigned start = 0;
          unsigned points = 0;
          unsigned label = 0;
          unsigned end = 0;
          unsigned line_cnt = 1;
          bool in_path = false;
          bool search_for_size = true;
          sReal size[2] = {0.0, 0.0};
          bool path_found = false;
          while ( getline (file, line) ) {
            //printf("%s\n", line.c_str());

            if(search_for_size){
              if( line.find("width=")  != string::npos){
                unsigned s = line.find_first_of("\"");
                unsigned e = line.find_last_of("\"");
                size[0] = (sReal) atof(line.substr(s+1, e).c_str());
              }

              if( line.find("height=")  != string::npos){
                unsigned s = line.find_first_of("\"");
                unsigned e = line.find_last_of("\"");
                size[1] = (sReal) atof(line.substr(s+1, e).c_str());
              }

              if(size[0] > 0.001 && size[1] > 0.001){
                search_for_size = false;
                //printf("[PathDrawer] svg width %g, height %g\n", size[0], size[1]);
              }
            }

            if( line.find("<path")  != string::npos){
              start = line_cnt;
              in_path = true;
            }

            if(in_path){
              if( line.find(" d=") != string::npos){
                points = line_cnt;
                line_to_parse = line;
              }

              if( line.find("path_to_follow") != string::npos){
                label = line_cnt;
              }

              if( line.find("</path>") != string::npos){
                end = line_cnt;
                in_path = false;
                printf("found vectors in line %d (start %d, d= %d, end %d)\n", points, start, label, end);

                if(   min( start, min( points, label ) ) == start
                  &&  max( end, max( points, label ) ) == end){
                  // we found what we were looking for
                  path_found = true;
                  break;
                }
              }
            }

            if(start == string::npos || end == string::npos){
              fprintf(stderr, "[PathDrawer] there is no path specified\n");
            }

            ++line_cnt;
          }

          if(!path_found){
            fprintf(stderr, "[PathDrawer] no path 'path_to_follow' in svg file\n");
            return;
          }

          // get the path information
          sReal v[3], offset[2];
          bool absolute_coordinates = false;
          unsigned offset_pos_start = line_to_parse.find("d=\"m") + 5;
          if(offset_pos_start < 6 || offset_pos_start > 15){  // valid range
            printf("searching fro M\n");
            offset_pos_start = line_to_parse.find("d=\"M") + 5;
            absolute_coordinates = true;
          }

          unsigned offset_pos_end = line_to_parse.find(" ", offset_pos_start);
          unsigned offset_delimiter = line_to_parse.find(",", offset_pos_start);
          printf("[PathDrawer] SVG line offset pos start %d, delimiter %d, end %d\n", offset_pos_start, offset_delimiter, offset_pos_end);
          //printf("[PathDrawer] SVG OffsetX = %s\n", line_to_parse.substr(offset_pos_start, offset_delimiter-offset_pos_start).c_str());
          //printf("[PathDrawer] SVG OffsetY = %s\n", line_to_parse.substr(offset_delimiter+1, offset_pos_end-offset_delimiter-1).c_str());

          offset[0] = (sReal) atof(line_to_parse.substr(offset_pos_start, offset_delimiter-offset_pos_start).c_str());
          offset[1] = (sReal) atof(line_to_parse.substr(offset_delimiter+1, offset_pos_end-offset_delimiter-1).c_str());

          // draw the first point
          v[0] = offset[0];
          v[1] = size[1]- offset[1];
          v[2] = getHeightFromScene(v[0], v[1]) + m_above_ground;
          l->appendData(osg_lines::Vector(v[0], v[1], v[2]));

          string val_str = line_to_parse.substr(offset_pos_end);
          //printf("%s\n", val_str.c_str());
          bool x_read = false;
          bool y_read = false;
          string x_str = "";
          string y_str = "";
          string* p = &x_str;
          bool ignore = false;
          // go through the line and get all points
          for(string::iterator it = val_str.begin() + 1; it != val_str.end(); ++it){
            //printf("%c\n", *it);
            if((isdigit(*it) || *it == '.' || *it == '-') && !ignore){
              // add the char to the corresponding string
              p->append(1, *it);
            }else{
              // handle the non digits (changing string to write char to and skip other stuff)
              if(*it == ' ' || *it == '\"'){
                //printf("found ' '\n");
                y_read = true;
                if(ignore){
                  ignore = false;
                  *p = 0.0;
                }
                p = &x_str;
              }else if(*it == ','){
                //printf("found ','\n");
                x_read = true;
                y_read = false;
                if(ignore){
                  ignore = false;
                  *p = 0.0;
                }
                p = &y_str;
              }else if(ignore){
                //printf("ignoring '%c'\n", *it);
              }else if(*it == 'e'){
                ignore = true;  // this indicates a very small number, so ignore the rest and handle it
              }else if(*it == 'L'){
                absolute_coordinates = true;
              }else if(*it == 'l'){
                absolute_coordinates = false;
              }else{
                printf("[PathDrawer] no handle for '%c'\n", *it);
              }

              // add the relative x and y information and draw the point
              if(x_read && y_read){
                  sReal x = (sReal) atof(x_str.c_str());
                  sReal y = (sReal) atof(y_str.c_str());
                  if(absolute_coordinates){
                    v[0] = x;
                    v[1] = size[1] - y;
                  }else{
                    v[0] += x;
                    v[1] -= y;
                  }
                  v[2] = getHeightFromScene(v[0], v[1]) + m_above_ground;
                  //printf("adding point %g / %g\n", v[0], v[1]);
                  l->appendData(osg_lines::Vector(v[0], v[1], v[2]));

                  x_read = false;
                  x_str.clear();
                  y_read = false;
                  y_str.clear();
              }
            }
          }

          file.close();
        }else{
          fprintf(stderr, "[PathDrawer] Error: Unable to open file '%s'\n", file_name.c_str());
        }

        l->setColor(osg_lines::Color(0.0, 1.0, 0.0, 1.0));
        l->setLineWidth(4);
        control->graphics->addOSGNode(l->getOSGNode());
      }

      sReal PathDrawer::getHeightFromScene(sReal x, sReal y){
        PhysicsInterface* physics = control->sim->getPhysics();
        const utils::Vector ray_origin(x, y, 10.0);
        const utils::Vector ray_vector(0.0, 0.0, -20);
        sReal value = 10.0 - physics->getVectorCollision(ray_origin, ray_vector);

        return value;
      }
    } // end of namespace path_drawer
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::path_drawer::PathDrawer);
CREATE_LIB(mars::plugins::path_drawer::PathDrawer);
