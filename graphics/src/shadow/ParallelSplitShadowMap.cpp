/**
 * \file ParallelSplitShadowMap.cpp
 * \author Malte Langosz
 * \brief The ParallelSplitShadowMap is a clone of the original osgShadow::ParallelSplitShadowMap
 *        with some adaptations for the MARS simulation framework.
 */


#include "ParallelSplitShadowMap.h"

#include <osgShadow/ShadowedScene>
#include <osg/Notify>
#include <osg/ComputeBoundsVisitor>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/io_utils>
#include <iostream>
#include <sstream>
#include <osg/Geode>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osg/Texture1D>
#include <osg/Depth>
#include <osg/ShadeModel>
#include <osg/LightSource>

using namespace osgShadow;

// split scheme
#define TEXTURE_RESOLUTION  1024




#define ZNEAR_MIN_FROM_LIGHT_SOURCE 5.0
#define MOVE_VIRTUAL_CAMERA_BEHIND_REAL_CAMERA_FACTOR 5.0

//#define SHOW_SHADOW_TEXTURE_DEBUG    // DEPTH instead of color for debug information texture display in a rectangle
//#define SHADOW_TEXTURE_DEBUG         // COLOR instead of DEPTH

#ifndef SHADOW_TEXTURE_DEBUG
#define SHADOW_TEXTURE_GLSL
#endif

namespace mars {
  namespace graphics {


    //////////////////////////////////////////////////////////////////////////
    // clamp variables of any type
    template<class Type> inline Type Clamp(Type A, Type Min, Type Max) {
      if(A<Min) return Min;
      if(A>Max) return Max;
      return A;
    }

#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define max(a,b)            (((a) > (b)) ? (a) : (b))

    //////////////////////////////////////////////////////////////////////////
    ParallelSplitShadowMap::ParallelSplitShadowMap(osg::Geode** gr, int icountplanes) :
      _textureUnitOffset(1),
      _debug_color_in_GLSL(false),
      _user_polgyonOffset_set(false),
      _resolution(TEXTURE_RESOLUTION),
      _setMaxFarDistance(10000.0),
      _isSetMaxFarDistance(false),
      _split_min_near_dist(ZNEAR_MIN_FROM_LIGHT_SOURCE),
      _move_vcam_behind_rcam_factor(MOVE_VIRTUAL_CAMERA_BEHIND_REAL_CAMERA_FACTOR),
      _userLight(NULL),
      _GLSL_shadow_filtered(false),
      _ambientBiasUniform(NULL),
      _ambientBias(0.1f,0.3f),
      isInit(false),
      haveLines(true)
    {
    _displayTexturesGroupingNode = gr;
    _number_of_splits = icountplanes;

    _polgyonOffset.set(0.0f,0.0f);
    setSplitCalculationMode(SPLIT_EXP);
    osg_lines::LinesFactory lF;
    //l = lF.createLines();
    //l->setColor(osg_lines::Color(0.0, 1.0, 0.0, 1.0));
    //l->setLineWidth(8);
    initIntern();
    }

    ParallelSplitShadowMap::ParallelSplitShadowMap(const ParallelSplitShadowMap& copy, const osg::CopyOp& copyop):
      ShadowTechnique(copy,copyop),
      _displayTexturesGroupingNode(0),
      _textureUnitOffset(copy._textureUnitOffset),
      _number_of_splits(copy._number_of_splits),
      _debug_color_in_GLSL(copy._debug_color_in_GLSL),
      _polgyonOffset(copy._polgyonOffset),
      _user_polgyonOffset_set(copy._user_polgyonOffset_set),
      _resolution(copy._resolution),
      _setMaxFarDistance(copy._setMaxFarDistance),
      _isSetMaxFarDistance(copy._isSetMaxFarDistance),
      _split_min_near_dist(copy._split_min_near_dist),
      _move_vcam_behind_rcam_factor(copy._move_vcam_behind_rcam_factor),
      _userLight(copy._userLight),
      _GLSL_shadow_filtered(copy._GLSL_shadow_filtered),
      _SplitCalcMode(copy._SplitCalcMode),
      _ambientBiasUniform(NULL),
      _ambientBias(copy._ambientBias),
      isInit(false),
      haveLines(true)
    {
      osg_lines::LinesFactory lF;
      //l = lF.createLines();
    }

    void ParallelSplitShadowMap::resizeGLObjectBuffers(unsigned int maxSize)
    {
      // for(PSSMShadowSplitTextureMap::iterator itr = _PSSMShadowSplitTextureMap.begin();
      //     itr != _PSSMShadowSplitTextureMap.end();
      //     ++itr)
      //   {
      //     itr->second.resizeGLObjectBuffers(maxSize);
      //   }
    }

    void ParallelSplitShadowMap::releaseGLObjects(osg::State* state) const
    {
      // for(PSSMShadowSplitTextureMap::const_iterator itr = _PSSMShadowSplitTextureMap.begin();
      //     itr != _PSSMShadowSplitTextureMap.end();
      //     ++itr)
      //   {
      //     itr->second.releaseGLObjects(state);
      //   }
    }


