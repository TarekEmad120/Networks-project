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
    //reading coordinator.txt file
    std::ifstream file("coordinator.txt");
    //CHECK IF FILE IS OPENED   
    if(!file.is_open()){
        std::cout<<"error in opening file"<<std::endl;
    }
    //getting  line which contain the starting node and starting time
    while(std::getline(file,temp)){
        int St_node = atoi(temp.c_str());
        int St_time = atoi(temp.c_str()+1);
        std::cout<<St_node;
        std::cout<<  St_time<<endl;
        //creating message
        Coordinatormsg_Base *msg = new Coordinatormsg_Base("coordinator");
        msg->setST_Node(St_node);
        msg->setST_Time(St_time);
        //sending message to the starting node
        if(St_node == 0){
            send(msg,"Coordinate_node0");
        }
        else if(St_node == 1){
            send(msg,"Coordinate_node1");
        }
    }
    file.close();

}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
