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

#include "src/physicallayer/simpleidealradio/SimpleIdealRadioMedium.h"

namespace iotorii {

namespace physicallayer {

using namespace inet;
using namespace physicallayer;

Define_Module(SimpleIdealRadioMedium);

SimpleIdealRadioMedium::SimpleIdealRadioMedium()
{

}

SimpleIdealRadioMedium::~SimpleIdealRadioMedium()
{

}

void SimpleIdealRadioMedium::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // initialize parameters
        //WATCH_VECTOR(radios);
    }
    else if (stage == INITSTAGE_LAST)
        EV_INFO << "Initialized " << endl;
}

void SimpleIdealRadioMedium::handleMessage(cMessage *message)
{
    throw cRuntimeError("Unknown message");
}

void SimpleIdealRadioMedium::finish()
{

}

void SimpleIdealRadioMedium::addRadio(SimpleIdealRadio *radio)
{
    radios.push_back(radio);
}

void SimpleIdealRadioMedium::removeRadio(SimpleIdealRadio *radio)
{
    int radioIndex = radio->getId() - radios[0]->getId();
    radios[radioIndex] = nullptr;
    int radioCount = 0;
    while (radios[radioCount] == nullptr && radioCount < (int)radios.size())
        radioCount++;
    if (radioCount != 0)
        radios.erase(radios.begin(), radios.begin() + radioCount);
}

SimpleRadioFrame *SimpleIdealRadioMedium::startTransmission(cModule *transmitter, Coord startPosition, cPacket *macFrame)
{
    //Enter_Method_Silent();
    //take(macFrame);
    SimpleRadioFrame *radioFrame = new SimpleRadioFrame;
    //radioFrame->setStartPosition( startPosition);
    radioFrame->encapsulate(macFrame);
    radioFrame->setName(macFrame->getName());

    SimpleIdealRadio *transmitterRadio = check_and_cast<SimpleIdealRadio *>(transmitter);
    sendToNeighbors(transmitterRadio, radioFrame);
    //sendToAllRadios(transmitterRadio, radioFrame);
    return radioFrame;
}

void SimpleIdealRadioMedium::sendToNeighbors(SimpleIdealRadio *transmitter, SimpleRadioFrame *radioFrame)
{

    Coord transmitterPosition = transmitter->getMobility()->getCurrentPosition();
    m communicationRange = transmitter->getCommunicationRange();
    //double communicationRange = transmitter->getCommunicationRange();
    //for(int i=0; i<radios.size(); i++){
    for (auto receiver : radios)
        if ((receiver != nullptr) && (receiver != transmitter)){
            double distance = transmitterPosition.distance(receiver->getMobility()->getCurrentPosition());
            if (distance < communicationRange.get()){  //if (distance <= communicationRange.get()){
            //if (distance <= communicationRange){
                 EV << "Successfull, " << getContainingNode(transmitter)->getFullName() << "sends the packet to the "
                        << getContainingNode(receiver)->getFullName() << ", distance is " << distance
                        << "communication range is " << communicationRange << endl;



                sendToRadio(transmitter, receiver, radioFrame->dup());
        }else
            EV << "Ignored, " << getContainingNode(transmitter)->getFullName() << "sends the packet to the "
                   << getContainingNode(receiver)->getFullName() << ", distance is " << distance
                   << "communication range is " << communicationRange << endl;

    }

    delete radioFrame;
/*    double a=9.6000000000000000, b=19.600000000000000, c=b-a, d,e,f;

      d = floorf(a * 100) / 100; //rounded_down
     e = floorf(a * 100 + 0.5) / 100;  //nearest
      f = ceilf(a * 100) / 100;  //rounded_up

    if ( c== 10)
        EV << "ok1 b-a == 10" << endl;
    if (a<=b-10)
        EV << "ok2 a<=b-10" << endl;
        */
}

void SimpleIdealRadioMedium::sendToAllRadios(SimpleIdealRadio *transmitter, SimpleRadioFrame *frame)
{
    for (auto radio : radios)
        if (radio != nullptr)
            sendToRadio(transmitter, radio, frame->dup());
    delete frame;
}

void SimpleIdealRadioMedium::sendToRadio(SimpleIdealRadio *transmitter, SimpleIdealRadio *receiver,  SimpleRadioFrame *radioFrame)
{
    //simtime_t propagationTime = 0.000001;
    //simtime_t duration = 0.000001;
    cGate *gate = receiver->getRadioGate()->getPathStartGate();

    //transmitter->sendDirect(radioFrame, propagationTime, duration, gate);
    transmitter->sendDirect(radioFrame, gate);

}

} // namespace physicallayer

} // namespace iotorii

