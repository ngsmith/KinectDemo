#include "KinectObject.h"

using namespace cvr;
using namespace std;
using namespace osg;

KinectObject::KinectObject()
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
        _firstRun=true;
        _cloudIsOn=false;
}

void KinectObject::cloudOn()
{
            cout << "Starting Thread\n";
            cm = new CloudManager();
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
            SceneObject* so = new SceneObject("testobject",false,false,false,true,false);
            so->addChild(switchNode);
            // PluginHelper::registerSceneObject(so,"testobject");
            so->attachToScene();
            switchNode->addChild(group);
            _cloudIsOn = true;

}


void KinectObject::attach()
{

}

void KinectObject::update()
{
    if (_cloudIsOn==false) return;
cout << "updating" << endl;
        //Update KinectManager
        //_cloudThread->update();
        //osg::ref_ptr<osg::Vec3Array> testVert = _cloudThread->kinectVertices;
        //cout << "Size=" << testVert->size() << "\n";
        if (true)
        {
            if (cm->firstRunStatus() != 0)
            {
cout << "1" << endl;

                float totalSize = 307200;

                // kinectVertices = _cloudThread->kinectVertices;
                // kinectColours = _cloudThread->kinectColours;
                if (cm->kinectVertices != NULL)
                {
cout << "2" << endl;
                    if (cm->kinectVertices->size() != 0)
                    {
cout << "3" << endl;
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
                            //      X[i] = x;
                            //    Y[i] = y;
                            //  Z[i] = z;
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
                            cout << "rerere"<<endl;
                        }
                    }
                }
            }
        }
}

void KinectObject::cloudOff()
{

}

