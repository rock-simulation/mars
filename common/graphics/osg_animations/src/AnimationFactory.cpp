/**
 * \file AnimationFactory.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#include "AnimationFactory.hpp"
#include "AnimationP.hpp"

namespace osg_animation {

  AnimationFactory::AnimationFactory() {

    // load animation resource
    // std::stringstream ss;
    // ss << SHARE_DIR;
    // ss << "/resources/animation.obj";
    // fprintf(stderr, "osg_animation: load %s\n", ss.str().c_str());
    // AnimationP::animationNode = osgDB::readNodeFile(ss.str());
  }

  AnimationFactory::~AnimationFactory(void) {
  }

  Animation* AnimationFactory::createAnimation(void) {
    AnimationP *animation = new AnimationP();
    return animation;
  }

} // end of namespace: osg_animation
