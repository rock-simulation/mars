/**
 * \file AnimationP.hpp
 * \author Malte Langosz
 * \brief
 **/

#ifndef OSG_ANIMATION_P_HPP
#define OSG_ANIMATION_P_HPP

#ifdef _PRINT_HEADER_
#warning "AnimationP.hpp"
#endif

#include "Animation.hpp"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Node>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg_frames/FramesFactory.hpp>

namespace osg_animation {

  typedef struct Bone {
    std::string name;
    osg::Vec3d init_pos, r_pos, pos;
    osg::Quat init_q, r_q, q;
    std::vector<Bone*> children;
    int index;
    osg_frames::Frame *frame;
  }Bone;

  class AnimationP : public Animation {

  public:
    AnimationP();
    ~AnimationP();

    virtual void loadBonesFile(std::string file);
    virtual void loadBonesString(std::string str);
    virtual void loadAnimation(std::string name, std::string file);
    virtual void playAnimation(std::string name, int repeat=-1, double speed_scale=1.0);

    // rotate bone by value
    virtual void rotateBone(std::string name, double x, double y, double z, double angle);

    // set rotation to initial*new
    virtual void setBoneRotation(std::string name, double x, double y, double z, double w);

    // translate bone by value
    virtual void translateBone(std::string bone, double x, double y, double z);

    // set rotation to initial*new
    virtual void setBoneRotation(std::string name, double x, double y, double z);

    virtual void setMaterialManager(osg_material_manager::OsgMaterialManager *materialManager);
    virtual void setMatrixTexture(std::string material_name, std::string texture_name);
    virtual void updatePose();
    virtual void getPose(const std::string &name, double *x, double *y, double *z, double *qx, double *qy, double *qz, double *qw);
    virtual void printBones();
    virtual void setGraphics(mars::interfaces::GraphicsManagerInterface *g);
    virtual void setName(std::string name);
    virtual std::string getName();
    virtual bool hasBone(const std::string &name);
    virtual void setLoadPath(std::string path);

  private:
    osg_material_manager::OsgMaterialManager *materialManager;
    mars::interfaces::GraphicsManagerInterface *graphics;

    Bone *armature;
    std::map<std::string, Bone*> boneMap;
    int pixel_offset; // used if we want to store the matrix information behind the vertex weights
    osg::ref_ptr<osg::Texture2D> matrixTexture, weightTexture;
    osg_frames::FramesFactory *framesFactory;
    configmaps::ConfigMap animations;
    size_t animation_index, loop;
    int repeat;
    double speed_scale;
    std::string current_animation, name, load_path;
    unsigned long next_update_time;

    void updateBone(Bone *bone, osg::Vec3d &parent_pos, osg::Quat &parent_q);
    void parseBone(Bone *bone, configmaps::ConfigMap &map, std::string name);
    Bone* getBone(std::string name, Bone *b);
    void addChildren(Bone *parent, configmaps::ConfigMap &map, configmaps::ConfigMap &bones);
    void loadBones(configmaps::ConfigMap &map);
    void printBone(Bone *bone, osg::Vec3d &parent_pos, osg::Quat &parent_q, int depth);
  };

} // end of namespace: osg_animation

#endif // OSG_ANIMATION_P_HPP
