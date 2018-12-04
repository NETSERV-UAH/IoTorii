# IoTorii

Implementation in Contiki-ng by Elisa Rojas, Hedayat Hosseini and David Carrascal :)

Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2), David Carrascal (1);

                     (1) GIST, University of Alcala, Spain.
                     
                     (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
                     
                     
                     
---                     
## Contiki-ng installation

There are different ways to install Contiki-ng, in this guide, we will explain how to install **Contiki-ng on Ubuntu 16.04** and launch the IoT simulator(GUI), called **Cooja**.

---
Requirements:

* **VirtualBox, VMware** (or the software that you use to virtualize. In the case that you have a partition made with Ubuntu 16.04, or this same installed on your machine would also be worth :relaxed:).

* Installation of an .iso of Ubuntu(16.04) in the software that you use to virtualize, in this case VirtualBox.
  * [How to install Ubuntu 16.04 in VirtualBox](https://www.youtube.com/watch?v=j5iFE6zBHPE)
  
---
### Installation

First of all, certain packages are required to build Cooja, let's add them! :turtle: :

> Let's update the list of available packages and their versions.

```bash
sudo apt update 
```

> In case you did not have it yet, it will be necessary to clone the project from GitHub.

```bash
sudo apt-get install git 
```

> While installing Wireshark, select enable the feature that lets non-superuser capture packets (select "yes"), or you can also run > cooja as super-user

```bash
sudo apt install build-essential doxygen curl wireshark python-serial srecord
```

> It is necessary to compile the programs in C for each mote

```bash
sudo apt install gcc-msp430
```

> It's necessary to compile Cooja

```bash
sudo apt install default-jdk ant
```

> Clone the project

```bash
git clone https://github.com/contiki-ng/contiki-ng.git
```

We enter the directory:

```bash
cd contiki-ng
```
And we update the modules with:

```bash
git submodule update --init --recursive
```
And It should let you open Cooja with the following command:

```bash
(Enter first)
cd contiki-ng/tools/cooja 
ant
```
<p align="center">
<img src="https://i.ibb.co/DMRYs6N/1.jpg" />
</p>

---
### Notes
* I only use the hardware  Sky motes, in case you want to use another type of motes, other additional packages may be required.
* Guide created Dec-2018, Contiki-ng 4.2 latest version. The process may vary in the future with the arrival of new versions by Contiki-ng.
