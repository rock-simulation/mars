 /**
 * \file ParallelSplitShadowMap.h
 * \author Malte Langosz
 * \brief The ParallelSplitShadowMap is a clone of the original osgShadow::ParallelSplitShadowMap
 *        with some adaptations for the MARS simulation framework.
 */


#ifndef _ParallelSplitShadowMap
#define _ParallelSplitShadowMap 1

#include <osg/Camera>
#include <osg/Material>
#include <osg/Depth>
#include <osg/ClipPlane>

#include <osgShadow/ShadowTechnique>

#include <osg_lines/LinesFactory.h>

namespace mars {
  namespace graphics {

    class ParallelSplitShadowMap :  public osgShadow::ShadowTechnique
    {
    public:
      ParallelSplitShadowMap(osg::Geode** debugGroup=NULL, int icountplanes=3);

      ParallelSplitShadowMap(const ParallelSplitShadowMap& es, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

      META_Object(osgShadow, ParallelSplitShadowMap);

      void setLight(osg::Light *light);
      void setLight(osg::LightSource *ls);
      void applyState(osg::StateSet* state);
      void initIntern();

      /** Initialize the ShadowedScene and local cached data structures.*/
      virtual void init();

      /** Run the update traversal of the ShadowedScene and update any loca chached data structures.*/
      virtual void update(osg::NodeVisitor& nv);

      /** Run the cull traversal of the ShadowedScene and set up the rendering for this ShadowTechnique.*/
      virtual void cull(osgUtil::CullVisitor& cv);

      /** Clean scene graph from any shadow technique specific nodes, state and drawables.*/
      virtual void cleanSceneGraph();

      /** Switch on the debug coloring in GLSL (only the first 3 texture/splits showed for visualisation */
      inline void setDebugColorOn() { _debug_color_in_GLSL = true; }

      /** Set the polygon offset osg::Vec2f(factor,unit) */
      inline void setPolygonOffset(const osg::Vec2f& p) { _polgyonOffset = p;_user_polgyonOffset_set=true;}

      /** Get the polygon offset osg::Vec2f(factor,unit) */
      inline const osg::Vec2f& getPolygonOffset() const { return _polgyonOffset;}

      /** Set the texture resolution */
      inline void setTextureResolution(unsigned int resolution) { _resolution = resolution; }

      /** Get the texture resolution */
      inline unsigned int getTextureResolution() const { return _resolution; }

      /** Set the max far distance */
      inline void setMaxFarDistance(double farDist) { _setMaxFarDistance = farDist; _isSetMaxFarDistance = true; }

      /** Get the max far distance */
      inline double getMaxFarDistance() const { return _setMaxFarDistance; }

      /** Set the factor for moving the virtual camera behind the real camera*/
      inline void setMoveVCamBehindRCamFactor(double distFactor ) { _move_vcam_behind_rcam_factor = distFactor; }

      /** Get the factor for moving the virtual camera behind the real camera*/
      inline double getMoveVCamBehindRCamFactor() const { return _move_vcam_behind_rcam_factor; }

      /** Set min near distance for splits */
      inline void setMinNearDistanceForSplits(double nd){ _split_min_near_dist=nd; }

      /** Get min near distance for splits */
      inline double getMinNearDistanceForSplits() const { return _split_min_near_dist; }

      /** set a user defined light for shadow simulation (sun light, ... )
       *    when this light get passed to pssm, the scene's light are no longer collected
       *    and simulated. just this user passed light, it needs to be a directional light.
       */
      inline void setUserLight(osg::Light* light) { _userLight = light; }

      /** get the user defined light for shadow simulation */
      inline const osg::Light* getUserLight() const { return _userLight.get(); }

      /** Set the values for the ambient bias the shader will use.*/
      void setAmbientBias(const osg::Vec2& ambientBias );

      /** Get the values for the ambient bias the shader will use.*/
      const osg::Vec2& getAmbientBias() const { return _ambientBias; }

      enum SplitCalcMode {
        SPLIT_LINEAR,
        SPLIT_EXP
      };

      /** set split calculation mode */
      inline void setSplitCalculationMode(SplitCalcMode scm=SPLIT_EXP) { _SplitCalcMode = scm; }

      /** get split calculation mode */
      inline SplitCalcMode getSplitCalculationMode() const { return _SplitCalcMode; }


      /** Resize any per context GLObject buffers to specified size. */
      virtual void resizeGLObjectBuffers(unsigned int maxSize);

      /** If State is non-zero, this function releases any associated OpenGL objects for
       * the specified graphics context. Otherwise, releases OpenGL objects
       * for all graphics contexts. */
      virtual void releaseGLObjects(osg::State* = 0) const;

      void removeTexture(osg::StateSet* state);
      void addTexture(osg::StateSet* state);

    protected :

      virtual ~ParallelSplitShadowMap() {}


      struct PSSMShadowSplitTexture {
        // RTT
        osg::ref_ptr<osg::Camera>       _camera;
        osg::ref_ptr<osg::TexGen>       _texgen;
        osg::ref_ptr<osg::Texture2D>    _texture;
        osg::ref_ptr<osg::StateSet>     _stateset;
        unsigned int                    _textureUnit;


        double                            _split_far;

        osg::ref_ptr<osg::Camera>       _debug_camera;
        osg::ref_ptr<osg::Texture2D>    _debug_texture;
        osg::ref_ptr<osg::StateSet>     _debug_stateset;
        unsigned int                    _debug_textureUnit;

        // Light (SUN)
        osg::Vec3d                        _lightCameraSource;
        osg::Vec3d                        _lightCameraTarget;
        osg::Vec3d                        _frustumSplitCenter;
        osg::Vec3d                        _lightDirection;
        double                            _lightNear;
        double                            _lightFar;

        osg::Matrix                       _cameraView;
        osg::Matrix                       _cameraProj;

        unsigned int                      _splitID;
        unsigned int                      _resolution;

        osg::ref_ptr<osg::Uniform>        _farDistanceSplit;

        void resizeGLObjectBuffers(unsigned int maxSize);
        void releaseGLObjects(osg::State* = 0) const;

      };

      osg::ref_ptr<osg::Light> light;
      osg::ref_ptr<osg::LightSource>  ls;

      typedef std::map<unsigned int,PSSMShadowSplitTexture> PSSMShadowSplitTextureMap;
      PSSMShadowSplitTextureMap _PSSMShadowSplitTextureMap;


    private:
      void calculateFrustumCorners(PSSMShadowSplitTexture &pssmShadowSplitTexture,osg::Vec3d *frustumCorners);
      void calculateLightInitialPosition(PSSMShadowSplitTexture &pssmShadowSplitTexture,osg::Vec3d *frustumCorners);
      void calculateLightNearFarFormFrustum(PSSMShadowSplitTexture &pssmShadowSplitTexture,osg::Vec3d *frustumCorners);
      void calculateLightViewProjectionFormFrustum(PSSMShadowSplitTexture &pssmShadowSplitTexture,osg::Vec3d *frustumCorners);

      osg::Geode** _displayTexturesGroupingNode;

      unsigned int _textureUnitOffset;

      unsigned int _number_of_splits;

      bool _debug_color_in_GLSL;

      osg::Vec2 _polgyonOffset;
      bool _user_polgyonOffset_set;

      unsigned int _resolution;

      double _setMaxFarDistance;
      bool _isSetMaxFarDistance;

      double _split_min_near_dist;

      double _move_vcam_behind_rcam_factor;

      osg::ref_ptr<osg::Light> _userLight;

      bool            _GLSL_shadow_filtered;
      SplitCalcMode   _SplitCalcMode;

      osg::Uniform*   _ambientBiasUniform;
      osg::Vec2       _ambientBias;

      osg::ref_ptr<osg::StateSet> sharedStateSet;
      std::vector< osg::ref_ptr<osg::Uniform> > uniformList;
      osg_lines::Lines *l;
      bool isInit;
      bool haveLines;
    };
  }
}

#endif
