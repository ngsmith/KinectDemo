#include "KinectDemo.h"

using namespace osg;
using namespace std;
using namespace cvr;

CVRPLUGIN(KinectDemo)
KinectDemo* KinectDemo::_kinectDemo = NULL;

KinectDemo::KinectDemo() {}

KinectDemo::~KinectDemo()
{
    if (skel_socket)
    {
        delete skel_socket;
        skel_socket = NULL;
    }

    if (depth_socket)
    {
        delete depth_socket;
        depth_socket = NULL;
    }

    if (cloud_socket)
    {
        delete cloud_socket;
        cloud_socket = NULL;
    }

    if (color_socket)
    {
        delete color_socket;
        color_socket = NULL;
    }
}

KinectDemo* KinectDemo::instance()
{
    if (!_kinectDemo)
        _kinectDemo = new KinectDemo();

    return _kinectDemo;
}

bool KinectDemo::init()
{
    _root = new osg::MatrixTransform();
    pointGeode = new osg::Geode();
    useKinect = true;
    kShowPCloud = ConfigManager::getBool("Plugin.KinectDemo.KinectDefaultOn.ShowPCloud");
   // kShowPCloud = false;
    //kinectThreaded = false;
    useKColor = ConfigManager::getBool("Plugin.KinectDemo.KinectDefaultOn.ShowColor");
    kShowDepth = false;
    kShowColor = false;
    kNavSpheres = false;
    kMoveWithCam = false;
    kFreezeCloud = false;
    std::cerr << "KinectDemo init\n";
    //Menu Setup:
    _avMenu = new SubMenu("Kinect Demo", "Kinect Demo");
    _avMenu->setCallback(this);
    _bookmarkLoc = new MenuButton("Save Location");
    _bookmarkLoc->setCallback(this);
    _testInteract = new MenuButton("Send Interact");
    _testInteract->setCallback(this);
    _kinectOn = new MenuCheckbox("Use Kinect", useKinect);  //new
    _kinectOn->setCallback(this);
    _avMenu->addItem(_kinectOn);
    _kShowColor = new MenuCheckbox("Show Color Map", kShowColor);  //new
    _kShowColor->setCallback(this);
    _kShowDepth = new MenuCheckbox("Show Depth Map", kShowDepth);
    _kShowDepth->setCallback(this);
    _kMoveWithCam = new MenuCheckbox("Move skeleton with camera", kMoveWithCam);
    _kMoveWithCam->setCallback(this);
    _kFreezeCloud = new MenuCheckbox("Move skeleton with camera", kFreezeCloud);
    _kFreezeCloud->setCallback(this);
    colorfps = 100;
    _kColorFPS = new MenuRangeValue("1/FPS for camera/depth", 1, 100, colorfps);
    _kColorFPS->setCallback(this);
    _kShowPCloud = new MenuCheckbox("Show Point Cloud", kShowPCloud);  //new
    _kShowPCloud->setCallback(this);


    _kShowInfoPanel = new MenuCheckbox("Show Info Panel", kShowInfoPanel);  //new
    _kShowInfoPanel->setCallback(this);
    _infoPanel = new TabbedDialogPanel(500, 20, 4, "Info Panel", "Plugin.KinectDemo.InfoPanel");
    _infoPanel->addTextTab("Info", "");
    _infoPanel->setVisible(false);
    _infoPanel->setActiveTab("Info");

    _kColorOn = new MenuCheckbox("Show Real Colors on Point Cloud", useKColor);  //new
    _kColorOn->setCallback(this);
    _kNavSpheres = new MenuCheckbox("Navigation Spheres", kNavSpheres);
    _kNavSpheres->setCallback(this);
    _avMenu->addItem(_kShowColor);
    _avMenu->addItem(_kShowDepth);
    _avMenu->addItem(_kShowPCloud);
    _avMenu->addItem(_kColorOn);
    _avMenu->addItem(_kColorFPS);
    _avMenu->addItem(_bookmarkLoc);
    _avMenu->addItem(_testInteract);
    _avMenu->addItem(_kMoveWithCam);
    _avMenu->addItem(_kFreezeCloud);
    _avMenu->addItem(_kNavSpheres);
    _avMenu->addItem(_kShowInfoPanel);
    MenuSystem::instance()->addMenuItem(_avMenu);
    SceneManager::instance()->getObjectsRoot()->addChild(_root);

    if (useKinect) kinectInit();

    std::cerr << "KinectDemo initialized\n";
    return true;
}

void KinectDemo::menuCallback(MenuItem* menuItem)
{
    if (menuItem == _kinectOn)
    {
        if (useKinect) kinectOff();
        else kinectInit();

        useKinect = !useKinect;
    }

    if (menuItem == _kColorFPS)
    {
        colorfps = _kColorFPS->getValue();
    }

    if (menuItem == _kMoveWithCam)
    {
        if (_kMoveWithCam->getValue())
        {
            moveWithCamOn();
            kMoveWithCam = true;
        }
        else
        {
            moveWithCamOff();
            kMoveWithCam = false;
        }
    }
    if (menuItem == _kFreezeCloud)
    {
        if (_kFreezeCloud->getValue())
        {
            kFreezeCloud = true;
        }
        else
        {
            kFreezeCloud = false;
        }
    }

    if (menuItem == _kShowPCloud)
    {
        if (_kShowPCloud->getValue())
        {
            kShowPCloud = true;
            cloudOn();
        }
        else
        {
            kShowPCloud = false;
            cloudOff();
        }
    }

    if (menuItem == _kShowDepth)
    {
        if (_kShowDepth->getValue())
        {
            depthOn();
            kShowDepth = true;
        }
        else
        {
            depthOff();
            kShowDepth = false;
        }
    }

    if (menuItem == _kNavSpheres)
    {
        if (_kNavSpheres->getValue())
        {
            navOn();
            kNavSpheres = true;
        }
        else
        {
            navOff();
            kNavSpheres = false;
        }
    }

    if (menuItem == _kShowColor)
    {
        if (_kShowColor->getValue())
        {
            colorOn();
            kShowColor = true;
        }
        else
        {
            colorOff();
            kShowColor = false;
        }
    }

    if (menuItem == _kColorOn)
    {
        if (_kColorOn->getValue())
        {
            useKColor = true;
        }
        else
        {
            useKColor = false;
        }
    }

    if (menuItem == _bookmarkLoc)
    {
        Matrixd camMat = PluginHelper::getObjectMatrix();
        float cscale = PluginHelper::getObjectScale();
        Vec3 camTrans = camMat.getTrans();
        Quat camQuad = camMat.getRotate();
        cerr << "Saved camera position: " << endl;
        cerr << cscale << ", " << (camTrans.x() / cscale) << ", " << (camTrans.y() / cscale) << ", " << (camTrans.z() / cscale) << ", " << camQuad.x() << ", " << camQuad.y() << ", " << camQuad.z() << ", " << camQuad.w() << endl;

        bool savePointCloud = true;
        if(savePointCloud && cloud_socket)
        {
          ExportPointCloud(); 
        }

    }
    if (menuItem == _testInteract)
    {
       sendEvents();
    }
    if(menuItem == sliderX)
    {
        kinectX = sliderX->getValue();
    }
    if(menuItem == sliderY)
    {
        kinectY = sliderY->getValue();
    }
    if(menuItem == sliderZ)
    {
        kinectZ = sliderZ->getValue();
    }

}

