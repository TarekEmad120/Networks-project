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

#ifndef __NETWORKS_PROJECT_NODE_H_
#define __NETWORKS_PROJECT_NODE_H_

#include <omnetpp.h>
#include "Nodemsg_m.h"

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
protected:
  int node_id;
  int sttime;
  int WS;
  int WR;
  int TO;
  double PT;
  double TD;
  double ED;
  double DD;
  double LP;
  double counter;
  Nodemsg_Base * tmsg;
  
  int lowest_indexAT_WS;
  int current_index_toSEND;
  int highest_indexAT_WS;
  int Seq_Num;
  int ExpectedFrame;
  int seqnumlost;
  int nackcheck;
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
};

#endif