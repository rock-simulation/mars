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

/*
 *  TerrainDrawObject2.cpp
 *  General TerrainDrawObject2 to inherit from.
 *
 *  Created by Roemmermann on 21.10.09.
 */

#ifdef USE_VERTEX_BUFFER
#include "VertexBufferTerrain.h"
#endif

#include "TerrainDrawObject.h"
#include <osg/ComputeBoundsVisitor>
#include <osg/CullFace>
#include <osg/Geometry>

namespace mars {
  namespace graphics {
  
    using mars::utils::Vector;
    using mars::interfaces::sReal;

    int TerrainDrawObject::countSubTiles = 0;

    TerrainDrawObject::TerrainDrawObject(const mars::interfaces::terrainStruct *ts)
      : DrawObject(), info(*ts) {
      info.name = ts->name;
      info.srcname = ts->srcname;
      info.texScale = ts->texScale;
      
#ifdef USE_VERTEX_BUFFER
      vbt = new VertexBufferTerrain(ts);
#endif
    }

    std::list< osg::ref_ptr< osg::Geode > > TerrainDrawObject::createGeometry() {
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      std::list< osg::ref_ptr< osg::Geode > > geodes;

#ifdef USE_VERTEX_BUFFER
      //vbt->init2();

      geode->addDrawable(vbt.get());
      geodes.push_back(geode);

      return geodes;
#endif
      osg::ref_ptr<osg::DrawElementsUInt> primitivSet;
      vertices = new osg::Vec3Array();
      texcoords = new osg::Vec2Array();
      normals = new osg::Vec3Array();
      normal_debug = new osg::Vec3Array();
      normal_geom = new osg::Geometry();

      double **tex_data_x, **tex_data_y;
      double calc;
      //double mod_val;
      // double mod_val2, mod;
      double tex_off_x = 0.0;
      double tex_off_y = 0.0;
      //double normalx, normaly, normalz;
      //double diff, diff2;
      int y = 0;
      int x = 0;
      int x_i, y_i;
      int y_off = 0;
      int x_off = 0;

      x_step = (double)info.targetWidth/(double)info.width;
      y_step = (double)info.targetHeight/(double)info.height;
      x_step2 = pow(x_step, 2);
      y_step2 = pow(y_step, 2);
      num_y =  (int)(1.0/y_step);
      num_x =  (int)(1.0/y_step);
      tex_scale_x = info.texScale;
      tex_scale_y = info.texScale;

      geom = new osg::Geometry();
      geom->setDataVariance(osg::Object::DYNAMIC);
      geom->setUseDisplayList(false);
      //geom->setUseVertexBufferObjects(true);
      tangents = new osg::Vec4Array();

      height_data = new double*[info.height+1];
      for(int i = 0; i < info.height + 1; ++i)
        height_data[i] = new double[info.width+1];

      tex_data_x = new double*[info.height+1];
      tex_data_y = new double*[info.height+1];
      for(int i = 0; i < info.height + 1; ++i) {
        tex_data_x[i] = new double[info.width+1];
        tex_data_y[i] = new double[info.width+1];
      }

      //mod_val = 0.1;
      //mod_val2 = mod_val * 0.5;

      for(int y = 0; y < info.height; ++y) {
        for(int x = 0; x < info.width; ++x) {
          // create height
          height_data[y][x] = info.pixelData[(y*info.width)+x] * info.scale;

          // create the tex_coords
          if(y<1 || x<1) {
            //mod = fmod(height_data[y][x], mod_val);
            //if(mod > mod_val2) mod -= mod_val;
            height_data[y][x] -= 0.1;
          }

          /*
            if(y > info.height -2 ||
            x > info.width -2) {
            mod = fmod(height_data[y][x], mod_val);
            //if(mod > mod_val2) mod -= mod_val;
            height_data[y][x] -= 0.11;
            }
          */
          if(y==0) {
            tex_data_y[y][x] = 0.0 + tex_off_y;
          }
          else {
            calc = fabs(height_data[y-1][x] - height_data[y][x]);
            calc = sqrt(pow(calc, 2) + y_step2);
            //tex_data_y[y][x] = tex_data_y[y-1][x] + calc;
            tex_data_y[y][x] = (y-1)*y_step + calc + tex_off_y;
          }
          if(x==0) {
            tex_data_x[y][x] = 0.0 + tex_off_x;
          }
          else {
            calc = fabs(height_data[y][x-1] - height_data[y][x]);
            calc = sqrt(pow(calc, 2) + x_step2);
            //tex_data_x[y][x] = tex_data_x[y][x-1] + calc;
            tex_data_x[y][x] = (x-1)*x_step + calc + tex_off_x;
          }
        }
      }
      for(int y = 0; y < info.height; ++y) {
        tex_data_y[y][info.width] = tex_data_y[y][info.width-1];
        tex_data_x[y][info.width] = tex_data_x[y][info.width-1]+x_step;
        height_data[y][info.width] = height_data[y][info.width-1] - 0.3;
      }
      for(int x = 0; x < info.width; ++x) {
        tex_data_x[info.height][x] = tex_data_x[info.height-1][x];
        tex_data_y[info.height][x] = tex_data_y[info.height-1][x]+y_step;
        height_data[info.height][x] = height_data[info.height-1][x] - 0.3;
      }
      tex_data_x[info.height][info.width] = tex_data_x[info.height][info.width-1]+x_step;
      tex_data_y[info.height][info.width] = tex_data_y[info.height-1][info.width]+y_step;
      height_data[info.height][info.width] = height_data[info.height-1][info.width-1] - 0.3;

      if(info.texScale == 0)
        {
          // override all the calculations in the top and scale the texture to
          // match the terrain
          for(int y = 0; y < info.height; ++y) {
            for(int x = 0; x < info.width; ++x) {
              tex_data_y[y][x] = 1.0 / (double)info.width * (y-1);
              tex_data_x[y][x] = 1.0 / (double)info.height * (x+1);
            }
          }

          tex_scale_x = 1.0;
          tex_scale_y = 1.0;
        }
      Vector n;
      osg::Vec3d t;
      for(int y = 0, y2; y < info.height + 1; ++y) {
        for(int x = 0, x2; x < info.width + 1; ++x) {
          y2 = y;
          x2 = x;

          n = getNormal(x, y, info.width+1, info.height+1,
                        x_step, y_step, height_data, &t, true);
          /*
          //if(y==info.height) y2 = y-1;
          //if(x==info.width) x2 = x-1;

          diff2 = x_step;
          if(x2==0) {
          diff = height_data[y2][x2+2] - height_data[y2][x2+1];
          }
          else if(x2==1) {
          diff = height_data[y2][x2+1] - height_data[y2][x2];
          }
          else if(x2==info.width-1) {
          diff = height_data[y2][x2] - height_data[y2][x2-1];
          }
          else if(x2>info.width-1) {
          diff = height_data[y2][x2-1] - height_data[y2][x2-2];
          }
          else {
          diff = height_data[y2][x2+1] - height_data[y2][x2-1];
          diff2 = x_step*2.0;
          }

          diff = atan2(diff, diff2);

          normalx = -sin(diff);
          normalz = cos(diff);

          diff2 = y_step;
          if(y2==0) {
          diff = height_data[y2+2][x2] - height_data[y2+1][x2];
          }
          else if(y2==1) {
          diff = height_data[y2+1][x2] - height_data[y2][x2];
          }
          else if(y==info.height-1) {
          diff = height_data[y2][x2] - height_data[y2-1][x2];
          }
          else if(y>info.height-1) {
          diff = height_data[y2-1][x2] - height_data[y2-2][x2];
          }
          else {
          diff = height_data[y2+1][x2] - height_data[y2-1][x2];
          diff2 = y_step*2.0;
          }
          diff = atan2(diff, diff2);

          normaly = -sin(diff);
          normalz += cos(diff);
          normalz *= 0.5;

          diff = 1/sqrt(pow(normalx, 2)+pow(normaly, 2)+pow(normalz, 2));
          normalx *= diff;
          normaly *= diff;
          normalz *= diff;
          */
          vertices->push_back(osg::Vec3(x*x_step, y*y_step, height_data[y2][x2]));
          //normals->push_back(osg::Vec3(normalx, normaly, normalz));
          normals->push_back(osg::Vec3(n.x(), n.y(), n.z()));
          tangents->push_back(osg::Vec4(t.x(), t.y(), t.z(), 0.0));
          normal_debug->push_back(osg::Vec3(x*x_step, y*y_step,
                                            height_data[y2][x2]));
          normal_debug->push_back(osg::Vec3(x*x_step, y*y_step,
                                            height_data[y2][x2])+
                                  osg::Vec3(n.x(), n.y(), n.z())*0.1);

          // should use tex_scale in shader
          tex_data_x[y][x] *= tex_scale_x;
          tex_data_y[y][x] *= tex_scale_y;

          texcoords->push_back(osg::Vec2(tex_data_x[y][x], tex_data_y[y][x]));
        }
      }

      geom->setVertexArray(vertices.get());
      geom->setNormalArray(normals.get());
      geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
      geom->setTexCoordArray(DEFAULT_UV_UNIT,texcoords.get());
      geom->setTexCoordArray(1,texcoords.get()); // TODO: y?

      normal_geom->setVertexArray(normal_debug.get());
      osg::Vec4Array* colours_debug = new osg::Vec4Array(1);
      (*colours_debug)[0].set(1.0, 0.0, 0.0, 1.0);
      normal_geom->setColorArray(colours_debug);
      normal_geom->setColorBinding(osg::Geometry::BIND_OVERALL);

      normal_geom->addPrimitiveSet(new osg::DrawArrays(GL_LINES,
                                                       0, normal_debug->size()));

      // create faces
      if(num_y < 1) num_y = 1;
      if(num_x < 1) num_x = 1;
      int l1, l2;
      int count = 0;

      while(y_off < info.height) {
        x_off = 0;
        while(x_off < info.width + 1) {
          y=0;
          while((y < num_y) && (y_off + y < info.height)) {
            y_i = y + y_off;
            primitivSet = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES,
                                                    0);
            l1 = (y_i+1)*(info.width+1)+x_off;
            l2 = y_i*(info.width+1)+x_off;

            x=1;
            while((x < num_x + 1) && (x_off + x < info.width + 1)) {
              x_i = x+x_off;

              primitivSet->push_back(l1);
              primitivSet->push_back(l2);
              l1 = l2;
              l2 = (y_i+1)*(info.width+1)+x_i;
              primitivSet->push_back(l2);

              primitivSet->push_back(l2);
              primitivSet->push_back(l1);
              l1 = l2;
              l2 = y_i*(info.width+1)+x_i;
              primitivSet->push_back(l2);
              x++;
            }
            geom->addPrimitiveSet(primitivSet.get());
            y++;
          }
          count++;
          x_off += num_x;
        }
        count = 0;
        y_off += num_y;
      }

