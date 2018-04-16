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

#ifndef IOTORII_SRC_LINKLAYER_GA3SWITCH_MACRELAYUNITGA3_H
#define IOTORII_SRC_LINKLAYER_GA3SWITCH_MACRELAYUNITGA3_H

#include "inet/common/INETDefs.h"

#include "inet/common/lifecycle/ILifecycle.h"
#include "src/linklayer/GA3switch/IHLMACAddressTable.h"
#include "inet/linklayer/csma/CSMAFrame_m.h"


namespace iotorii {
using namespace inet;


class MACRelayUnitGA3 : public cSimpleModule, public ILifecycle
{
  protected:
    // GA3 parameters
    // Switch parameters
    unsigned int broadcastSeed = 0;     //Seed for random broadcasting (to be configured at the *.ini file)

    bool isCoreSwitch = false;
    int corePrefix = -1;            // main core prefix, seted by ned parameter
    HLMACAddress *switchPrio = nullptr; //switch priority
    std::vector<int> corePrefixes;  //other core prefixes

    //Hello parameters
    simtime_t helloInterval; //"Hello" interval time, every helloInterval seconds a node broadcasts Hello messages
    cMessage *HelloTimer;
    long HelloRcvd;                 //Number of Hello messages received
    /** HeT(Hello Table) **/
    std::vector<MACAddress> neighborList;
    int numVirtualPorts; //number of neighbors discovered by Hello message


    // port parameters
    int *ports;         // a mapping between port# & id, indexs indicate port#, values indicate IDs for each port
    HLMACAddress **prio = nullptr; // a mapping between port# & highest Priority for each port, indexs indicate port#, values indicate highest Priority for each port

    IHLMACAddressTable *hlmacTable = nullptr;

    //cMessage *startCoreEvent = nullptr;
    cMessage *startPortEvent;
    int startPortQueue = -1;
    double timeBetweenCores, timeBetweenPorts;

    // Parameters for statistics collection
    long numProcessedFrames = 0;
    long numDiscardedFrames = 0;

    bool isOperational = false;    // for lifecycle

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    /**
     * Updates address table with source address, determines output port
     * and sends out (or broadcasts) frame on ports. Includes calls to
     * updateTableWithAddress() and getPortForAddress().
     *
     * The message pointer should not be referenced any more after this call.
     */
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

    //using in GA3

    void sendHello();

    void startGA3Process();

    //using in GA3
    virtual void flood(CSMAFrame *frame, int inputport);

    //using in GA3
    virtual void startCore(int core,int port);

    //using in GA3
    virtual void receiveMessage(CSMAFrame *SetHLMAC,int inputPort);




  protected:
    virtual void start();
    virtual void stop();
};

} // namespace iotorii

#endif // ifndef IOTORII_SRC_LINKLAYER_GA3SWITCH_MACRELAYUNITGA3_H

