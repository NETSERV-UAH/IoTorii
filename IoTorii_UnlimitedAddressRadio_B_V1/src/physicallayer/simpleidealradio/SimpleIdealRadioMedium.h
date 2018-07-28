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

#ifndef __SRC_PHYSICALLAYER_SIMPLEIDEALRADIO_SIMPLEIDEALRADIOMEDIUM_H
#define __SRC_PHYSICALLAYER_SIMPLEIDEALRADIO_SIMPLEIDEALRADIOMEDIUM_H

#include "src/physicallayer/simpleIdealRadio/SimpleRadioFrame_m.h"
#include "src/physicallayer/simpleidealradio/SimpleIdealRadio.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Units.h"


namespace iotorii {

namespace physicallayer {

using namespace inet;
using namespace physicallayer;

class SimpleIdealRadio;

class SimpleIdealRadioMedium : public cSimpleModule
{

  protected:

    /** @name State */
    //@{
    /**
     * The list of radios that can transmit and receive radio signals on the
     * radio medium. The radios follow each other in the order of their unique
     * id. Radios are only removed from the beginning. This list may contain
     * nullptr values.
     */
    std::vector<SimpleIdealRadio *> radios;

  public:
    SimpleIdealRadioMedium();
    virtual ~SimpleIdealRadioMedium();

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void finish() override;

  public:
    virtual void addRadio(SimpleIdealRadio *radio);
    virtual void removeRadio(SimpleIdealRadio *radio);
    virtual SimpleRadioFrame *startTransmission(cModule *transmitter, Coord startPosition, cPacket *macFrame);
    virtual void sendToNeighbors(SimpleIdealRadio *transmitter, SimpleRadioFrame *radioFrame);
    virtual void sendToAllRadios(SimpleIdealRadio *transmitter, SimpleRadioFrame *frame);
    virtual void sendToRadio(SimpleIdealRadio *transmitter, SimpleIdealRadio *receiver,  SimpleRadioFrame *radioFrame);






};

} // namespace physicallayer

} // namespace iotorii

#endif // ifndef __SRC_PHYSICALLAYER_SIMPLEIDEALRADIO_SIMPLEIDEALRADIOMEDIUM_H

