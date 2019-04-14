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

#ifndef IOTORII_SRC_LINKLAYER_GA3SWITCH_HLMACADDRESSTABLE_H_
#define IOTORII_SRC_LINKLAYER_GA3SWITCH_HLMACADDRESSTABLE_H_

//#include "inet/linklayer/common/MACAddress.h"
#include "../IoTorii/IHLMACAddressTable.h"
#include "src/linklayer/common/HLMACAddress.h"

namespace iotorii {
using namespace inet;

/**
 * This module handles the mapping between ports and MAC addresses. See the NED definition for details.
 */
class HLMACAddressTable : public cSimpleModule, public IHLMACAddressTable
{
  protected:
    struct AddressEntry
    {
        unsigned int vid = 0;    // VLAN ID
        int portno = -1;    // Input port
        simtime_t insertionTime;    // Arrival time of Lookup Address Table entry
        AddressEntry() {}
        AddressEntry(unsigned int vid, int portno, simtime_t insertionTime) :
            vid(vid), portno(portno), insertionTime(insertionTime) {}
    };

    friend std::ostream& operator<<(std::ostream& os, const AddressEntry& entry);

    struct MAC_compare
    {
        bool operator()(const HLMACAddress& u1, const HLMACAddress& u2) const { return u1.compareTo(u2) < 0; }
    };

    typedef std::map<HLMACAddress, AddressEntry, MAC_compare> HLMACTable;
    typedef std::map<unsigned int, HLMACTable *> VlanHLMACTable;

    simtime_t agingTime;    // Max idle time for address table entries
    simtime_t lastPurge;    // Time of the last call of removeAgedEntriesFromAllVlans()
    HLMACTable *hlmacTable = nullptr;    // VLAN-unaware address lookup (vid = 0)
    VlanHLMACTable vlanHLMACTable;    // VLAN-aware address lookup

  protected:

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    /**
     * @brief Returns a MAC Address Table for a specified VLAN ID
     */
    HLMACTable *getTableForVid(unsigned int vid);

  public:

    HLMACAddressTable();
    ~HLMACAddressTable();

  public:
    // Table management

    /**
     * @brief For a known arriving port, V-TAG and destination MAC. It finds out the port where relay component should deliver the message
     * @param address MAC destination
     * @param vid VLAN ID
     * @return Output port for address, or -1 if unknown.
     */
    virtual int getPortForAddress(HLMACAddress& address, unsigned int vid = 0) override;

    /**
     * @brief Register a new MAC address at HLMACTable.
     * @return True if refreshed. False if it is new.
     */
    virtual bool updateTableWithAddress(int portno, HLMACAddress& address, unsigned int vid = 0) override;

    /**
     *  @brief Clears portno cache
     */
    // TODO: find a better name
    virtual void flush(int portno) override;

    /**
     *  @brief Prints cached data
     */
    virtual void printState() override;

    /**
     * @brief Copy cache from portA to portB port
     */
    virtual void copyTable(int portA, int portB) override;

    /**
     * @brief Remove aged entries from a specified VLAN
     */
    virtual void removeAgedEntriesFromVlan(unsigned int vid = 0) override;
    /**
     * @brief Remove aged entries from all VLANs
     */
    virtual void removeAgedEntriesFromAllVlans() override;

    /*
     * It calls removeAgedEntriesFromAllVlans() if and only if at least
     * 1 second has passed since the method was last called.
     */
    virtual void removeAgedEntriesIfNeeded() override;

    /**
     * For lifecycle: clears all entries from the vlanHLMACTable.
     */
    virtual void clearTable() override;

    /*
     * Some (eg.: STP, RSTP) protocols may need to change agingTime
     */
    virtual void setAgingTime(simtime_t agingTime) override;
    virtual void resetDefaultAging() override;

    virtual bool isPortInTable(int portno, unsigned int vid = 0) override;

    //EXTRA BEGIN
    virtual HLMACAddress getlongestMatchedPrefix(HLMACAddress hlmac, unsigned int vid = 0) override;

    virtual unsigned int getNumberOfAddresses(unsigned int vid = 0) override;

    //Used for hopCount metric
    virtual HLMACAddress getAddress(unsigned int addressIndex, unsigned int vid = 0) override;

    //Used for hopCount metric
    virtual int getMinHopCount(HLMACAddress dstAddress, unsigned int vid) override;

    //EXTRA END

};

} // namespace iotorii

#endif // ifndef IOTORII_SRC_LINKLAYER_GA3SWITCH_HLMACADDRESSTABLE_H_