void KinectDemo::preFrame()
{
    if (!useKinect || !kinectInitialized) return;

    // loop getting new data from Kinect server
    ThirdLoop();

    updateInfoPanel();
    // show image from Kinect camera every ... frames
    if (kShowColor || kShowDepth)
    {
        bcounter = ++bcounter % (int)colorfps;

        if (bcounter == (int)colorfps - 1)
        {
            if (kShowColor) showCameraImage();

            if (kShowDepth) showDepthImage();
        }
    }
if(!skeletonThreaded)
{
    // for every skeleton in mapIdSkel - draw, navigation spheres, check intersection with objects
    std::map< osg::ref_ptr<osg::Geode>, int >::iterator iter;

    for (std::map<int, Skeleton>::iterator it = mapIdSkel.begin(); it != mapIdSkel.end(); ++it)
    {
        int sk_id = it->first;
        Skeleton* sk = &(it->second);

        if (kNavSpheres)
        {
            if (sk->navSphere.lock == -1)
            {
                Vec3 navPos = sk->joints[3].position;
                navPos.y() += 0.3;
                sk->navSphere.update(navPos, Vec4(0, 0, 0, 1));
            }
        }

        /******One hand selection ********/
        if (sk->cylinder.attached == false)
            checkHandsIntersections(sk_id);

        //        cout << sk->joints[M_LHAND].position.x() << " lbusy " << sk->leftHandBusy << endl;
        Vec3 StartPoint = sk->joints[M_LHAND].position;
        Vec3 EndPoint = sk->joints[M_RHAND].position;
        // if cylinder would be >distanceMIN && <distanceMAX, draw it and check for collisions
        double distance = (StartPoint - EndPoint).length();
        float HAND_ELBOW_OFFSET = -0.15;

        //printf("Distance Cylinder: %g \n",distance);
        if (((sk->joints[9].position.z() - sk->joints[7].position.z() > HAND_ELBOW_OFFSET) && (sk->joints[15].position.z() - sk->joints[13].position.z() > HAND_ELBOW_OFFSET))) sk->cylinder.handsBeenAboveElbows = true;

        sk->cylinder.update(StartPoint, EndPoint);

        if (sk->cylinder.attached)
        {
            bool detachC = false;

            if (sk->cylinder.locked && ((sk->joints[9].position.z() - sk->joints[7].position.z() < HAND_ELBOW_OFFSET) && (sk->joints[15].position.z() - sk->joints[13].position.z() < HAND_ELBOW_OFFSET)))
                detachC = true;

            if (sk->cylinder.handsBeenAboveElbows && (distance > distanceMAX  || distance < distanceMIN || detachC))
            {
                sk->cylinder.detach(_pointClouds[0]->switchNode);

                // unlock all the objects that were locked by this cylinder
                for (int j = 0; j < selectableItems.size(); j++)
                {
                    if (selectableItems[j].lock == sk_id) selectableItems[j].unlock();
                }

                // and NavSpheres
                sk->navSphere.lock = -1;
                navLock = -1;
                sk->navSphere.activated = false;
                sk->cylinder.locked = false;
            }
            else
            {
                // if sk->leton's cylinder is locked to some object, do not lock any more
                if (sk->cylinder.locked == false)
                {
                    // for every selectable item, check if it intersects with the current cylinder
                    const osg::BoundingBox& bboxCyl = sk->cylinder.geode->getDrawable(0)->getBound();

                    // NavSphere
                    if (kNavSpheres)
                    {
                        Sphere* tempNav = new Sphere(sk->navSphere.position, 0.03 * 2);
                        ShapeDrawable* ggg3 = new ShapeDrawable(tempNav);
                        const osg::BoundingBox& fakenav = ggg3->getBound();

                        if (bboxCyl.intersects(fakenav) && sk->navSphere.lock == -1)
                        {
                            sk->navSphere.lock = sk_id;
                            navLock = sk_id;
                            sk->cylinder.locked = true;
                            sk->cylinder.prevVec = (sk->joints[M_LHAND].position - osg::Vec3d((StartPoint.x() + EndPoint.x()) / 2, (StartPoint.y() + EndPoint.y()) / 2, (StartPoint.z() + EndPoint.z()) / 2));
                            sk->cylinder.handsBeenAboveElbows = true;
                            break;
                        }
                    }

                    for (int j = 0; j < selectableItems.size(); j++)
                    {
                        // fake sphere to easily calculate boundary
                        Vec3 center2 = Vec3(0, 0, 0) * (selectableItems[j].mt->getMatrix());
                        Box* fakeSphere = new Box(center2, selectableItems[j].scale);
                        ShapeDrawable* ggg2 = new ShapeDrawable(fakeSphere);
                        const osg::BoundingBox& fakeBbox = ggg2->getBound();

                        if (bboxCyl.intersects(fakeBbox) && selectableItems[j].lock == -1)
                        {
                            sk->cylinder.locked = true;
                            selectableItems[j].lockType = 0;
                            selectableItems[j].lockTo(sk_id);
                            sk->cylinder.prevVec = (sk->joints[M_LHAND].position - osg::Vec3d((StartPoint.x() + EndPoint.x()) / 2, (StartPoint.y() + EndPoint.y()) / 2, (StartPoint.z() + EndPoint.z()) / 2));
                            sk->cylinder.handsBeenAboveElbows = false;
                            break; // lock only one object
                        }
                    }
                }
            }
        }
        else
        {
            // Cylinder is not attached
            // Don't create a cylinder between hands if any of them is holding an object
            if (sk->leftHandBusy == false && sk->rightHandBusy == false)
            {
                // CONDITIONS TO CREATE CYLINDER
                if (distance < distanceMAX / 2 && distance > distanceMIN && ((sk->joints[9].position.z() - sk->joints[7].position.z() > HAND_ELBOW_OFFSET) && (sk->joints[15].position.z() - sk->joints[13].position.z() > HAND_ELBOW_OFFSET)))
                {
                    sk->cylinder.attach(_pointClouds[0]->switchNode);
                }
            }
        }
    }

    // move all the objects that are locked to centers and rotate to centers rotation

    for (int j = 0; j < selectableItems.size(); j++)
    {
        SelectableItem sel = selectableItems[j];

        if (sel.lock == -1) continue;

        if (sel.lockType == 0)
        {
            int cylinderId = sel.lock;
            Matrix rotMat0;
            rotMat0.makeRotate(mapIdSkel[cylinderId].cylinder.prevVec, mapIdSkel[cylinderId].cylinder.currVec);
            selectableItems[j].rt->postMult(rotMat0);
            Matrix posMat;
            posMat.setTrans(mapIdSkel[cylinderId].cylinder.center);
            selectableItems[j].mt->setMatrix(posMat);
            double newscale = selectableItems[j].scale;
            newscale *= (1 + ((mapIdSkel[cylinderId].cylinder.length - mapIdSkel[cylinderId].cylinder.prevLength) / (500 / 1000.0)));

            if (newscale < 1  / 1000.0) newscale = 1  / 1000.0;

            selectableItems[j].setScale(newscale);
        }
        else if (sel.lockType != -1)
        {
            // moving artifact by one hand
            Matrix posMat;

            if (sel.lockType == 1) posMat.setTrans(mapIdSkel[sel.lock].joints[M_LHAND].position);
            else if (sel.lockType == 2) posMat.setTrans(mapIdSkel[sel.lock].joints[M_RHAND].position);
            else cout << "Error - unknown type of a lock (" << sel.lockType << ") on an object" << endl;

            selectableItems[j].mt->setMatrix(posMat);
        }
    }

    if (kNavSpheres && navLock != -1)
    {
        int cylinderId = navLock;
        Vec3 diff = mapIdSkel[cylinderId].cylinder.center - mapIdSkel[cylinderId].navSphere.position;
        bool navSphereActivated = mapIdSkel[cylinderId].navSphere.activated;

        if (!navSphereActivated)
        {
            const osg::BoundingBox& bboxCyl = mapIdSkel[cylinderId].cylinder.geode->getDrawable(0)->getBound();
            //Vec3 navPos = mapIdSkel[cylinderId].joints[3].position; navbang
            Vec3 center2 = mapIdSkel[cylinderId].joints[3].position;
            center2.y() += 0.4;
            //center2.z() += 0.2;
            Box* fakeSphere = new Box(center2, _sphereRadius * 1);
            ShapeDrawable* ggg2 = new ShapeDrawable(fakeSphere);
            const osg::BoundingBox& fakeBbox = ggg2->getBound();

            if (bboxCyl.intersects(fakeBbox))
            {
                //printf("NavSphere Activated\n");
                navSphereActivated = true;
                mapIdSkel[cylinderId].navSphere.activated = true;
                mapIdSkel[cylinderId].navSphere.update(mapIdSkel[cylinderId].navSphere.position, Vec4(0, 0, 0, 1));
            }

            if (!navSphereActivated)
            {
                mapIdSkel[cylinderId].navSphere.activated = false;
                mapIdSkel[cylinderId].navSphere.update(mapIdSkel[cylinderId].cylinder.center, Vec4(0, 0, 0, 1));
            }
        }
        else
        {
            Vec3 center2 = mapIdSkel[cylinderId].joints[3].position;
            center2.y() += 0.4;
            mapIdSkel[cylinderId].navSphere.update(center2, Vec4(0, 0, 0, 1));
        }

        Matrix rotMat0;
        rotMat0.makeRotate(mapIdSkel[cylinderId].cylinder.prevVec, mapIdSkel[cylinderId].cylinder.currVec);

        if (navSphereActivated)
        {
            double x, y, z;
            x = y = z = 0.0;
            double rx, ry, rz;
            rx = ry = rz = 0.0;
            double tranScale = -75;
            bool moved = false;

            if (diff.x() > 0.1 || diff.x() < -0.1)
            {
                x = diff.x() * tranScale;
                moved = true;
            }

            if (diff.y() > 0.1 || diff.y() < -0.1)
            {
                y = diff.y() * tranScale;
                moved = true;
            }

            if (diff.z() > 0.1 || diff.z() < -0.1)
            {
                z = diff.z() * tranScale;
                moved = true;
            }

            if (moved)
            {
                Quat handQuad = rotMat0.getRotate();
                double rscale = 1;
                rx = 0.0; //handQuad.x();
                ry = 0.0; //handQuad.y();
                rz = handQuad.z();

                if (rz > 0.03 || rz < -0.03)
                {
                    rz = 0.0;
                }
                else
                {
                    rz = 0.0;
                }

                Matrixd finalmat;
                Matrix view = PluginHelper::getHeadMat();
                Vec3 campos = view.getTrans();
                Vec3 trans = Vec3(x, y, z);
                trans = (trans * view) - campos;
                Matrix tmat;
                tmat.makeTranslate(trans);
                Vec3 xa = Vec3(1.0, 0.0, 0.0);
                Vec3 ya = Vec3(0.0, 1.0, 0.0);
                Vec3 za = Vec3(0.0, 0.0, 1.0);
                xa = (xa * view) - campos;
                ya = (ya * view) - campos;
                za = (za * view) - campos;
                Matrix rot;
                rot.makeRotate(rx, xa, ry, ya, rz, za);
                Matrix ctrans, nctrans;
                ctrans.makeTranslate(campos);
                nctrans.makeTranslate(-campos);
                finalmat = PluginHelper::getObjectMatrix() * nctrans * rot * tmat * ctrans;
                PluginHelper::setObjectMatrix(finalmat);
            }
        }
    }
}
}

