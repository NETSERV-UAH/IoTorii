/*
 * The implementation of HLMACAddressTable is inspired by the MACAddressTable implementation.
 *  Copyright (C) 2013 OpenSim Ltd.
*/

/*
 * Copyright (C) 2017 Elisa Rojas(1), SeyedHedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *
 * Main paper:
 * Rojas, Elisa, et al. "GA3: scalable, distributed address assignment
 * for dynamic data center networks." Annals of Telecommunications (2017): 1-10.þ
 * DOI: http://dx.doi.org/10.1007/s12243-017-0569-4
 *
 * Developed in OMNet++5.2, based on INET framework.
 * LAST UPDATE OF THE INET FRAMEWORK: inet3.6.2 @ October 2017
*/

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

#include "../IoTorii/HLMACAddressTable.h"

#include <map>
#include <vector>

namespace iotorii {
using namespace inet;


#define MAX_LINE    100

Define_Module(HLMACAddressTable);

std::ostream& operator<<(std::ostream& os, const HLMACAddressTable::AddressEntry& entry)
{
    os << "{VID=" << entry.vid << ", port=" << entry.portno << ", insertionTime=" << entry.insertionTime << "}";
    return os;
}

HLMACAddressTable::HLMACAddressTable()
{
    hlmacTable = new HLMACTable();
    // Set hlmacTable for VLAN ID 0
    vlanHLMACTable[0] = hlmacTable;
}

void HLMACAddressTable::initialize()
{
    agingTime = par("agingTime");
    lastPurge = SIMTIME_ZERO;

    HLMACTable& hlmacTable = *this->hlmacTable;    // magic to hide the '*' from the name of the watch below
    WATCH_MAP(hlmacTable);
}

/**
 * Function reads from a file stream pointed to by 'fp' and stores characters
 * until the '\n' or EOF character is found, the resultant string is returned.
 * Note that neither '\n' nor EOF character is stored to the resultant string,
 * also note that if on a line containing useful data that EOF occurs, then
 * that line will not be read in, hence must terminate file with unused line.
 */

void HLMACAddressTable::handleMessage(cMessage *)
{
    throw cRuntimeError("This module doesn't process messages");
}

/*
 * getTableForVid
 * Returns a MAC Address Table for a specified VLAN ID
 * or nullptr pointer if it is not found
 */

HLMACAddressTable::HLMACTable *HLMACAddressTable::getTableForVid(unsigned int vid)
{
    if (vid == 0)
        return hlmacTable;

    auto iter = vlanHLMACTable.find(vid);
    if (iter != vlanHLMACTable.end())
        return iter->second;
    return nullptr;
}

/*
 * For a known arriving port, V-TAG and destination MAC. It generates a vector with the ports where relay component
 * should deliver the message.
 * returns false if not found
 */

int HLMACAddressTable::getPortForAddress(HLMACAddress& address, unsigned int vid)
{
    Enter_Method("HLMACAddressTable::getPortForAddress()");

    HLMACTable *table = getTableForVid(vid);
    // VLAN ID vid does not exist
    if (table == nullptr)
        return -1;

    auto iter = table->find(address);

    if (iter == table->end()) {
        // not found
        return -1;
    }
    if (iter->second.insertionTime + agingTime <= simTime()) {
        // don't use (and throw out) aged entries
        EV << "Ignoring and deleting aged entry: " << iter->first << " --> port" << iter->second.portno << "\n";
        table->erase(iter);
        return -1;
    }
    return iter->second.portno;
}

/*
 * Register a new MAC address at hlmacTable.
 * True if refreshed. False if it is new.
 */

bool HLMACAddressTable::updateTableWithAddress(int portno, HLMACAddress& address, unsigned int vid)
{
    Enter_Method("HLMACAddressTable::updateTableWithAddress()");
    //if (address.isBroadcast())
    //    return false;

    HLMACTable::iterator iter;
    HLMACTable *table = getTableForVid(vid);

    if (table == nullptr) {
        // MAC Address Table does not exist for VLAN ID vid, so we create it
        table = new HLMACTable();

        // set 'the hlmacTable' to VLAN ID 0
        if (vid == 0)
            hlmacTable = table;

        vlanHLMACTable[vid] = table;
        iter = table->end();
    }
    else
        iter = table->find(address);

    if (iter == table->end()) {
        removeAgedEntriesIfNeeded();

        // Add entry to table
        EV << "Adding entry to Address Table: " << address << " --> port" << portno << "\n";
        (*table)[address] = AddressEntry(vid, portno, simTime());
        return false;
    }
    else {
        // Update existing entry
        EV << "Updating entry in Address Table: " << address << " --> port" << portno << "\n";
        AddressEntry& entry = iter->second;
        entry.insertionTime = simTime();
        entry.portno = portno;
    }
    return true;
}

/*
 * Clears portno MAC cache.
 */

void HLMACAddressTable::flush(int portno)
{
    Enter_Method("HLMACAddressTable::flush():  Clearing gate %d cache", portno);
    for (auto & elem : vlanHLMACTable) {
        HLMACTable *table = elem.second;
        for (auto j = table->begin(); j != table->end(); ) {
            auto cur = j++;
            if (cur->second.portno == portno)
                table->erase(cur);
        }
    }
}

/*
 * Prints verbose information
 */

void HLMACAddressTable::printState()
{
    EV << endl << "MAC Address Table" << endl;
    EV << "VLAN ID    MAC    Port    Inserted" << endl;
    for (auto & elem : vlanHLMACTable) {
        HLMACTable *table = elem.second;
        for (auto & table_j : *table)
            EV << table_j.second.vid << "   " << table_j.first << "   " << table_j.second.portno << "   " << table_j.second.insertionTime << endl;
    }
}

void HLMACAddressTable::copyTable(int portA, int portB)
{
    for (auto & elem : vlanHLMACTable) {
        HLMACTable *table = elem.second;
        for (auto & table_j : *table)
            if (table_j.second.portno == portA)
                table_j.second.portno = portB;

    }
}

void HLMACAddressTable::removeAgedEntriesFromVlan(unsigned int vid)
{
    HLMACTable *table = getTableForVid(vid);
    if (table == nullptr)
        return;
    // TODO: this part could be factored out
    for (auto iter = table->begin(); iter != table->end(); ) {
        auto cur = iter++;    // iter will get invalidated after erase()
        AddressEntry& entry = cur->second;
        if (entry.insertionTime + agingTime <= simTime()) {
            EV << "Removing aged entry from Address Table: "
               << cur->first << " --> port" << cur->second.portno << "\n";
            table->erase(cur);
        }
    }
}

void HLMACAddressTable::removeAgedEntriesFromAllVlans()
{
    for (auto & elem : vlanHLMACTable) {
        HLMACTable *table = elem.second;
        // TODO: this part could be factored out
        for (auto j = table->begin(); j != table->end(); ) {
            auto cur = j++;    // iter will get invalidated after erase()
            AddressEntry& entry = cur->second;
            if (entry.insertionTime + agingTime <= simTime()) {
                EV << "Removing aged entry from Address Table: "
                   << cur->first << " --> port" << cur->second.portno << "\n";
                table->erase(cur);
            }
        }
    }
}

void HLMACAddressTable::removeAgedEntriesIfNeeded()
{
    simtime_t now = simTime();

    if (now >= lastPurge + 1)
        removeAgedEntriesFromAllVlans();

    lastPurge = simTime();
}

void HLMACAddressTable::clearTable()
{
    for (auto & elem : vlanHLMACTable)
        delete elem.second;

    vlanHLMACTable.clear();
    hlmacTable = nullptr;
}

HLMACAddressTable::~HLMACAddressTable()
{
    for (auto & elem : vlanHLMACTable)
        delete elem.second;
}

void HLMACAddressTable::setAgingTime(simtime_t agingTime)
{
    this->agingTime = agingTime;
}

void HLMACAddressTable::resetDefaultAging()
{
    agingTime = par("agingTime");
}

bool HLMACAddressTable::isPortInTable(int portno, unsigned int vid)
{
    HLMACTable *table = getTableForVid(vid);
        if (table == nullptr)
            return false;

        for (auto iter = table->begin(); iter != table->end();iter++ ) {

            AddressEntry& entry = iter->second;
            if (entry.portno == portno) {
               return true;
            }
        }
        return false;
}

//EXTRA BEGIN
HLMACAddress HLMACAddressTable::getlongestMatchedPrefix(HLMACAddress address, unsigned int vid)
{
    HLMACTable *table = getTableForVid(vid);
        if (table == nullptr)
            return HLMACAddress::UNSPECIFIED_ADDRESS;

        while(address != HLMACAddress::UNSPECIFIED_ADDRESS){
            auto iter = table->find(address);
            if (iter != table->end()) {
                if (iter->second.insertionTime + agingTime <= simTime()) {
                    // don't use (and throw out) aged entries
                    EV << "Ignoring and deleting aged entry: " << iter->first << " --> port" << iter->second.portno << "\n";
                    table->erase(iter);
                }else
                    return address;
            }
            address.removeLastId();
        }//while

        return HLMACAddress::UNSPECIFIED_ADDRESS;


}
//EXTRA END

} // namespace iotorii

