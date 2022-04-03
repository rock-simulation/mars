/**
 * \file Animation.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#include "AnimationP.hpp"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <mars/utils/misc.h>

osg::ref_ptr<osg::MatrixTransform> scaleTransform;

namespace osg_animation {

  void writeInt(int value, unsigned char *buffer) {
    double s256 = 1./256;
    int v = floor(value*s256);
    if(v < 0) v = 0;
    if(v>255) v = 255;
    buffer[0] = (unsigned char)v;
    v = (int)value % 256;
    if(v < 0) v = 0;
    if(v>255) v = 255;
    buffer[1] = (char)v;
  }

  void writeFloat(float value, unsigned char *buffer) {
    value *= 65535;
    writeInt(round(value), buffer);
  }

  AnimationP::AnimationP() {
    framesFactory = new osg_frames::FramesFactory();
    graphics = NULL;
    animation_index = 0;
    current_animation = "";
  }

  AnimationP::~AnimationP(void) {
  }

  void AnimationP::parseBone(Bone *bone, configmaps::ConfigMap &map, std::string name) {
    bone->name = name;
    bone->init_pos.x() = map["pos"][0];
    bone->init_pos.y() = map["pos"][1];
    bone->init_pos.z() = map["pos"][2];
    bone->init_q.w() = map["rot"][0];
    bone->init_q.x() = map["rot"][1];
    bone->init_q.y() = map["rot"][2];
    bone->init_q.z() = map["rot"][3];
    bone->r_pos = bone->init_pos;
    bone->r_q = bone->init_q;
    bone->q.w() = 1.0;
    bone->q.x() = bone->q.y() = bone->q.z() = 0.0;
    // for absolute positioning of bones
    bone->q = bone->init_q;
    bone->pos = bone->init_pos;
    bone->index = map["index"];
    bone->frame = framesFactory->createFrame();
    bone->frame->setPosition(bone->init_pos.x(), bone->init_pos.y(), bone->init_pos.z());
    bone->frame->setRotation(bone->init_q.x(), bone->init_q.y(), bone->init_q.z(), bone->init_q.w());
    bone->frame->setScale(0.2);
    if(graphics) {
      //graphics->addOSGNode(bone->frame->getOSGNode());
    }
  }

  void AnimationP::addChildren(Bone *parent, configmaps::ConfigMap &map, configmaps::ConfigMap &bones) {
    for(auto it:map["children"]) {
      struct Bone *child = new Bone;
      configmaps::ConfigMap &b = bones[it];
      parseBone(child, b, it);
      osg::Quat invQ = parent->init_q;
      invQ.x() *= -1;
      invQ.y() *= -1;
      invQ.z() *= -1;
      child->r_q = child->init_q*invQ;
      child->r_pos = invQ*(child->init_pos-parent->init_pos);
      addChildren(child, b, bones);
      parent->children.push_back(child);
    }
    for(std::vector<Bone*>::iterator it = parent->children.begin(); it != parent->children.end(); ++it) {
      boneMap[(*it)->name] = *it;
    }
  }

  void AnimationP::loadBones(configmaps::ConfigMap &map) {
    std::string root = "";
    // search for root:
    for(auto it:map) {
      if(it.second["parent"] == "") {
        root = it.first;
        break;
      }
    }
    if(root != "") {
      configmaps::ConfigMap &b = map[root];
      armature = new Bone;
      parseBone(armature, b, root);
      boneMap[root] = armature;
      addChildren(armature, b, map);
    }
    else {
      fprintf(stderr, "Error: no root found in bones definition!\n");
    }
  }

  void AnimationP::loadBonesFile(std::string file) {
    fprintf(stderr, "load bones file: %s\n", file.c_str());
    file = mars::utils::pathJoin(load_path, file);
    configmaps::ConfigMap map = configmaps::ConfigMap::fromYamlFile(file);
    loadBones(map);
  }

  void AnimationP::loadBonesString(std::string str) {
    configmaps::ConfigMap map = configmaps::ConfigMap::fromYamlString(str);
    loadBones(map);
  }

  void AnimationP::loadAnimation(std::string name, std::string file) {
    file = mars::utils::pathJoin(load_path, file);
    animations[name] = configmaps::ConfigMap::fromYamlFile(file);
    //current_animation = name;
  }

  void AnimationP::playAnimation(std::string name, int repeat) {
    this->repeat = repeat;
    if(animations.hasKey(name)) {
      current_animation = name;
      animation_index = 0;
      loop = 0;
      next_update_time = 0;
    }
  }

  Bone* AnimationP::getBone(std::string name, Bone *b) {
    Bone *r=NULL;
    if(b->name == name) return b;
    for(auto c: b->children) {
      r = getBone(name, c);
      if(r) return r;
    }
    return r;
  }

  // rotate bone by value
  void AnimationP::rotateBone(std::string name, double x, double y, double z, double angle) {
    Bone *bone = getBone(name, armature);
    if(bone) {
      bone->q *= osg::Quat(angle, osg::Vec3d(x, y, z));
    }
  }

  // set rotation to initial*new
  void AnimationP::setBoneRotation(std::string name, double x, double y, double z, double w) {
    Bone *bone = getBone(name, armature);
    if(bone) {
      bone->q = bone->init_q * osg::Quat(x, y, z, w);
    }
  }

  // translate bone by value
  void AnimationP::translateBone(std::string name, double x, double y, double z) {
    Bone *bone = getBone(name, armature);
    if(bone) {
      bone->pos += osg::Vec3d(x, y, z);
    }
  }

  // set rotation to initial*new
  void AnimationP::setBoneRotation(std::string name, double x, double y, double z) {
    Bone *bone = getBone(name, armature);
    if(bone) {
      bone->pos = bone->init_pos + osg::Vec3d(x, y, z);
    }
  }

  void AnimationP::setMaterialManager(osg_material_manager::OsgMaterialManager *materialManager) {
    this->materialManager = materialManager;
  }

  void AnimationP::setMatrixTexture(std::string material_name, std::string texture_name) {
    osg::ref_ptr<osg_material_manager::OsgMaterial> material = materialManager->getOsgMaterial(material_name);
    if(material.valid()) {
      matrixTexture = material->getTexture(texture_name);
      if(!matrixTexture.valid()) {
        fprintf(stderr, "Error: getting matrixTexture: %s\n", texture_name.c_str());
      }
      else {
        weightTexture = material->getTexture("vertex_weights");
      }
    }
    else {
      fprintf(stderr, "Error: getting material: %s\n", material_name.c_str());
    }
  }

  void AnimationP::updateBone(Bone *bone, osg::Vec3d &parent_pos, osg::Quat &parent_q) {
    osg::Vec3d pos = parent_pos + parent_q*bone->r_pos;
    osg::Quat q = bone->r_q*parent_q;
    pos += q*bone->pos;
    q = q*bone->q;
    // for absolute positioning of bones
    pos = bone->pos;
    q = bone->q;

    bone->frame->setPosition(pos.x(), pos.y(), pos.z());
    bone->frame->setRotation(q.x(), q.y(), q.z(), q.w());

    // calculate the diff of init to current:
    //osg::Vec3d dPos = pos-bone->init_pos;
    osg::Quat invQ = bone->init_q;
    invQ.x() *= -1;
    invQ.y() *= -1;
    invQ.z() *= -1;
    osg::Quat dQ = q*invQ;
    //dQ.x() = dQ.y() = dQ.z() = 0.0;
    //dQ.w() = 1.0;
    osg::Matrixd m1;
    m1.makeTranslate(-bone->init_pos);
    osg::Matrixd m2(invQ);
    osg::Matrixd m3(q);
    osg::Matrixd m33(bone->q);
    osg::Matrixd m4;
    m4.makeTranslate(pos);

    invQ.x() = -q.x();
    invQ.y() = -q.y();
    invQ.z() = -q.z();
    invQ.w() = q.w();
    osg::Matrixd m5(invQ);

    osg::Matrixd m = m1*m2*m3*m4;
    // if(bone->name == "thigh_l") { //"ball_l") {
    //   fprintf(stderr, "q: %g %g %g %g\n", dQ.x(), dQ.y(), dQ.z(), dQ.w());
    //   for(int i=0; i<4; ++i) {

    //     fprintf(stderr, "    %g %g %g %g\n", m(0, i), m(1, i), m(2, i), m(3, i));
    //   }
    // }

    // write matrix into texture at index bone->index
    osg::ref_ptr<osg::Image> i = matrixTexture->getImage();
    int size = i->s()*i->t();
    int matrix_buffer_size = 16*2; // two bytes per value
    int offset = bone->index*matrix_buffer_size;
    if(size <= offset+matrix_buffer_size) {
      fprintf(stderr, "Error: matrix texture to small for bones.\n");
      return;
    }
    unsigned char *data = i->data();
    // should check the image dimension
    float v;
    for(int c=0; c<4; ++c) {
      for(int r=0; r<4; ++r) {
        v = (m(c,r)+2.0)/4.0;
        writeFloat(v, data+offset+(c*4+r)*2);
        // if(bone->name == "ball_l") {
        //   fprintf(stderr, " %d", offset+(c*4+r)*2);
        // }
      }
      // if(bone->name == "ball_l") {
      //   fprintf(stderr, "\n");
      // }
    }

    for(auto it:bone->children) {
      updateBone(it, pos, q);
    }
  }

  void AnimationP::updatePose() {
    static long numFile = 0;
    static double t = 0.0;
    static bool init_vertex_weights = true;
    bool refresh = false;

    if(matrixTexture.valid()) {
      if(init_vertex_weights) {
        refresh = true;
        init_vertex_weights = false;
        std::string file = mars::utils::pathJoin(load_path, "vertex_weights.json");
        configmaps::ConfigMap map = configmaps::ConfigMap::fromYamlFile(file);
        unsigned long i = 0;
        int index;
        double weight;
        int offset;
        osg::ref_ptr<osg::Image> image = weightTexture->getImage();
        // todo: changing the pixel format does not always work have to check that
        image->setPixelFormat(GL_RGBA);
        unsigned char *data = image->data();
        for(auto it: map["weights"]) {
          offset = i*8;
          index = it[0][0];
          weight = it[0][1];
          writeInt(index, data+offset);
          writeFloat(weight, data+offset+2);
          index = it[1][0];
          weight = it[1][1];
          writeInt(index, data+offset+4);
          writeFloat(weight, data+offset+6);
          ++i;
        }
        image->dirty();
        //osgDB::writeImageFile(*(image.get()), "weights.png");
      }
      // check if we should update
      if(current_animation != "") {
        unsigned long t = mars::utils::getTime();
        if(next_update_time == 0) {
          next_update_time = t+40;
          refresh = true;
        }
        if(t >= next_update_time) {
          refresh = true;
          next_update_time += 40;
          animation_index += 1;
          if(animation_index >= animations[current_animation][armature->name].size()) {
            if(repeat == -1 or loop < repeat) {
              animation_index = 0;
              loop += 1;
            }
            else {
              animation_index -= 1;
              current_animation = "";
              refresh = false;
            }
          }
        }
        // apply bone state
        if(refresh) {
          //fprintf(stderr, "animation_index: %lu of %s\n", animation_index, current_animation.c_str());
          for(auto it:(configmaps::ConfigMap)animations[current_animation]) {
            Bone *b = boneMap[it.first];
            //fprintf(stderr, "bone: %s\n", it.first.c_str());
            boneMap[it.first]->pos.x() = it.second[animation_index][0][0];
            boneMap[it.first]->pos.y() = it.second[animation_index][0][1];
            boneMap[it.first]->pos.z() = it.second[animation_index][0][2];
            boneMap[it.first]->q.x() = it.second[animation_index][1][1];
            boneMap[it.first]->q.y() = it.second[animation_index][1][2];
            boneMap[it.first]->q.z() = it.second[animation_index][1][3];
            boneMap[it.first]->q.w() = it.second[animation_index][1][0];
          }
        }
        // unsigned char *data = i->data();
        // float f;
        // double s = 1./255;
        // int offset;
      }

      if(refresh) {
        osg::Vec3d v;
        osg::Quat q(0.0, 0.0, 0.0, 1.0);
        osg::ref_ptr<osg::Image> i = matrixTexture->getImage();
        // todo: changing the pixel format does not always work have to check that
        i->setPixelFormat(GL_RGBA);
        updateBone(armature, v, q);
        i->dirty();
      }
      // generate some movement at the pelvis
      //osg::Quat tq(0.2+sin(t)*0.2, osg::Vec3d(0.0, 0.0, 1.0));
      // //boneMap["Root"]->q = tq;
      //boneMap["upperarm_l"]->q = tq;
      //boneMap["upperarm_r"]->q = tq;
      //fprintf(stderr, "index: %d\n", boneMap["thigh_l"]->index);
      //boneMap["pelvis"]->q = tq;
      // iterate over bones
      // osg::Vec3d v;
      // osg::Quat q(0.0, 0.0, 0.0, 1.0);
      // osg::ref_ptr<osg::Image> i = matrixTexture->getImage();
      // // todo: changing the pixel format does not always work have to check that
      // i->setPixelFormat(GL_RGBA);
      // updateBone(armature, v, q);
      // unsigned char *data = i->data();
      // float f;
      // double s = 1./255;
      // int offset;
      // fprintf(stderr, "debug root bone\n");
      // for(int c=0; c<4; ++c) {
      //   for(int r=0; r<4; ++r) {
      //     offset = c*4*2+r*2;
      //     f = data[offset]*s * 0.996108955 + data[offset+1]*s*0.003891045;
      //     fprintf(stderr, " %f", f);
      //   }
      //   fprintf(stderr, "\n");
      // }
      // char filename[55];
      // sprintf(filename, "test_%05d.png", numFile);
      // osgDB::writeImageFile(*(i.get()), filename);
    }
    else {
      //fprintf(stderr, "Error: no matrix texture set!\n");
    }
    t += 0.03;
    if(t>6.28) t-=6.28;
  }

  void AnimationP::printBone(Bone *bone, osg::Vec3d &parent_pos, osg::Quat &parent_q, int depth) {
    osg::Vec3d pos = parent_pos + parent_q*bone->init_pos;
    osg::Quat q = bone->q * parent_q;
    pos += q*bone->pos;
    for(int i=0; i<depth; ++i) fprintf(stderr, "  ");
    fprintf(stderr, "%s: [%g, %g, %g]\n", bone->name.c_str(), pos.x(), pos.y(), pos.z());
    for(auto it:bone->children) {
      printBone(it, pos, q, depth+1);
    }
  }

  void AnimationP::printBones() {
    return;
    osg::Vec3d v;
    osg::Quat q(1.0, 0.0, 0.0, 0.0);
    printBone(armature, v, q, 0);
  }

  void AnimationP::setGraphics(mars::interfaces::GraphicsManagerInterface *g) {
    graphics = g;
  }

  void AnimationP::setName(std::string name) {
    this->name = name;
  }

  std::string AnimationP::getName() {
    return name;
  }

  void AnimationP::setLoadPath(std::string path) {
    load_path = path;
  }

} // end of namespace: osg_animation
