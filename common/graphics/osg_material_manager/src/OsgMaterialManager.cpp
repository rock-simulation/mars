/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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
 * \file OsgMaterialManager.cpp
 * \author Malte Langosz (malte.langosz@dfki.de)
 * \brief
 *
 * Version 0.1
 * Todo:
 *        - provide interface (handle) for shadow texture
 *        - provide light management
 *        - provide method for materialnodes (the positioning is the main
 *          problem, the manager needs to know the node position for the light
 *          update, at least for scenes with multiple light sources).
 *          setNodePos can be used from outside (we give the materialnode to
 *          outside, set position can be called from there)
 */

#include "OsgMaterialManager.h"
#include "MaterialNode.h"
#include <osgDB/ReadFile>

namespace osg_material_manager {

  std::vector<OsgMaterialManager::textureFileStruct> OsgMaterialManager::textureFiles;
  std::vector<OsgMaterialManager::imageFileStruct> OsgMaterialManager::imageFiles;

  OsgMaterialManager::OsgMaterialManager(lib_manager::LibManager *theManager) :
    lib_manager::LibInterface(theManager) {
    mainStateGroup = new osg::Group();
    
    cfg = theManager->getLibraryAs<mars::cfg_manager::CFGManagerInterface>("cfg_manager", true);
    resPath.sValue = std::string(MARS_PREFERENCES_DEFAULT_RESOURCES_PATH);
    shadowSamples.iValue = 1;
    if(cfg) {
      resPath = cfg->getOrCreateProperty("Preferences", "resources_path",
                                         resPath.sValue, this);
      shadowSamples = cfg->getOrCreateProperty("Graphics",
                                               "shadowSamples",
                                               shadowSamples.iValue, this);
    }
    noiseImage = new osg::Image();
    noiseImage->allocateImage(128, 128, 4, GL_RGBA, GL_UNSIGNED_BYTE);
    updateShadowSamples();
    useShader = true;
    shadowTextureSize = 2048;
    shadowScale = 1.0;
    useFog = true;
    useNoise = true;
    drawLineLaser = false;
    useShadow = true;
    defaultMaxNumNodeLights = 1;
    brightness = 1.0;
  }

  OsgMaterialManager::~OsgMaterialManager(void) {
    if(cfg) libManager->releaseLibrary("cfg_manager");
    //fprintf(stderr, "Delete osg_material_manager\n");
  }

  osg::ref_ptr<osg::Texture2D> OsgMaterialManager::loadTexture(std::string filename) {
    std::vector<OsgMaterialManager::textureFileStruct>::iterator iter;

    for (iter = textureFiles.begin();
         iter != textureFiles.end(); iter++) {
      if ((*iter).fileName == filename) {
        return (*iter).texture;
      }
    }
    OsgMaterialManager::textureFileStruct newTextureFile;
    newTextureFile.fileName = filename;
    newTextureFile.texture = new osg::Texture2D;
    newTextureFile.texture->setDataVariance(osg::Object::DYNAMIC);
    newTextureFile.texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    newTextureFile.texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
    newTextureFile.texture->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);

    osg::Image* textureImage = loadImage(filename);
    newTextureFile.texture->setImage(textureImage);
    textureFiles.push_back(newTextureFile);