    void ParallelSplitShadowMap::PSSMShadowSplitTexture::resizeGLObjectBuffers(unsigned int maxSize)
    {
      // osg::resizeGLObjectBuffers(_camera, maxSize);
      // osg::resizeGLObjectBuffers(_texture, maxSize);
      // osg::resizeGLObjectBuffers(_stateset, maxSize);
      // osg::resizeGLObjectBuffers(_debug_camera, maxSize);
      // osg::resizeGLObjectBuffers(_debug_texture, maxSize);
      // osg::resizeGLObjectBuffers(_debug_stateset, maxSize);
    }

    void ParallelSplitShadowMap::PSSMShadowSplitTexture::releaseGLObjects(osg::State* state) const
    {
      // osg::releaseGLObjects(_camera, state);
      // osg::releaseGLObjects(_texture, state);
      // osg::releaseGLObjects(_stateset, state);
      // osg::releaseGLObjects(_debug_camera, state);
      // osg::releaseGLObjects(_debug_texture, state);
      // osg::releaseGLObjects(_debug_stateset, state);
    }

    void ParallelSplitShadowMap::setLight(osg::Light *l) {
      light = l;
    }


    void ParallelSplitShadowMap::setLight(osg::LightSource *l) {
      ls = l;
      light = ls->getLight();
    }

    void ParallelSplitShadowMap::setAmbientBias(const osg::Vec2& ambientBias)
    {
      _ambientBias = ambientBias;
      if (_ambientBiasUniform ) _ambientBiasUniform->set(osg::Vec2f(_ambientBias.x(), _ambientBias.y()));
    }

    void ParallelSplitShadowMap::init()
    {
      if (!_shadowedScene) return;
      if (!isInit) initIntern();
      //_shadowedScene->addChild((osg::Node*)(l->getOSGNode()));
    }

