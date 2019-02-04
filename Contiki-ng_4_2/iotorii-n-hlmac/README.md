Metrics is directly measured in the RPL code.

Repeatedly run the simulation(in Docker):

If you use Docker, log in to Docker.

./simexec-article.sh 





../.././simexec.sh ../../Scenario25.csc ../../../../.. log 1234 20

 ../../.././simexec2.sh ../../../Scenario25.csc ../../../../.. log 123456 20

../../../.././simexec2.sh ../../../25nodes/Scenario25-1.csc ../../../../../.. log 123456 10

log parser (script):

./a.out BASE_NAME SEED NUM_RUN

./a.out log 123456 10

../.././parser.out log 123456 10

