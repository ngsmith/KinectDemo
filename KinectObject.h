#include "CloudManager.h"
#include <osg/PolygonMode>
#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/SceneObject.h>
#include "Skeleton.h"

class KinectObject
{
    public:
    KinectObject();
    CloudManager* cm;
    osg::Group* group;
    osg::Geometry* geom;
    osg::Switch* switchNode;
    osg::Program* pgm1;
    float initialPointScale;
    bool _firstRun;
    bool _cloudIsOn;

    osg::ref_ptr<osg::Vec4Array> kinectColours;
    osg::ref_ptr<osg::Vec3Array> kinectVertices;

    void cloudOn();
    void attach();
    void update();
    void cloudOff();
};
