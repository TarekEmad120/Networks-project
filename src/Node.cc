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

#include "Node.h"
#include <vector>
#include <string>
#include <bitset>
#include <iostream>
#include <fstream>
#include "Coordinatormsg_m.h"
#include "Nodemsg_m.h"
#define FLAG '#'
#define ESC '/'
Define_Module(Node);

bool issend0 = false;
bool issend1 = false;

// creating vector of string to save the messages
std::vector<std::string> messages;
// creating vector of bitset to save the bit error
std::vector<std::bitset<4>> bit_errors;

// creating vector for storing Sequence numbers for receivers
std::vector<int> SeqNumbers;
std::vector<int> ErroredFrames;
std::vector<int> lostFrames;

const char *fileName = "example.txt";
// Create an ofstream object (output file stream)
std::ofstream outFile;
// Open the file in write mode

void Node::initialize()
{
    // TODO - Generated method body
    this->WS = getParentModule()->par("WS"); //.intValue();
    this->WR = getParentModule()->par("WR"); //.intValue();
    this->TO = getParentModule()->par("TO"); //.intValue();
    this->PT = getParentModule()->par("PT"); //.doubleValue();
    this->TD = getParentModule()->par("TD"); //.doubleValue();
    this->ED = getParentModule()->par("ED"); //.doubleValue();
    this->DD = getParentModule()->par("DD"); //.doubleValue();
    this->LP = getParentModule()->par("LP"); //.doubleValue();
    this->counter = 0.0;

    // geting node id from parameter of node not the parent
    this->node_id = par("node_id");
    this->lowest_indexAT_WS = 0;
    this->current_index_toSEND = 0;
    this->highest_indexAT_WS = this->WS - 1;
    this->Seq_Num = 0;
    this->ExpectedFrame = 0;
    this->sttime = 0.0;
    // reading input0.txt

    outFile.open(fileName);
}

