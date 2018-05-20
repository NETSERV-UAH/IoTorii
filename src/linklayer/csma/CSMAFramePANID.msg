//
// Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
//                    (1) GIST, University of Alcala, Spain.
//                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
//                    OMNeT++ 5.2.1 & INET 3.6.3
//

cplusplus {{
    #include "inet/linklayer/csma/CSMAFrame_m.h"
    #include "inet/linklayer/common/MACAddress.h"
    
}}

namespace inet;

class noncobject MACAddress;

packet CSMAFrame;

//
// A CSMA mac packet format definition
//
packet CSMAFramePANID extends CSMAFrame
{
    MACAddress       dstPANID; // Destination PAN ID in form of short address (2 bytes), size is considered in header size parameter
    MACAddress       srcPANID; // Source PAN ID in form of short address (2 bytes), size is considered in header size parameter
}