void KinectDemo::kinectInit()
{
    // moving from points to spheres in kinect point cloud
//bang
    kinectThreaded = ConfigManager::getBool("Plugin.KinectDemo.KinectDefaultOn.KinectThreaded");
    skeletonThreaded = false;
if(!kinectThreaded)
{
    initialPointScale = ConfigManager::getFloat("Plugin.KinectDemo.KinectDefaultOn.KinectPointSize");
    cerr << "PointScale=" << initialPointScale << "\n";
    pgm1 = new osg::Program;
    pgm1->setName("Sphere");
    std::string shaderPath = ConfigManager::getEntry("Plugin.Points.ShaderPath");
    pgm1->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(shaderPath + "/Sphere.vert")));
    pgm1->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(shaderPath + "/Sphere.frag")));
    pgm1->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile(shaderPath + "/Sphere.geom")));
    pgm1->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
    pgm1->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS);
    pgm1->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
    // move camera to the kinect-person
}
//Get KinectSkeleton Offset

    kinectX = ConfigManager::getFloat("x","Plugin.KinectDemo.KinectSkeleton",0.0f);
    kinectY = ConfigManager::getFloat("y","Plugin.KinectDemo.KinectSkeleton",0.0f);
    kinectZ = ConfigManager::getFloat("z","Plugin.KinectDemo.KinectSkeleton",0.0f);
    kinectRX = ConfigManager::getFloat("rx","Plugin.KinectDemo.KinectSkeleton",0.0f);
    kinectRY = ConfigManager::getFloat("ry","Plugin.KinectDemo.KinectSkeleton",0.0f);
    kinectRZ = ConfigManager::getFloat("rz","Plugin.KinectDemo.KinectSkeleton",0.0f);
    kinectRW = ConfigManager::getFloat("rw","Plugin.KinectDemo.KinectSkeleton",0.0f);

//Show info Panel


    kShowInfoPanel = ConfigManager::getBool("Plugin.KinectDemo.KinectDefaultOn.ShowInfoPanel");
    if(kShowInfoPanel)
    {
      _kShowInfoPanel->setValue(true);
      _infoPanel->setVisible(true);

      sliderX = new MenuRangeValue("X",-1000.0,1000.0,kinectX,0.01);
      sliderX->setCallback(this);
      _infoPanel->addMenuItem(sliderX);

      sliderY = new MenuRangeValue("Y",-1000.0,1000.0,kinectY,0.01);
      sliderY->setCallback(this);
      _infoPanel->addMenuItem(sliderY);

      sliderZ = new MenuRangeValue("Z",-1000.0,1000.0,kinectZ,0.01);
      sliderZ->setCallback(this);
      _infoPanel->addMenuItem(sliderZ);
/*
      sliderRX = new MenuRangeValue("RX",-1000.0,1000.0,kinectRX,0.01);
      sliderRX->setCallback(this);
      _infoPanel->addMenuItem(sliderRX);

      sliderRY = new MenuRangeValue("RY",-1000.0,1000.0,kinectRY,0.01);
      sliderRY->setCallback(this);
      _infoPanel->addMenuItem(sliderRY);

      sliderRZ = new MenuRangeValue("RZ",-1000.0,1000.0,kinectRZ,0.01);
      sliderRZ->setCallback(this);
      _infoPanel->addMenuItem(sliderRZ);

      sliderRW = new MenuRangeValue("RW",-1000.0,1000.0,kinectRW,0.01);
      sliderRW->setCallback(this);
      _infoPanel->addMenuItem(sliderRW);
*/
    }

    float camX = ConfigManager::getFloat("x","Plugin.KinectDemo.CamStart",0.0f);
    float camY = ConfigManager::getFloat("y","Plugin.KinectDemo.CamStart",0.0f);
    float camZ = ConfigManager::getFloat("z","Plugin.KinectDemo.CamStart",0.0f);
    float camS = ConfigManager::getFloat("s","Plugin.KinectDemo.CamStart",0.0f);
    float camRX = ConfigManager::getFloat("rx","Plugin.KinectDemo.CamStart",0.0f);
    float camRY = ConfigManager::getFloat("ry","Plugin.KinectDemo.CamStart",0.0f);
    float camRZ = ConfigManager::getFloat("rz","Plugin.KinectDemo.CamStart",0.0f);
    float camRW = ConfigManager::getFloat("rw","Plugin.KinectDemo.CamStart",0.0f);
    //moveCam(273.923, 0.178878, -1.27967, 1.64388, -0.0247491, 0.294768, 0.952783, 0.0685912);
    //moveCam(camS, camX, camY, camZ, camRX, camRY, camRZ, camRW);
if(!skeletonThreaded)
{
    distanceMAX = ConfigManager::getFloat("Plugin.KinectDemo.Cylinder.Max");
    distanceMIN = ConfigManager::getFloat("Plugin.KinectDemo.Cylinder.Min");
    Skeleton::navSpheres = false;
    bcounter = 0;
    _modelFileNode1 = osgDB::readNodeFile(ConfigManager::getEntry("Plugin.KinectDemo.3DModelFolder").append("kinect_mm.obj"));
   // _modelFileNode2 = osgDB::readNodeFile(ConfigManager::getEntry("Plugin.KinectDemo.3DModelFolder").append("dumptruck.osg"));
  //  _modelFileNode5 = osgDB::readNodeFile(ConfigManager::getEntry("Plugin.KinectDemo.3DModelFolder").append("cessna.osg"));
  //  _modelFileNode4 = osgDB::readNodeFile(ConfigManager::getEntry("Plugin.KinectDemo.3DModelFolder").append("cow.osg"));
  //  _modelFileNode3 = osgDB::readNodeFile(ConfigManager::getEntry("Plugin.KinectDemo.3DModelFolder").append("robot.osg"));
    _sphereRadius = 0.07;
   //  Group* kinectgrp = new Group();
   //  kinectgrp->addChild(_modelFileNode3);
    // createSceneObject(kinectgrp);

   // createSelObj(Vec3(-0.70,  -2.0,  0.15),   string("DD"), 0.002, _modelFileNode1);
   // createSelObj(Vec3(-0.40,  -2.0,  0.15),   string("FD"), 0.007, _modelFileNode2);
   // createSelObj(Vec3(0,      -2.0,  0.15),   string("GD"), 0.02,  _modelFileNode3);
  //  createSelObj(Vec3(0.40,   -2.0,  0.15),   string("ED"), 0.02,  _modelFileNode4);
  //  createSelObj(Vec3(0.70,   -2.0,  0.15),   string("ZD"), 0.002, _modelFileNode5);

//CreateSceneObject for all Kinect Data

    createSceneObject();

}
    minDistHSV = 700;
    maxDistHSV = 5000;
    minDistHSVDepth = 300;
    maxDistHSVDepth = 6000;
if(!kinectThreaded)
{
    // precompute colors for ... mm distances
    for (int i = 0; i < 10001; i++) getColorRGB(i);

    for (int i = 0; i < 15001; i++) getColorRGBDepth(i);

    // precompute packed values for colors on depthmap
    for (int i = 0; i < 15000; i++)
    {
        // XXX
        //
        // TODO SEE IF THERE IS RED/BLUE MAP ONCE CONVERTING TO MM
        // TODO ADD MORE POINTS SEE IF LOOKS BETTER
        // TODO CHECK IF O3 MAKES A DIFFERENCE
        //
        // XXX
        //http://graphics.stanford.edu/~mdfisher/Kinect.html
        //     float valf = 0;
        //     if (i < 2047) { valf = float(1000.0 / (double(i) * -0.0030711016 + 3.3309495161));  }
        //     int val = valf;
        //int val = float(1000.0 / (double(i) * -0.0030711016 + 3.3309495161)); // kinect data to milimeters
        //cout << val << " ";
        osg::Vec4 color = getColorRGBDepth(i);
        char rrr = (char)((float)color.r() * 255.0);
        char ggg = (char)((float)color.g() * 255.0);
        char bbb = (char)((float)color.b() * 255.0);
        uint32_t packed = (((rrr << 0) | (ggg << 8) | (bbb << 16)) & 0x00FFFFFF);
        dpmap[i] = packed;
    }
}
    cout << endl << endl;
    ThirdInit();
    kinectInitialized = true;
}



void KinectDemo::createSelObj(osg::Vec3 pos, string color, float scalenumber, osg::Node* model)
{
    Matrixd scale;
    double snum = scalenumber;
    scale.makeScale(snum, snum, snum);
    MatrixTransform* modelScaleTrans = new MatrixTransform();
    modelScaleTrans->setMatrix(scale);
    modelScaleTrans->addChild(model);
    //Vec3d poz0(0, 0, 0);
    //Box* sphereShape = new Box(poz0, radius);
    //ShapeDrawable* ggg2 = new ShapeDrawable(sphereShape);
    //ggg2->setColor(_colors[KinectDemo::dc2Int(color)]);
    osg::Geode* boxGeode = new osg::Geode;
    //boxGeode->addDrawable(ggg2);
    MatrixTransform* rotate = new osg::MatrixTransform();
    Matrix rotMat;
    rotMat.makeRotate(0, 1, 0, 1);
    rotate->setMatrix(rotMat);
    //rotate->addChild(boxGeode);
    MatrixTransform* translate = new osg::MatrixTransform();
    osg::Matrixd tmat;
    tmat.makeTranslate(pos);
    translate->setMatrix(tmat);
    translate->addChild(rotate);
    _root->addChild(translate);
    selectableItems.push_back(SelectableItem(boxGeode, modelScaleTrans, translate, rotate, snum));
}