      geode->addDrawable(geom);
      geodes.push_back(geode);

      normal_geode = new osg::Geode;
      normal_geode->addDrawable(normal_geom.get());
      normal_geode->getOrCreateStateSet()->setMode(GL_LIGHTING,
                                                   osg::StateAttribute::OFF);
      return geodes;
    }


    void TerrainDrawObject::generateTangents() {
#ifdef USE_VERTEX_BUFFER
      return;
#endif
      geom->setVertexAttribData(TANGENT_UNIT,
                                osg::Geometry::ArrayData(tangents.get(),
                                                         osg::Geometry::BIND_PER_VERTEX ) );
    }

    void TerrainDrawObject::collideSphere(Vector pos, sReal radius) {
      pos += pivot_ - position_;
#ifdef USE_VERTEX_BUFFER
      vbt->collideSphere(pos.x(), pos.y(), pos.z(), radius);
      return;
#endif

      std::vector<SubTile*>::iterator iter;

      for(iter=vSubTiles.begin(); iter!=vSubTiles.end(); ++iter) {
        (*iter)->frame++;
      }

      if(vSubTiles.size() > 1000) {
        std::vector<SubTile*> copy;
        int remove = 0;
        for(iter=vSubTiles.begin(); iter!=vSubTiles.end(); ++iter) {
          if((*iter)->frame < 400 || remove > 1) {
            copy.push_back(*iter);
          }
          else {
            remove++;
            removeSubTile(*iter);
            subTiles.erase((*iter)->mapIndex);
            TerrainDrawObject::countSubTiles--;
            //geom->dirtyBound();
            //geom->dirtyDisplayList();
            //vertices->dirty();
          }
        }
        if(remove) {
          vSubTiles.clear();
          for(iter=copy.begin(); iter!=copy.end(); ++iter) {
            vSubTiles.push_back(*iter);
          }
        }
      }

      // first check if we are within the current heightmap
      if((pos.x() < 0 || pos.y() < 0) ||
         (pos.x() > info.targetWidth || pos.y() > info.targetHeight)) {

        std::vector<SubTile*> copy;
        int remove = 0;
        for(iter=vSubTiles.begin(); iter!=vSubTiles.end(); ++iter) {
          if((*iter)->frame < 4000 || remove > 1) {
            copy.push_back(*iter);
          }
          else {
            remove++;
            removeSubTile(*iter);
            subTiles.erase((*iter)->mapIndex);
            TerrainDrawObject::countSubTiles--;
            //geom->dirtyBound();
            //geom->dirtyDisplayList();
          }
        }
        if(remove) {
          vSubTiles.clear();
          for(iter=copy.begin(); iter!=copy.end(); ++iter) {
            vSubTiles.push_back(*iter);
          }
        }
        return;
      }

      std::vector<SubTile*> toProcess;
      osg::Vec3d v1(pos.x(), pos.y(), pos.z());
      osg::Vec3d v2;
      //osg::Vec3d sphereCorner1 = v1-osg::Vec3d(radius, radius, 0);
      //osg::Vec3d sphereCorner2 = v1+osg::Vec3d(radius, radius, 0);
      //int where;
      double interpolation = 0.00;
      // then search for subtiles
      /*
        for(iter=subTiles.begin(); iter!=subTiles.end(); ++iter) {
        v1 = osg::Vec3d((*iter)->xPos, (*iter)->yPos, 0.0);
        v2.x() = v1.x() + (*iter)->xSize;
        v2.y() = v1.y() + (*iter)->ySize;
        v2.z() = 0.0;
        if((where = rectangleIntersect(v1, v2, sphereCorner1, sphereCorner2))) {
        (*iter)->where = where;
        toProcess.push_back(*iter);
        }
        }
      */
      int x1 = floor((pos.x()-radius*1.5) / x_step);
      int y1 = floor((pos.y()-radius*1.5) / y_step);
      int x2 = ceil((pos.x()+radius*1.5) / x_step);
      int y2 = ceil((pos.y()+radius*1.5) / y_step);

      // find new resolution
      double x_res = x_step;
      double y_res = y_step;
      while( (x_res=x_res*0.5) > radius * 0.35) ;
      while( (y_res=y_res*0.5) > radius * 0.35) ;
      // gen new size:
      /*
        double x_size = ((*(vertices.get()))[y1*(info.width+1)+x2][0]-
        (*(vertices.get()))[y1*(info.width+1)+x1][0]);
        double y_size = ((*(vertices.get()))[y2*(info.width+1)+x1][1]-
        (*(vertices.get()))[y1*(info.width+1)+x1][1]);
        int x_count = round(x_size / x_res);
        int y_count = round(y_size / y_res);
      */
      double x_size = x_step;
      double y_size = y_step;
      int x_count = round(x_size / x_res);
      int y_count = round(y_size / y_res);
      std::map<int, SubTile*>::iterator it;
      bool dirty = false;

      //cutHole(x1, x2, y1, y2);

      for(int i=y1; i<y2; ++i) {
        for(int j=x1; j<x2; ++j) {
          it = subTiles.find(i*(info.width)+j);
          if(it != subTiles.end()) {
            toProcess.push_back(it->second);
          }
          else {
            cutHole(j, j+1, i, i+1);
            SubTile *newSubTile = new SubTile;
            newSubTile->xRes = x_res;
            newSubTile->yRes = y_res;
            newSubTile->xSize = x_size;
            newSubTile->ySize = y_size;
            newSubTile->xCount = x_count+1;
            newSubTile->yCount = y_count+1;
            newSubTile->xPos = (*(vertices.get()))[i*(info.width+1)+j][0];
            newSubTile->yPos = (*(vertices.get()))[i*(info.width+1)+j][1];
            newSubTile->vStart = 0;//vertices->size();
            newSubTile->vCount = 0; //(x_count+1) * (y_count+1);
            newSubTile->x = j;
            newSubTile->y = i;
            newSubTile->heightData = new double*[y_count+1];
            for(int l=0; l<y_count+1; ++l) {
              newSubTile->heightData[l] = new double[x_count+1];
            }
            newSubTile->frame = 0;
            createNewSubTile(newSubTile, pos+Vector(0.0, 0.0, -interpolation), radius-interpolation);
            newSubTile->mapIndex = i*(info.width)+j;
            subTiles[newSubTile->mapIndex] = newSubTile;
            vSubTiles.push_back(newSubTile);
            dirty = true;
            TerrainDrawObject::countSubTiles++;
            toProcess.push_back(newSubTile);
          }
        }
      }

      for(iter=toProcess.begin(); iter!=toProcess.end(); ++iter) {
        (*iter)->frame = 0;
        if(adaptSubtile(*iter, pos+Vector(0.0, 0.0, -interpolation),
                        radius-interpolation)) dirty = true;
      }

      if(dirty) {

        //geom->dirtyDisplayList();
        //geom->dirtyBound();
      }

    }

