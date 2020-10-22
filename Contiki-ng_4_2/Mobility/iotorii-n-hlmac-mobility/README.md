Build:

Change current directori to ~/contiki-ng/examples/iotorii-n-hlmac-mobility/code/common-node
To build for sky mote: make TARGET=sky
To clean: make clean TARGET=sky

Change current directori to ~/contiki-ng/examples/iotorii-n-hlmac-mobility/code/root
To build for sky mote: make TARGET=sky
To clean: make clean TARGET=sky

---
Repeatedly run the simulation(in Docker):

If you use Docker, log in to Docker.

To run the Sky mote simulation:
./simexec-article-sky.sh 

To run the Cooja mote simulation:
./simexec-article-cooja.sh 





../.././simexec.sh ../../Scenario25.csc ../../../../.. log 1234 20

 ../../.././simexec2.sh ../../../Scenario25.csc ../../../../.. log 123456 20

../../../.././simexec2.sh ../../../25nodes/Scenario25-1.csc ../../../../../.. log 123456 10

log parser (script):

./a.out BASE_NAME SEED NUM_RUN

./a.out log 123456 10

../.././parser.out log 123456 10

