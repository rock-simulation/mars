/**
 * \file FramesFactory.hpp
 * \author Malte Langosz
 * \brief
 **/

#ifndef OSG_ANIMATION_FACTORY_HPP
#define OSG_ANIMATION_FACTORY_HPP

#ifdef _PRINT_HEADER_
#warning "AnimationFactory.hpp"
#endif

#include "Animation.hpp"

namespace osg_animation {

  class AnimationP;

  class AnimationFactory {

  public:
    AnimationFactory();
    ~AnimationFactory();

    Animation* createAnimation(void);
  };

} // end of namespace: osg_animation

#endif // OSG_ANIMATION_FACTORY_H