void Node::handleMessage(cMessage *msg)
{
    outFile.open(fileName);

    simtime_t time = simTime();
    // checking that the message is from coordinator or from other node
    if (msg->arrivedOn("Coordinator") && issend0 == false && issend1 == false)
    {

        Coordinatormsg_Base *cord_msg = check_and_cast<Coordinatormsg_Base *>(msg);
        int st_node = cord_msg->getST_Node();
        int st_time = cord_msg->getST_Time();
        // TODO - Generated method body
        std::cout << "Node " << this->node_id << " recieved message from coordinator" << std::endl;
        std::cout << "st_node = " << st_node << std::endl;
        std::cout << "st_time = " << st_time << std::endl;
        delete msg;

        if (st_node == 0)
        {
            issend0 = true;
            std::cout << "Node " << this->node_id << " is sending" << std::endl;
            std::ifstream file("input0.txt");
            std::string line;
            while (std::getline(file, line))
            {
                // line is in form like 0000 hello
                // first 4 bits is bit error and it needed to saved in string
                std::string bit_error = line.substr(0, 4);
                // we need to save the message in string
                std::string message = line.substr(5);
                // inserting the message in vector of string
                messages.push_back(message);

                // we need to convert the bit error to  bitset
                std::bitset<4> bit_error_bitset(bit_error);
                // inserting the bit error in vector of bitset
                bit_errors.push_back(bit_error_bitset);

                // print the bit error
                // std::cout << bit_error_bitset << std::endl;

                // intializing the SeqNumbers vector and the ErrorFrames vector for the receiver
                SeqNumbers.push_back(0);
                ErroredFrames.push_back(0);
                lostFrames.push_back(0);
            }
            file.close();
        }
        else
        {
            issend0 = true;
            std::cout << "Node " << this->node_id << " is sending" << std::endl;
            std::ifstream file("input1.txt");
            std::string line;
            while (std::getline(file, line))
            {
                // line is in form like 0000 hello
                // first 4 bits is bit error and it needed to saved in string
                std::string bit_error = line.substr(0, 4);
                // we need to save the message in string
                std::string message = line.substr(5);
                // inserting the message in vector of string
                messages.push_back(message);

                // we need to convert the bit error to  bitset
                std::bitset<4> bit_error_bitset(bit_error);
                // inserting the bit error in vector of bitset
                bit_errors.push_back(bit_error_bitset);

                // print the bit error
                // std::cout << bit_error_bitset << std::endl;

                // intializing the SeqNumbers vector and the ErrorFrames vector for the receiver
                SeqNumbers.push_back(0);
                ErroredFrames.push_back(0);
                lostFrames.push_back(0);
            }
            file.close();
        }
    }

    // check if i get messages from myself
    if (msg->isSelfMessage() && std::string(msg->getName()) == "timout")
    {
        Nodemsg_Base *nmsg = check_and_cast<Nodemsg_Base *>(msg);

        EV << "Time out event at time [" << simTime() << "], at Node[" << this->node_id << "], for frame with seq_num= [" << nmsg->getSeq_Num() << "\n";

        EV << "At time [" << this->PT + simTime() << "], Node[" << this->node_id << "] [sent] frame with seq_num = [" << nmsg->getSeq_Num() << "] and payload =[" << nmsg->getPayload() << "] and trailer = [" << nmsg->getMycheckbits() << "], Modified[" << 0 << "], Lost[" << 0 << "], Duplicate[" << 0 << "], Delayed[" << 0 << "].\n";
        std::cout << "self message" << std::endl;
        // checking the seqqnum of the message in lostFrames vector
        int seqqnum = nmsg->getSeq_Num();
        sendDelayed(nmsg, this->PT + this->TD, "Nodeout");
    }

    // checking if the message is ack or nack that we recieved from other node before timeout
    // Node 0 receives control frames from Node 1
    if (msg->arrivedOn("Nodein") && this->node_id == 0 && issend0 == 1 && std::string(msg->getName()) != "timout")
    {
        std::cout << "Node " << this->node_id << " recieved message from other node" << std::endl;
        Nodemsg_Base *nmsg = check_and_cast<Nodemsg_Base *>(msg);
        int m_type = nmsg->getM_Type();
        if (m_type == 1)
        {
            // we recieved ack
            int ackNum = nmsg->getAck_Num();
            std::cout << "Node " << this->node_id << " recieved ACK" << std::endl;

            this->lowest_indexAT_WS++;
            if (this->highest_indexAT_WS < messages.size())
            {
                this->highest_indexAT_WS++;
            }
            std::cout << "lowestindex - highestindex " << this->lowest_indexAT_WS << " - " << this->highest_indexAT_WS << std::endl;
        }
        else if (m_type == 2)
        {
            // we recieved nack
            std::cout << "Node " << this->node_id << " recieved NACK" << std::endl;

            int seqqnum = nmsg->getAck_Num();
            std::string message = messages[seqqnum];
            // checking if the seqqnum is in lostFrames vector
            std::cout << "lostFrames[" << seqqnum << "] = " << lostFrames[seqqnum] << std::endl;
            if (lostFrames[seqqnum] > 0)
            {
                // check if the messsage is isSelfMessage
                if (this->tmsg->isSelfMessage())
                {
                    // cancel the message
                    cancelEvent(this->tmsg);
                    lostFrames[seqqnum]--;
                }
                else
                {
                    return;
                }
            }

            int countframe = message.size();

            std::bitset<8> temp;
            std::bitset<8> bs_countframe(countframe);

            std::bitset<8> Xorbyte = bs_countframe;

            for (int i = 0; i < message.size(); i++)
            {

                temp = std::bitset<8>(message[i]);
                Xorbyte ^= temp;
            }
            std::string new_message = "";
            for (int i = 0; i < message.size(); i++)
            {
                if (message[i] == FLAG || message[i] == ESC)
                {
                    new_message += ESC;
                    new_message += message[i];
                }
                else
                {
                    new_message += message[i];
                }
            }
            // we need to add FLAG before the start and end of message
            new_message = FLAG + new_message + FLAG;

            double sendingtime;
            double PT = this->PT;
            double TD = this->TD;
            sendingtime = PT + TD;
            Nodemsg_Base *nmsg = new Nodemsg_Base("send");
            nmsg->setMycheckbits(Xorbyte);
            nmsg->setPayload(new_message.c_str());
            std::cout << "sending message " << new_message << std::endl;
            nmsg->setSeq_Num(seqqnum);
            nmsg->setM_Type(0);
            nmsg->setAck_Num(0);
            EV << "At time [" << sendingtime - TD + simTime() << "], Node[" << this->node_id << "] [Resent] frame with seq_num = [" << seqqnum << "] and payload =[" << new_message << "] and trailer = [" << Xorbyte << "], Modified[" << 0 << "], Lost[" << 0 << "], Duplicate[" << 0 << "], Delayed[" << 0 << "].\n";

            sendDelayed(nmsg, sendingtime, "Nodeout");
        }
        delete msg;
    }

    // checking if the message is ack or nack that we recieved from other node before timeout
    // Node 1 receives control frames from Node 0
    if (msg->arrivedOn("Nodein") && this->node_id == 1 && issend1 == 1 && std::string(msg->getName()) != "timout")
    {
        std::cout << "Node " << this->node_id << " recieved message from other node" << std::endl;
        Nodemsg_Base *nmsg = check_and_cast<Nodemsg_Base *>(msg);
        int m_type = nmsg->getM_Type();
        if (m_type == 1)
        {
            // we recieved ack
            int ackNum = nmsg->getAck_Num();
            std::cout << "Node " << this->node_id << " recieved ACK" << std::endl;

            this->lowest_indexAT_WS++;
            if (this->highest_indexAT_WS < messages.size())
            {
                this->highest_indexAT_WS++;
            }
            std::cout << "lowestindex - highestindex " << this->lowest_indexAT_WS << " - " << this->highest_indexAT_WS << std::endl;
        }
        else if (m_type == 2)
        {
            // we recieved nack
            std::cout << "Node " << this->node_id << " recieved NACK" << std::endl;

            int seqqnum = nmsg->getAck_Num();
            std::string message = messages[seqqnum];
            // checking if the seqqnum is in lostFrames vector
            std::cout << "lostFrames[" << seqqnum << "] = " << lostFrames[seqqnum] << std::endl;
            if (lostFrames[seqqnum] > 0)
            {
                // check if the messsage is isSelfMessage
                if (this->tmsg->isSelfMessage())
                {
                    // cancel the message
                    cancelEvent(this->tmsg);
                    lostFrames[seqqnum]--;
                }
                else
                {
                    return;
                }
            }

            int countframe = message.size();

            std::bitset<8> temp;
            std::bitset<8> bs_countframe(countframe);

            std::bitset<8> Xorbyte = bs_countframe;

            for (int i = 0; i < message.size(); i++)
            {

                temp = std::bitset<8>(message[i]);
                Xorbyte ^= temp;
            }
            std::string new_message = "";
            for (int i = 0; i < message.size(); i++)
            {
                if (message[i] == FLAG || message[i] == ESC)
                {
                    new_message += ESC;
                    new_message += message[i];
                }
                else
                {
                    new_message += message[i];
                }
            }
            // we need to add FLAG before the start and end of message
            new_message = FLAG + new_message + FLAG;

            double sendingtime;
            double PT = this->PT;
            double TD = this->TD;
            sendingtime = PT + TD;
            Nodemsg_Base *nmsg = new Nodemsg_Base("send");
            nmsg->setMycheckbits(Xorbyte);
            nmsg->setPayload(new_message.c_str());
            std::cout << "sending message " << new_message << std::endl;
            nmsg->setSeq_Num(seqqnum);
            nmsg->setM_Type(0);
            nmsg->setAck_Num(0);
            EV << "At time [" << sendingtime - TD + simTime() << "], Node[" << this->node_id << "] [Resent] frame with seq_num = [" << seqqnum << "] and payload =[" << new_message << "] and trailer = [" << Xorbyte << "], Modified[" << 0 << "], Lost[" << 0 << "], Duplicate[" << 0 << "], Delayed[" << 0 << "].\n";

            sendDelayed(nmsg, sendingtime, "Nodeout");
        }
        delete msg;
    }

    //  node 0  ->  node 1
    // Node 0 sends msgs frames to Node 1
    if (issend0 == true && this->node_id == 0)
    {
        if (this->current_index_toSEND == messages.size())
        {
            return;
        }
        std ::cout << "//////entering sender before while" << std::endl;
        this->counter = 0; // Reset the counter that handels the Processing time delay between messages
        while (this->current_index_toSEND <= this->highest_indexAT_WS)
        {
            std ::cout << "lowest_index_WS " << this->lowest_indexAT_WS << std::endl;
            std ::cout << "current_index_toSEND " << this->current_index_toSEND << std::endl;
            std ::cout << "highest_indexAT_WS " << this->highest_indexAT_WS << std::endl;
            if (this->current_index_toSEND > this->highest_indexAT_WS)
            {
                std ::cout << "highest_indexAT_WS > current_index_toSEND" << std::endl;
                return;
            }
            else
            {

                std::string message = messages[this->current_index_toSEND]; // 0 1 2
                // getting bit error from vector of bitset and save it in bitset

                std::bitset<4> bit_error_bitset = bit_errors[this->current_index_toSEND];
                EV << "At time [" << simTime() + counter << "], Node[" << this->node_id << "] , Introducing channel error with code =[" << bit_error_bitset << "].\n";
                int countframe = message.size();

                std::bitset<8> temp;
                std::bitset<8> bs_countframe(countframe);

                std::bitset<8> Xorbyte = bs_countframe;

                for (int i = 0; i < message.size(); i++)
                {

                    temp = std::bitset<8>(message[i]);
                    Xorbyte ^= temp;
                }
                bool duplicate = false;
                bool islost = false;
                bool ismodified = false;
                bool isdelayed = false;

                if (bit_error_bitset[3] == 1)
                {
                    // we need to modify the original message
                    // we need to change the first bit of message
                    message[0]++;
                    ismodified = true;
                }
                // let start byte stuffing
                // first we need to check for #and / in message and if we find it we need to add / before it
                std::string new_message = "";
                for (int i = 0; i < message.size(); i++)
                {
                    if (message[i] == FLAG || message[i] == ESC)
                    {
                        new_message += ESC;
                        new_message += message[i];
                    }
                    else
                    {
                        new_message += message[i];
                    }
                }
                // we need to add FLAG before the start and end of message
                new_message = FLAG + new_message + FLAG;

                // save the sending time in double and get its value from time
                double sendingtime = 0;
                double PT = this->PT;
                double TD = this->TD;
                double DD = this->DD;
                sendingtime += (double)PT + TD + this->counter;
                this->counter += PT; // to add the delay of Processing time between succssive messages
                std::cout << "sendingtime = " << sendingtime << std::endl;

                if (bit_error_bitset[0] == 1) // checking for error delay
                {                             // here we add the error delay to sending time
                    isdelayed = true;
                    int systemdelay = (int)this->ED;
                    sendingtime += this->ED;
                }
                if (bit_error_bitset[1] == 1)
                { // duplicate state in which we will send the message twice by time difference 0.1

                    duplicate = true;
                }
                if (bit_error_bitset[2] == 1)
                { // we need to drop the message
                    islost = true;
                }
                EV << "At time [" << sendingtime - TD + simTime() << "], Node[" << this->node_id << "] [sent] frame with seq_num = [" << Seq_Num << "] and payload =[" << new_message << "] and trailer = [" << Xorbyte << "], Modified[" << ismodified << "], Lost[" << islost << "], Duplicate[" << duplicate << "], Delayed[" << isdelayed << "].\n";

                if (islost == true)
                {
                    std::string message = messages[this->current_index_toSEND];
                    lostFrames[this->current_index_toSEND]++;
                    std::cout << "lostFrames[" << this->current_index_toSEND << "] = " << lostFrames[this->current_index_toSEND] << std::endl;
                    this->tmsg = new Nodemsg_Base("timout");
                    std::string new_message = "";
                    for (int i = 0; i < message.size(); i++)
                    {
                        if (message[i] == FLAG || message[i] == ESC)
                        {
                            new_message += ESC;
                            new_message += message[i];
                        }
                        else
                        {
                            new_message += message[i];
                        }
                    }
                    new_message = FLAG + new_message + FLAG;
                    this->tmsg->setMycheckbits(Xorbyte);
                    this->tmsg->setPayload(new_message.c_str());
                    std::cout << "sending message " << new_message << std::endl;
                    this->tmsg->setSeq_Num(this->Seq_Num);
                    Seq_Num++;
                    this->tmsg->setM_Type(0);
                    this->tmsg->setAck_Num(0);
                    scheduleAt(simTime() + sendingtime + this->TO, this->tmsg);
                }
                if (islost == false) // if the message is not lost we will send it
                {
                    Nodemsg_Base *nmsg = new Nodemsg_Base("send");
                    nmsg->setMycheckbits(Xorbyte);
                    nmsg->setPayload(new_message.c_str());
                    std::cout << "sending message " << new_message << std::endl;
                    nmsg->setSeq_Num(this->Seq_Num);
                    Seq_Num++;
                    nmsg->setM_Type(0);
                    nmsg->setAck_Num(0);
                    sendDelayed(nmsg, sendingtime, "Nodeout");
                    if (duplicate)
                    {
                        // sending message to other node with delay using scheduleAt
                        // converting the sendingtime to simtime_t to use send
                        // scheduleAt(simTime() + sendingtime + 0.1, nmsg->dup());
                        sendingtime += DD;
                        EV << "At time [" << simTime() + sendingtime - TD << "], Node[" << this->node_id << "] [sent] frame with seq_num = [" << Seq_Num - 1 << "] and payload =[" << new_message << "] and trailer = [" << Xorbyte << "], Modified[" << ismodified << "], Lost[" << islost << "], Duplicate[" << 2 << "], Delayed[" << isdelayed << "].\n";
                        sendDelayed(nmsg->dup(), sendingtime, "Nodeout");
                    }
                }
                this->current_index_toSEND++;
            }
        }
        std ::cout << "exit sender after while\\\\\\" << std::endl;
    }

    //  node 1  ->  node 0
    // Node 1 sends msgs frames to Node 0   [to do .....]
    if (issend1 == true && this->node_id == 1)
    {

        if (this->current_index_toSEND == messages.size())
        {
            return;
        }
        std ::cout << "//////entering sender before while" << std::endl;
        this->counter = 0; // Reset the counter that handels the Processing time delay between messages
        while (this->current_index_toSEND <= this->highest_indexAT_WS)
        {
            std ::cout << "lowest_index_WS " << this->lowest_indexAT_WS << std::endl;
            std ::cout << "current_index_toSEND " << this->current_index_toSEND << std::endl;
            std ::cout << "highest_indexAT_WS " << this->highest_indexAT_WS << std::endl;
            if (this->current_index_toSEND > this->highest_indexAT_WS)
            {
                std ::cout << "highest_indexAT_WS > current_index_toSEND" << std::endl;
                return;
            }
            else
            {

                std::string message = messages[this->current_index_toSEND]; // 0 1 2
                // getting bit error from vector of bitset and save it in bitset

                std::bitset<4> bit_error_bitset = bit_errors[this->current_index_toSEND];
                EV << "At time [" << simTime() + counter << "], Node[" << this->node_id << "] , Introducing channel error with code =[" << bit_error_bitset << "].\n";
                int countframe = message.size();

                std::bitset<8> temp;
                std::bitset<8> bs_countframe(countframe);

                std::bitset<8> Xorbyte = bs_countframe;

                for (int i = 0; i < message.size(); i++)
                {

                    temp = std::bitset<8>(message[i]);
                    Xorbyte ^= temp;
                }
                bool duplicate = false;
                bool islost = false;
                bool ismodified = false;
                bool isdelayed = false;

                if (bit_error_bitset[3] == 1)
                {
                    // we need to modify the original message
                    // we need to change the first bit of message
                    message[0]++;
                    ismodified = true;
                }
                // let start byte stuffing
                // first we need to check for #and / in message and if we find it we need to add / before it
                std::string new_message = "";
                for (int i = 0; i < message.size(); i++)
                {
                    if (message[i] == FLAG || message[i] == ESC)
                    {
                        new_message += ESC;
                        new_message += message[i];
                    }
                    else
                    {
                        new_message += message[i];
                    }
                }
                // we need to add FLAG before the start and end of message
                new_message = FLAG + new_message + FLAG;

                // save the sending time in double and get its value from time
                double sendingtime = 0;
                double PT = this->PT;
                double TD = this->TD;
                double DD = this->DD;
                sendingtime += (double)PT + TD + this->counter;
                this->counter += PT; // to add the delay of Processing time between succssive messages
                std::cout << "sendingtime = " << sendingtime << std::endl;

                if (bit_error_bitset[0] == 1) // checking for error delay
                {                             // here we add the error delay to sending time
                    isdelayed = true;
                    int systemdelay = (int)this->ED;
                    sendingtime += this->ED;
                }
                if (bit_error_bitset[1] == 1)
                { // duplicate state in which we will send the message twice by time difference 0.1

                    duplicate = true;
                }
                if (bit_error_bitset[2] == 1)
                { // we need to drop the message
                    islost = true;
                }
                EV << "At time [" << sendingtime - TD + simTime() << "], Node[" << this->node_id << "] [sent] frame with seq_num = [" << Seq_Num << "] and payload =[" << new_message << "] and trailer = [" << Xorbyte << "], Modified[" << ismodified << "], Lost[" << islost << "], Duplicate[" << duplicate << "], Delayed[" << isdelayed << "].\n";

                if (islost == true)
                {
                    std::string message = messages[this->current_index_toSEND];
                    lostFrames[this->current_index_toSEND]++;
                    std::cout << "lostFrames[" << this->current_index_toSEND << "] = " << lostFrames[this->current_index_toSEND] << std::endl;
                    this->tmsg = new Nodemsg_Base("timout");
                    std::string new_message = "";
                    for (int i = 0; i < message.size(); i++)
                    {
                        if (message[i] == FLAG || message[i] == ESC)
                        {
                            new_message += ESC;
                            new_message += message[i];
                        }
                        else
                        {
                            new_message += message[i];
                        }
                    }
                    new_message = FLAG + new_message + FLAG;
                    this->tmsg->setMycheckbits(Xorbyte);
                    this->tmsg->setPayload(new_message.c_str());
                    std::cout << "sending message " << new_message << std::endl;
                    this->tmsg->setSeq_Num(this->Seq_Num);
                    Seq_Num++;
                    this->tmsg->setM_Type(0);
                    this->tmsg->setAck_Num(0);
                    scheduleAt(simTime() + sendingtime + this->TO, this->tmsg);
                }
                if (islost == false) // if the message is not lost we will send it
                {
                    Nodemsg_Base *nmsg = new Nodemsg_Base("send");
                    nmsg->setMycheckbits(Xorbyte);
                    nmsg->setPayload(new_message.c_str());
                    std::cout << "sending message " << new_message << std::endl;
                    nmsg->setSeq_Num(this->Seq_Num);
                    Seq_Num++;
                    nmsg->setM_Type(0);
                    nmsg->setAck_Num(0);
                    sendDelayed(nmsg, sendingtime, "Nodeout");
                    if (duplicate)
                    {
                        // sending message to other node with delay using scheduleAt
                        // converting the sendingtime to simtime_t to use send
                        // scheduleAt(simTime() + sendingtime + 0.1, nmsg->dup());
                        sendingtime += DD;
                        EV << "At time [" << simTime() + sendingtime - TD << "], Node[" << this->node_id << "] [sent] frame with seq_num = [" << Seq_Num - 1 << "] and payload =[" << new_message << "] and trailer = [" << Xorbyte << "], Modified[" << ismodified << "], Lost[" << islost << "], Duplicate[" << 2 << "], Delayed[" << isdelayed << "].\n";
                        sendDelayed(nmsg->dup(), sendingtime, "Nodeout");
                    }
                }
                this->current_index_toSEND++;
            }
        }
        std ::cout << "exit sender after while\\\\\\" << std::endl;
    }

    // we must check that the reciever is node1
    // Node 1 sends control frames to Node 0
    if (!issend1 && this->node_id == 1)
    {
        std::cout << "entering the receiver" << std::endl;

        // recieving message from other node
        issend0 = true;

        Nodemsg_Base *nmsg = check_and_cast<Nodemsg_Base *>(msg);
        std::string message = nmsg->getPayload();
        std::bitset<8> checksum = nmsg->getMycheckbits();

        // we must destuff the message
        std::string new_message = "";
        for (int i = 1; i < message.size() - 1; i++)
        {

            if (message[i] == ESC)
            {
                if (message[i + 1] == FLAG)
                {
                    new_message += FLAG;
                    i++;
                }
                else if (message[i + 1] == ESC)
                {
                    new_message += ESC;
                    i++;
                }
            }
            else
            {
                new_message += message[i];
            }
        }
        std::cout << "recieed message " << new_message << std::endl;
        // we must calculate the checksum
        int countframe = new_message.size();
        std::bitset<8> bs_countframe(countframe);
        std::bitset<8> temp;
        std::bitset<8> Xorbyte = bs_countframe;
        for (int i = 0; i < new_message.size(); i++)
        {
            temp = std::bitset<8>(new_message[i]);
            Xorbyte ^= temp;
        }
        // we must check if the checksum is correct or not by checking the xorbyte and checksum
        int receivedFrame = nmsg->getSeq_Num();
        if (SeqNumbers[receivedFrame] == 1)
        {
            std::cout << "i have received this msg before , return;" << std::endl;
            return;
        }
        if (ErroredFrames[receivedFrame] == 1 && Xorbyte != checksum)
        {
            return;
        }
        if (Xorbyte == checksum && ExpectedFrame == nmsg->getSeq_Num())
        {
            if (SeqNumbers[receivedFrame] == 0)
            {
                SeqNumbers[receivedFrame] = 1;
            }
            // sending ack
            EV << "At time [" << simTime() + this->PT << "], Node[" << this->node_id << "], sending [ACK] with number [" << nmsg->getSeq_Num() + 1 << "], loss[NO]\n";

            ExpectedFrame++;
            Nodemsg_Base *ack = new Nodemsg_Base("ack");
            ack->setSeq_Num(0);
            ack->setM_Type(1);
            int ackNum = nmsg->getSeq_Num() + 1;
            ack->setAck_Num(ackNum);
            sendDelayed(ack, this->PT + this->TD, "Nodeout");
            int i = receivedFrame + 1;
            this->counter = this->PT;
            while (SeqNumbers[i] == 1)
            {

                Nodemsg_Base *aack = new Nodemsg_Base("ack");
                std::cout << "i = " << i << std::endl;
                std::cout << "SeqNumbers[i] = " << SeqNumbers[i] << std::endl;
                ExpectedFrame++;
                aack->setSeq_Num(0);
                aack->setM_Type(1);
                ackNum++;
                aack->setAck_Num(ackNum);
                EV << "At time [" << simTime() + this->PT + this->TD << "], Node[" << this->node_id << "], sending [ACK] with number [" << ackNum << "], loss[NO]\n";

                sendDelayed(aack, this->PT + this->TD + this->counter, "Nodeout");

                this->counter += this->PT;
                i++;
            }
        }
        else if (Xorbyte == checksum && nmsg->getSeq_Num() < ExpectedFrame + this->WR)
        {
            if (SeqNumbers[receivedFrame] == 0)
            {
                std::cout << "SeqNumbers[receivedFrame] = " << SeqNumbers[receivedFrame] << std::endl;
                SeqNumbers[receivedFrame] = 1;
            }
            std::cout << "SeqNumbers[receivedFrame] = " << SeqNumbers[receivedFrame] << std::endl;

            if (ErroredFrames[ExpectedFrame] < 1)
            {
                ErroredFrames[ExpectedFrame]++;
                Nodemsg_Base *nack = new Nodemsg_Base("nack");
                nack->setSeq_Num(0);
                nack->setM_Type(2);
                int ackNum = nmsg->getSeq_Num();
                nack->setAck_Num(ExpectedFrame);
                EV << "At time [" << simTime() + this->PT << "], Node[" << this->node_id << "], sending [NACK] with number [" << ExpectedFrame << "], loss[NO]\n";

                sendDelayed(nack, this->PT + this->TD, "Nodeout");
            }
        }
        else
        {
            if (ErroredFrames[receivedFrame] == 0)
            {
                ErroredFrames[receivedFrame] = 1;
            }
            // sending nack
            Nodemsg_Base *nack = new Nodemsg_Base("nack");
            nack->setSeq_Num(0);
            nack->setM_Type(2);
            int ackNum = nmsg->getSeq_Num();
            nack->setAck_Num(ackNum);
            EV << "At time [" << simTime() + this->PT << "], Node[" << this->node_id << "], sending [NACK] with number [" << ackNum << "], loss[NO]\n";

            sendDelayed(nack, this->PT + this->TD, "Nodeout");
        }
        std::cout << "exiting the receiver" << std::endl;
    }

    // we must check that the reciever is node0
    // Node 0 sends control frames to Node 1
    if (!issend0 && this->node_id == 0)
    {
        std::cout << "entering the receiver" << std::endl;

        // recieving message from other node
        issend1 = true;

        Nodemsg_Base *nmsg = check_and_cast<Nodemsg_Base *>(msg);
        std::string message = nmsg->getPayload();
        std::bitset<8> checksum = nmsg->getMycheckbits();

        // we must destuff the message
        std::string new_message = "";
        for (int i = 1; i < message.size() - 1; i++)
        {

            if (message[i] == ESC)
            {
                if (message[i + 1] == FLAG)
                {
                    new_message += FLAG;
                    i++;
                }
                else if (message[i + 1] == ESC)
                {
                    new_message += ESC;
                    i++;
                }
            }
            else
            {
                new_message += message[i];
            }
        }
        std::cout << "recieed message " << new_message << std::endl;
        // we must calculate the checksum
        int countframe = new_message.size();
        std::bitset<8> bs_countframe(countframe);
        std::bitset<8> temp;
        std::bitset<8> Xorbyte = bs_countframe;
        for (int i = 0; i < new_message.size(); i++)
        {
            temp = std::bitset<8>(new_message[i]);
            Xorbyte ^= temp;
        }
        // we must check if the checksum is correct or not by checking the xorbyte and checksum
        int receivedFrame = nmsg->getSeq_Num();
        if (SeqNumbers[receivedFrame] == 1)
        {
            std::cout << "i have received this msg before , return;" << std::endl;
            return;
        }
        if (ErroredFrames[receivedFrame] == 1 && Xorbyte != checksum)
        {
            return;
        }
        if (Xorbyte == checksum && ExpectedFrame == nmsg->getSeq_Num())
        {
            if (SeqNumbers[receivedFrame] == 0)
            {
                SeqNumbers[receivedFrame] = 1;
            }
            // sending ack
            EV << "At time [" << simTime() + this->PT << "], Node[" << this->node_id << "], sending [ACK] with number [" << nmsg->getSeq_Num() + 1 << "], loss[NO]\n";

            ExpectedFrame++;
            Nodemsg_Base *ack = new Nodemsg_Base("ack");
            ack->setSeq_Num(0);
            ack->setM_Type(1);
            int ackNum = nmsg->getSeq_Num() + 1;
            ack->setAck_Num(ackNum);
            sendDelayed(ack, this->PT + this->TD, "Nodeout");
            int i = receivedFrame + 1;
            this->counter = this->PT;
            while (SeqNumbers[i] == 1)
            {

                Nodemsg_Base *aack = new Nodemsg_Base("ack");
                std::cout << "i = " << i << std::endl;
                std::cout << "SeqNumbers[i] = " << SeqNumbers[i] << std::endl;
                ExpectedFrame++;
                aack->setSeq_Num(0);
                aack->setM_Type(1);
                ackNum++;
                aack->setAck_Num(ackNum);
                EV << "At time [" << simTime() + this->PT + this->TD << "], Node[" << this->node_id << "], sending [ACK] with number [" << ackNum << "], loss[NO]\n";

                sendDelayed(aack, this->PT + this->TD + this->counter, "Nodeout");

                this->counter += this->PT;
                i++;
            }
        }
        else if (Xorbyte == checksum && nmsg->getSeq_Num() < ExpectedFrame + this->WR)
        {
            if (SeqNumbers[receivedFrame] == 0)
            {
                std::cout << "SeqNumbers[receivedFrame] = " << SeqNumbers[receivedFrame] << std::endl;
                SeqNumbers[receivedFrame] = 1;
            }
            std::cout << "SeqNumbers[receivedFrame] = " << SeqNumbers[receivedFrame] << std::endl;

            if (ErroredFrames[ExpectedFrame] < 1)
            {
                ErroredFrames[ExpectedFrame]++;
                Nodemsg_Base *nack = new Nodemsg_Base("nack");
                nack->setSeq_Num(0);
                nack->setM_Type(2);
                int ackNum = nmsg->getSeq_Num();
                nack->setAck_Num(ExpectedFrame);
                EV << "At time [" << simTime() + this->PT << "], Node[" << this->node_id << "], sending [NACK] with number [" << ExpectedFrame << "], loss[NO]\n";

                sendDelayed(nack, this->PT + this->TD, "Nodeout");
            }
        }
        else
        {
            if (ErroredFrames[receivedFrame] == 0)
            {
                ErroredFrames[receivedFrame] = 1;
            }
            // sending nack
            Nodemsg_Base *nack = new Nodemsg_Base("nack");
            nack->setSeq_Num(0);
            nack->setM_Type(2);
            int ackNum = nmsg->getSeq_Num();
            nack->setAck_Num(ackNum);
            EV << "At time [" << simTime() + this->PT << "], Node[" << this->node_id << "], sending [NACK] with number [" << ackNum << "], loss[NO]\n";

            sendDelayed(nack, this->PT + this->TD, "Nodeout");
        }
        std::cout << "exiting the receiver" << std::endl;
    }

    outFile.close();

}