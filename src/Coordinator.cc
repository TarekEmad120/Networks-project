//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "Coordinator.h"
#include <fstream>
#include <string>
#include "Coordinatormsg_m.h"

Define_Module(Coordinator);

void Coordinator::initialize()
{
    // TODO - Generated method body
    std::string temp;
    // reading coordinator.txt file
    std::ifstream file("coordinator.txt");
    // CHECK IF FILE IS OPENED
    if (!file.is_open())
    {
        std::cout << "error in opening file" << std::endl;
    }
    // getting  line which contain the starting node and starting time
    while (std::getline(file, temp))
    {
        this->node = atoi(temp.c_str());
        int St_time = atoi(temp.c_str() + 1);
        std::cout << St_time << endl;
        // creating message
        Coordinatormsg_Base *msg = new Coordinatormsg_Base("coordinator");
        msg->setST_Node(this->node);
        msg->setST_Time(St_time);
        // sending message to the starting node
        scheduleAt(SimTime() + St_time, msg);
    }
    file.close();
}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    Coordinatormsg_Base *mmsg = check_and_cast<Coordinatormsg_Base *>(msg);
    if (this->node == 0)
    {
        send(mmsg, "Coordinate_node0");
    }
    else 
    {
        send(mmsg, "Coordinate_node1");
    }
}
