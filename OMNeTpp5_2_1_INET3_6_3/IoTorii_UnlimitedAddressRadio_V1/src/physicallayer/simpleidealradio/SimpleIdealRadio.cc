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

#include "src/physicallayer/simpleidealradio/SimpleIdealRadio.h"

namespace iotorii {

namespace physicallayer {

Define_Module(SimpleIdealRadio);


SimpleIdealRadio::SimpleIdealRadio() :
mobility(nullptr),
medium(nullptr),
mediumModuleId(-1),
communicationRange(NaN)
{}

SimpleIdealRadio::~SimpleIdealRadio()
{
    // NOTE: can't use the medium module here, because it may have been already deleted
    cModule *medium = getSimulation()->getModule(mediumModuleId);
    if (medium != nullptr)
        check_and_cast<SimpleIdealRadioMedium *>(medium)->removeRadio(this);
}

void SimpleIdealRadio::initialize(int stage)
{
    PhysicalLayerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        //medium = getModuleFromPar<SimpleIdealRadioMedium>(par("radioMediumModule"), this);
        medium = check_and_cast<SimpleIdealRadioMedium *>(getModuleByPath(par("radioMediumModule")));
        //medium = check_and_cast<SimpleIdealRadioMedium *>(this->getParentModule()->getParentModule()->getSubmodule("radioMedium"));
        mediumModuleId = check_and_cast<cModule *>(medium)->getId();

        //mobility = getModuleFromPar<IMobility>(par("mobilityModule"), getContainingNode(this));
        mobility = check_and_cast<IMobility *>(getContainingNode(this)->getSubmodule("mobility"));
        upperLayerIn = gate("upperLayerIn");
        upperLayerOut = gate("upperLayerOut");
        radioIn = gate("radioIn");
        radioIn->setDeliverOnReceptionStart(true);
        double compar=par("communicationRange").doubleValue();
        communicationRange = m(compar);
        //communicationRange = m(par("communicationRange").doubleValue());
        //communicationRange = par("communicationRange").doubleValue();
        WATCH(communicationRange);

    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        medium->addRadio(this);
    }
    else if (stage == INITSTAGE_LAST) {
        EV_INFO << "Initialized " << endl;
    }
}

void SimpleIdealRadio::handleMessageWhenDown(cMessage *message)
{
    if (message->getArrivalGate() == radioIn)
        delete message;
    else
        OperationalBase::handleMessageWhenDown(message);
}

void SimpleIdealRadio::handleMessageWhenUp(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else if (message->getArrivalGate() == upperLayerIn) {
        if (!message->isPacket()) {
            handleUpperCommand(message);
            delete message;
        }
        else
            handleUpperPacket(check_and_cast<cPacket *>(message));
    }
    else if (message->getArrivalGate() == radioIn) {
        if (!message->isPacket()) {
            handleLowerCommand(message);
            delete message;
        }
        else
            handleLowerPacket(check_and_cast<SimpleRadioFrame *>(message));
    }
    else
        throw cRuntimeError("Unknown arrival gate '%s'.", message->getArrivalGate()->getFullName());
}

void SimpleIdealRadio::handleSelfMessage(cMessage *message)
{
    throw cRuntimeError("Unknown self message");
}

void SimpleIdealRadio::handleUpperCommand(cMessage *message)
{
    throw cRuntimeError("Unsupported command");
}

void SimpleIdealRadio::handleLowerCommand(cMessage *message)
{
    throw cRuntimeError("Unsupported command");
}

void SimpleIdealRadio::handleUpperPacket(cPacket *packet)
{
    emit(LayeredProtocolBase::packetReceivedFromUpperSignal, packet);
    startTransmission(packet);
}

void SimpleIdealRadio::handleLowerPacket(SimpleRadioFrame *radioFrame)
{

    EV_INFO << "Reception started: " << radioFrame->getName() << endl;
    cPacket *macFrame = radioFrame->decapsulate();
    sendUp(macFrame);
}

bool SimpleIdealRadio::handleNodeStart(IDoneCallback *doneCallback)
{
    return PhysicalLayerBase::handleNodeStart(doneCallback);
}

bool SimpleIdealRadio::handleNodeShutdown(IDoneCallback *doneCallback)
{
    return PhysicalLayerBase::handleNodeShutdown(doneCallback);
}

void SimpleIdealRadio::handleNodeCrash()
{
    PhysicalLayerBase::handleNodeCrash();
}

void SimpleIdealRadio::startTransmission(cPacket *macFrame)
{
    EV_INFO << "Transmission started: " << macFrame->getName() << endl;

    //SimpleRadioFrame *radioFrame = check_and_cast<SimpleRadioFrame *>(medium->startTransmission(this, mobility->getCurrentPosition(), macFrame));
    medium->startTransmission(this, mobility->getCurrentPosition(), macFrame);

}

void SimpleIdealRadio::sendUp(cPacket *macFrame)
{
    EV_INFO << "Sending up " << macFrame->getName() << endl;
    send(macFrame, upperLayerOut);
}

} // namespace physicallayer

} // namespace iotorii

