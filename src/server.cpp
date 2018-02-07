#include <yarp/os/Network.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Port.h>
#include <yarp/os/Property.h>
#include <yarp/os/Time.h>
#include <yarp/os/RFModule.h>

#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IControlMode2.h>
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/IPositionControl2.h>
#include <yarp/dev/IControlLimits2.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::dev;

class ServerMod : public yarp::os::RFModule
{
private:

    // define one input port to receive the angle from the client
    // hint: think about which port you need, buffered? simple?
    // FILL IN THE CODE
    BufferedPort<Bottle>             inPort;
    PolyDriver                       driver;
    IControlMode2                   *imod;
    IEncoders                       *ienc;
    IPositionControl2               *ipos;
    IControlLimits2                 *ilim;

    double                          min, max;
    bool                            idle;
    int                             nAxes;


    //Implement the BufferedPort callback, as soon as new data is coming
    void moveArm(Bottle* bot)
    {
        static bool invert = true;
        if (!bot->get(0).isDouble())
        {
            yError()<<"Expecting a double on read...";
            return;
        }
        double angle = bot->get(0).asDouble();
        if(ipos)
        {
            // the arm will move between -angle and angle
            if (invert)
            {
                angle = -1 * angle;
                invert = false;
            }
            else
            {
                invert = true;
            }

            // check that "angle" is inside the joint
            // limits before moving
            // FILL IN CODE
            angle = angle > max ? max : angle;
            angle = angle < min ? min : angle;

            if (idle)
            {
                // move the arm passing "angle"
                // FIll IN CODE
                ipos->positionMove(2, angle);
                idle = false;
            }
        }
    }

public:

    ServerMod() : imod(nullptr), ienc(nullptr), ipos(nullptr), ilim(nullptr),
      min(0.0), max(0.0), idle(true), nAxes(0)
    {}
    /****************************************************/
    bool configPorts()
    {

        // open the input port /server/input
        if(!inPort.open("/server/input")){
            yError()<<"Cannot open the input port";
            return false;
        }
        return true;
    }

    /****************************************************/
    bool configDevice()
    {
        // configure the options for the driver
        Property option;
        option.put("device","remote_controlboard");
        option.put("remote","/icubSim/left_arm");
        option.put("local","/server");
        int joint = 2;

        // open the driver
        if (!driver.open(option))
        {
            yError()<<"Unable to open the device driver";
            return false;
        }

        // open the views
        if (!driver.view(imod) || !driver.view(ienc) || !driver.view(ipos) || !driver.view(ilim))
        {
           yError()<<"Failed to view one of the interfaces";
           return false;
        }

        // get joint's limits
        ilim->getLimits(joint, &min, &max);
        // tell the device we aim to control
        // in position mode all the joints
        ienc->getAxes(&nAxes);

        // tell the device we aim to control
        // in position mode all the joints
        // FILL IN THE CODE
        imod->setControlMode(joint,VOCAB_CM_POSITION);

        // set ref speed for the target joint
        ipos->setRefSpeed(joint, 40.0);

        return true;
    }

    /****************************************************/
    bool configure(ResourceFinder &rf)
    {
        bool config_ok;

        // configure the device for controlling the head
        config_ok = configDevice();

        // configure the ports
        config_ok = configPorts() && config_ok;

        return config_ok;
    }

    /****************************************************/
    double getPeriod()
    {
        return 0.0;
    }

    /****************************************************/
    bool close()
    {
        // close the port and close the PolyDriver
        // FILL IN THE CODE
        inPort.close();
        return true;
    }

    /****************************************************/
    bool interrupt()
    {

        // interrupt the port
        inPort.interrupt();
        close();
        return true;
    }

    /****************************************************/
    bool updateModule()
    {
        Bottle* bot = nullptr;
        // read from the input port passing "bot"

        bot = inPort.read(false);

        if (bot)
        {
            // check that the movement requested has been done and
            // update the boolean variable "idle"
            // FILL IN THE CODE
            idle = true;
            // try to move the arm
            moveArm(bot);
        }

        return true;
    }

};

int main(int argc, char *argv[]) {
    Network yarp;
    if (!yarp.checkNetwork())
    {
        yError()<<"YARP doesn't seem to be available";
        return 1;
    }

    ResourceFinder rf;
    rf.setDefaultContext("assignment_motor-control");
    rf.configure(argc,argv);

    ServerMod mod;
    return mod.runModule(rf);
}
