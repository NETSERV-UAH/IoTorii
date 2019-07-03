# IoTorii

Implementation in Contiki-ng by Elisa Rojas, Hedayat Hosseini and David Carrascal

Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2), David Carrascal (1);

                     (1) GIST, University of Alcala, Spain.
                     
                     (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
                     

---
### The contents of this folder

* **[iotorii-n-hlmac](https://github.com/gistnetserv-uah/IoTorii/tree/master/Contiki-ng_4_2/iotorii-n-hlmac)** is the current version of the original IoTorii which we used to collect our results. In this simulation, there is a configuration file ([here, for a root node](https://github.com/gistnetserv-uah/IoTorii/blob/master/Contiki-ng_4_2/iotorii-n-hlmac/code/root/project-conf.h#L56) and [here, for other nodes](https://github.com/gistnetserv-uah/IoTorii/blob/master/Contiki-ng_4_2/iotorii-n-hlmac/code/common-node/project-conf.h#L56)) to set parameters. One of the parameters is "HLMAC_CONF_MAX_HLMAC". This parameter is used to identify how many HLMAC addresses can be assigned to a node. For example, if "#define HLMAC_CONF_MAX_HLMAC 1", it means that one HLMAC address can be assigned to each node. If "#define HLMAC_CONF_MAX_HLMAC 3", it means that at most 3 HLMAC address can be assigned to each node.

* **[iotorii-1-hlmac](https://github.com/gistnetserv-uah/IoTorii/tree/master/Contiki-ng_4_2/iotorii-1-hlmac)** is a simple version of the original IoTorii which is same as iotorii-n-hlmac using "#define HLMAC_CONF_MAX_HLMAC 1". This is the first version of the original IoTorii, but we didn't use this simulation to collect our results because it cannot assign 3 HLMAC addresses to each node. Note that the iotorii-n-hlmac version is more complete than iotorii-1-hlmac in terms of queuing the control packets (SetHLMAC) and ... .

* **[iotorii-n-hlmac-2](https://github.com/gistnetserv-uah/IoTorii/tree/master/Contiki-ng_4_2/iotorii-n-hlmac-2)** is a new version of IoTorii and isn't the original version. Since the capacity of a packet is limited in IEEE802.11, a node may  not advertise its HLMAC address to a/some neighbor(s). Therefore, this neighbors may get a HLMAC address from another node and its hierarchy may increase. In this simulation, if a node has many neighbors, it sends several SetHlMACs to cover all its neighbors.

* **[iotorii-n-hlmac-3](https://github.com/gistnetserv-uah/IoTorii/tree/master/Contiki-ng_4_2/iotorii-n-hlmac-3)** is another version of IoTorii and isn't the original version. If a node has a HLMAC address, it may get a better address after a while. In this simulation, a node can update its address if it receives a better address.

* **[iotorii-all-in-one](https://github.com/gistnetserv-uah/IoTorii/tree/master/Contiki-ng_4_2/iotorii-all-in-one)** can use the advantages of iotorii-n-hlmac,  iotoriin-n-hlmac-2 and iotoriin-n-hlmac-3 together by setting the parameters of "HLMAC_CONF_MAX_HLMAC", "IOTORII_CONF_ENABLE_EXCHANGE_ADDRESS" and "IOTORII_CONF_MULTI_MESSAGE".  

Note that we can define each version of IoTorii as an optimization problem, i.e. min Delay for iotorii-n-hlmac and ... , or a new optimization problem with a new metric.
 
---                     
## Contiki-ng installation

There are different ways to install Contiki-ng, in this guide, we will explain how to install Contiki-ng on Linux.

---
Requirements:

* **VirtualBox, VMware** (or the software that you use to virtualize. In the case that you have a partition made with Ubuntu 16.04, or this same installed on your machine would also be worth :relaxed:).

* Installation of an .iso of Ubuntu(16.04) in the software that you use to virtualize, in this case VirtualBox.
  * [How to install Ubuntu 16.04 in VirtualBox](https://www.youtube.com/watch?v=j5iFE6zBHPE)
  
---
### Installation

#### Docker image (Linux)

First of all, install Docker. An easy way to install Docker on deb and rpm distributions of linux is using a [shell script](https://get.docker.com/) provided by docker.com (to install Docker on Arch Linux, use [this link](https://wiki.archlinux.org/index.php/Docker)). The script finds a suitable Docker version for your system. If there is, it will install Docker on the system. Let's run the script: :

> Let's get the script.
```bash
curl -fsSL get.docker.com -o get-docker.sh
```

> Let's run the Script.
```bash
sh get-docker.sh
```

> Let's see the Docker version after the installation.
```bash
docker version
```

> Now, the Root user can use Docker. Let's add the access to your user-name.
```bash
sudo usermod -aG docker user-name
```

> If your linux distribution is based on the deb, you can restart(or log out then log in again), and Docker is ready to use. Otherwise if the linux distribution is based on the rpm, you must start the Docker service. Let's start the service.
```bash
systemctl start docker
```
```bash
systemctl status docker
```

> Let's start the Docker while the system is turning on.
```bash
systemctl enable docker
```

> The steps to install the Contiki-NG is according to the Contiki-NG Wiki. A brief review is available here. Let's download the Contiki-NG image.
```bash
docker pull contiker/contiki-ng
```
> In case you did not have git yet, it will be necessary to clone the project from GitHub.
```bash
sudo apt install git
```

> ([Noted from the Contiki-NG Wiki](https://github.com/contiki-ng/contiki-ng/wiki/Docker)) "This will automatically download contiker/contiki-ng:latest, which is the image used in Travis and which we recommend for development. The image is meant for use with Contiki-NG as a bind mount, which means you make the Contiki-NG repository on the host accessible from inside the container. This way, you can work on the codebase using host tools / editors, and build/run commands on the same codebase from the container. If you do not have it already, you need to check out Contiki-NG." 
```bash
git clone https://github.com/contiki-ng/contiki-ng.git
```
```bash
cd contiki-ng
```
```bash
git submodule update --init --recursive
```

> ([Noted from the Contiki-NG Wiki](https://github.com/contiki-ng/contiki-ng/wiki/Docker)) "Then, it is a good idea to create an alias that will help start docker with all required options. On Linux, you can add the following to ~/.profile:"
```bash
export CNG_PATH=<absolute-path-to-your-contiki-ng>
```
```bash
alias contiker="docker run --privileged --mount type=bind,source=$CNG_PATH,destination=/home/user/contiki-ng -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v /dev/bus/usb:/dev/bus/usb -ti contiker/contiki-ng"
```

#### Native toolchain installation (Linux)

In this section, we will explain how to install **Contiki-ng on Ubuntu 16.04** and launch the IoT simulator(GUI), called **Cooja**.

First of all, certain packages are required to build Cooja, let's add them! :turtle: :

> Let's update the list of available packages and their versions.

```bash
sudo apt update 
```

> In case you did not have it yet, it will be necessary to clone the project from GitHub.

```bash
sudo apt-get install git 
```

> While installing Wireshark, select enable the feature that lets non-superuser capture packets (select "yes"), or you can also run  cooja as super-user

```bash
sudo apt install build-essential doxygen curl wireshark python-serial srecord
```

> It is necessary to compile the programs in C for each mote. 

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