void KinectDemo::ThirdLoop()
{
if(!skeletonThreaded)
{
    // if skeletons are to be displayed in coorinates relative to the camera, position of the camera is saved in Skeleton::camPos,camRot (one for all skeletons)
    Skeleton::camPos = Vec3(kinectX, kinectY, kinectZ);
  //  Skeleton::camPos = Vec3d(0, kinectY, 0);
   float rotDegrees[3]; 
   rotDegrees[0] = 0;
   rotDegrees[1] = 0;
   rotDegrees[2] = 0;
		rotDegrees[0] = DegreesToRadians(rotDegrees[0]);
		rotDegrees[1] = DegreesToRadians(rotDegrees[1]);
		rotDegrees[2] = DegreesToRadians(rotDegrees[2]);
Quat rot = osg::Quat(rotDegrees[0], osg::Vec3d(1,0,0),rotDegrees[1], osg::Vec3d(0,1,0),rotDegrees[2], osg::Vec3d(0,0,1)); 
   // Skeleton::camRot = rot;

    if (Skeleton::moveWithCam)
    {
        //Matrixd camMat = PluginHelper::getWorldToObjectTransform(); //This will get us actual real world coordinates that the camera is at (not sure about how it does rotation)
        Matrixd camMat = PluginHelper::getHeadMat(0); //This will get us actual real world coordinates that the camera is at (not sure about how it does rotation)
        float cscale = 1; //Want to keep scale to actual Kinect which is is meters
        Vec3 camTrans = camMat.getTrans();
        Quat camQuad = camMat.getRotate();  //Rotation of cam will cause skeleton to be off center--need Fix!!
        //double xOffset = (camTrans.x() / cscale);
        //double yOffset = (camTrans.y() / cscale) + 3; //Added Offset of Skeleton so see a little ways from camera (i.e. 5 meters, works at this scale,only)
        //double zOffset = (camTrans.z() / cscale);
        double xOffset = camTrans.x();
        double yOffset = camTrans.y(); //Added Offset of Skeleton so see a little ways from camera (i.e. 5 meters, works at this scale,only)
        double zOffset = camTrans.z();
        Skeleton::camPos = Vec3d(xOffset, yOffset, zOffset);
        Skeleton::camRot = camQuad;
    }
if(!kFreezeCloud)
{
    while (skel_socket->recv(*sf))
    {
        //return;
        // remove all the skeletons that are no longer reported by the server
        for (std::map<int, Skeleton>::iterator it2 = mapIdSkel.begin(); it2 != mapIdSkel.end(); ++it2)
        {
            bool found = false;

            for (int i = 0; i < sf->skeletons_size(); i++)
            {
                if (sf->skeletons(i).skeleton_id() == it2->first)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                mapIdSkel[it2->first].detach(_pointClouds[0]->switchNode);
            }
        }

        // update all skeletons' joints' positions
        for (int i = 0; i < sf->skeletons_size(); i++)
        {
            // Skeleton reported but not in the map -> create a new one
            if (mapIdSkel.count(sf->skeletons(i).skeleton_id()) == 0)
            {
                mapIdSkel[sf->skeletons(i).skeleton_id()] = Skeleton(); ///XXX remove Skeleton(); part
               // mapIdSkel[sf->skeletons(i).skeleton_id()].attach(_root);
               cerr << "Found Skeleton\n";
                mapIdSkel[sf->skeletons(i).skeleton_id()].attach(_pointClouds[0]->switchNode);
            }

            // Skeleton previously detached (stopped being reported), but is again reported -> reattach
            if (mapIdSkel[sf->skeletons(i).skeleton_id()].attached == false)
                mapIdSkel[sf->skeletons(i).skeleton_id()].attach(_pointClouds[0]->switchNode);

            for (int j = 0; j < sf->skeletons(i).joints_size(); j++)
            {
                mapIdSkel[sf->skeletons(i).skeleton_id()].update(
                    sf->skeletons(i).joints(j).type(),
                    sf->skeletons(i).joints(j).x(),
                    sf->skeletons(i).joints(j).z(),
                    sf->skeletons(i).joints(j).y(),
                    sf->skeletons(i).joints(j).qx(),
                    sf->skeletons(i).joints(j).qz(),
                    sf->skeletons(i).joints(j).qy(),
                    sf->skeletons(i).joints(j).qw());
            }
        }
    }
}
}
    if (kShowPCloud && cloud_socket != NULL && !kinectThreaded && !kFreezeCloud)
    {
//	cerr << ".";
        float r, g, b, a;
        kinectVertices = new osg::Vec3Array;
        kinectVertices->empty();
        osg::Vec3Array* normals = new osg::Vec3Array;
        osg::Vec4Array* kinectColours = new osg::Vec4Array;
        kinectColours->empty();

        if (cloud_socket->recv(*packet))
        {
            for (int i = 0; i < packet->points_size(); i++)
            {
                osg::Vec3 ppos(packet->points(i).x() + Skeleton::camPos.x(),
                                packet->points(i).z() + Skeleton::camPos.y(),
                                packet->points(i).y() + Skeleton::camPos.z());
               // ppos *= 0.0001;
               if(i == 200000)
               {
                // cerr << "Y=" << ppos.y() << endl;
               }
                kinectVertices->push_back(ppos);

                if (useKColor)
                {
                    r = (packet->points(i).r() / 255.);
                    g = (packet->points(i).g() / 255.);
                    b = (packet->points(i).b() / 255.);
                    a = 1;
		    kinectColours->push_back(osg::Vec4f(r, g, b, a));
                    //kinectColours->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
                }
                else
                {
                    kinectColours->push_back(getColorRGB(packet->points(i).z()));
                }
            }
            if(true)
	    {
            osg::Geode* kgeode = new osg::Geode();
            kgeode->setCullingActive(false);
            osg::Geometry* nodeGeom = new osg::Geometry();
            osg::StateSet* state = nodeGeom->getOrCreateStateSet();
            state->setMode(GL_LIGHTING, StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
            nodeGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, kinectVertices->size()));
            osg::VertexBufferObject* vboP = nodeGeom->getOrCreateVertexBufferObject();
            vboP->setUsage(GL_STREAM_DRAW);
            nodeGeom->setUseDisplayList(false);
            nodeGeom->setUseVertexBufferObjects(true);
            nodeGeom->setVertexArray(kinectVertices);
            nodeGeom->setColorArray(kinectColours);
            nodeGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
            kgeode->addDrawable(nodeGeom);
            kgeode->dirtyBound();
            //if (kinectgrp != NULL) _root->removeChild(kinectgrp);
            kinectgrp->removeChild(0, 1);
            kinectgrp->addChild(kgeode);
	    }
	    else
            {
            	osg::Geode* kgeode = new osg::Geode();
            	kgeode->setCullingActive(false);
            	osg::Geometry* geometry = new osg::Geometry();
                    geometry->setUseDisplayList(false);
                    geometry->setUseVertexBufferObjects(true);
                    // geometry->setVertexArray(kinectVertices);
                    geometry->setVertexArray(kinectVertices);
                    geometry->setNormalArray(normals);
                    geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
                    geometry->setColorArray(kinectColours);
                    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
                    geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, kinectVertices->size()));
                    //_root->removeChild(pointGeode);
                    //pointGeode = new Geode();
                    StateSet* ss = kgeode->getOrCreateStateSet();
                    ss->setMode(GL_LIGHTING, StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
                    kgeode->addDrawable(geometry);
            	    kinectgrp->removeChild(0, 1);
                    kinectgrp->addChild(kgeode);
                    //_root->addChild(pointGeode);

	    }
        }
    }

    if (kShowColor && false)
    {
        if (color_socket->recv(*cm))
        {
            for (int y = 0; y < 480; y++)
            {
                for (int x = 0; x < 640; x++)
                {
                    uint32_t packed = cm->pixels(y * 640 + x);
                    color_pixels[640 * (479 - y) + x] = packed;
                }
            }
        }
    }

    if (kShowDepth && false)
    {
        if (depth_socket->recv(*dm))
        {
            for (int y = 0; y < 480; y++)
            {
                for (int x = 0; x < 640; x++)
                {
                    int val = dm->depths(y * 640 + x);
                    //              if (dpmap.count(val) == 0)
                    //              {
                    //                  osg::Vec4 color = getColorRGBDepth(val);
                    //                  char rrr = (char)((float)color.r() * 255.0);
                    //                  char ggg = (char)((float)color.g() * 255.0);
                    //                  char bbb = (char)((float)color.b() * 255.0);
                    //                  uint32_t packed = (((rrr << 0) | (ggg << 8) | (bbb << 16)) & 0x00FFFFFF);
                    //                  dpmap[val] = packed;
                    //              }
                    depth_pixels[640 * (479 - y) + x] = dpmap[val];//packed;
                }
            }
        }
    }
}

