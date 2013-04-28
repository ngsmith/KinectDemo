#include "CloudManager.h"
#include <osg/PolygonMode>
#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/SceneObject.h>
#include "Skeleton.h"
#include <cvrKernel/PluginHelper.h>

class KinectObject
{
public:
    KinectObject(std::string server);
    CloudManager* cm;
    osg::Group* group;
    osg::Geometry* geom;
    osg::Switch* switchNode;
    osg::Program* pgm1;
    osg::Node* _modelFileNode1;
    float initialPointScale;
    bool _firstRun;
    bool _cloudIsOn;

    float kinectX;
    float kinectY;
    float kinectZ;
    std::string kinectServer;

    osg::ref_ptr<osg::Vec4Array> kinectColours;
    osg::ref_ptr<osg::Vec3Array> kinectVertices;

    void cloudOn();
    void update();
    void cloudOff();
};
