//#ifndef _ANIMATION_MANAGER_MULT
//#define _ANIMATION_MANAGER_MULT

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <cvrKernel/SceneManager.h>
#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrKernel/CVRPlugin.h>
#include <cvrMenu/MenuCheckbox.h>
#include <cvrMenu/MenuButton.h>
#include <cvrMenu/MenuRangeValue.h>

#include <shared/PubSub.h>
#include <protocol/skeletonframe.pb.h>
#include <protocol/colormap.pb.h>
#include <protocol/depthmap.pb.h>
#include <protocol/pointcloud.pb.h>
#include <zmq.hpp>
#include "kUtils.h"
//#include "Skeleton.h"
#include <unordered_map>


#include <iostream>
#include <string>
#include <map>

#include <OpenThreads/Thread>


//  zmq::context_t contextCloud(1);

class CloudManager : public OpenThreads::Thread {

public:

    CloudManager();
    ~CloudManager();
    //contextCloud(1);
    SubSocket<RemoteKinect::PointCloud>* cloudT_socket;
    void update();
    bool isCacheDone();
    RemoteKinect::PointCloud* packet;

    virtual void run();
    void quit();
    int firstRunStatus();
    osg::ref_ptr<osg::Vec4Array> kinectColours;
    osg::ref_ptr<osg::Vec3Array> kinectVertices;
    osg::ref_ptr<osg::Vec3Array> kinectNormals;
    osg::ref_ptr<osg::Geometry> tnodeGeom;
protected:

    osg::ref_ptr<osg::Vec4Array> newColours;
    osg::ref_ptr<osg::Vec3Array> newVertices;
    osg::ref_ptr<osg::Vec3Array> newNormals;
    bool _cacheDone;
    bool _next;
    bool useKColor;
    bool pause;
    bool should_quit;
    int _firstRun;
    int minDistHSV, maxDistHSV;
    int minDistHSVDepth, maxDistHSVDepth;
    std::unordered_map<float, osg::Vec4f> distanceColorMap;
    osg::Vec4f getColorRGB(int dist);
    void processNewCloud();
};

//#endif
