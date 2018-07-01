//
// Copyright (C) 03/2012 Elisa Rojas
//
/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    INET 3.6.3 adaptation, also adapted for using in the wARP-PATH protocol
*/
/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    INET 3.6.3 adaptation, also adapted for using in the IoTorii(WSN) protocol
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

#ifndef IOTORII_SRC_SIMULATIONMODELS_FLOWMODELS_UDPFLOWGENERATOR_H_
#define IOTORII_SRC_SIMULATIONMODELS_FLOWMODELS_UDPFLOWGENERATOR_H_

#include <omnetpp.h>

#include "src/simulationmodels/flowmodels/FlowGeneratorBase.h"

namespace iotorii {
using namespace inet;

/**
 * It allows the user to configure a simulation model for UDP traffic generation
 *
 * For more info please see the NED file.
 */

class UDPFlowGenerator : public FlowGeneratorBase
{

   protected:
    virtual void initialize(int stage) override;
    virtual void startRandomFlow(); //Redefined, specific for UDP
};



#endif /* IOTORII_SRC_SIMULATIONMODELS_FLOWMODELS_UDPFLOWGENERATOR_H_ */

} //namespace iotorii