void KinectDemo::cloudOff()
{
if(!kinectThreaded)
{
    if (cloud_socket) {
        delete cloud_socket;
        cloud_socket = NULL;
    }

    if (kinectgrp)
    {
        pgm1->ref();
        kinectgrp->removeChild(0, 1);
       // _root->removeChild(kinectgrp);
       SceneManager::instance()->getScene()->removeChild(kinectgrp);
        kinectgrp = NULL;
    }
}
else
{
        _cloudThread->quit();
        _cloudThread->cancel();
}
}

void KinectDemo::cloudOn()
{

if(kShowPCloud)
{
  if(!kinectThreaded)
  { 
   if (!cloud_socket) cloud_socket = new SubSocket<RemoteKinect::PointCloud> (context, ConfigManager::getEntry("Plugin.KinectDemo.KinectServer.PointCloud"));

    kinectgrp = new osg::Group();
    osg::StateSet* state = kinectgrp->getOrCreateStateSet();
    state->setAttribute(pgm1);
    state->addUniform(new osg::Uniform("pointScale", initialPointScale));
    state->addUniform(new osg::Uniform("globalAlpha", 1.0f));
    float pscale = initialPointScale;
    osg::Uniform*  _scaleUni = new osg::Uniform("pointScale", 1.0f * pscale);
    kinectgrp->getOrCreateStateSet()->addUniform(_scaleUni);
    _pointClouds[0]->switchNode->addChild(kinectgrp);
//bangKN

   // SceneManager::instance()->getScene()->addChild(kinectgrp);
   // _root->addChild(kinectgrp);
  }
  else
  {
	cout << "Starting Thread\n";   
        _cloudThread = new CloudManager();
	cout <<"Started\n";
        _cloudThread->start();
  }
}
}

void KinectDemo::colorOff()
{
    _root->removeChild(bitmaptransform);

    if (color_socket) {
        delete color_socket;
        color_socket = NULL;
    }
}

void KinectDemo::colorOn()
{
    bitmaptransform = new osg::MatrixTransform();
    osg::Vec3 pos(0, 0, 0);
    pos = Vec3(1.0, -2, 0.4);
    osg::Matrixd tmat;
    tmat.makeTranslate(pos);
    osg::Matrixd rmat;
    rmat.makeRotate(45, 1, 0, 0);
    osg::Matrix combined;
    combined.makeIdentity();
    combined.preMult(tmat);
    combined.preMult(rmat);
    bitmaptransform->setMatrix(combined);
    image = new osg::Image();
    pTex = new osg::Texture2D();
    pGeode = new osg::Geode();
    pStateSet = pGeode->getOrCreateStateSet();
    pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    pStateSet->setTextureAttributeAndModes(0, pTex, osg::StateAttribute::ON);
    pGeode->setStateSet(pStateSet);
    geometry = new osg::Geometry();
    pGeode->addDrawable(geometry);
    vertexArray = new osg::Vec3Array();
    vertexArray->push_back(osg::Vec3(0, 0, 0));
    vertexArray->push_back(osg::Vec3(640 * 0.0015, 0, 0));
    vertexArray->push_back(osg::Vec3(640 * 0.0015, 480 * 0.0015, 0));
    vertexArray->push_back(osg::Vec3(0, 480 * 0.0015, 0));
    geometry->setVertexArray(vertexArray);
    colorArray = new osg::Vec4Array();
    colorArray->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
    geometry->setColorArray(colorArray);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    texCoordArray = new osg::Vec2Array();
    texCoordArray->push_back(osg::Vec2(0.f, 0.f));
    texCoordArray->push_back(osg::Vec2(1.f, 0.f));
    texCoordArray->push_back(osg::Vec2(1.f, 1.f));
    texCoordArray->push_back(osg::Vec2(0.f, 1.f));
    geometry->setTexCoordArray(0, texCoordArray);
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 4));
    bitmaptransform->addChild(pGeode);
    _root->addChild(bitmaptransform);
    color_socket = new SubSocket<RemoteKinect::ColorMap> (context, ConfigManager::getEntry("Plugin.KinectDemo.KinectServer.ColorMap"));
}

void KinectDemo::navOff()
{
    for (std::map<int, Skeleton>::iterator it = mapIdSkel.begin(); it != mapIdSkel.end(); ++it)
    {
        it->second.navSphere.translate->ref();
        _root->removeChild(it->second.navSphere.translate);
    }

    Skeleton::navSpheres = false;
}

void KinectDemo::navOn()
{
    for (std::map<int, Skeleton>::iterator it = mapIdSkel.begin(); it != mapIdSkel.end(); ++it)
    {
        _root->addChild(it->second.navSphere.translate);
    }

    Skeleton::navSpheres = true;
}

void KinectDemo::depthOff()
{
    if (depth_socket) {
        delete depth_socket;
        depth_socket = NULL;
    }

    _root->removeChild(depthBitmaptransform);
}

void KinectDemo::depthOn()
{
    depthBitmaptransform = new osg::MatrixTransform();
    osg::Vec3 pos(0, 0, 0);
    pos = Vec3(-2.0, -2, 0.4);
    osg::Matrixd tmat;
    tmat.makeTranslate(pos);
    osg::Matrixd rmat;
    rmat.makeRotate(45, 1, 0, 0);
    osg::Matrix combined;
    combined.makeIdentity();
    combined.preMult(tmat);
    combined.preMult(rmat);
    depthBitmaptransform->setMatrix(combined);
    depthImage = new osg::Image();
    depthPTex = new osg::Texture2D();
    depthPGeode = new osg::Geode();
    depthPStateSet = depthPGeode->getOrCreateStateSet();
    depthPStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    depthPStateSet->setTextureAttributeAndModes(0, depthPTex, osg::StateAttribute::ON);
    depthPGeode->setStateSet(depthPStateSet);
    depthGeometry = new osg::Geometry();
    depthPGeode->addDrawable(depthGeometry);
    depthVertexArray = new osg::Vec3Array();
    depthVertexArray->push_back(osg::Vec3(0, 0, 0));
    depthVertexArray->push_back(osg::Vec3(640 * 0.0015, 0, 0));
    depthVertexArray->push_back(osg::Vec3(640 * 0.0015, 480 * 0.0015, 0));
    depthVertexArray->push_back(osg::Vec3(0, 480 * 0.0015, 0));
    depthGeometry->setVertexArray(depthVertexArray);
    depthColorArray = new osg::Vec4Array();
    depthColorArray->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
    depthGeometry->setColorArray(depthColorArray);
    depthGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    depthTexCoordArray = new osg::Vec2Array();
    depthTexCoordArray->push_back(osg::Vec2(0.f, 0.f));
    depthTexCoordArray->push_back(osg::Vec2(1.f, 0.f));
    depthTexCoordArray->push_back(osg::Vec2(1.f, 1.f));
    depthTexCoordArray->push_back(osg::Vec2(0.f, 1.f));
    depthGeometry->setTexCoordArray(0, depthTexCoordArray);
    depthGeometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 4));
    depthBitmaptransform->addChild(depthPGeode);
    _root->addChild(depthBitmaptransform);
    depth_socket = new SubSocket<RemoteKinect::DepthMap> (context, ConfigManager::getEntry("Plugin.KinectDemo.KinectServer.DepthMap"));
}

void KinectDemo::moveWithCamOff()
{
    Skeleton::moveWithCam = false;
}
void KinectDemo::moveWithCamOn()
{
    Skeleton::moveWithCam = true;
}

void KinectDemo::moveCam(double bscale, double x, double y, double z, double o1, double o2, double o3, double o4)
{
    Vec3 trans = Vec3(x, y, z) * bscale;
    Matrix tmat;
    tmat.makeTranslate(trans);
    Matrix rot;
    rot.makeRotate(osg::Quat(o1, o2, o3, o4));
    Matrixd gotoMat = rot * tmat;
    Matrixd camMat = PluginHelper::getObjectMatrix();
    float cscale = PluginHelper::getObjectScale();
    Vec3 camTrans = camMat.getTrans();
    Quat camQuad = camMat.getRotate();
    PluginHelper::setObjectMatrix(gotoMat);
    PluginHelper::setObjectScale(bscale);
}

void KinectDemo::showCameraImage()
{
    // draw color_pixels somewhere
    image->setImage(640, 480, 1, GL_RGBA16, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*) &color_pixels[0], osg::Image::NO_DELETE);
    pTex->setImage(image);
    //    for (std::map<int, Skeleton>::iterator it = mapIdSkel.begin(); it != mapIdSkel.end(); ++it)
    //    {
    //        pos = Vec3(it->second.joints[1].position.x() , it->second.joints[1].position.y() , it->second.joints[1].position.z());
    //    }
}

void KinectDemo::showDepthImage()
{
    // draw color_pixels somewhere
    depthImage->setImage(640, 480, 1, GL_RGBA16, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*) &depth_pixels[0], osg::Image::NO_DELETE);
    depthPTex->setImage(depthImage);
    //    for (std::map<int, Skeleton>::iterator it = mapIdSkel.begin(); it != mapIdSkel.end(); ++it)
    //    {
    //        pos = Vec3(it->second.joints[1].position.x() , it->second.joints[1].position.y() , it->second.joints[1].position.z());
    //    }
}

