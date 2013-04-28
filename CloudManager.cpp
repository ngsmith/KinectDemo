#include "CloudManager.h"

#include <cvrKernel/ComController.h>
#include <sstream>
#include <algorithm>
#include <cstdio>

#include <sys/syscall.h>
#include <sys/stat.h>

using namespace cvr;
using namespace std;

CloudManager::CloudManager(std::string server)
{
    useKColor = true;
    pause = false;
    should_quit = false;
    minDistHSV = 700;
    maxDistHSV = 5000;

    kinectVertices = new osg::Vec3Array;
    kinectNormals = new osg::Vec3Array;
    kinectColours = new osg::Vec4Array;

    _firstRun = 0;
    _next = true;
    kinectServer = server;
// precomputing colors for heat coloring
    for (int i=0;i<10000;i++) getColorRGB(i);

}

CloudManager::~CloudManager()
{
}

bool CloudManager::isCacheDone()
{
    return _next;
}
int CloudManager::firstRunStatus()
{
    return _firstRun;
}
void CloudManager::update()
{
    static bool frameLoading = false;
    bool cDone;

/*
        if (cvr::ComController::instance()->isMaster())
        {
            cDone = _cacheDone;
            int numSlaves = cvr::ComController::instance()->getNumSlaves();
            bool sDone[numSlaves];
            cvr::ComController::instance()->readSlaves(sDone, sizeof(bool));

            for (int i = 0; i < numSlaves; i++)
            {
                cDone = cDone && sDone[i];
            }

            cvr::ComController::instance()->sendSlaves(&cDone, sizeof(bool));
        }
        else
        {
            cDone = _cacheDone;
            cvr::ComController::instance()->sendMaster(&cDone, sizeof(bool));
            cvr::ComController::instance()->readMaster(&cDone, sizeof(bool));
        }

   cDone = _cacheDone;
       // run();
       
        if (!cDone)
        {
            //std::cerr << "Waiting for load to finish." << std::endl;
           // return;
        }

       // cvr::ComController::instance()->sync();
        //Add not here?
            //kinectVertices = newVertices;
            //kinectNormals = newNormals;
            //kinectColours = newColours;
   //     return;
        _next = true;
       // run();
    }

    if (cDone)
    {
        std::cerr << "Load Finished." << std::endl;
        //Add loaded node to root
    }
    else
    {
          std::cerr << "Waiting for GPU load finish." << std::endl;
    }
*/
}

void CloudManager::run()
{
    //Do functions
    _cacheDone = false;
    bool cDone = false;

    if (true)
    {
        //TODO:ZMQ does not want to be in init with the cloud socket-should only initialize this at the very beginning.
        zmq::context_t context2(1);
        
        cloudT_socket = new SubSocket<RemoteKinect::PointCloud> (context2, kinectServer);
        packet = new RemoteKinect::PointCloud();

        while (!should_quit)
        {

            if(!_next)
            {
             // printf("X");
            }
            else
            {
             // printf("O");

            if (cloudT_socket != NULL)
            {
                        //printf("NotNull");
                if (cloudT_socket->recv(*packet))
                {
   if(true)
   {
                    float r, g, b, a;
    newVertices = new osg::Vec3Array;
    newNormals = new osg::Vec3Array;
    newColours = new osg::Vec4Array;

                    if (true)
                    {
                      //cerr << "Size:" << packet->points_size() << "\n";
                        for (int i = 0; i < packet->points_size(); i++)
                        {
                            osg::Vec3f ppos((packet->points(i).x()),
                                            (packet->points(i).z()),
                                            (packet->points(i).y()));
                            newVertices->push_back(ppos);

                            if (useKColor)
                            {
                                r = (packet->points(i).r() / 255.);
                                g = (packet->points(i).g() / 255.);
                                b = (packet->points(i).b() / 255.);
                                a = 1;
                                newColours->push_back(osg::Vec4f(r, g, b, a));
                            }
                            else
                            {
                                newColours->push_back(getColorRGB(packet->points(i).z()));
                            }
                        }
                    }
      }
                }
                else
                {

                  //cerr << "PointCloud " << kinectServer << " Empty\n";
                }
            }

                _next = false;
                _cacheDone = true;
            }
       // if (cvr::ComController::instance()->isMaster())
       // printf(".");
        /*
        if (cvr::ComController::instance()->isMaster())
        {
            cDone = _cacheDone;
            int numSlaves = cvr::ComController::instance()->getNumSlaves();
            bool sDone[numSlaves];
            cvr::ComController::instance()->readSlaves(sDone, sizeof(bool));

            for (int i = 0; i < numSlaves; i++)
            {
                cDone = cDone && sDone[i];
            }

            cvr::ComController::instance()->sendSlaves(&cDone, sizeof(bool));
        }
        else
        {
            cDone = _cacheDone;
            cvr::ComController::instance()->sendMaster(&cDone, sizeof(bool));
            cvr::ComController::instance()->readMaster(&cDone, sizeof(bool));
        }
        */
        if(true)
        {
        //   cvr::ComController::instance()->sync();
            kinectVertices = newVertices;
            kinectColours = newColours;
            kinectNormals = newNormals;
           if(_firstRun == 1)
           {
             _firstRun = 2;
           }
           else if (_firstRun == 0)
           {
           _firstRun = 1;
           }
           _cacheDone = false;
           _next = true;
        }


        }
    }

    delete cloudT_socket;
    cloudT_socket = NULL;

    //This is the initial Function that runs any type of processing on Kinect PointCloud sets before overwriting old!
    //processNewCloud();

    //When Finished
    std::cerr << "All frames loaded." << std::endl;
}

void  CloudManager::quit()
{
    should_quit = true;
}

osg::Vec4f CloudManager::getColorRGB(int dist)
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
void CloudManager::processNewCloud()
{
  //Here is where we could run ICP or do checking against original cloud.

}
