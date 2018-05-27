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

#ifndef IOTORII_SRC_ADAPTION_6LOWPAN_H
#define IOTORII_SRC_ADAPTION_6LOWPAN_H

#include "inet/common/INETDefs.h"

#include "inet/common/lifecycle/ILifecycle.h"

namespace iotorii {
using namespace inet;

class _6LoWPAN : public cSimpleModule, public ILifecycle
{
public:
  /** @brief Gate ids */
  //@{
  int netIn;
  int netOut;
  int ifIn;
  int ifOut;
  //@}

  int udpHeaderLength;
  int ipv6HeaderLength;  //HC1 header
  int icmpHeaderLength;  //only for ping traffic
  int nsHeaderLength;
  int naHeaderLength;

  bool isOperational;    // for lifecycle

  protected:
    /** @brief Length of the header*/
  public:

    _6LoWPAN();

    ~_6LoWPAN();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

  protected:

    virtual void initialize(int stage) override;

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

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

  protected:

    virtual void handleUpperPacket(cMessage *msg);

    virtual void handleLowerPacket(cMessage *msg);

    virtual void sendUp(cMessage *message);

    virtual void sendDown(cMessage *message);

    virtual bool isUpperMessage(cMessage *message);

    virtual bool isLowerMessage(cMessage *message);

    virtual void start();

    virtual void stop();
};

} // namespace iotorii

#endif // ifndef IOTORII_SRC_ADAPTION_6LOWPAN_H

