#include "KinectObject.h"

using namespace cvr;
using namespace std;
using namespace osg;

KinectObject::KinectObject(std::string server)
{
    // load sphere shader
    pgm1 = new osg::Program;
    pgm1->setName("Sphere");
    std::string shaderPath = ConfigManager::getEntry("Plugin.Points.ShaderPath");
    pgm1->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(shaderPath + "/Sphere.vert")));
    pgm1->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(shaderPath + "/Sphere.frag")));
    pgm1->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile(shaderPath + "/Sphere.geom")));
    pgm1->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
    pgm1->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
    pgm1->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
    initialPointScale = ConfigManager::getFloat("Plugin.KinectDemo.KinectDefaultOn.KinectPointSize");
    _firstRun = true;
    _cloudIsOn = false;
    kinectX = ConfigManager::getFloat("x", "Plugin.KinectDemo.KinectSkeleton", 0.0f);
    kinectY = ConfigManager::getFloat("y", "Plugin.KinectDemo.KinectSkeleton", 0.0f);
    kinectZ = ConfigManager::getFloat("z", "Plugin.KinectDemo.KinectSkeleton", 0.0f);
    kinectServer = server;
}

void KinectObject::cloudOn()
{
  if(_firstRun)
  {
    cout << "Starting Thread\n";
    cm = new CloudManager(kinectServer);
    cout << "Started\n";
    cm->start();
    group = new osg::Group();
    osg::StateSet* state = group->getOrCreateStateSet();
    state->setAttribute(pgm1);
    state->addUniform(new osg::Uniform("pointScale", initialPointScale));
    state->addUniform(new osg::Uniform("globalAlpha", 1.0f));
    float pscale = initialPointScale;
    osg::Uniform*  _scaleUni = new osg::Uniform("pointScale", 1.0f * pscale);
    group->getOrCreateStateSet()->addUniform(_scaleUni);
    switchNode = new osg::Switch();
    SceneObject* so = new SceneObject("testobject", false, false, false, true, false);
    so->addChild(switchNode);
    PluginHelper::registerSceneObject(so, "testobject");
    so->attachToScene();
    switchNode->addChild(group);
    _cloudIsOn = true;

    // kinect model
    if (ConfigManager::getBool("Plugin.KinectDemo.ShowKinectModel"))
    {
        //Loads Kinect Obj file
        Matrixd scale;
        double snum = 1;
        scale.makeScale(snum, snum, snum);
        MatrixTransform* modelScaleTrans = new MatrixTransform();
        modelScaleTrans->setMatrix(scale);
        _modelFileNode1 = osgDB::readNodeFile(ConfigManager::getEntry("Plugin.KinectDemo.3DModelFolder").append("kinect_mm.obj"));
        modelScaleTrans->addChild(_modelFileNode1);
        MatrixTransform* rotate = new osg::MatrixTransform();
        float rotDegrees[3];
        rotDegrees[0] = -90;
        rotDegrees[1] = 0;
        rotDegrees[2] = 180;
        rotDegrees[0] = DegreesToRadians(rotDegrees[0]);
        rotDegrees[1] = DegreesToRadians(rotDegrees[1]);
        rotDegrees[2] = DegreesToRadians(rotDegrees[2]);
        Quat rot = osg::Quat(rotDegrees[0], osg::Vec3d(1, 0, 0), rotDegrees[1], osg::Vec3d(0, 1, 0), rotDegrees[2], osg::Vec3d(0, 0, 1));
        Matrix rotMat;
        rotMat.makeRotate(rot);
        rotate->setMatrix(rotMat);
        rotate->addChild(modelScaleTrans);
        MatrixTransform* translate = new osg::MatrixTransform();
        osg::Matrixd tmat;
        Vec3 pos = Vec3(kinectX, kinectY, kinectZ);
        tmat.makeTranslate(pos);
        translate->setMatrix(tmat);
        translate->addChild(rotate);
        switchNode->addChild(translate);
    }

    so->setNavigationOn(true);
    so->setMovable(false);
    so->addMoveMenuItem();
    so->addNavigationMenuItem();
    /* float min = 0.0001;
     float max = 1;
     so->addScaleMenuItem("Scale", min, max, currentScale);
     SubMenu* sm = new SubMenu("Position");
     so->addMenuItem(sm);
     MenuButton* mb;
     mb = new MenuButton("Load");
     mb->setCallback(this);
     sm->addItem(mb);
     SubMenu* savemenu = new SubMenu("Save");
     sm->addItem(savemenu);
     mb = new MenuButton("Save");
     mb->setCallback(this);
     savemenu->addItem(mb);
     _pointClouds[i]->saveMap = mb;
     mb = new MenuButton("Save New Kml");
     mb->setCallback(this);
     savemenu->addItem(mb);
     _pointClouds[i]->saveNewMap = mb;
     mb = new MenuButton("Reset to Origin");
     mb->setCallback(this);
     so->addMenuItem(mb);
     _pointClouds[i]->resetMap = mb;
     MenuCheckbox* mc;
     mc = new MenuCheckbox("Active", false);
     mc->setCallback(this);
     so->addMenuItem(mc);
     _pointClouds[i]->activeMap = mc;
     mc = new MenuCheckbox("Visible", true);
     mc->setCallback(this);
     so->addMenuItem(mc);
     _pointClouds[i]->visibleMap = mc;
     _pointClouds[i]->visible = true;
     float rValue = 0;
     min = -1;
     max = 1;
     MenuRangeValue* rt = new MenuRangeValue("rx", min, max, rValue);
     rt->setCallback(this);
     so->addMenuItem(rt);
     _pointClouds[i]->rxMap = rt;
     rt = new MenuRangeValue("ry", min, max, rValue);
     rt->setCallback(this);
     so->addMenuItem(rt);
     _pointClouds[i]->ryMap = rt;
     rt = new MenuRangeValue("rz", min, max, rValue);
     rt->setCallback(this);
     so->addMenuItem(rt);
     _pointClouds[i]->rzMap = rt;*/
  }
  else
  {
    cerr << "Restarting\n";
    _firstRun = true;
    cm->should_quit = false;
    cm->start();
    _cloudIsOn = true;
  }
}