    void ParallelSplitShadowMap::initIntern()
    {

      OSG_DEBUG << "------ init PSSM " << std::endl;
      
      sharedStateSet = new osg::StateSet;
      sharedStateSet->setDataVariance(osg::Object::DYNAMIC);

      unsigned int iCamerasMax=_number_of_splits;
      for (unsigned int iCameras=0;iCameras<iCamerasMax;iCameras++)
        {
          PSSMShadowSplitTexture pssmShadowSplitTexture;
          pssmShadowSplitTexture._splitID = iCameras;
          pssmShadowSplitTexture._textureUnit = iCameras+_textureUnitOffset;

          pssmShadowSplitTexture._resolution = _resolution;

          OSG_DEBUG << "ParallelSplitShadowMap : Texture ID=" << iCameras << " Resolution=" << pssmShadowSplitTexture._resolution << std::endl;
          // set up the texture to render into
          {
            pssmShadowSplitTexture._texture = new osg::Texture2D;
            pssmShadowSplitTexture._texture->setTextureSize(pssmShadowSplitTexture._resolution, pssmShadowSplitTexture._resolution);
#ifndef SHADOW_TEXTURE_DEBUG
            pssmShadowSplitTexture._texture->setInternalFormat(GL_DEPTH_COMPONENT);
            pssmShadowSplitTexture._texture->setShadowComparison(true);
            pssmShadowSplitTexture._texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
#else
            pssmShadowSplitTexture._texture->setInternalFormat(GL_RGBA);
#endif
            pssmShadowSplitTexture._texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
            pssmShadowSplitTexture._texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
            pssmShadowSplitTexture._texture->setBorderColor(osg::Vec4(1.0,1.0,1.0,1.0));
            pssmShadowSplitTexture._texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
            pssmShadowSplitTexture._texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
          }
          // set up the render to texture camera.
          {
            // create the camera
            pssmShadowSplitTexture._camera = new osg::Camera;
            //pssmShadowSplitTexture._camera->setReadBuffer(GL_BACK);
            //pssmShadowSplitTexture._camera->setDrawBuffer(GL_BACK);
            pssmShadowSplitTexture._camera->setCullCallback(new CameraCullCallback(this));


#ifndef SHADOW_TEXTURE_DEBUG
            pssmShadowSplitTexture._camera->setClearMask(GL_DEPTH_BUFFER_BIT);
            pssmShadowSplitTexture._camera->setClearColor(osg::Vec4(1.0,1.0,1.0,1.0));
#else
            pssmShadowSplitTexture._camera->setClearMask(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
            switch(iCameras)
              {
              case 0:
                pssmShadowSplitTexture._camera->setClearColor(osg::Vec4(1.0,0.0,0.0,1.0));
                break;
              case 1:
                pssmShadowSplitTexture._camera->setClearColor(osg::Vec4(0.0,1.0,0.0,1.0));
                break;
              case 2:
                pssmShadowSplitTexture._camera->setClearColor(osg::Vec4(0.0,0.0,1.0,1.0));
                break;
              default:
                pssmShadowSplitTexture._camera->setClearColor(osg::Vec4(1.0,1.0,1.0,1.0));
                break;
              }
#endif
            pssmShadowSplitTexture._camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
            pssmShadowSplitTexture._camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT);

            // set viewport
            pssmShadowSplitTexture._camera->setViewport(0,0,pssmShadowSplitTexture._resolution,pssmShadowSplitTexture._resolution);

            // set the camera to render before the main camera.
            pssmShadowSplitTexture._camera->setRenderOrder(osg::Camera::PRE_RENDER);

            // tell the camera to use OpenGL frame buffer object where supported.
            pssmShadowSplitTexture._camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

            // attach the texture and use it as the color buffer.
#ifndef SHADOW_TEXTURE_DEBUG
            pssmShadowSplitTexture._camera->attach(osg::Camera::DEPTH_BUFFER, pssmShadowSplitTexture._texture.get());
#else
            pssmShadowSplitTexture._camera->attach(osg::Camera::COLOR_BUFFER, pssmShadowSplitTexture._texture.get());
#endif
            osg::StateSet* stateset = pssmShadowSplitTexture._camera->getOrCreateStateSet();



            //////////////////////////////////////////////////////////////////////////
            if ( _user_polgyonOffset_set ) {
              float factor = _polgyonOffset.x();
              float units  = _polgyonOffset.y();
              osg::ref_ptr<osg::PolygonOffset> polygon_offset = new osg::PolygonOffset;
              polygon_offset->setFactor(factor);
              polygon_offset->setUnits(units);
              stateset->setAttribute(polygon_offset.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
              stateset->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            }


            //////////////////////////////////////////////////////////////////////////
            if ( ! _GLSL_shadow_filtered ) {
              // if not glsl filtering enabled then we should force front face culling to reduce the number of shadow artifacts.
              osg::ref_ptr<osg::CullFace> cull_face = new osg::CullFace;
              cull_face->setMode(osg::CullFace::FRONT);
              stateset->setAttribute(cull_face.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
              stateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            }

            //////////////////////////////////////////////////////////////////////////
            osg::ShadeModel* shademodel = dynamic_cast<osg::ShadeModel*>(stateset->getAttribute(osg::StateAttribute::SHADEMODEL));
            if (!shademodel){shademodel = new osg::ShadeModel;stateset->setAttribute(shademodel);}
            shademodel->setMode( osg::ShadeModel::FLAT );
            stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
          }

          //////////////////////////////////////////////////////////////////////////
          // set up stateset and append texture, texGen ,...
          {
            pssmShadowSplitTexture._stateset = sharedStateSet.get();//new osg::StateSet;
            pssmShadowSplitTexture._stateset->setTextureAttributeAndModes(pssmShadowSplitTexture._textureUnit,pssmShadowSplitTexture._texture.get(),osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            pssmShadowSplitTexture._stateset->setTextureMode(pssmShadowSplitTexture._textureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
            pssmShadowSplitTexture._stateset->setTextureMode(pssmShadowSplitTexture._textureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
            pssmShadowSplitTexture._stateset->setTextureMode(pssmShadowSplitTexture._textureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
            pssmShadowSplitTexture._stateset->setTextureMode(pssmShadowSplitTexture._textureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);


            /// generate a TexGen object
            pssmShadowSplitTexture._texgen = new osg::TexGen;

          }

          //////////////////////////////////////////////////////////////////////////
          // set up shader (GLSL)
#ifdef SHADOW_TEXTURE_GLSL



          //////////////////////////////////////////////////////////////////////////
          // UNIFORMS
          std::stringstream strST; strST << "shadowTexture" << (pssmShadowSplitTexture._textureUnit-_textureUnitOffset);
          osg::Uniform* shadowTextureSampler = new osg::Uniform(strST.str().c_str(),(int)(pssmShadowSplitTexture._textureUnit));
          pssmShadowSplitTexture._stateset->addUniform(shadowTextureSampler);
          uniformList.push_back(shadowTextureSampler);
          //TODO: NOT YET SUPPORTED in the current version of the shader
          if ( ! _ambientBiasUniform ) {
            _ambientBiasUniform = new osg::Uniform("ambientBias",_ambientBias);
            pssmShadowSplitTexture._stateset->addUniform(_ambientBiasUniform);
          }


          std::stringstream strzShadow; strzShadow << "zShadow" << (pssmShadowSplitTexture._textureUnit-_textureUnitOffset);
          pssmShadowSplitTexture._farDistanceSplit = new osg::Uniform(strzShadow.str().c_str(),1.0f);
          pssmShadowSplitTexture._stateset->addUniform(pssmShadowSplitTexture._farDistanceSplit);
          uniformList.push_back(pssmShadowSplitTexture._farDistanceSplit);
#endif


          //////////////////////////////////////////////////////////////////////////
          // DEBUG
          if ( _displayTexturesGroupingNode ) {
            {
              pssmShadowSplitTexture._debug_textureUnit = 1;
              pssmShadowSplitTexture._debug_texture = new osg::Texture2D;
              pssmShadowSplitTexture._debug_texture->setTextureSize(TEXTURE_RESOLUTION, TEXTURE_RESOLUTION);
#ifdef SHOW_SHADOW_TEXTURE_DEBUG
              pssmShadowSplitTexture._debug_texture->setInternalFormat(GL_DEPTH_COMPONENT);
              pssmShadowSplitTexture._debug_texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
#else
              pssmShadowSplitTexture._debug_texture->setInternalFormat(GL_RGBA);
#endif
              pssmShadowSplitTexture._debug_texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
              pssmShadowSplitTexture._debug_texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
              // create the camera
              pssmShadowSplitTexture._debug_camera = new osg::Camera;
              pssmShadowSplitTexture._debug_camera->setCullCallback(new CameraCullCallback(this));
              pssmShadowSplitTexture._debug_camera->setClearMask(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
              pssmShadowSplitTexture._debug_camera->setClearColor(osg::Vec4(1.0,1.0,1.0,1.0));
              pssmShadowSplitTexture._debug_camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

              // set viewport
              pssmShadowSplitTexture._debug_camera->setViewport(0,0,TEXTURE_RESOLUTION,TEXTURE_RESOLUTION);
              // set the camera to render before the main camera.
              pssmShadowSplitTexture._debug_camera->setRenderOrder(osg::Camera::PRE_RENDER);
              // tell the camera to use OpenGL frame buffer object where supported.
              pssmShadowSplitTexture._debug_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
              // attach the texture and use it as the color buffer.
#ifdef SHOW_SHADOW_TEXTURE_DEBUG
              pssmShadowSplitTexture._debug_camera->attach(osg::Camera::DEPTH_BUFFER, pssmShadowSplitTexture._debug_texture.get());
#else
              pssmShadowSplitTexture._debug_camera->attach(osg::Camera::COLOR_BUFFER, pssmShadowSplitTexture._debug_texture.get());
#endif
              // osg::StateSet* stateset = pssmShadowSplitTexture._debug_camera->getOrCreateStateSet();

              pssmShadowSplitTexture._debug_stateset = new osg::StateSet;
              pssmShadowSplitTexture._debug_stateset->setTextureAttributeAndModes(pssmShadowSplitTexture._debug_textureUnit,pssmShadowSplitTexture._debug_texture.get(),osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
              pssmShadowSplitTexture._debug_stateset->setTextureMode(pssmShadowSplitTexture._debug_textureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
              pssmShadowSplitTexture._debug_stateset->setTextureMode(pssmShadowSplitTexture._debug_textureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
              pssmShadowSplitTexture._debug_stateset->setTextureMode(pssmShadowSplitTexture._debug_textureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
              pssmShadowSplitTexture._debug_stateset->setTextureMode(pssmShadowSplitTexture._debug_textureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);
            }

            osg::Geode* geode = _displayTexturesGroupingNode[iCameras];
            geode->getOrCreateStateSet()->setTextureAttributeAndModes(0,pssmShadowSplitTexture._debug_texture.get(),osg::StateAttribute::ON);

          }
          //////////////////////////////////////////////////////////////////////////

          //_PSSMShadowSplitTextureMap.insert(PSSMShadowSplitTextureMap::value_type(iCameras,pssmShadowSplitTexture));
          _PSSMShadowSplitTextureMap[iCameras] = pssmShadowSplitTexture;


        }
      OSG_DEBUG << "------ finish init PSSM " << std::endl;

      isInit = true;
      _dirty = false;
    }

    void ParallelSplitShadowMap::applyState(osg::StateSet* state) {
      if(_PSSMShadowSplitTextureMap.size() < 1) {
        return;
      }
      for(unsigned int i=0; i<_number_of_splits; ++i) {
        state->setTextureAttributeAndModes(_PSSMShadowSplitTextureMap[i]._textureUnit, _PSSMShadowSplitTextureMap[i]._texture.get(),
                                           osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        state->setTextureMode(_PSSMShadowSplitTextureMap[i]._textureUnit, GL_TEXTURE_GEN_S,
                              osg::StateAttribute::ON);
        state->setTextureMode(_PSSMShadowSplitTextureMap[i]._textureUnit, GL_TEXTURE_GEN_T,
                              osg::StateAttribute::ON);
        state->setTextureMode(_PSSMShadowSplitTextureMap[i]._textureUnit, GL_TEXTURE_GEN_R,
                              osg::StateAttribute::ON);
        state->setTextureMode(_PSSMShadowSplitTextureMap[i]._textureUnit, GL_TEXTURE_GEN_Q,
                              osg::StateAttribute::ON);
      }

      // // add the uniform list to the stateset
      for(std::vector< osg::ref_ptr<osg::Uniform> >::const_iterator itr=uniformList.begin();
          itr!=uniformList.end(); ++itr) {
        state->addUniform(itr->get());
      }
    }

    void ParallelSplitShadowMap::removeState(osg::StateSet* state) {
      if(_PSSMShadowSplitTextureMap.size() < 1) {
        return;
      }
      for(unsigned int i=0; i<_number_of_splits; ++i) {
        state->setTextureAttributeAndModes(_PSSMShadowSplitTextureMap[i]._textureUnit, NULL,
                                           osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
      }

      // // add the uniform list to the stateset
      for(std::vector< osg::ref_ptr<osg::Uniform> >::const_iterator itr=uniformList.begin();
          itr!=uniformList.end(); ++itr) {
        state->removeUniform(itr->get());
      }
    }

    void ParallelSplitShadowMap::update(osg::NodeVisitor& nv){
      _shadowedScene->osg::Group::traverse(nv);
    }

    void ParallelSplitShadowMap::cull(osgUtil::CullVisitor& cv){

      OSG_DEBUG << "------ cull PSSM " << std::endl;
      std::list<osg_lines::Vector> points;
      //l->setData(points);
      //l->drawStrip(false);
      // record the traversal mask on entry so we can reapply it later.
      unsigned int traversalMask = cv.getTraversalMask();
      osgUtil::RenderStage* orig_rs = cv.getRenderStage();

#ifdef SHADOW_TEXTURE_GLSL
      OSG_DEBUG << "size texture map: "  << _PSSMShadowSplitTextureMap.size() << std::endl;
      PSSMShadowSplitTextureMap::iterator tm_itr=_PSSMShadowSplitTextureMap.begin();
      OSG_DEBUG  << tm_itr->first << std::endl;
#else
      // do traversal of shadow receiving scene which does need to be decorated by the shadow map
      for (PSSMShadowSplitTextureMap::iterator tm_itr=_PSSMShadowSplitTextureMap.begin();tm_itr!=_PSSMShadowSplitTextureMap.end();tm_itr++)
#endif
        {
          PSSMShadowSplitTexture &pssmShadowSplitTexture = tm_itr->second;
          cv.pushStateSet(sharedStateSet.get());
          //cv.pushStateSet(pssmShadowSplitTexture._stateset.get());

          //////////////////////////////////////////////////////////////////////////
          // DEBUG
          if ( _displayTexturesGroupingNode ) {
            cv.pushStateSet(pssmShadowSplitTexture._debug_stateset.get());
          }
          //////////////////////////////////////////////////////////////////////////

          _shadowedScene->osg::Group::traverse(cv);

          cv.popStateSet();

        }

      //////////////////////////////////////////////////////////////////////////
      const osg::Light* selectLight = 0;

      /// light pos and light direction
      osg::Vec4 lightpos;
      osg::Vec3 lightDirection;


      osgUtil::PositionalStateContainer::AttrMatrixList& aml = orig_rs->getPositionalStateContainer()->getAttrMatrixList();
      for(osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin(); itr != aml.end(); ++itr) {
        const osg::Light* light_ = dynamic_cast<const osg::Light*>(itr->first.get());
        if (light_) {
          if( light.valid()) {
            if( light.get() == light_ )
              selectLight = light_;
            else
              continue;
          }
          else
            selectLight = light_;

          osg::RefMatrix* matrix = itr->second.get();
          if (matrix) {
            lightpos = light_->getPosition() * (*matrix);
            lightDirection = osg::Matrix::transform3x3( light_->getDirection(),
                                                        *matrix );
          }
          else {
            lightpos = light_->getPosition();
            lightDirection = light_->getDirection();
          }
        }
      }

      osg::Matrix eyeToWorld;
      eyeToWorld.invert(*cv.getModelViewMatrix());

      lightpos = lightpos * eyeToWorld;
      // currently the implementation is only correct for
      // directional lightning where we calculate the direction
      // by the postion
      lightDirection = osg::Vec3(-lightpos.x(), -lightpos.y(), -lightpos.z());
      //lightDirection = osg::Matrix::transform3x3( lightDirection, eyeToWorld );
      lightDirection.normalize();
      // fprintf(stderr, "lightpos: %g %g %g\ndirection: %g %g %g\n",
      //         lightpos.x(), lightpos.y(), lightpos.z(),
      //         lightDirection.x(), lightDirection.y(), lightDirection.z());
      // l->appendData(osg_lines::Vector(lightpos.x(), lightpos.y(), lightpos.z()));
      // l->appendData(osg_lines::Vector(lightpos.x()+lightDirection.x(),
      //                                 lightpos.y()+lightDirection.y(),
      //                                 lightpos.z()+lightDirection.z()));
      
      if (selectLight)
        {

          // do traversal of shadow receiving scene which does need to be decorated by the shadow map
          //unsigned int iMaxSplit = _PSSMShadowSplitTextureMap.size();

          for(PSSMShadowSplitTextureMap::iterator it=_PSSMShadowSplitTextureMap.begin();it!=_PSSMShadowSplitTextureMap.end();it++)
            {
              PSSMShadowSplitTexture &pssmShadowSplitTexture = it->second;


              //////////////////////////////////////////////////////////////////////////
              // SETUP pssmShadowSplitTexture for rendering
              //
              //lightDirection.normalize();
              pssmShadowSplitTexture._lightDirection = lightDirection;
              pssmShadowSplitTexture._cameraView    = cv.getRenderInfo().getView()->getCamera()->getViewMatrix();
              pssmShadowSplitTexture._cameraProj    = cv.getRenderInfo().getView()->getCamera()->getProjectionMatrix();

              //////////////////////////////////////////////////////////////////////////
              // CALCULATE



              // Calculate corner points of frustum split
              //
              // To avoid edge problems, scale the frustum so
              // that it's at least a few pixels larger
              //
              osg::Vec3d pCorners[8];
              calculateFrustumCorners(pssmShadowSplitTexture,pCorners);

              // Init Light (Directional Light)
              //
              calculateLightInitialPosition(pssmShadowSplitTexture,pCorners);

              // Calculate near and far for light view
              //
              calculateLightNearFarFormFrustum(pssmShadowSplitTexture,pCorners);

              // Calculate view and projection matrices
              //
              calculateLightViewProjectionFormFrustum(pssmShadowSplitTexture,pCorners);

              //////////////////////////////////////////////////////////////////////////
              // set up shadow rendering camera
              pssmShadowSplitTexture._camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);

              //////////////////////////////////////////////////////////////////////////
              // DEBUG
              if ( _displayTexturesGroupingNode ) {
                pssmShadowSplitTexture._debug_camera->setViewMatrix(pssmShadowSplitTexture._camera->getViewMatrix());
                pssmShadowSplitTexture._debug_camera->setProjectionMatrix(pssmShadowSplitTexture._camera->getProjectionMatrix());
                pssmShadowSplitTexture._debug_camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
              }

              //////////////////////////////////////////////////////////////////////////
              // compute the matrix which takes a vertex from local coords into tex coords
              // will use this later to specify osg::TexGen..

              osg::Matrix MVPT = pssmShadowSplitTexture._camera->getViewMatrix() *
                pssmShadowSplitTexture._camera->getProjectionMatrix() *
                osg::Matrix::translate(1.0,1.0,1.0) *
                osg::Matrix::scale(0.5,0.5,0.5);

              pssmShadowSplitTexture._texgen->setMode(osg::TexGen::EYE_LINEAR);
              pssmShadowSplitTexture._texgen->setPlanesFromMatrix(MVPT);
              //////////////////////////////////////////////////////////////////////////


              //////////////////////////////////////////////////////////////////////////
              //cv.setTraversalMask( traversalMask & getShadowedScene()->getCastsShadowTraversalMask() );
              cv.setTraversalMask( getShadowedScene()->getCastsShadowTraversalMask() );

              // do RTT camera traversal
              pssmShadowSplitTexture._camera->accept(cv);

              //////////////////////////////////////////////////////////////////////////
              // DEBUG
              if ( _displayTexturesGroupingNode ) {
                pssmShadowSplitTexture._debug_camera->accept(cv);
              }


              orig_rs->getPositionalStateContainer()->addPositionedTextureAttribute(pssmShadowSplitTexture._textureUnit, cv.getModelViewMatrix(), pssmShadowSplitTexture._texgen.get());


            }
        } // if light



      // reapply the original traversal mask
      cv.setTraversalMask( traversalMask );
      haveLines = true;
    }

    void ParallelSplitShadowMap::cleanSceneGraph(){

    }


    //////////////////////////////////////////////////////////////////////////
    // Computes corner points of a frustum
    //
    //
    //unit box representing frustum in clip space
    const osg::Vec3d const_pointFarTR(   1.0,  1.0,  1.0);
    const osg::Vec3d const_pointFarBR(   1.0, -1.0,  1.0);
    const osg::Vec3d const_pointFarTL(  -1.0,  1.0,  1.0);
    const osg::Vec3d const_pointFarBL(  -1.0, -1.0,  1.0);
    const osg::Vec3d const_pointNearTR(  1.0,  1.0, -1.0);
    const osg::Vec3d const_pointNearBR(  1.0, -1.0, -1.0);
    const osg::Vec3d const_pointNearTL( -1.0,  1.0, -1.0);
    const osg::Vec3d const_pointNearBL( -1.0, -1.0, -1.0);
    //////////////////////////////////////////////////////////////////////////


    void ParallelSplitShadowMap::calculateFrustumCorners(PSSMShadowSplitTexture &pssmShadowSplitTexture, osg::Vec3d *frustumCorners)
    {
      // get user cameras
      double fovy,aspectRatio,camNear,camFar;
      pssmShadowSplitTexture._cameraProj.getPerspective(fovy,aspectRatio,camNear,camFar);


      // force to max far distance to show shadow, for some scene it can be solve performance problems.
      if ((_isSetMaxFarDistance) && (_setMaxFarDistance < camFar))
        camFar = _setMaxFarDistance;


      // build camera matrix with some offsets (the user view camera)
      osg::Matrixd viewMat;
      osg::Vec3d camEye,camCenter,camUp;
      pssmShadowSplitTexture._cameraView.getLookAt(camEye,camCenter,camUp);
      osg::Vec3d viewDir = camCenter - camEye;
      //viewDir.normalize(); //we can assume that viewDir is still normalized in the viewMatrix
      camEye = camEye  - viewDir * _move_vcam_behind_rcam_factor;
      camFar += _move_vcam_behind_rcam_factor * viewDir.length();
      viewMat.makeLookAt(camEye,camCenter,camUp);



      //////////////////////////////////////////////////////////////////////////
      /// CALCULATE SPLIT
      double maxFar = camFar;
      // double minNear = camNear;
      double camNearFar_Dist = maxFar - camNear;
      if ( _SplitCalcMode == SPLIT_LINEAR )
        {
          camFar  = camNear + (camNearFar_Dist) * ((double)(pssmShadowSplitTexture._splitID+1))/((double)(_number_of_splits));
          camNear = camNear + (camNearFar_Dist) * ((double)(pssmShadowSplitTexture._splitID))/((double)(_number_of_splits));
        }
      else
        {
          // Exponential split scheme:
          //
          // Ci = (n - f)*(i/numsplits)^(bias+1) + n;
          //
          static double fSplitSchemeBias[2]={0.25f,2.26f};
          fSplitSchemeBias[1]=Clamp(fSplitSchemeBias[1],0.0,3.0);
          double* pSplitDistances =new double[_number_of_splits+1];

          for(int i=0;i<(int)_number_of_splits;i++)
            {
              double fIDM=(double)(i)/(double)(_number_of_splits);
              pSplitDistances[i]=camNearFar_Dist*(pow(fIDM,fSplitSchemeBias[1]+1))+camNear;
            }
          // make sure border values are right
          pSplitDistances[0]=camNear;
          pSplitDistances[_number_of_splits]=camFar;

          camNear = pSplitDistances[pssmShadowSplitTexture._splitID];
          camFar  = pSplitDistances[pssmShadowSplitTexture._splitID+1];

          delete[] pSplitDistances;
        }


      pssmShadowSplitTexture._split_far = camFar;



      //////////////////////////////////////////////////////////////////////////
      /// TRANSFORM frustum corners (Optimized for Orthogonal)


      osg::Matrixd projMat;
      projMat.makePerspective(fovy,aspectRatio,camNear,camFar);
      osg::Matrixd projViewMat(viewMat*projMat);
      osg::Matrixd invProjViewMat;
      invProjViewMat.invert(projViewMat);

      //transform frustum vertices to world space
      frustumCorners[0] = const_pointFarBR * invProjViewMat;
      frustumCorners[1] = const_pointNearBR* invProjViewMat;
      frustumCorners[2] = const_pointNearTR* invProjViewMat;
      frustumCorners[3] = const_pointFarTR * invProjViewMat;
      frustumCorners[4] = const_pointFarTL * invProjViewMat;
      frustumCorners[5] = const_pointFarBL * invProjViewMat;
      frustumCorners[6] = const_pointNearBL* invProjViewMat;
      frustumCorners[7] = const_pointNearTL* invProjViewMat;

      if(!haveLines) {
        for(int i=0; i<8; ++i) {
          l->appendData(osg_lines::Vector(frustumCorners[i].x(),
                                          frustumCorners[i].y(),
                                          frustumCorners[i].z()));
        }
      }
      //std::cout << "camFar : "<<pssmShadowSplitTexture._splitID << " / " << camNear << "," << camFar << std::endl;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // compute directional light initial position;
    void ParallelSplitShadowMap::calculateLightInitialPosition(PSSMShadowSplitTexture &pssmShadowSplitTexture,osg::Vec3d *frustumCorners)
    {
      pssmShadowSplitTexture._frustumSplitCenter = frustumCorners[0];
      for(int i=1;i<8;i++)
        {
          pssmShadowSplitTexture._frustumSplitCenter +=frustumCorners[i];
        }
      //    pssmShadowSplitTexture._frustumSplitCenter /= 8.0;
      pssmShadowSplitTexture._frustumSplitCenter *= 0.125;
    }

    void ParallelSplitShadowMap::calculateLightNearFarFormFrustum(
                                                                  PSSMShadowSplitTexture &pssmShadowSplitTexture,
                                                                  osg::Vec3d *frustumCorners
                                                                  ) {

      //calculate near, far
      double zFar(-DBL_MAX);

      // calculate zFar (as longest distance)
      for(int i=0;i<8;i++) {
        double dist_z_from_light = fabs(pssmShadowSplitTexture._lightDirection*(frustumCorners[i] -  pssmShadowSplitTexture._frustumSplitCenter));
        if ( zFar  < dist_z_from_light ) zFar  = dist_z_from_light;
      }
      // update camera position and look at center
      pssmShadowSplitTexture._lightCameraSource = pssmShadowSplitTexture._frustumSplitCenter - pssmShadowSplitTexture._lightDirection*(zFar+_split_min_near_dist);
      pssmShadowSplitTexture._lightCameraTarget = pssmShadowSplitTexture._frustumSplitCenter + pssmShadowSplitTexture._lightDirection*(zFar);

      // l->appendData(osg_lines::Vector(pssmShadowSplitTexture._lightCameraSource.x(),
      //                                    pssmShadowSplitTexture._lightCameraSource.y(),
      //                                    pssmShadowSplitTexture._lightCameraSource.z()));
      // l->appendData(osg_lines::Vector(pssmShadowSplitTexture._lightCameraTarget.x(),
      //                                    pssmShadowSplitTexture._lightCameraTarget.y(),
      //                                    pssmShadowSplitTexture._lightCameraTarget.z()));

      // calculate [zNear,zFar]
      zFar = (-DBL_MAX);
      double zNear(DBL_MAX);
      for(int i=0;i<8;i++) {
        double dist_z_from_light = fabs(pssmShadowSplitTexture._lightDirection*(frustumCorners[i] -  pssmShadowSplitTexture._lightCameraSource));
        if ( zFar  < dist_z_from_light ) zFar  = dist_z_from_light;
        if ( zNear > dist_z_from_light ) zNear  = dist_z_from_light;
      }
      // update near - far plane
      pssmShadowSplitTexture._lightNear = max(zNear - _split_min_near_dist - 0.01,0.01);
      pssmShadowSplitTexture._lightFar  = zFar;

      if(!haveLines) {
        osg::Vec3 d = pssmShadowSplitTexture._lightDirection;
        osg::Vec3 v = pssmShadowSplitTexture._lightCameraSource + d* pssmShadowSplitTexture._lightNear;
        l->appendData(osg_lines::Vector(v.x(), v.y(), v.z()));
        v = pssmShadowSplitTexture._lightCameraSource + d* pssmShadowSplitTexture._lightFar;
        l->appendData(osg_lines::Vector(v.x(), v.y(), v.z()));
      }

    }




    void ParallelSplitShadowMap::calculateLightViewProjectionFormFrustum(PSSMShadowSplitTexture &pssmShadowSplitTexture,osg::Vec3d *frustumCorners)
    {

      // calculate the camera's coordinate system
      osg::Vec3d camEye,camCenter,camUp;
      pssmShadowSplitTexture._cameraView.getLookAt(camEye,camCenter,camUp);
      osg::Vec3d viewDir(camCenter-camEye);
      osg::Vec3d camRight(viewDir^camUp);


      if(!haveLines) {
        l->appendData(osg_lines::Vector(camEye.x(),camEye.y(),camEye.z()));
        l->appendData(osg_lines::Vector(camCenter.x(),camCenter.y(),camCenter.z()));
        l->appendData(osg_lines::Vector(camEye.x(),camEye.y(),camEye.z()));
        l->appendData(osg_lines::Vector(camUp.x(),camUp.y(),camUp.z()));
      }
      
      // we force to have normalized vectors (camera's view)
      camUp.normalize();
      viewDir.normalize();
      camRight.normalize();

      // use quaternion -> numerical more robust
      osg::Quat qRot;
      qRot.makeRotate(viewDir,pssmShadowSplitTexture._lightDirection);
      osg::Vec3d top =  qRot * camUp;
      osg::Vec3d right = qRot * camRight;

      if(!haveLines) {
        osg::Vec3 v = pssmShadowSplitTexture._lightCameraSource;
        l->appendData(osg_lines::Vector(v.x(),v.y(),v.z()));
        l->appendData(osg_lines::Vector(v.x()+top.x(),v.y()+top.y(),v.z()+top.z()));
        l->appendData(osg_lines::Vector(v.x(),v.y(),v.z()));
        l->appendData(osg_lines::Vector(v.x()+right.x(),v.y()+right.y(),v.z()+right.z()));
      }

      // calculate the camera's frustum right,right,bottom,top parameters
      double maxRight(-DBL_MAX),maxTop(-DBL_MAX);
      double minRight(DBL_MAX),minTop(DBL_MAX);

      for(int i(0); i < 8; i++)
        {

          osg::Vec3d diffCorner(frustumCorners[i] - pssmShadowSplitTexture._frustumSplitCenter);
          double lright(diffCorner*right);
          double lTop(diffCorner*top);

          if ( lright > maxRight ) maxRight  =  lright;
          if ( lTop  > maxTop  ) maxTop   =  lTop;

          if ( lright < minRight ) minRight  =  lright;
          if ( lTop  < minTop  ) minTop   =  lTop;
        }

      // make the camera view matrix
      pssmShadowSplitTexture._camera->setViewMatrixAsLookAt(pssmShadowSplitTexture._lightCameraSource,pssmShadowSplitTexture._lightCameraTarget,top);

      // use ortho projection for light (directional light only supported)
      pssmShadowSplitTexture._camera->setProjectionMatrixAsOrtho(minRight,maxRight,minTop,maxTop,pssmShadowSplitTexture._lightNear,pssmShadowSplitTexture._lightFar);


#ifdef SHADOW_TEXTURE_GLSL
      // get user cameras
      osg::Vec3d vProjCamFraValue = (camEye + viewDir * pssmShadowSplitTexture._split_far) * (pssmShadowSplitTexture._cameraView * pssmShadowSplitTexture._cameraProj);
      pssmShadowSplitTexture._farDistanceSplit->set((float)vProjCamFraValue.z());
#endif


    }

    void ParallelSplitShadowMap::removeTexture(osg::StateSet* state) {
      for(unsigned int i=0; i<_number_of_splits; ++i) {
        state->setTextureAttributeAndModes(i+_textureUnitOffset, _PSSMShadowSplitTextureMap[i]._texture.get(), osg::StateAttribute::OFF);
      }
    }

    void ParallelSplitShadowMap::addTexture(osg::StateSet* state) {
      for(unsigned int i=0; i<_number_of_splits; ++i) {
        state->setTextureAttributeAndModes(i+_textureUnitOffset, _PSSMShadowSplitTextureMap[i]._texture.get(), osg::StateAttribute::ON);
      }
    }

  }
}


