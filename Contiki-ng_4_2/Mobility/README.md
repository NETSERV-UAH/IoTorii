# IoTorii-Mobility

Implementation in Contiki-ng by Elisa Rojas, Hedayat Hosseini and David Carrascal

Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2), David Carrascal (1);

                     (1) GIST, University of Alcala, Spain.
                     
                     (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
                     

---
## Installing the mobility plugin in Contiki-ng

---
### Introduction

To use the plugin in Contiki-NG, we have used the following links.

* **[Source code of the mobility plugin](http://sourceforge.net/p/contikiprojects/code/HEAD/tree/sics.se/mobility/)** This link redirects you to download the mobility plugin for the Cooja simulator. Note that, the code works in old versions of Contiki-OS. To be worked in Contiki-NG, this README file helps you step-by-step.  

* **[A tutorial regarding the mobility of nodes in Cooja](https://anrg.usc.edu/contiki/index.php/Mobility_of_Nodes_in_Cooja)** This tutorial shows how to download, install, and use the mobility plugin in the Cooja Simulator.

* **[Mobility Issues in Contiki-OS 3.0](https://github.com/contiki-os/contiki/issues/2341)** A discussion about issues of the mobility plugin while using Contiki-OS 3.0.

---
### Downloading the plugin

The source code of the mobility plugin is available [here](http://sourceforge.net/p/contikiprojects/code/HEAD/tree/sics.se/mobility/). After downloading the code, you must creat a directory in ~/contiki-ng/tools/cooja/apps.

> Let's change the current directory:
```bash
cd contiki-ng/tools/cooja/apps
```

> Let's create the directory:
```bash
mkdir mobility
```
The downloaded plugin includes a directory named mobility. Copy all contents of that directory into the created directory.

---
### Update the plugin to work in Contiki-NG and building it

* **build.xml** Navigate the current directory to ~/contiki-ng/tools/cooja/apps/mobility/ and open build.xml and change ../../../contiki-2.x/tools/cooja/dist/cooja.jar to ../../dist/cooja.jar at line 7 and save the update.

* **Mobility.java** Navigate the current directory to ~/contiki-ng/tools/cooja/apps/mobility/java/ and open Mobility.java and update the following changes. change "import org.contikios.cooja.GUI;" to "import org.contikios.cooja.Cooja;". Then, change all of "import se.sics.cooja." to "import org.contikios.cooja."
change getGUI() to getCooja(). change "GUI" to "Cooja". Change MessageList to MessageListUI.

* **cooja.config** Navigate the current directory to ~/contiki-ng/tools/cooja/apps/mobility/ and open build.xml and change all "se.sics.cooja.GUI" to "org.contikios.cooja.Cooja" =>tools menu

sudo ant clean

sudo ant jar

---                     
## Using the mobility plugin in Cooja
