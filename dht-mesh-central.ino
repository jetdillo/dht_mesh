
//Argon Controller code for 3D-printing mesh network monitoring system
//Assumes a network of 3 Xenon boards and 1 Argon
//Yes, there is a mixture of String Class and cstring types here because 
//The Particle Mesh series is still a little buggy around where it's okay to use c-strings. 

// Local name of the Argon board running this
char devname[15];

//This holds a short array of nodes and their assigned roles in the network.
typedef struct SENSOR { 
       char device[15];
       char role[15];
       unsigned long lastheard;
}SENSOR;

SENSOR dht_dev[3];

float dht_dry=25.00;
float dht_wet=50.00;

int numsensors=2;

//STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));
STARTUP(System.enableFeature(FEATURE_ETHERNET_DETECTION));

void setup() {
    
    dht_dev[0] = (SENSOR) { "tetrion","printer",0};
    dht_dev[1] = (SENSOR) { "verteron","filament_box",0};
    dht_dev[2] = (SENSOR) { "chroniton", "dryer",0 };

 //   USBSerial1.begin();
    sprintf(devname,"%s","filament-ctl");
    if (System.enableFeature(FEATURE_ETHERNET_DETECTION)) {
        Ethernet.on();
        Particle.publish("FEATURE_ETHERNET_DETECTION"," enabled", 60, PRIVATE);
        if(Ethernet.ready()) {
             Particle.publish("Ethernet ready (has link and IP address) ",Ethernet.localIP().toString().c_str(), 60, PRIVATE); 

        } else {
                 Particle.publish("Ethernet detected but not ready","check cable",60,PRIVATE );
        }
    } else {
            Particle.publish("FEATURE_ETHERNET_DETECTION"," failed",60,PRIVATE);
          //  WiFi.on();
         //   WiFi.connect();
    }


//Subscribe to a few topics the nodes will talk to us on. 
//Note these are local Mesh topics and not published to the Particle cloud 
    Mesh.subscribe("DHT_DATA",dhtDataHandler);
    Mesh.subscribe("DHT_CMDC",dhtCmdHandler);
    Mesh.publish("DHT_CMDS","INIT");
    Particle.publish("MESH_UP","FINISHED SETUP");
}

void loop() {
    // Blink a few times to indicate the start of an activity cycle   
    float cur_volts = 0.00;
    float soc = 0.00;
    
    for (int i=0;i < 60;i++) {
        digitalWrite(D7,HIGH);
        delay(500);
        digitalWrite(D7,LOW);
        delay(500);
    }
    
    
     //Read Battery ADC
    cur_volts= analogRead(BATT) * 0.0011224;
    
    if (cur_volts < 3.6) {
        String batt_msg = String("LOW BATTERY:");
        batt_msg.concat(String(cur_volts));
        Particle.publish("BATTERY",batt_msg);
        //We'll eventually beep or something here...
    } 
     
}

int read_sensor() { 
    int val=0;
    return val;
}

void dhtDataHandler(const char *event, const char *data) {
     //Pick out reports from the different sensors and package it up to publish in a summary
     
      
      String dht_report = String(data);
      
      int reportlen=dht_report.length();
      
      float dht_vals[4]={0.0,0.0,0.0,0.0};
      
      char dhtmsg[80]="";
      
      int strpos = 0;
      int valindex = 0;
      int devindex=0;
      
      if (dht_report.indexOf("nan") == -1) { 
          Particle.publish("MESH_DATA",dht_report);
          
          String dev_client = dht_report.substring((dht_report.indexOf(":"))+1,dht_report.indexOf(","));
          strpos = dev_client.length()+10;

          dht_vals[0]=dht_report.substring(strpos+1,strpos+6).toFloat();
          dht_vals[1]=dht_report.substring(strpos+7,strpos+12).toFloat();
          dht_vals[2]=dht_report.substring(strpos+13,strpos+18).toFloat();
          dht_vals[3]=dht_report.substring(strpos+19,strpos+25).toFloat();
          
        //  Particle.publish("DHT_REPORT RESULTS:",dev_client+" "+dht_report.substring(strpos+1,strpos+6)+" "+dht_report.substring(strpos+7,strpos+12)+" "+dht_report.substring(strpos+13,strpos+18)+" "+dht_report.substring(strpos+19,strpos+25));

          if (dht_vals[3] < 3.65) { 
              
              Particle.publish("LOW BATTERY",dev_client);
          }
          
          if (dht_vals[0] < dht_dry) { 
              char devstr[15]="";
              
              dev_client.toCharArray(devstr,(dev_client.length())+2);
              devindex=get_dht_role(devstr);
              
             // snprintf(dhtmsg,32," %s is at %2.2f",devstr,dht_vals[0]);
             snprintf(dhtmsg,32, "%s is at %2.2f",dht_dev[devindex].role,dht_vals[0]);
              Particle.publish("DRYPOINT",dhtmsg);
          }
          
          if (dht_vals[0] > dht_wet) { 
              char devstr[15]="";
              dev_client.toCharArray(devstr,dev_client.length()+2);
              devindex=get_dht_role(devstr);
              //snprintf(dhtmsg,32," %s is at %2.2f",devstr,dht_vals[0]);
             snprintf(dhtmsg,32, "%s is at %2.2f",dht_dev[devindex].role,dht_vals[0]);
              Particle.publish("WETPOINT",dhtmsg);
          }
      }
}

int get_dht_role(char *device) {
    int dev_pos=0;
    boolean found_dev=false;
    for (int i=0; i < sizeof(dht_dev);i++) {
        if strcmp(device,dht_dev[i][0]) {
            dev_pos=i;
            found_dev=true;
            break;
    }
    if (!found_dev) {
        dev_pos=-1;
    }
    return dev_pos;
}

void dhtCmdHandler(const char *event, const char *data) { 
     String cnc_cmd = String(data);   
     Particle.publish("HEARD_FROM",cnc_cmd);
     
}
