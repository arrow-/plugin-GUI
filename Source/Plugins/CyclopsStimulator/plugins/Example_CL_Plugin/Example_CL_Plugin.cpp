#include "Example_CL_Plugin.h"

Example_CL_Plugin::Example_CL_Plugin()
{
    std::cout << "Creating Example_CL_Plugin" << std::endl;
    count = 0;
    Random *r = new Random();
    r->setSeedRandomly();
    secret = r->nextInt();
    delete r;
}

void Example_CL_Plugin::handleEvent(int eventType, MidiMessage& event, int samplePosition/* = 0 */)
{

}

void Example_CL_Plugin::timerTask()
{
    std::cout << secret << " " << ++count << std::endl;
}