    return newTextureFile.texture;
  }

  osg::ref_ptr<osg::Image> OsgMaterialManager::loadImage(std::string filename) {
    std::vector<OsgMaterialManager::imageFileStruct>::iterator iter;

    for (iter = imageFiles.begin();
         iter != imageFiles.end(); iter++) {
      if ((*iter).fileName == filename) {
        return (*iter).image;
      }
    }
    OsgMaterialManager::imageFileStruct newImageFile;
    newImageFile.fileName = filename;
    osg::Image* image = osgDB::readImageFile(filename);
    newImageFile.image = image;
    imageFiles.push_back(newImageFile);

    return newImageFile.image;
  }

  void OsgMaterialManager::updateShadowSamples() {
    static int count = 0;
    osg::Vec2 v;
    double x1, y1, r1, r2;
    double scale1 = 1./shadowSamples.iValue;
    unsigned char *data = noiseImage->data();
    int sampleX = 0, sampleY = 0;
    double noise = 0.5;
    for(int i=0; i<128; ++i) {
      for(int l=0; l<128; ++l) {
        if(!count) {
          r1 = ((double) rand()/RAND_MAX)*2-1; // -1 to 1
          r2 = ((double) rand()/RAND_MAX)*2-1;
          x1 = scale1*0.5+scale1*sampleX+r1*scale1*0.5*noise;
          y1 = scale1*0.5+scale1*sampleY+r2*scale1*0.5*noise;
          r1 = (sqrt(y1)*cos(6.28*x1)*0.5 + .5)*255;
          r2 = (sqrt(y1)*sin(6.28*x1)*0.5 + .5)*255;
          data[i*128*4+l*4+0] = (unsigned char) r1;
          data[i*128*4+l*4+1] = (unsigned char) r2;
        }
        data[i*128*4+l*4+2] = (unsigned char) (((double)rand()/RAND_MAX)*255);
        data[i*128*4+l*4+3] = (unsigned char) (((double)rand()/RAND_MAX)*255);
        if(++sampleX == shadowSamples.iValue) {
          sampleX = 0;
        }
      }
      if(++sampleY == shadowSamples.iValue) {
        sampleY = 0;
      }
    }
    noiseImage->dirty();
    //count = 1;
  }

  void OsgMaterialManager::cfgUpdateProperty(mars::cfg_manager::cfgPropertyStruct _property) {
    if(_property.paramId == shadowSamples.paramId) {
      setShadowSamples(_property.iValue);
      return;
    }
    if(_property.paramId == resPath.paramId) {
      resPath.sValue = _property.sValue;
      return;
    }
  }

  void OsgMaterialManager::setShadowSamples(int v) {
    std::map<std::string, osg::ref_ptr<OsgMaterial> >::iterator it;
    shadowSamples.iValue = v;
    for(it=materialMap.begin(); it!=materialMap.end(); ++it) {
      it->second->setShadowSamples(v);
    }
  }

  void OsgMaterialManager::createMaterial(const std::string &name,
                                          const configmaps::ConfigMap &map) {
    std::map<std::string, osg::ref_ptr<OsgMaterial> >::iterator it;
    it = materialMap.find(name);
    if(it == materialMap.end()) {
      OsgMaterial *m = new OsgMaterial(resPath.sValue+"/mars/osg_material_manager/resources");
      m->setMaxNumLights(defaultMaxNumNodeLights);
      m->setShadowTextureSize(shadowTextureSize);
      m->setMaterial(map);
      m->setUseShader(useShader);
      m->setNoiseImage(noiseImage.get());
      m->setShadowSamples(shadowSamples.iValue);
      materialMap[name] = m;
      mainStateGroup->addChild(m);
    }
  }

  void OsgMaterialManager::setMaterial(const std::string &name,
                                       const configmaps::ConfigMap &map) {
    std::map<std::string, osg::ref_ptr<OsgMaterial> >::iterator it;
    it = materialMap.find(name);
    if(it != materialMap.end()) {
      it->second->setMaterial(map);
    }
  }

  void OsgMaterialManager::editMaterial(const std::string &name,
                                        const std::string &key,
                                        const std::string &value) {
    std::map<std::string, osg::ref_ptr<OsgMaterial> >::iterator it;
    it = materialMap.find(name);
    if(it != materialMap.end()) {
      it->second->edit(key, value);
    }
  }

  osg::ref_ptr<MaterialNode> OsgMaterialManager::getNewMaterialGroup(const std::string &name) {
    std::map<std::string, osg::ref_ptr<OsgMaterial> >::iterator it;
    it = materialMap.find(name);
    if(it != materialMap.end()) {
      MaterialNode *n = new MaterialNode();
      n->setMaxNumLights(defaultMaxNumNodeLights);
      n->createNodeState();
      n->setUseFog(useFog);
      n->setUseNoise(useNoise);
      n->setDrawLineLaser(drawLineLaser);
      n->setUseShadow(useShadow);
      n->setBrightness(brightness);
      it->second->addMaterialNode(n);
      it->second->addChild(n);
      materialNodes.push_back(n);
      return n;
    }
    else {
      return NULL;
    }
  }

  void OsgMaterialManager::removeMaterialGroup(osg::ref_ptr<osg::Group> group) {
    // todo
  }

  void OsgMaterialManager::updateLights(std::vector<mars::interfaces::LightData*> &lightList) {
    {
      std::vector<osg::ref_ptr<MaterialNode> >::iterator it = materialNodes.begin();
      for(; it!=materialNodes.end(); ++it) {
        (*it)->updateLights(lightList);
      }
    }

    {
      std::map<std::string, osg::ref_ptr<OsgMaterial> >::iterator it;
      for(it=materialMap.begin(); it!=materialMap.end(); ++it) {
        it->second->update();
      }
    }
  }

  void OsgMaterialManager::setUseShader(bool v) {
    fprintf(stderr, "set use shader: %d %d\n", useShader, v);
    useShader = v;
    std::map<std::string, osg::ref_ptr<OsgMaterial> >::iterator it = materialMap.begin();
    for(; it!=materialMap.end(); ++it) {
      it->second->setUseShader(useShader);
    }
  }

  void OsgMaterialManager::setShadowTextureSize(int size) {
    shadowTextureSize = size;
    std::map<std::string, osg::ref_ptr<OsgMaterial> >::iterator it = materialMap.begin();
    for(; it!=materialMap.end(); ++it) {
      it->second->setShadowTextureSize(size);
    }
  }

  void OsgMaterialManager::setShadowScale(float v) {
    shadowScale = v;
    std::map<std::string, osg::ref_ptr<OsgMaterial> >::iterator it = materialMap.begin();
    for(; it!=materialMap.end(); ++it) {
      it->second->setShadowScale(shadowScale);
    }
  }

  void OsgMaterialManager::setDefaultMaxNumLights(int v) {
    defaultMaxNumNodeLights = v;
  }

  void OsgMaterialManager::setUseFog(bool v) {
    useFog = v;
    std::vector<osg::ref_ptr<MaterialNode> >::iterator it = materialNodes.begin();
    for(; it!=materialNodes.end(); ++it) {
      (*it)->setUseFog(useFog);
    }    
  }

  void OsgMaterialManager::setUseNoise(bool v) {
    useNoise = v;
    std::vector<osg::ref_ptr<MaterialNode> >::iterator it = materialNodes.begin();
    for(; it!=materialNodes.end(); ++it) {
      (*it)->setUseNoise(useNoise);
    }    
  }
  
  void OsgMaterialManager::setDrawLineLaser(bool v) {
    drawLineLaser = v;
    std::vector<osg::ref_ptr<MaterialNode> >::iterator it = materialNodes.begin();
    for(; it!=materialNodes.end(); ++it) {
      (*it)->setDrawLineLaser(drawLineLaser);
    }    
  }
  
  void OsgMaterialManager::setUseShadow(bool v) {
    useShadow = v;
    std::vector<osg::ref_ptr<MaterialNode> >::iterator it = materialNodes.begin();
    for(; it!=materialNodes.end(); ++it) {
      (*it)->setUseShadow(useShadow);
    }    
  }

  void OsgMaterialManager::setBrightness(float v) {
    brightness = v;
    std::vector<osg::ref_ptr<MaterialNode> >::iterator it = materialNodes.begin();
    for(; it!=materialNodes.end(); ++it) {
      (*it)->setBrightness(brightness);
    }    
  }

  std::vector<configmaps::ConfigMap> OsgMaterialManager::getMaterialList() {
    std::vector<configmaps::ConfigMap> list;
    std::map<std::string, osg::ref_ptr<OsgMaterial> >::iterator it = materialMap.begin();
    for(; it!=materialMap.end(); ++it) {
      list.push_back(it->second->getMaterialData());
    }
    return list;
  }

  void OsgMaterialManager::setExperimentalLineLaser(mars::utils::Vector pos,
                                                    mars::utils::Vector normal,
                                                    mars::utils::Vector color,
                                                    mars::utils::Vector laserAngle,
                                                    float openingAngle) {
    std::vector<osg::ref_ptr<MaterialNode> >::iterator it = materialNodes.begin();
    for(; it!=materialNodes.end(); ++it) {
      (*it)->setExperimentalLineLaser(pos, normal, color,
                                      laserAngle, openingAngle);
    }    
  }
  
} // end of namespace: osg_material_manager

DESTROY_LIB(osg_material_manager::OsgMaterialManager);
CREATE_LIB(osg_material_manager::OsgMaterialManager);
