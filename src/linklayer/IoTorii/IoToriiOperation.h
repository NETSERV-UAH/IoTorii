/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    OMNeT++ 5.2.1 & INET 3.6.3
*/

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

#ifndef IOTORII_SRC_LINKLAYER_GA3SWITCH_IOTORIIOPERATION_H
#define IOTORII_SRC_LINKLAYER_GA3SWITCH_IOTORIIOPERATION_H

#include "IHLMACAddressTable.h"
#include "inet/common/INETDefs.h"

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/linklayer/csma/CSMAFrame_m.h"


namespace iotorii {
using namespace inet;


class IoToriiOperation : public cSimpleModule, public ILifecycle
{
  protected:
    // GA3 parameters
    // Switch parameters
    bool isCoreSwitch = false;
    int corePrefix = -1;            // main core prefix, set by ned parameter
    simtime_t coreStartTime;

    //Hello parameters
    simtime_t helloInterval; //"Hello" interval time, every helloInterval seconds a node broadcasts Hello messages
    cMessage *HelloTimer = nullptr;
    long HelloRcvd;                 //Number of Hello messages received
    /** HeT(Hello Table) **/
    std::vector<MACAddress> neighborList;
    int numNeighbors; //number of neighbors discovered by Hello message
    int maxNeighbors; //maximum number of neighbors. changing this value needs to change HLMACAddress and eGA3Frame structure.

    IHLMACAddressTable *hlmacTable = nullptr;

    cMessage *startCoreEvent = nullptr;

    // Parameters for statistics collection
    long numProcessedFrames;
    long numDiscardedFrames;

    bool isOperational = false;    // for lifecycle

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleAndDispatchFrame(CSMAFrame *frame);

    /**
     * Calls handleIncomingFrame() for frames arrived from outside,
     * and processFrame() for self messages.
     */
    virtual void handleMessage(cMessage *msg) override;

    /**
     * Writes statistics.
     */
    virtual void finish() override;

    // for lifecycle:

  public:
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

  protected:

    //Method to create and send hello frames to other switches (currently only at the beginning).
    //Sends broadcast hello messages, src is MAC address and dst is broadcast MAC address
    virtual void sendAndScheduleHello();

    //Sends unicast SetHLMAC message to each neighbor, src is advertised HLMAC address to each node, dst is MAC address of each neighbor
    virtual void sendToNeighbors(CSMAFrame *frame);

    //functionality of sink node
    virtual void startCore(int core);

    //functionality of each node when it receives a SetHLMAC message
    virtual void receiveSetHLMACMessage(CSMAFrame *SetHLMAC);

    //check whether received HLMAC address creates a loop or not.
    virtual bool hasLoop(HLMACAddress hlmac);

    //saves received HLMAC address in the HLMAC table
    virtual void saveHLMAC(HLMACAddress hlmac);


  protected:
    virtual void start();
    virtual void stop();
};

} // namespace iotorii

#endif // ifndef IOTORII_SRC_LINKLAYER_GA3SWITCH_IOTORIIOPERATION_H
