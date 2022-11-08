/**
 * \file Animation.hpp
 * \author Malte Langosz
 * \brief
 **/

#ifndef OSG_ANIMATION_HPP
#define OSG_ANIMATION_HPP

#ifdef _PRINT_HEADER_
#warning "Animation.hpp"
#endif

#include <mars/osg_material_manager/OsgMaterialManager.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

namespace osg_animation {

  class Animation {

  public:
    Animation() {}
    virtual ~Animation() {}

    virtual void loadBonesFile(std::string file) = 0;
    virtual void loadBonesString(std::string str) = 0;
    virtual void loadAnimation(std::string name, std::string file) = 0;
    virtual void playAnimation(std::string name, int repeat=-1, double speed_scale=1.0) = 0;
    // rotate bone by value
    virtual void rotateBone(std::string name, double x, double y, double z, double angle) = 0;
    // set rotation to initial*new
    virtual void setBoneRotation(std::string name, double x, double y, double z, double w) = 0;
    // translate bone by value
    virtual void translateBone(std::string bone, double x, double y, double z) = 0;
    // set rotation to initial*new
    virtual void setBoneRotation(std::string name, double x, double y, double z) = 0;
    virtual void setMaterialManager(osg_material_manager::OsgMaterialManager *materialManager) = 0;
    virtual void setMatrixTexture(std::string material_name, std::string texture_name) = 0;
    // todo: add method to set pixel offset
    virtual void updatePose() = 0;
    virtual void getPose(const std::string &name, double *x, double *y, double *z, double *qx, double *qy, double *qz, double *qw) = 0;
    virtual void printBones() = 0;
    virtual void setGraphics(mars::interfaces::GraphicsManagerInterface *g) = 0;
    virtual void setName(std::string name) = 0;
    virtual std::string getName() = 0;
    virtual bool hasBone(const std::string &name) = 0;
    virtual void setLoadPath(std::string path) = 0;
  };

} // end of namespace: osg_animation

#endif // OSG_ANIMATION_HPP