void KinectDemo::kinectOff()
{
    //    TrackingManager::instance()->setUpdateHeadTracking(true);
    printf("turning kinect off\n");

    for (int i = 0; i < selectableItems.size(); i++)
        _root->removeChild(selectableItems[i].mt);

    cloudOff();
    colorOff();
    depthOff();

    if (skel_socket) {
        delete skel_socket;
        skel_socket = NULL;
    }

//    _kShowColor->setValue(false);
    _kShowDepth->setValue(false);
    _kShowPCloud->setValue(false);
    _kNavSpheres->setValue(false);
    this->menuCallback(_kShowColor);
    this->menuCallback(_kShowDepth);
    this->menuCallback(_kShowPCloud);
    this->menuCallback(_kNavSpheres);

    for (std::map<int, Skeleton>::iterator it = mapIdSkel.begin(); it != mapIdSkel.end(); ++it)
    {
        it->second.detach(_pointClouds[0]->switchNode);
    }

    mapIdSkel.clear();
    selectableItems.clear();
}
void KinectDemo::ThirdInit()
{
if(!skeletonThreaded)
{ 
    sf = new RemoteKinect::SkeletonFrame();
    skel_socket = new SubSocket<RemoteKinect::SkeletonFrame> (context, ConfigManager::getEntry("Plugin.KinectDemo.KinectServer.Skeleton"));

}
else
{
	cout << "Starting Skeleton Thread\n";   
      //  _skeletonThread = new SkeletonManager();
	cout <<"Started\n";
      //  _skeletonThread->start();
}
if(!kinectThreaded)
{
    packet = new RemoteKinect::PointCloud();

}
    //cm = new RemoteKinect::ColorMap();
    //dm = new RemoteKinect::DepthMap();
    //cloud_socket = NULL;
    cloudOn();

}

osg::Vec4f KinectDemo::getColorRGB(int dist)
{
    if (distanceColorMap.count(dist) == 0) // that can be commented out after precomputing completely if the range of Z is known (and it is set on the server side)
    {
        float r, g, b;
        float h = depth_to_hue(minDistHSV, dist, maxDistHSV);
        HSVtoRGB(&r, &g, &b, h, 1, 1);
        distanceColorMap[dist] = osg::Vec4f(r, g, b, 1);
    }

    return distanceColorMap[dist];
}
osg::Vec4f KinectDemo::getColorRGBDepth(int dist)
{
    if (distanceColorMapDepth.count(dist) == 0) // that can be commented out after precomputing completely if the range of Z is known (and it is set on the server side)
    {
        float r, g, b;
        float h = depth_to_hue(minDistHSVDepth, dist, maxDistHSVDepth);
        HSVtoRGB(&r, &g, &b, h, 1, 1);
        distanceColorMapDepth[dist] = osg::Vec4f(r, g, b, 1);
    }

    return distanceColorMapDepth[dist];
}

void KinectDemo::checkHandsIntersections(int skel_id)
{
    Skeleton* skel = &mapIdSkel[skel_id];
    Sphere* handSphere = new Sphere(skel->joints[M_LHAND].position, 0.1);
    ShapeDrawable* ggg3 = new ShapeDrawable(handSphere);
    const osg::BoundingBox& bboxHandL = ggg3->getBound();
    handSphere = new Sphere(skel->joints[M_RHAND].position, 0.1);
    ggg3 = new ShapeDrawable(handSphere);
    const osg::BoundingBox& bboxHandR = ggg3->getBound();

    for (int j = 0; j < selectableItems.size(); j++)
    {
        // fake sphere to easily calculate boundary
        Vec3 center2 = Vec3(0, 0, 0) * (selectableItems[j].mt->getMatrix());
        Box* fakeSphere = new Box(center2, 0.35 * selectableItems[j].scale);
        ShapeDrawable* ggg2 = new ShapeDrawable(fakeSphere);
        const osg::BoundingBox& fakeBbox = ggg2->getBound();

        if (bboxHandL.intersects(fakeBbox) /*&& selectableItems[j].lock == -1*/ && skel->leftHandBusy == false)
        {
            selectableItems[j].lockTo(skel_id);
            selectableItems[j].lockType = 1;
            skel->leftHandBusy = true;
            break; // lock only one object
        }

        if (bboxHandR.intersects(fakeBbox) /*&& selectableItems[j].lock == -1*/ && skel->rightHandBusy == false)
        {
            selectableItems[j].lockTo(skel_id);
            selectableItems[j].lockType = 2;
            skel->rightHandBusy = true;
            break; // lock only one object
        }

        if (selectableItems[j].lock == skel_id && (selectableItems[j].lockType == 1))
        {
            if (bboxHandL.intersects(fakeBbox) == false)
            {
                selectableItems[j].unlock();
                skel->leftHandBusy = false;
            }
        }

        if (selectableItems[j].lock == skel_id && (selectableItems[j].lockType == 2))
        {
            if (bboxHandR.intersects(fakeBbox) == false)
            {
                selectableItems[j].unlock();
                skel->rightHandBusy = false;
            }
        }
    }
}