void KinectObject::update()
{
    if (_cloudIsOn == false) return;

    if (true)
    {
        if (cm->firstRunStatus() != 0)
        {
            float totalSize = 307200;

            if (cm->kinectVertices != NULL)
            {
                if (cm->kinectVertices->size() != 0)
                {
                    if (_firstRun)
                    {
                        _firstRun = false;
                        osg::Geode* kgeode = new osg::Geode();
                        kgeode->setCullingActive(false);
                        geom = new osg::Geometry();
                        StateSet* state = geom->getOrCreateStateSet();
                        state->setMode(GL_LIGHTING, StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
                        geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, cm->kinectVertices.get()->size()));
                        osg::VertexBufferObject* vboP = geom->getOrCreateVertexBufferObject();
                        //vboP->setUsage(GL_STREAM_DRAW);
                        vboP->setUsage(GL_DYNAMIC_DRAW);
                        //vboP->setUsage(GL_STATIC_DRAW);
                        geom->setUseDisplayList(false);
                        geom->setUseVertexBufferObjects(true);
                        kinectVertices = new Vec3Array;
                        kinectColours = new Vec4Array;
                        osg::Vec3 ppos(Skeleton::camPos.x(),
                                       Skeleton::camPos.y(),
                                       Skeleton::camPos.z());
                        kinectVertices->push_back(ppos);
                        kinectColours->push_back(osg::Vec4f(1, 1, 1, 1));
                        //knodeGeom->setVertexArray(kinectVertices);
                        //knodeGeom->setColorArray(kinectColours);
                        geom->setVertexArray(cm->kinectVertices.get());
                        geom->setColorArray(cm->kinectColours.get());
                        geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
                        kgeode->addDrawable(geom);
                        //kgeode->addDrawable(_cloudThread->tnodeGeom.get());
                        // kgeode->dirtyBound();
                        //if (kinectgrp != NULL) _root->removeChild(kinectgrp);
                        group->removeChild(0, 1);
                        group->addChild(kgeode);
                        //printf("Made it\n");
                    }
                    else
                    {
                        //printf("Made it\n");
                        geom->setVertexArray(cm->kinectVertices.get());
                        geom->setColorArray(cm->kinectColours.get());
                    }
                }
            }
        }
    }
}

void KinectObject::cloudOff()
{
 if(cm != NULL)
 {
    _cloudIsOn = false;
    cm->quit();
    group->removeChild(0, 1);
 } 
}

