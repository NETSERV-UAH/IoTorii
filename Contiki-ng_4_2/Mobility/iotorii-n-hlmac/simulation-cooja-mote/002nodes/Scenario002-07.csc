<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[APPS_DIR]/mrm</project>
  <project EXPORT="discard">[APPS_DIR]/mspsim</project>
  <project EXPORT="discard">[APPS_DIR]/avrora</project>
  <project EXPORT="discard">[APPS_DIR]/serial_socket</project>
  <project EXPORT="discard">[APPS_DIR]/powertracker</project>
  <simulation>
    <title>02 Nodes 07</title>
    <randomseed>generated</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      org.contikios.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>100.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
	<motetype>
		org.contikios.cooja.contikimote.ContikiMoteType
		<identifier>cooja1</identifier>
		<description>Cooja Sink</description>
		<source>[CONTIKI_DIR]/examples/iotorii-n-hlmac/code/root/iotorii-root.c</source>
		<commands>make iotorii-root.cooja TARGET=cooja</commands>
		<moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
		<moteinterface>org.contikios.cooja.interfaces.Battery</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiVib</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
		<moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiButton</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiClock</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiLED</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiEEPROM</moteinterface>
		<moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
		<moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
		<symbols>false</symbols>
	</motetype>
	<motetype>
		org.contikios.cooja.contikimote.ContikiMoteType
		<identifier>cooja2</identifier>
		<description>Cooja Mote</description>
		<source>[CONTIKI_DIR]/examples/iotorii-n-hlmac/code/common-node/iotorii-common-node.c</source>
		<commands>make iotorii-common-node.cooja TARGET=cooja</commands>
		<moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
		<moteinterface>org.contikios.cooja.interfaces.Battery</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiVib</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
		<moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiButton</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiClock</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiLED</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
		<moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiEEPROM</moteinterface>
		<moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
		<moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
		<symbols>false</symbols>
	</motetype>
	<mote>
		<interface_config>
		org.contikios.cooja.interfaces.Position
		<x>19.808526311968496</x>
		<y>22.133179123346103</y>
		<z>0.0</z>
		</interface_config>
		<interface_config>
		org.contikios.cooja.contikimote.interfaces.ContikiMoteID
		<id>1</id>
		</interface_config>
		<interface_config>
		org.contikios.cooja.contikimote.interfaces.ContikiRadio
		<bitrate>250.0</bitrate>
		</interface_config>
		<interface_config>
		org.contikios.cooja.contikimote.interfaces.ContikiEEPROM
		<eeprom>AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==</eeprom>
		</interface_config>
		<motetype_identifier>cooja1</motetype_identifier>
	</mote>
	<mote>
		<interface_config>
		org.contikios.cooja.interfaces.Position
		<x>8.48478197285425</x>
		<y>3.390019084385218</y>
		<z>0.0</z>
		</interface_config>
		<interface_config>
		org.contikios.cooja.contikimote.interfaces.ContikiMoteID
		<id>2</id>
		</interface_config>
		<interface_config>
		org.contikios.cooja.contikimote.interfaces.ContikiRadio
		<bitrate>250.0</bitrate>
		</interface_config>
		<interface_config>
		org.contikios.cooja.contikimote.interfaces.ContikiEEPROM
		<eeprom>AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==</eeprom>
		</interface_config>
		<motetype_identifier>cooja2</motetype_identifier>
	</mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>280</width>
    <z>5</z>
    <height>160</height>
    <location_x>400</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <moterelations>true</moterelations>
      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.GridVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <viewport>16.781879570553613 0.0 0.0 16.781879570553613 -43.40744614422106 -41.16361928875843</viewport>
    </plugin_config>
    <width>400</width>
    <z>4</z>
    <height>400</height>
    <location_x>1</location_x>
    <location_y>1</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <formatted_time />
      <coloring />
    </plugin_config>
    <width>895</width>
    <z>3</z>
    <height>240</height>
    <location_x>400</location_x>
    <location_y>160</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.TimeLine
    <plugin_config>
      <mote>0</mote>
      <mote>1</mote>
      <showRadioRXTX />
      <showRadioHW />
      <showLEDs />
      <zoomfactor>500.0</zoomfactor>
    </plugin_config>
    <width>1295</width>
    <z>2</z>
    <height>166</height>
    <location_x>0</location_x>
    <location_y>450</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Notes
    <plugin_config>
      <notes>Enter notes here</notes>
      <decorations>true</decorations>
    </plugin_config>
    <width>615</width>
    <z>1</z>
    <height>160</height>
    <location_x>680</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>/* Make test automatically fail (timeout) after 30 simulated seconds */

var i, idMax = 0;
var numHLMAC = new Array(500);

for (i=0; i&lt;500; i++)
    numHLMAC[i] = 0;
GENERATE_MSG(30000, "check convergence");
while (true) {
        if (msg.equals("check convergence")){
            log.log(time + " " + msg + "\n");
            checkConvergence(idMax, numHLMAC);            
        }else
        {
	       log.log(time + " ID:" + id + " " + msg + "\n");
           if (id &gt; idMax)
            idMax = id;
           if ((msg.indexOf("Periodic Statistics: HLMAC address:") &gt; -1) &amp;&amp; (msg.indexOf("saved to HLMAC table.") &gt; -1))
            numHLMAC[id-1]++;
        }
    YIELD();
}

function checkConvergence(idMax, numHLMAC){
    var i, converged = 1;
    for (i=0; i&lt;idMax; i++)
        if (numHLMAC[i] == 0)
            converged = 0;
    
    if (converged == 1){
        log.log("converged\n");
		//log.testOK();
		log.log("TEST OK");
		exit(0);
    }else{
        log.log("Not converged\n");
        log.testFailed(); /* Report test failure and quit */
    }
}</script>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>0</z>
    <height>526</height>
    <location_x>698</location_x>
    <location_y>56</location_y>
  </plugin>
</simconf>

