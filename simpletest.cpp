#include <iostream>
#include <fstream>
#include <string>
#include "osm.h"
#include <stdexcept>
using namespace std;


int main(int argc, char* argv[])
{
    // osm_init();// if you don't have an init in you library mark this line in comment
    timeMeasurmentStructure times = measureTimes(50000, 50000, 50000, 50000);

    ofstream myfile;
    myfile.open ("test.txt");   
    myfile << "hostname:" << times.machineName << endl;    
    myfile << "inst_time:" << times.instructionTimeNanoSecond << endl;
    myfile << "func_time:" << times.functionTimeNanoSecond << endl;
    myfile << "trap_time:" << times.trapTimeNanoSecond << endl;
    myfile << "disk_time:" << times.diskTimeNanoSecond << endl;
    myfile << "trap_inst_ratio:" <<  times.trapInstructionRatio << endl;
    myfile << "disk_inst_ratio:" <<  times.diskInstructionRatio << endl;
    
    myfile.close();
    osm_finalizer();//if you don't have a finalizer in you library mark this line in comment
    return 0;
}