    void TerrainDrawObject::createNewSubTile(SubTile *newSubTile, Vector pos,
                                             double radius) {
      int x, y;
      Vector n;
      double calcX, calcY;
      osg::Vec3d v;
      osg::Vec3d v2;
      osg::Vec3d sPos(pos.x(), pos.y(), pos.z());
      osg::ref_ptr<osg::Vec3Array> normalDebug = new osg::Vec3Array();
      newSubTile->normalGeom = new osg::Geometry();

      for(y=0; y<newSubTile->yCount; ++y) {
        for(x=0; x<newSubTile->xCount; ++x) {
          newSubTile->heightData[y][x] = getHeight(x, y, newSubTile);;
        }
      }

      /*
        for(y=0; y<newSubTile->yCount; ++y) {
        for(x=0; x<newSubTile->xCount; ++x) {
        v.x() = newSubTile->xPos + x*newSubTile->xRes;
        v.y() = newSubTile->yPos + y*newSubTile->yRes;

        v2 = v - sPos;
        v2.z() = 0;
        if(v2.length() <= radius) {
        v2 = intersectReplacementSphere(sPos, radius, v);
        if(v2.z() < v.z()-0.001) {
        newSubTile->heightData[y][x] = v2.z();
        }
        }
        }
        }
      */

      newSubTile->vertices = new osg::Vec3Array();
      newSubTile->normals = new osg::Vec3Array();
      newSubTile->tangents = new osg::Vec4Array();
      newSubTile->texcoords = new osg::Vec2Array();
      newSubTile->geom = new osg::Geometry();
      newSubTile->geom->setDataVariance(osg::Object::DYNAMIC);
      //newSubTile->geom->setUseVertexBufferObjects(true);
      //newSubTile->geom->setUseDisplayList(false);
      osg::Vec3d t;
      for(y=0; y<newSubTile->yCount; ++y) {
        for(x=0; x<newSubTile->xCount; ++x) {
          v.x() = newSubTile->xPos + x*newSubTile->xRes;
          v.y() = newSubTile->yPos + y*newSubTile->yRes;
          v.z() = newSubTile->heightData[y][x];

          n = getNormal(x, y, newSubTile->xCount, newSubTile->yCount,
                        newSubTile->xRes, newSubTile->yRes,
                        newSubTile->heightData, &t);
          newSubTile->vertices->push_back(osg::Vec3(v.x(), v.y(), v.z()));
          newSubTile->normals->push_back(osg::Vec3(n.x(), n.y(), n.z()));
          newSubTile->tangents->push_back(osg::Vec4(t.x(), t.y(), t.z(), 0.0));

          if(y>0) {
            calcY = fabs(newSubTile->heightData[y-1][x] -
                         newSubTile->heightData[y][x]);
            calcY = sqrt(pow(calcY, 2) + newSubTile->yRes);
          }
          calcY = 0;
          if(x>0) {
            calcX = fabs(newSubTile->heightData[y][x-1] -
                         newSubTile->heightData[y][x]);
            calcX = sqrt(pow(calcX, 2) + newSubTile->xRes);
          }
          calcX = 0;

          newSubTile->texcoords->push_back(osg::Vec2(v.x()*tex_scale_x+calcX,
                                                     v.y()*tex_scale_y+calcY));

          normalDebug->push_back(v);
          normalDebug->push_back(v+osg::Vec3(n.x(), n.y(), n.z())*0.1);

        }
      }

      newSubTile->geom->setVertexArray(newSubTile->vertices.get());
      newSubTile->geom->setNormalArray(newSubTile->normals.get());
      newSubTile->geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
      newSubTile->geom->setTexCoordArray(DEFAULT_UV_UNIT,
                                         newSubTile->texcoords.get());
      newSubTile->geom->setTexCoordArray(1,newSubTile->texcoords.get()); // TODO: y?
      newSubTile->geom->setVertexAttribData(TANGENT_UNIT,
                                            osg::Geometry::ArrayData(newSubTile->tangents.get(),
                                                                     osg::Geometry::BIND_PER_VERTEX ) );

      newSubTile->geode = new osg::Geode;
      newSubTile->geode->addDrawable(newSubTile->geom);
      //geodes.push_back(newSubTile->geode);
      group_->addChild(newSubTile->geode);
      /*
        if(geom->getVertexAttribArray(tangentUnit)) {
        geom->getVertexAttribArray(tangentUnit)->dirty();
        }
      */
      /*
        geom->setVertexAttribData(tangentUnit, osg::Geometry::ArrayData( tangents,
        osg::Geometry::BIND_PER_VERTEX, GL_FALSE ) );
      */

      newSubTile->normalGeom->setVertexArray(normalDebug.get());
      osg::Vec4Array* colours_debug = new osg::Vec4Array(1);
      (*colours_debug)[0].set(1.0, 0.0, 0.0, 1.0);
      newSubTile->normalGeom->setColorArray(colours_debug);
      newSubTile->normalGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

      newSubTile->normalGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINES,
                                                                  0, normalDebug->size()));
      //normal_geode->addDrawable(newSubTile->normalGeom.get());

      int o = newSubTile->vStart;
      newSubTile->pSet = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
      for(y=0; y<newSubTile->yCount-1; ++y) {
        for(x=0; x<newSubTile->xCount-1; ++x) {
          newSubTile->pSet->push_back(o+(y+1)*newSubTile->xCount+x);
          newSubTile->pSet->push_back(o+y*newSubTile->xCount+x);
          newSubTile->pSet->push_back(o+(y+1)*newSubTile->xCount+x+1);

          newSubTile->pSet->push_back(o+(y+1)*newSubTile->xCount+x+1);
          newSubTile->pSet->push_back(o+y*newSubTile->xCount+x);
          newSubTile->pSet->push_back(o+y*newSubTile->xCount+x+1);
        }
      }
      newSubTile->geom->addPrimitiveSet(newSubTile->pSet.get());
      //geom->addPrimitiveSet(newSubTile->pSet.get());
    }

    Vector TerrainDrawObject::getNormal(int x, int y, int mx, int my,
                                        double x_step, double y_step,
                                        double **height_data, osg::Vec3d* t, bool skipBorder) {
      double nx, ny, nz;
      double vx1, vz1, vy2, vz2;

      if(skipBorder) {
        if(x < mx-2 && x > 1) {
          vz1 = height_data[y][x+1] - height_data[y][x-1];
          vx1 = x_step*2.0;
        }
        else if(x==0) {
          vz1 = height_data[y][x+2] - height_data[y][x+1];
          vx1 = x_step;
        }
        else if(x==1) {
          vz1 = height_data[y][x+1] - height_data[y][x];
          vx1 = x_step;
        }
        else if(x==mx-1) {
          vz1 = height_data[y][x-1] - height_data[y][x-2];
          vx1 = x_step;
        }
        else {
          vz1 = height_data[y][x] - height_data[y][x-1];
          vx1 = x_step;
        }

        if(y > 1 && y < my-2) {
          vz2 = height_data[y+1][x] - height_data[y-1][x];
          vy2 = y_step*2.0;
        }
        else if(y==0) {
          vz2 = height_data[y+2][x] - height_data[y+1][x];
          vy2 = y_step;
        }
        else if(y==1) {
          vz2 = height_data[y+1][x] - height_data[y][x];
          vy2 = y_step;
        }
        else if(y==my-1) {
          vz2 = height_data[y-1][x] - height_data[y-2][x];
          vy2 = y_step;
        }
        else {
          vz2 = height_data[y][x] - height_data[y-1][x];
          vy2 = y_step;
        }
      }
      else {
        if(x != 0 && x != mx-1) {
          vz1 = height_data[y][x+1] - height_data[y][x-1];
          vx1 = x_step*2.0;
        }
        else if(x==0) {
          vz1 = height_data[y][x+1] - height_data[y][x];
          vx1 = x_step;
        }
        else {
          vz1 = height_data[y][x] - height_data[y][x-1];
          vx1 = x_step;
        }

        if(y != 0 && y != my-1) {
          vz2 = height_data[y+1][x] - height_data[y-1][x];
          vy2 = y_step*2.0;
        }
        else if(y==0) {
          vz2 = height_data[y+1][x] - height_data[y][x];
          vy2 = y_step;
        }
        else {
          vz2 = height_data[y][x] - height_data[y-1][x];
          vy2 = y_step;
        }
      }

      nx = -vz1*vy2;
      ny = -vz2*vx1;
      nz = vx1*vy2;
      t->x() = vx1;
      t->y() = vy2;
      t->z() = vz1 + vz2;
      t->normalize();
      Vector n(nx, ny, nz);
      return n.normalized();
    }

    inline double sqr(double x) {
      return x*x;
    }

    osg::Vec3d TerrainDrawObject::intersectReplacementSphere(osg::Vec3 center,
                                                             double radius,
                                                             osg::Vec3 vertex) {
      osg::Vec3d ret = vertex;
      ret.z() = center.z() - sqrt(sqr(radius)-sqr(center.x()-vertex.x())-
                                  sqr(center.y()-vertex.y()));
      return ret;
    }

    void TerrainDrawObject::cutHole(int x1, int x2, int y1, int y2) {
      std::set<int> iPSet;
      std::vector<int> iVert;
      std::set<int>::iterator iter;
      std::vector<int>::iterator jter;
      int px, py, py2, pi;
      int da = (int)ceil(((double)info.width+1.0)/(double)num_x);
      osg::PrimitiveSet *pSet;
      int id;
      bool found;
      osg::ref_ptr<osg::DrawElementsUInt> primitiveSet;

      // change vertices
      for(int i=y1; i<y2; ++i) {
        for(int j=x1; j<x2; ++j) {
          //(*(vertices.get()))[i*(info.width+1)+j][2] = pos.z;
          px = j/num_x;
          py = i/num_y;
          py2 = i-(py*num_y);
          pi = py*da*num_y + px*num_y + py2;

          if(i<y2 && j <x2) {
            iPSet.insert(pi);
            iVert.push_back(i*(info.width+1)+j);
          }
        }
      }

      for(iter=iPSet.begin(); iter!=iPSet.end(); ++iter) {
        if((pSet = geom->getPrimitiveSet(*iter))) {
          primitiveSet = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
          for(int i=0; i<(int)pSet->getNumIndices()/6; ++i) {
            id = pSet->index(i*6+1);
            found = false;
            for(jter=iVert.begin(); jter!=iVert.end(); ++jter) {
              if(id == *jter) {
                found = true;
              }
            }
            if(!found) {
              primitiveSet->push_back(pSet->index(i*6));
              primitiveSet->push_back(pSet->index(i*6+1));
              primitiveSet->push_back(pSet->index(i*6+2));
              primitiveSet->push_back(pSet->index(i*6+3));
              primitiveSet->push_back(pSet->index(i*6+4));
              primitiveSet->push_back(pSet->index(i*6+5));
            }
          }
          geom->removePrimitiveSet(*iter);
          geom->insertPrimitiveSet(*iter, primitiveSet.get());
        }
      }
      //vertices->dirty();
      //normals->dirty();
      //tangents->dirty();
      /*
        if(geom->getVertexAttribArray(tangentUnit)) {
        geom->getVertexAttribArray(tangentUnit)->dirty();
        }
      */
    }

    bool operator<=(const osg::Vec3& a, const osg::Vec3& b)
    {
      return a.x()<=b.x() && a.y()<=b.y();
    }


    template<typename T>
    bool between(const T& down, const T& up, const T& query)
    {
      return down<=query && query<=up;
    }

    int TerrainDrawObject::rectangleIntersect(const osg::Vec3& first_leftup,
                                              const osg::Vec3& first_downright,
                                              const osg::Vec3& second_leftup,
                                              const osg::Vec3& second_downright) {

      if (between(first_leftup, first_downright, second_leftup)&&
          between(first_leftup, first_downright, second_downright))
        return 9;

      if ( between(second_leftup, second_downright, first_leftup) )
        return 1;
      if ( between(second_leftup, second_downright,
                   osg::Vec3(first_downright.x(), first_leftup.y(), 0)) )
        return 3;
      if ( between(second_leftup, second_downright,
                   osg::Vec3(first_leftup.x(), first_downright.y(), 0)) )
        return 6;
      if ( between(second_leftup, second_downright, first_downright) )
        return 8;

      if (between(second_leftup.y(),second_downright.y(), first_leftup.y()) &&
          between(first_leftup.x(), first_downright.x(), second_leftup.x()))
        return 2;
      if (between(second_leftup.x(), second_downright.x(), first_leftup.x()) &&
          between(first_leftup.y(), first_downright.y(), second_leftup.y()))
        return 4;
      if (between(second_leftup.x(), second_downright.x(), first_downright.x()) &&
          between(first_leftup.y(), first_downright.y(), second_leftup.y()))
        return 5;
      if (between(second_leftup.y(),second_downright.y(), first_downright.y()) &&
          between(first_leftup.x(), first_downright.x(), second_leftup.x()))
        return 7;

      return 0;
    }

    bool TerrainDrawObject::adaptSubtile(SubTile* tile, Vector pos, double radius) {
      int x, y;
      Vector n;
      osg::Vec3d v;
      osg::Vec3d v2;
      osg::Vec3d sPos(pos.x(), pos.y(), pos.z());
      bool adapt = false;
      int vid;
      //double calcX, calcY;
      double interpolation = 4.0;
      int x1 = floor((pos.x() - tile->xPos - radius) / tile->xRes)-2;
      int y1 = floor((pos.y() - tile->yPos - radius) / tile->yRes)-2;
      int x2 = ceil((pos.x() - tile->xPos + radius) / tile->xRes)+2;
      int y2 = ceil((pos.y() - tile->yPos + radius) / tile->yRes)+2;
      if(x1 < 0) x1 = 0;
      if(y1 < 0) y1 = 0;
      if(x2 > tile->xCount) x2 = tile->xCount;
      if(y2 > tile->yCount) y2 = tile->yCount;
      double r2 = radius*radius;
      for(y=y1; y<y2; ++y) {
        for(x=x1; x<x2; ++x) {
          v.x() = tile->xPos + x*tile->xRes;
          v.y() = tile->yPos + y*tile->yRes;
          v.z() = tile->heightData[y][x];

          v2 = v - sPos;
          v2.z() = 0;
          if(v2.length2() <= r2) {
            v2 = intersectReplacementSphere(sPos, radius, v);
            if(v2.z() < v.z()-0.001) {
              adapt = true;
              tile->heightData[y][x] = v2.z();
            }
          }
          if(adapt && 0) {
            if(y > 1 && (tile->heightData[y][x] <
                         tile->heightData[y-1][x]-tile->yRes*interpolation)) {
              tile->heightData[y-1][x] = tile->heightData[y][x]+tile->yRes*interpolation;
            }

            if(x > 1 && (tile->heightData[y][x] <
                         tile->heightData[y][x-1]-tile->xRes*interpolation)) {
              tile->heightData[y][x-1] = tile->heightData[y][x]+tile->xRes*interpolation;
            }

            if(y < tile->yCount-2 && (tile->heightData[y][x] <
                                      tile->heightData[y+1][x]-tile->yRes*interpolation)) {
              tile->heightData[y+1][x] = tile->heightData[y][x]+tile->yRes*interpolation;
            }

            if(x < tile->xCount-2 && (tile->heightData[y][x] <
                                      tile->heightData[y][x+1]-tile->xRes*interpolation)) {
              tile->heightData[y][x+1] = tile->heightData[y][x]+tile->xRes*interpolation;
            }

            // the corners
            if(y > 1 && x > 1 && (tile->heightData[y][x] <
                                  tile->heightData[y-1][x-1]-(tile->yRes+tile->xRes)*interpolation*0.5)) {
              tile->heightData[y-1][x-1] = tile->heightData[y][x]+(tile->yRes+tile->xRes)*interpolation*0.5;
            }

            if(y > 1 &&
               x < tile->xCount-2 && (tile->heightData[y][x] <
                                      tile->heightData[y-1][x+1]-(tile->yRes+tile->xRes)*interpolation*0.5)) {
              tile->heightData[y-1][x+1] = tile->heightData[y][x]+(tile->yRes+tile->xRes)*interpolation*0.5;
            }

            if(y < tile->yCount-2 &&
               x > 1 && (tile->heightData[y][x] <
                         tile->heightData[y+1][x-1]-(tile->yRes+tile->xRes)*interpolation*0.5)) {
              tile->heightData[y+1][x-1] = tile->heightData[y][x]+(tile->yRes+tile->xRes)*interpolation*0.5;
            }

            if(y < tile->yCount-2 &&
               x < tile->xCount-2 && (tile->heightData[y][x] <
                                      tile->heightData[y+1][x+1]-(tile->yRes+tile->xRes)*interpolation*0.5)) {
              tile->heightData[y+1][x+1] = tile->heightData[y][x]+(tile->yRes+tile->xRes)*interpolation*0.5;
            }
          }

        }
      }

      if(adapt) {
        osg::Vec3d t;
        for(y=y1; y<y2; ++y) {
          for(x=x1; x<x2; ++x) {
            v.x() = tile->xPos + x*tile->xRes;
            v.y() = tile->yPos + y*tile->yRes;
            v.z() = tile->heightData[y][x];

            n = getNormal(x, y, tile->xCount, tile->yCount,
                          tile->xRes, tile->yRes, tile->heightData, &t);
            vid = tile->vStart+y*(tile->xCount)+x;
            (*(tile->vertices.get()))[vid] = v;
            (*(tile->normals.get()))[vid] = osg::Vec3(n.x(), n.y(), n.z());
            (*(tile->tangents.get()))[vid] = osg::Vec4(t.x(), t.y(), t.z(), 0.0);

            /*
              if(y>0) {
              calcY = fabs(tile->heightData[y-1][x] - tile->heightData[y][x]);
              calcY = sqrt(pow(calcY, 2) + tile->yRes);
              }
              calcY = 0;
              if(x>0) {
              calcX = fabs(tile->heightData[y][x-1] - tile->heightData[y][x]);
              calcX = sqrt(pow(calcX, 2) + tile->xRes);
              }
              calcX = 0;

              (*(tile->texcoords.get()))[vid] = osg::Vec2(v.x()*tex_scale_x+calcX,
              v.y()*tex_scale_y+calcY);
            */
            /*
              (*(normalDebug.get()))[vid*2] = v;
              (*(normalDebug.get()))[vid*2+1] = v+osg::Vec3(n.x, n.y, n.z)*0.1;
            */
          }
        }

        //tile->vertices->dirty();
        //tile->geom->dirtyBound();
        //tile->geom->dirtyDisplayList();
      }
      return adapt;
    }


    void TerrainDrawObject::drawSubTile(SubTile *tile) {
      osg::Vec3d v;
      int vid;

      for(int y=0; y<tile->yCount; ++y) {
        for(int x=0; x<tile->xCount; ++x) {
          v.x() = tile->xPos + x*tile->xRes;
          v.y() = tile->yPos + y*tile->yRes;
          v.z() = tile->heightData[y][x];
          vid = tile->vStart+y*(tile->xCount)+x;
          (*(tile->vertices.get()))[vid] = v;
        }
      }
    }

    double TerrainDrawObject::getHeight(int x, int y, SubTile *tile) {
      double hs[4];
      double dx, dy;
      osg::Vec3d v;
      int x2, y2;

      v.x() = tile->xPos + x*tile->xRes;
      v.y() = tile->yPos + y*tile->yRes;
      x2 = floor((v.x()) / x_step);
      y2 = floor((v.y()) / y_step);

      hs[0] = height_data[y2][x2];
      if(y2 < info.height) hs[1] = height_data[y2+1][x2];
      else hs[1] = hs[0];
      if(x2 < info.width) hs[2] = height_data[y2][x2+1];
      else hs[2] = hs[0];
      if(y2 < info.height && x2 < info.width) hs[3] = height_data[y2+1][x2+1];
      else hs[3] = hs[0];
      dx = (v.x() - x2*x_step)/x_step;
      dy = (v.y() - y2*y_step)/y_step;
      return (hs[0] * (1-dx)*(1-dy) + hs[2] * dx * (1-dy) +
              hs[1] * (1-dx) * dy + hs[3] * dx * dy);
    }

    void TerrainDrawObject::removeSubTile(SubTile *tile) {
      int da = (int)ceil(((double)info.width+1.0)/(double)num_x);
  
      //geom->removePrimitiveSet(geom->getPrimitiveSetIndex(tile->pSet.get()));
      int id = tile->geom->getPrimitiveSetIndex(tile->pSet.get());
      tile->geom->removePrimitiveSet(id);

      for(int i=0; i<tile->yCount; ++i) delete tile->heightData[i];
      delete tile->heightData;
  
      // check for subtile neihbours
      std::map<int, SubTile*>::iterator it;
      if(tile->y > 0) {
        it = subTiles.find((tile->y-1)*(info.width)+tile->x);
        if(it != subTiles.end()) {
          for(int x=0; x<tile->xCount; ++x) 
            (it->second)->heightData[(it->second)->yCount-1][x] = getHeight(x, it->second->yCount-1, it->second);
          drawSubTile(it->second);
        }
      }

      if(tile->y > 0 && tile->x > 0) {
        it = subTiles.find((tile->y-1)*(info.width)+tile->x-1);
        if(it != subTiles.end()) {
          (it->second)->heightData[(it->second)->yCount-1][(it->second)->xCount-1] = getHeight(it->second->xCount-1,
                                                                                               it->second->yCount-1, it->second);
          drawSubTile(it->second);
        }
      }

      if(tile->x > 0) {
        it = subTiles.find((tile->y)*(info.width)+tile->x-1);
        if(it != subTiles.end()) {
          for(int y=0; y<tile->yCount; ++y) 
            (it->second)->heightData[y][(it->second)->xCount-1] = getHeight(it->second->xCount-1, y, it->second);
          drawSubTile(it->second);
        }
      }

      if(tile->y < info.height) {
        it = subTiles.find((tile->y+1)*(info.width)+tile->x);
        if(it != subTiles.end()) {
          for(int x=0; x<tile->xCount; ++x) 
            (it->second)->heightData[0][x] = getHeight(x, 0, it->second);
          drawSubTile(it->second);
        }
      }

      if(tile->y < info.height && tile->x < info.width) {
        it = subTiles.find((tile->y+1)*(info.width)+tile->x+1);
        if(it != subTiles.end()) {
          (it->second)->heightData[0][0] = getHeight(0, 0, it->second);
          drawSubTile(it->second);
        }
      }

      if(tile->x < info.width) {
        it = subTiles.find((tile->y)*(info.width)+tile->x+1);
        if(it != subTiles.end()) {
          for(int y=0; y<tile->yCount; ++y) 
            (it->second)->heightData[y][0] = getHeight(0, y, it->second);
          drawSubTile(it->second);
        }
      }

      if(tile->y > 0 && tile->x < info.width) {
        it = subTiles.find((tile->y-1)*(info.width)+tile->x+1);
        if(it != subTiles.end()) {
          (it->second)->heightData[(it->second)->yCount-1][0] = getHeight(0, it->second->yCount-1, it->second);
          drawSubTile(it->second);
        }
      }

      if(tile->y < info.height && tile->x > 0) {
        it = subTiles.find((tile->y+1)*(info.width)+tile->x-1);
        if(it != subTiles.end()) {
          (it->second)->heightData[0][(it->second)->xCount-1] = getHeight(it->second->xCount-1, 0, it->second);
          drawSubTile(it->second);
        }
      }


      /*
      // copy vertices, normal, etc
      for(int i=0; i<tile->vCount; ++i) {
      (*(vertices.get())).erase(vertices->begin()+tile->vStart);
      (*(normals.get())).erase(normals->begin()+tile->vStart);
      (*(tangents.get())).erase(tangents->begin()+tile->vStart);
      (*(texcoords.get())).erase(texcoords->begin()+tile->vStart);
      }
      */
      group_->removeChild(tile->geode);
      //normal_geode->removeDrawable(tile->normalGeom);

      // get pset index
      int x = floor(tile->xPos/x_step);
      int y = floor(tile->yPos/y_step);
      int px = x/num_x;
      int py = y/num_y;
      int py2 = y-(py*num_y);

      int pi = py*da*num_y + px*num_y + py2;


      osg::DrawElementsUInt *pSet = dynamic_cast<osg::DrawElementsUInt *>(geom->getPrimitiveSet(pi));

      pSet->push_back((y+1)*(info.width+1)+x);
      pSet->push_back(y*(info.width+1)+x);
      pSet->push_back((y+1)*(info.width+1)+x+1);

      pSet->push_back((y+1)*(info.width+1)+x+1);
      pSet->push_back(y*(info.width+1)+x);
      pSet->push_back(y*(info.width+1)+x+1);

      /*
        vertices->dirty();
        normals->dirty();
        tangents->dirty();
        if(geom->getVertexAttribArray(tangentUnit)) {
        geom->getVertexAttribArray(tangentUnit)->dirty();
        }
      */
    }

    void TerrainDrawObject::updateShadowObject(Vector pos) {
      /*
        unsigned int i;

        osg::Node* node2 = theObject->getChild(0);
        osg::Geode* geode2;
        osg::Drawable* drawable2;
        osg::Geometry* geometry2;
        osg::Vec3Array* v2;

        if(!node2) return;

        geode2 = dynamic_cast<osg::Geode*>(node2);
        if(!geode2) return;
        drawable2 = geode2->getDrawable(0);
        if(!drawable2) return;

        geometry2 = drawable2->asGeometry();
        if(!geometry2) return;
  
        v2 = dynamic_cast<osg::Vec3Array*>(geometry2->getVertexArray());
        if(!v2) return;


        if(!shadowNode) {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry());
        osg::ref_ptr<osg::Vec3Array> normals(new osg::Vec3Array());
        osg::ref_ptr<osg::Vec3Array> v(new osg::Vec3Array());
        osg::Vec3 off(0.0f-pivot.x, 0.0f-pivot.y, 0.0f-pivot.z);
        off = scaleTransform->getMatrix() * off;
        off[2] += 0.002;
        off += posTransform->getPosition();
    
        shadowNode = new osg::Group();
        geometry->setUseDisplayList(false);
    
        for(i=0; i<v2->size(); i++) v->push_back((scaleTransform->getMatrix()*(*v2)[i])+off);
        geometry->setVertexArray(v.get());
    
        geometry->setDataVariance(osg::Object::DYNAMIC);
        normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
        geometry->setNormalArray(normals.get());
        geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
        osg::Vec4Array* colours = new osg::Vec4Array(1);
      
        (*colours)[0].set(1.0, 1.0, 1.0, 1.0);
  
        geometry->setColorArray(colours);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

        geometry->setTexCoordArray(0, geometry2->getTexCoordArray(0));
    
        geode->addDrawable(geometry.get());
        shadowNode->addChild(geode.get());
        osg::StateSet *state = shadowNode->getOrCreateStateSet();
        state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        state->setMode(GL_BLEND,osg::StateAttribute::ON);
        state->setRenderBinDetails(1, "DepthSortedBin");
        osg::CullFace *cull = new osg::CullFace(); 
        cull->setMode(osg::CullFace::BACK); 
        state->setAttributeAndModes(cull, osg::StateAttribute::ON); 
    
        osg::Texture2D* theTexture = new osg::Texture2D;
        theTexture->setDataVariance(osg::Object::DYNAMIC);
        theTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
        theTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
  
        osg::Image *textureImage = osgDB::readImageFile("diffuse.jpg");
        theTexture->setImage(textureImage);
        state->setTextureAttributeAndModes(0, theTexture,
        osg::StateAttribute::ON |
        osg::StateAttribute::PROTECTED);

        / *
        state->setMode(GL_LIGHTING, osg::StateAttribute::ON |
        osg::StateAttribute::PROTECTED);
        state->setMode(GL_FOG, osg::StateAttribute::ON);
        * /

        //scaleTransform->addChild(shadowNode.get());
        theFactory->addShadowObject(shadowNode.get());    
    
        osg::Program* myShaderProgram = new osg::Program;
        osg::Shader* myShader = new osg::Shader(osg::Shader::FRAGMENT,
        myFragmentShaderSource);
        myShaderProgram->addShader(myShader);
        osg::Uniform* textureUniform = new osg::Uniform("textureUniform",(int)0);
        osg::Uniform* shadowUniform = new osg::Uniform("shadowUniform",(int)1);
        state->addUniform(textureUniform);
        state->addUniform(shadowUniform);

        state->setAttributeAndModes(myShaderProgram, osg::StateAttribute::ON);

        }
  
        osg::Node* node3 = shadowNode->getChild(0);
        osg::Geode* geode3;
        osg::Drawable* drawable3;
        osg::Geometry* geometry3;
        osg::DrawElementsUShort* draw;
        osg::Vec3Array* v3;
  
        if(!node3) return;
        geode3 = dynamic_cast<osg::Geode*>(node3);
        if(!geode3) return;
        drawable3 = geode3->getDrawable(0);
        if(!drawable3) return;
        geometry3 = drawable3->asGeometry();
        if(!geometry3) return;
  

        geometry3->removePrimitiveSet(0, geometry3->getNumPrimitiveSets());

        v3 = dynamic_cast<osg::Vec3Array*>(geometry3->getVertexArray());
        if(!v3) return;

        osg::PrimitiveSet* primitiveSet;

        for(unsigned int i=0; i<geometry2->getNumPrimitiveSets(); i++) {
        primitiveSet = geometry2->getPrimitiveSet(i);
        draw = dynamic_cast<osg::DrawElementsUShort*>(primitiveSet);
        if(draw) {
        for(unsigned int k=0; k<draw->getNumIndices(); ++k) {
        if(((*v3)[draw->index(k)][0] < pos.x + 2) &&
        ((*v3)[draw->index(k)][0] > pos.x -2)) {

        if(((*v3)[draw->index(k)][1] < pos.y + 2) &&
        ((*v3)[draw->index(k)][1] > pos.y -2)) {
        if(((*v3)[draw->index(k)][2] < pos.z + 2) &&
        ((*v3)[draw->index(k)][2] > pos.z -2)) {
        geometry3->addPrimitiveSet(primitiveSet);
        break;
        }
        }
        }       
        }
        }
        }
        geometry3->dirtyDisplayList();
        geometry3->dirtyBound();
      */
    }

  } // end of namespace graphics
} // end of namespace mars