void KinectDemo::updateInfoPanel()
{

    std::stringstream ss;
//kinectUsers

if(cloud_socket)
{
 if(cloud_socket->recv(*packet))
 {



 }



}
  //  ss << "FPS: " << navSphereTimer << endl;
   // ss << "X: " << kinectX << endl;
        Matrixd camMat = PluginHelper::getHeadMat(0); //This will get us actual real world coordinates that the camera is at (not sure about how it does rotation)
        float cscale = 1; //Want to keep scale to actual Kinect which is is meters
        Vec3 camTrans = camMat.getTrans();
        Quat camQuad = camMat.getRotate();  //Rotation of cam will cause skeleton to be off center--need Fix!!
        double xOffset = camTrans.x();
        double yOffset = camTrans.y(); //Added Offset of Skeleton so see a little ways from camera (i.e. 5 meters, works at this scale,only)
        double zOffset = camTrans.z();
        double rxOffset = camQuad.x();
        double ryOffset = camQuad.y();
        double rzOffset = camQuad.z();
        double rwOffset = camQuad.w();
   //  ss << "HeadMat X:" << xOffset << " Y:" << yOffset << " Z:" << zOffset << " RX:" << rxOffset << " RY:" << ryOffset << " RZ:" << rzOffset << " RW:" << rwOffset << endl;
        
        Matrixd handMat = PluginHelper::getHandMat(0); //This will get us actual real world coordinates that the camera is at (not sure about how it does rotation)
        Vec3 handTrans = handMat.getTrans();
        Quat handQuad = handMat.getRotate();  //Rotation of cam will cause skeleton to be off center--need Fix!!
        xOffset = handTrans.x();
        yOffset = handTrans.y(); //Added Offset of Skeleton so see a little ways from camera (i.e. 5 meters, works at this scale,only)
        zOffset = handTrans.z();
        rxOffset = handQuad.x();
        ryOffset = handQuad.y();
        rzOffset = handQuad.z();
        rwOffset = handQuad.w();

   //  ss << "HandMat X:" << xOffset << " Y:" << yOffset << " Z:" << zOffset << " RX:" << rxOffset << " RY:" << ryOffset << " RZ:" << rzOffset << " RW:" << rwOffset << endl;


//GetSkel
ss << "Skeletons Tracked: " << sf->skeletons_size() << endl;

    std::map< osg::ref_ptr<osg::Geode>, int >::iterator iter;

    for (std::map<int, Skeleton>::iterator it = mapIdSkel.begin(); it != mapIdSkel.end(); ++it)
    {
        int sk_id = it->first;
        Skeleton* sk = &(it->second);
        Vec3 lhand = sk->joints[M_LHAND].position;
        Vec3 rhand = sk->joints[M_RHAND].position;
        Vec3 head = sk->joints[M_HEAD].position;
        ss << "kLeftHand\n" << "X:" << lhand.x() << "\n" << "Y:" << lhand.y() << "\n" << "Z:" << lhand.z() << "\n";
        ss << "kRightHand\n" << "X:" << rhand.x() << "\n" << "Y:" << rhand.y() << "\n" << "Z:" << rhand.z() << "\n";
        ss << "kHead\n" << "X:" << head.x() << "\n" << "Y:" << head.y() << "\n" << "Z:" << head.z() << "\n";
        //TODO:Only first Skeleton for now!
        break;
    }







    _infoPanel->updateTabWithText("Info", ss.str());
}
void KinectDemo::ExportPointCloud()
{

                    printf("Triggered\n");
                    osg::Vec3Array* vertices = kinectVertices;
                    osg::Vec4Array* colours = kinectColours;
                    FILE* pFile;
                    string file = ConfigManager::getEntry("Plugin.KinectDemo.3DModelFolder").append("kinectDump.ply");
                    pFile = fopen(file.c_str(), "w");
                    fprintf(pFile, "ply\nformat ascii 1.0\ncomment VCGLIB generated\n");
                    fprintf(pFile, "element vertex %i\n", vertices->size());
                    fprintf(pFile, "property float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue\nproperty uchar alpha\nelement face 0\nproperty list uchar int vertex_indices\nend_header\n");

                    for (int i = 0; i < vertices->size() ; i++)
                    {
                        double x = vertices->at(i).x(); // * 1000;
                        double y = vertices->at(i).y();// * 1000;
                        double z = vertices->at(i).z();//* 1000;
                        double r = colours->at(i).r() * 255;
                        double g = colours->at(i).g() * 255;
                        double b = colours->at(i).b() * 255;
                        double a = colours->at(i).a();
                        int ri = int (r);
                        int gi = int (g);
                        int bi = int (b);
                        int ai = int (a);
                        fprintf(pFile, "%f %f %f %i %i %i %i\n", x, y, z, ri, gi, bi, ai);

                    }

                    fclose(pFile);
                
}
void KinectDemo::createSceneObject()
{
cerr << "Creating SceneObject\n";           
	    PointCloud* newModel = new PointCloud();
            _pointClouds.push_back(newModel);
            string name = "test";
           int i = _pointClouds.size() -1;
           if(i == -1) return;
           // float currentScale = _pointClouds[i]->scale;
            float currentScale = 1;
      
	    SceneObject * so;
	    so = new SceneObject(name, false, false, false, true, false);
	    osg::Switch* switchNode = new osg::Switch();
	    so->addChild(switchNode);
	    PluginHelper::registerSceneObject(so,"Test");
	    so->attachToScene();
//Add currentNode to switchNode
     // _models3d[i]->currentModelNode = modelNode; 
     cerr << "here\n"; 
//	switchNode->addChild(kinectgrp);
        if(i == 0)
        {
//	switchNode->addChild(_modelFileNode4);

     if(false)
     {
	    Vec3d poz0(kinectX, kinectY, kinectZ);
	    Box* sphereShape = new Box(poz0, 50.0);
	    ShapeDrawable* ggg2 = new ShapeDrawable(sphereShape);
	    ggg2->setColor(Vec4(1,1,1,1));
	    osg::Geode* boxGeode = new osg::Geode;
	    boxGeode->addDrawable(ggg2);

	    switchNode->addChild(boxGeode);
     }

            if(ConfigManager::getBool("Plugin.KinectDemo.ShowKinectModel"))
            {
              //Loads Kinect Obj file
		    Matrixd scale;
		    double snum = 1;
		    scale.makeScale(snum, snum, snum);
		    MatrixTransform* modelScaleTrans = new MatrixTransform();
		    modelScaleTrans->setMatrix(scale);
		    modelScaleTrans->addChild(_modelFileNode1);
		    MatrixTransform* rotate = new osg::MatrixTransform();


                   float rotDegrees[3];
		   rotDegrees[0] = -90;
		   rotDegrees[1] = 0;
		   rotDegrees[2] = 180;
			rotDegrees[0] = DegreesToRadians(rotDegrees[0]);
			rotDegrees[1] = DegreesToRadians(rotDegrees[1]);
			rotDegrees[2] = DegreesToRadians(rotDegrees[2]);
			Quat rot = osg::Quat(rotDegrees[0], osg::Vec3d(1,0,0),rotDegrees[1], osg::Vec3d(0,1,0),rotDegrees[2], osg::Vec3d(0,0,1)); 

		    Matrix rotMat;
		    rotMat.makeRotate(rot);
		    rotate->setMatrix(rotMat);
		    rotate->addChild(modelScaleTrans);
		    MatrixTransform* translate = new osg::MatrixTransform();
		    osg::Matrixd tmat;
                    Vec3 pos = Vec3(kinectX,kinectY,kinectZ);
		    tmat.makeTranslate(pos);
		    translate->setMatrix(tmat);
		    translate->addChild(rotate);
	            switchNode->addChild(translate);



            }
            if(ConfigManager::getBool("Plugin.KinectDemo.ShowScreenFrames"))
            {
              //Draw Configured Screens
              int numWindows =ScreenConfig::instance()->getNumWindows();
              float width;
              float height;
              float h;
              float p;
              float r;
              Vec3 offsetScreen;
              cerr << "NumWindows: " << numWindows << endl;
              //TODO:Get Screen Info from Config file
              for (int j = 0; j < numWindows; j++)
              {
                ScreenInfo* si = ScreenConfig::instance()->getScreenInfo(j);
                 width = si->width;
                 height = si->height;
                 h = si->h;
                 p = si->p;
                 r = si->r;
                 offsetScreen = si->xyz;
              
        	//Create Quad Face
	//	float width = 300;
	//	float height = 500;
	         Vec3 pos = Vec3(-(width/2),0,-(height/2));
                 Vec4f color = Vec4f(0, 0.42, 0.92, 1);
                 //Ofset Pos
                 pos += offsetScreen; 
		    osg::Geometry * geo = new osg::Geometry();
		    osg::Vec3Array* verts = new osg::Vec3Array();
		    verts->push_back(pos);
		    verts->push_back(pos + osg::Vec3(width,0,0));
		    verts->push_back(pos + osg::Vec3(width,0,height));
		    verts->push_back(pos + osg::Vec3(0,0,height));

		    geo->setVertexArray(verts);

		    osg::DrawElementsUInt * ele = new osg::DrawElementsUInt(
			    osg::PrimitiveSet::QUADS,0);

		    ele->push_back(0);
		    ele->push_back(1);
		    ele->push_back(2);
		    ele->push_back(3);
		    geo->addPrimitiveSet(ele);

		    Geode* fgeode = new Geode();
		    StateSet* state(fgeode->getOrCreateStateSet());
		    Material* mat(new Material);

		    mat->setColorMode(Material::DIFFUSE);
		    mat->setDiffuse(Material::FRONT_AND_BACK, color);
		    state->setAttribute(mat);
		    state->setRenderingHint(StateSet::TRANSPARENT_BIN);
		    state->setMode(GL_BLEND, StateAttribute::ON);
		    state->setMode(GL_LIGHTING, StateAttribute::OFF);
		    osg::PolygonMode* polymode = new osg::PolygonMode;
		    polymode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
		    state->setAttributeAndModes(polymode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
		    fgeode->setStateSet(state);
	  
		   // _annotations[inc]->geo = geo;
		    fgeode->addDrawable(geo);
                   float rotDegrees[3];
		   rotDegrees[0] = h;
		   rotDegrees[1] = p;
		   rotDegrees[2] = r;
			rotDegrees[0] = DegreesToRadians(rotDegrees[0]);
			rotDegrees[1] = DegreesToRadians(rotDegrees[1]);
			rotDegrees[2] = DegreesToRadians(rotDegrees[2]);
			Quat rot = osg::Quat(rotDegrees[0], osg::Vec3d(1,0,0),rotDegrees[1], osg::Vec3d(0,1,0),rotDegrees[2], osg::Vec3d(0,0,1)); 

		    MatrixTransform* rotate = new osg::MatrixTransform();
		    Matrix rotMat;
		    rotMat.makeRotate(rot);
		    rotate->setMatrix(rotMat);
		    rotate->addChild(fgeode);

	            switchNode->addChild(rotate);
                 }
            if(ConfigManager::getBool("Plugin.KinectDemo.ShowKinectFOV"))
            {
              //Draw Kinect FOV
              float width;
              float height;
              Vec3 offsetScreen = Vec3(0,500,0);
              Vec3 pos;
              Vec4f color = Vec4f(0, 0.42, 0.92, 1);
        	//Create Quad Face
		width = 543;
		height = 394;
	        pos = Vec3(-(width/2),0,-(height/2));
                 pos += Vec3(kinectX,kinectY,kinectZ); 
                 pos += offsetScreen;
		    osg::Geometry * geo = new osg::Geometry();
		    osg::Vec3Array* verts = new osg::Vec3Array();
		    verts->push_back(pos);
		    verts->push_back(pos + osg::Vec3(width,0,0));
		    verts->push_back(pos + osg::Vec3(width,0,height));
		    verts->push_back(pos + osg::Vec3(0,0,height));
                //do it Again
		width = 3800.6;
		height = 2756;
                offsetScreen = Vec3(0,3500,0);
	        pos = Vec3(-(width/2),0,-(height/2));
                 pos += Vec3(kinectX,kinectY,kinectZ); 
                 pos += offsetScreen;
		    verts->push_back(pos);
		    verts->push_back(pos + osg::Vec3(width,0,0));
		    verts->push_back(pos + osg::Vec3(width,0,height));
		    verts->push_back(pos + osg::Vec3(0,0,height));
               //....................................


                    int size = verts->size() / 2;
                    
            Geometry* geom = new Geometry();
            Geometry* tgeom = new Geometry();
            Geode* fgeode = new Geode();
            Geode* lgeode = new Geode();
            geom->setVertexArray(verts);
            tgeom->setVertexArray(verts);

            for (int n = 0; n < size; n++)
            {
                DrawElementsUInt* face = new DrawElementsUInt(PrimitiveSet::QUADS, 0);
                face->push_back(n);
                face->push_back(n + size);
                face->push_back(((n + 1) % size) + size);
                face->push_back((n + 1) % size);
                geom->addPrimitiveSet(face);
            }
             
            StateSet* state(fgeode->getOrCreateStateSet());
            Material* mat(new Material);
            mat->setColorMode(Material::DIFFUSE);
            mat->setDiffuse(Material::FRONT_AND_BACK, color);
            state->setAttribute(mat);
            state->setRenderingHint(StateSet::OPAQUE_BIN);
            state->setMode(GL_BLEND, StateAttribute::ON);
            state->setMode(GL_LIGHTING, StateAttribute::OFF);
            osg::PolygonMode* polymode = new osg::PolygonMode;
            polymode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
            state->setAttributeAndModes(polymode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
            fgeode->setStateSet(state);
            fgeode->addDrawable(geom);
                    if(false)
                    {
		    geo->setVertexArray(verts);

		    osg::DrawElementsUInt * ele = new osg::DrawElementsUInt(
			    osg::PrimitiveSet::QUADS,0);

		    ele->push_back(0);
		    ele->push_back(1);
		    ele->push_back(2);
		    ele->push_back(3);


		    ele->push_back(4);
		    ele->push_back(5);
		    ele->push_back(6);
		    ele->push_back(7);
		    geo->addPrimitiveSet(ele);

		    Geode* fgeode = new Geode();
		    StateSet* state(fgeode->getOrCreateStateSet());
		    Material* mat(new Material);

		    mat->setColorMode(Material::DIFFUSE);
		    mat->setDiffuse(Material::FRONT_AND_BACK, color);
		    state->setAttribute(mat);
		    state->setRenderingHint(StateSet::TRANSPARENT_BIN);
		    state->setMode(GL_BLEND, StateAttribute::ON);
		    state->setMode(GL_LIGHTING, StateAttribute::OFF);
		    osg::PolygonMode* polymode = new osg::PolygonMode;
		    polymode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
		    state->setAttributeAndModes(polymode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
		    fgeode->setStateSet(state);
	  
		   // _annotations[inc]->geo = geo;
		    fgeode->addDrawable(geo);
                   }
                   float rotDegrees[3];
		   rotDegrees[0] = 0;
		   rotDegrees[1] = 0;
		   rotDegrees[2] = 0;
			rotDegrees[0] = DegreesToRadians(rotDegrees[0]);
			rotDegrees[1] = DegreesToRadians(rotDegrees[1]);
			rotDegrees[2] = DegreesToRadians(rotDegrees[2]);
			Quat rot = osg::Quat(rotDegrees[0], osg::Vec3d(1,0,0),rotDegrees[1], osg::Vec3d(0,1,0),rotDegrees[2], osg::Vec3d(0,0,1)); 

		    MatrixTransform* rotate = new osg::MatrixTransform();
		    Matrix rotMat;
		    rotMat.makeRotate(rot);
		    rotate->setMatrix(rotMat);
		    rotate->addChild(fgeode);

	            switchNode->addChild(rotate);
                 }
            }



        }
      _pointClouds[i]->switchNode = switchNode;

//Add menu system
	    so->setNavigationOn(true);
	    so->setMovable(false);
	    so->addMoveMenuItem();
	    so->addNavigationMenuItem();
            float min = 0.0001;
            float max = 1;
            so->addScaleMenuItem("Scale",min,max,currentScale);
	    SubMenu * sm = new SubMenu("Position");
	    so->addMenuItem(sm);

	    MenuButton * mb;
	    mb = new MenuButton("Load");
	    mb->setCallback(this);
	    sm->addItem(mb);

	    SubMenu * savemenu = new SubMenu("Save");
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

            MenuCheckbox * mc;
	    mc = new MenuCheckbox("Active",false);
	    mc->setCallback(this);
	    so->addMenuItem(mc);
            _pointClouds[i]->activeMap = mc;

            
	    mc = new MenuCheckbox("Visible",true);
	    mc->setCallback(this);
	    so->addMenuItem(mc);
            _pointClouds[i]->visibleMap = mc;
            _pointClouds[i]->visible = true;

            float rValue = 0;
            min = -1;
            max = 1;
            MenuRangeValue* rt = new MenuRangeValue("rx",min,max,rValue);
            rt->setCallback(this);
	    so->addMenuItem(rt);
            _pointClouds[i]->rxMap = rt;

            rt = new MenuRangeValue("ry",min,max,rValue);
            rt->setCallback(this);
	    so->addMenuItem(rt);
            _pointClouds[i]->ryMap = rt;

            rt = new MenuRangeValue("rz",min,max,rValue);
            rt->setCallback(this);
	    so->addMenuItem(rt);
            _pointClouds[i]->rzMap = rt;
/*
	    mc = new MenuCheckbox("Panel Visible",true);
	    mc->setCallback(this);
	    so->addMenuItem(mc);
 //           _query[q]->artifacts[inc]->model->pVisibleMap = mc;
           // _query[q]->artifacts[inc]->model->pVisible = true;
*/
//Quat currentRot = _pointClouds[i]->rot;
//Vec3 currentPos = _pointClouds[i]->pos;
//Vec3 orig = currentPos; 
//cerr << "Pos: " << orig.x() << " " << orig.y() << " " << orig.z() << "\n";

// so->setPosition(currentPos);     
 so->setScale(1);
 if(i == 0)
 {








   Vec3 currentPos = Vec3(0,0,0);
   float rotDegrees[3];
   rotDegrees[0] = 0;
   rotDegrees[1] = 0;
   rotDegrees[2] = 180;
		rotDegrees[0] = DegreesToRadians(rotDegrees[0]);
		rotDegrees[1] = DegreesToRadians(rotDegrees[1]);
		rotDegrees[2] = DegreesToRadians(rotDegrees[2]);
Quat rot = osg::Quat(rotDegrees[0], osg::Vec3d(1,0,0),rotDegrees[1], osg::Vec3d(0,1,0),rotDegrees[2], osg::Vec3d(0,0,1)); 
 //so->setRotation(rot);     
//so->setPosition(currentPos);     
 }
    _pointClouds[i]->so = so;
    _pointClouds[i]->pos = so->getPosition();
    _pointClouds[i]->rot = so->getRotation();
    _pointClouds[i]->active = false;
    _pointClouds[i]->loaded = true;



}
void KinectDemo::sendEvents()
{
    cerr << "Sending Event\n";
    TrackingManager * tConfig = TrackingManager::instance();
    TrackerPlugin::TrackerPlugin* _trackerSystem;
    
    cerr << "Total Tracking Systems=" << tConfig->getNumTrackingSystems() << "\n";
    for (int i=1; i < tConfig->getNumTrackingSystems(); i++)
    {
      //TrackerBase
      //Currently testing with first found tracking system
       _trackerSystem = dynamic_cast<TrackerPlugin::TrackerPlugin *> (tConfig->getTrackingSystem(i));
        if (_trackerSystem != NULL)
            break;
    }
    if (_trackerSystem == NULL)
    {
        std::cerr<<"Cannot initialize tracker.\n";
        
    }
    else
    {
       int numButtons = _trackerSystem->getNumButtons();
       int numBodies = _trackerSystem->getNumBodies();
       cerr << "Buttons=" << numButtons << " Bodies=" << numBodies << endl;

        //Test updating Body
        if(true)
        {
	TrackerBase::TrackedBody* body = _trackerSystem->getBody(0);
        	
                float x; ///< position x
                float y; ///< position y
                float z; ///< position z
                float qx; ///< rotation x (quat)
                float qy; ///< rotation y (quat)
                float qz; ///< rotation z (quat)
                float qw; ///< rotation w (quat)
		  x = body->x;
		  y = body->y;
		  z = body->z;
		  cerr << "Body:" << x << "," << y << "," << z << endl;
		 float rotDegrees[3];
		 rotDegrees[0] = 0; 
		 rotDegrees[1] = 0; 
		 rotDegrees[2] = 5; 
                //Convert Degrees to Radians
		rotDegrees[0] = DegreesToRadians(rotDegrees[0]);
		rotDegrees[1] = DegreesToRadians(rotDegrees[1]);
		rotDegrees[2] = DegreesToRadians(rotDegrees[2]);
		osg::Quat q = osg::Quat(rotDegrees[0], osg::Vec3d(1,0,0),rotDegrees[1], osg::Vec3d(0,1,0),rotDegrees[2], osg::Vec3d(0,0,1)); 
               //Get old Bodies transform into quat
               osg::Quat qOld = osg::Quat(body->qx,body->qy,body->qz,body->qw);
               //Add new update to old Quat
                qOld *= q;
               //Set new quat for creating new TrackedBody
                q = qOld;
            //Setup tracked body TODO:Use a KinectSensor for trackedBody;
	    TrackerBase::TrackedBody * tb = new TrackerBase::TrackedBody;
	    tb->x = tb->y = tb->z = 0.0;
	    tb->qx = q.x();
	    tb->qy = q.y();
	    tb->qz = q.z();
	    tb->qw = q.w();
            //Set Current Tracker System Body to new tb
            _trackerSystem->setBody(0,tb);
            delete tb;
         }
         //Test Button Interaction
         if(true)
         {
            _trackerSystem->setButton(0,true);  
        //    _trackerSystem->setButton(0,false); 
         } 
    }

    cerr << "Finised\n";
}

bool KinectDemo::processEvent(InteractionEvent* event)
{
    TrackedButtonInteractionEvent* tie = event->asTrackedButtonEvent();

    if ((event->getInteraction() == BUTTON_DOWN) && tie->getHand() == 3)
    {
        //For Testing Simulated Hand Interaction
        cerr << "Hand 4 Button " << tie->getButton() << "\n";
        
    }
    if ((event->getInteraction() == BUTTON_DOWN) && tie->getHand() == 0)
    {
        cerr << "Hand 1 Button 0\n";
    }

    return false;
}

