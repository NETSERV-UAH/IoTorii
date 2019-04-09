# IoTorii

Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);

                     (1) GIST, University of Alcala, Spain.
                     
                     (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
                     
Developed in OMNet++5.2.1, based on INET framework.

LAST UPDATE OF THE INET FRAMEWORK: inet3.6.3 @ December 22, 2017


### Compile and run ###

To compile and run the project, you can follow the next steps.

1. After extracting and before installing OMNeT++, exchange "PREFER_CLANG=yes" to "PREFER_CLANG=no" in the "configure.user" file in your OMNeT++ installation folder.
1. Install OMNeT++.
1. To check if your OMNeT++ correctly work, run an example of OMNeT++ such as dyna, aloha, tictoc, or etc.
1. Install and build INET.
1. To check if your INET correctly work, run an example of INET such as inet/examples/adhoc/ieee80211, or etc.
1. Since our files only include implementation code, the project may not be probably imported by IDE, and you must manually create a new OMNeT++ project by the same name. So, do the next substeps.
   1. Click on the menu of "File".
   1. Select "New".
   1. Select "OMNeT++ Project".
   1. Type the name of project.
   1. Click on the "Next".
   1. Select "Empty Project".
   1. click on the "Finish".
1. Copy all the files/folders in this folder to the project folder you created.
1. You must introduce INET to your project as a reference/library, so you must do the next substeps.
   1. Right-click on the project in "Project Explorer" window.
   1. Select "Properties".
   1. Select "Project Reference" in the left list.
   1. Select "INET" in the right list.
   1. Click on the "Apply and close".
1. Build and run the project.
