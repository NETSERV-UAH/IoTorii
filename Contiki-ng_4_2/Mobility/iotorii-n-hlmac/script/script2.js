/* Make test automatically fail (timeout) after 30 simulated seconds */
//TIMEOUT(31000);

var i, idMax = 0;
var numHLMAC = new Array(500);

for (i=0; i<500; i++)
    numHLMAC[i] = 0;
GENERATE_MSG(30000, "check convergence");
while (true) {
        if (msg.equals("check convergence")){
            log.log(time + " " + msg + "\n");
            checkConvergence(idMax, numHLMAC);            
        }else
        {
	       log.log(time + " ID:" + id + " " + msg + "\n");
           if (id > idMax)
            idMax = id;
           if ((msg.indexOf("Periodic Statistics: HLMAC address:") > -1) && (msg.indexOf("saved to HLMAC table.") > -1))
            numHLMAC[id-1]++;
        }
    YIELD();
}

function checkConvergence(idMax, numHLMAC){
    var i, converged = 1;
    for (i=0; i<idMax; i++)
        if (numHLMAC[i] == 0)
            converged = 0;
    
    if (converged == 1){
        log.log("converged\n");
        //log.testOK();  //It doesn't correctly work for the Cooja mote
        log.log("TEST OK\n");
        exit(0);
    }else{
        log.log("Not converged\n");
        log.testFailed(); /* Report test failure and quit */
    }
}
