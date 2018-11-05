//
// Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
//                    (1) GIST, University of Alcala, Spain.
//                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
//                    OMNeT++ 5.2.1 & INET 3.6.3
//

cplusplus {{
    #include "inet/linklayer/common/MACAddress.h"
    #include "src/linklayer/common/eGA3Frame.h"

   typedef struct {
       inet::MACAddress MAC;
       iotorii::eGA3Frame eGA3;
   } AddressStruct;
    
    
}}

namespace iotorii;
class noncobject AddressStruct;

packet CSMAFrameIoTorii
{
    AddressStruct srcAddr;
    AddressStruct destAddr;
}
