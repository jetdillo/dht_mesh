// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_DHT.h>

#define DHTTYPE  DHT22      // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   D5    

// This #include statement was automatically added by the Particle IDE.

String dht_version("1.0");
int blink_count = 0;

float dht_data[3] = {0.0,0.0,0.0};

int watermark=0;
int dryhours=0;

char devname[32];
//String devname= String("devicename");

boolean alert_enabled=true;

boolean drytimer_flag=false;
Timer drytimer(36000,drytimer_handler,false);

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    
    pinMode(D7,OUTPUT); // Running light LED
    pinMode(D6,OUTPUT); // DHT11 sensor power
    
    pinMode(D5,INPUT_PULLUP); // DHT11 sensor
    
    for (int i=0;i< 32;i++) {
           devname[i]=0;
    }
    
//    USBSerial1.begin();
   // USBSerial1.println("BOB Startup");
   
    dht.begin();
    read_dht();
    strcpy(devname,"devicename");
    
    //subscribe us to a bunch of stuff
    
    Particle.subscribe("particle/device/name",devname_handler);
    Particle.publish("particle/device/name");
    Mesh.subscribe("DHT_CMDS",dht_cmdHandler);
    
    selectExternalMeshAntenna();
    
    Mesh.publish("DHT_CMDC",devname);
}

void loop() {
        read_dht();
        delay(5000);
        //jsonify(watermark);
        publish_dht();
        delay(600000);
}

void publish_dht() { 
    
    char dhtstr[96];
      //publish initial battery status
    double voltage = analogRead(BATT) * 0.0011224; 
  
    // Refresh/re-poll for devicename JIC
     Particle.publish("particle/device/name");
    
     sprintf(dhtstr,"{\"device\":%s,%.2f,%.2f,%.2f,%.2f}",devname,dht_data[0],dht_data[1],dht_data[2],voltage);

    Mesh.publish("DHT_DATA",dhtstr);
}

void read_dht() {
    
    dht_data[0]=0.0;
    dht_data[1]=0.0;
    dht_data[2]=0.0;
    
    delay(2000);
    
    dht_data[0] = dht.getHumidity();
	delay(500);
// Read temperature as Celsius
	dht_data[1]= dht.getTempCelcius();
	delay(500);
	dht_data[2] = dht.getDewPoint();
	delay(500);
	
	
    if (isnan(dht_data[0]) || isnan(dht_data[1]) || isnan(dht_data[2])) {
	    Particle.publish("DHT_ERROR",devname);
    }
}

void selectExternalMeshAntenna() {

#if (PLATFORM_ID == PLATFORM_ARGON)
    digitalWrite(ANTSW1, 1);
    digitalWrite(ANTSW2, 0);
#elif (PLATFORM_ID == PLATFORM_BORON)
    digitalWrite(ANTSW1, 0);
#else
    digitalWrite(ANTSW1, 0);
    digitalWrite(ANTSW2, 1);
#endif
}

void drytimer_handler() {
 
    dryhours++;
    char reportstr[32]="";
    
    sprintf(reportstr,"%s:DRYTIME:%.2f:%l",devname,dht_data[0],dryhours);
    
    Mesh.publish("DHT_CMDC",reportstr);
    
}


void devname_handler(const char *topic, const char *data) {
    
    String devstr=String(data);
    devstr.toCharArray(devname,devstr.length()+1);
    Particle.publish("DEVNAME",devname);
    Particle.publish("CALLBACK DATA",data);
    
    //strncpy(devname, data, sizeof(data)-1);
}


void dht_cmdHandler(const char *topic, const char *data) {
 
 //Apparently ParticleOS 12.x doesn't like C-str functions and we have to use Strings inside handlers, or at least not C-strings. 
 //And this is another reason we cannot have nice things. 
    String cmdstr = String(data);
    if (cmdstr.compareTo("DRYSTART")==0) {
        drytimer_flag=true;
        drytimer.start();
     
    }
    if (cmdstr.compareTo("DRYSTOP")==0) {
        drytimer_flag=false;
        drytimer.stop();
     //stop timer
   }
}
