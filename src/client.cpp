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

using namespace std;
using namespace yarp::os;
using namespace yarp::dev;


class ClientMod : public yarp::os::RFModule
{
private:

    // define one output port to comunicate to the server and
    // one input port to receive the trigger to start moving the
    // arm
    // hint: think about which port you need, buffered? simple? both?
    // FILL IN THE CODE
    double angle, period;
    bool triggered;
    BufferedPort<Bottle> inPort, outPort;

public:

    // set the correct angle to send to the server and the period of the thread
    // FILL IN THE CODE
    ClientMod(): angle(30.0), period(2.0), triggered(false)
    {}
    /****************************************************/
    bool configPorts()
    {
        // open all ports and check that everything is fine
        // output port: /client/output
        // input port: /client/input
        // FILL IN THE CODE
        if(!inPort.open("/client/input")){
            yError()<<"Error opening the input port";
            return false;
        }
        if(!outPort.open("/client/output")){
            yError()<<"Error opening the output port";
            return false;
        }

        return true;
    }

    /****************************************************/
    bool configure(ResourceFinder &rf)
    {
        // get "angle" input parameter
        // configure the ports
        bool conf = configPorts();

        // Check if angle is passed by command line argument
        if (rf.check("angle"))
        {
            angle  = rf.find("angle").asDouble();
        }

        // Check if period is passed by command line argument
        if (rf.check("period"))
        {
            period = rf.find("period").asDouble();
        }
        return conf;
    }

    /****************************************************/
    double getPeriod()
    {
        return period;
    }

    /****************************************************/
    bool close()
    {
        // close ports
        inPort.close();
        outPort.close();
        return true;
    }

    /****************************************************/
    bool interrupt()
    {
        // interrupt ports
        inPort.interrupt();
        outPort.interrupt();

        close();
        return true;
    }

    /****************************************************/
    bool updateModule()
    {
        if (!triggered)
        {
            // read from the input port the signal from the
            // trigger for starting to send data to the server
            Bottle *input;
            input = inPort.read(true);
            if(input){
                triggered = true;
            }
        }

        // once triggered prepare the bottle containing the
        // angle and send it to the server through the
        // output port
        // FILL IN CODE
        if(triggered){
            Bottle &response = outPort.prepare();
            response.addDouble(angle);
            outPort.write();
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

    ClientMod mod;
    return mod.runModule(rf);
}
