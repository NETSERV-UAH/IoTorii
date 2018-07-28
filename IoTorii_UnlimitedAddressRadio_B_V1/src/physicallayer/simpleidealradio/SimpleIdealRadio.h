// this module is inspired from physical layer of INET framework
// Copyright (C) 2013 OpenSim Ltd.
//

/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    OMNeT++ 5.2.1 & INET 3.6.3
*/

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __SRC_PHYSICALLAYER_SIMPLEIDEALRADIO_SIMPLEIDEALRADIO_H
#define __SRC_PHYSICALLAYER_SIMPLEIDEALRADIO_SIMPLEIDEALRADIO_H

#include "src/physicallayer/simpleIdealRadio/SimpleRadioFrame_m.h"
#include "src/physicallayer/simpleidealradio/SimpleIdealRadioMedium.h"
#include "inet/physicallayer/base/packetlevel/PhysicalLayerBase.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Units.h"

namespace iotorii {

namespace physicallayer {

using namespace inet;
using namespace physicallayer;

class SimpleIdealRadioMedium;

 class SimpleIdealRadio : public PhysicalLayerBase
{
  protected:

    IMobility *mobility;

    SimpleIdealRadioMedium *medium = nullptr;

    /**
     * The module id of the medium model.
     */
    int mediumModuleId = -1;


    m communicationRange;
    //double communicationRange;
    //float communicationRange;

    /** Gates */
    //@{
    cGate *upperLayerOut = nullptr;
    cGate *upperLayerIn = nullptr;
    cGate *radioIn = nullptr;
    //@}

  public:
    SimpleIdealRadio();
    virtual ~SimpleIdealRadio();

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenDown(cMessage *message);
    virtual void handleMessageWhenUp(cMessage *message);
    virtual void handleSelfMessage(cMessage *message);
    virtual void handleUpperCommand(cMessage *message);
    virtual void handleLowerCommand(cMessage *message);
    virtual void handleUpperPacket(cPacket *packet);
    virtual void handleLowerPacket(SimpleRadioFrame *radioFrame);
    virtual bool handleNodeStart(IDoneCallback *doneCallback);
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback);
    virtual void handleNodeCrash();
    virtual void startTransmission(cPacket *macFrame);
    virtual void sendUp(cPacket *macFrame);

  public:
    virtual cGate *getRadioGate() { return radioIn; }

    virtual m getCommunicationRange() {return communicationRange;}
    //virtual double getCommunicationRange() {return communicationRange;}

    virtual IMobility *getMobility() { return mobility; }

};

} // namespace physicallayer

} // namespace iotorii

#endif // ifndef __SRC_PHYSICALLAYER_SIMPLEIDEALRADIO_SIMPLEIDEALRADIO_H

