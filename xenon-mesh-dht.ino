// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_DHT.h>

// This #include statement was automatically added by the Particle IDE.
#include <neopixel.h>

// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_SSD1306.h>

#define HAS_OLED 0 
#define HAS_LIGHTRING 0

#define DHTTYPE  DHT22      // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   D5    
#define PIXEL_PIN D2
#define PIXEL_COUNT 16
#define PIXEL_TYPE WS2812B
#define OLED_RESET D4

// This #include statement was automatically added by the Particle IDE.

String dht_version("1.0");
int blink_count = 0;

float dht_data[3] = {0.0,0.0,0.0};

int watermark=0;

//char devname[15];
String devname= String("devicename");

boolean alert_enabled=true;

//PietteTech_DHT DHT(DHTPIN, DHTTYPE);

#ifdef HAS_OLED == 1
Adafruit_SSD1306 oled(OLED_RESET);
#endif

#ifdef HAS_LIGHTRING == 1

int color_ring[16][3] = {{255,0,255},{128,0,255},{64,0,255},{0,0,255},{0,64,255},{0,128,255},{0,255,255},{64,255,255},{128,255,255},{255,255,255},{255,255,128},{255,255,64},{255,128,0},{255,64,0},{255,0,0},{255,0,0}};
Adafruit_NeoPixel ring(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
#endif

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
//STARTUP(selectExternalMeshAntenna());

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    
    pinMode(D7,OUTPUT); // Running light LED
    pinMode(D6,OUTPUT); // DHT11 sensor power
    
    pinMode(D5,INPUT_PULLUP); // DHT11 sensor
    
//    USBSerial1.begin();
   // USBSerial1.println("BOB Startup");
   
    dht.begin();
    read_dht();
    //strcpy(devname,"devicename");
    
    //subscribe us to a bunch of stuff
    
    Particle.subscribe("particle/device/name",devname_handler);
    Particle.publish("particle/device/name");
    Mesh.subscribe("DHT_CMDS",dht_cmdHandler);
    
    selectExternalMeshAntenna();
    
   #ifdef HAS_OLED == 1
       oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // init done
  
       oled.display(); // show splashscreen
       delay(2000);
       oled.clearDisplay();   // clears the screen and buffer
       oled.setTextSize(1);
       oled.setTextColor(WHITE);
       oled.println("Filament Monitor Starting...");
       oled.display();
       delay(2000);
       
       oled.clearDisplay();
       display_dht();
       delay(2000);
   #endif
   
   #ifdef HAS_LIGHTRING == 1
    ring.begin();
    ring.show();
    for (int i=0; i < PIXEL_COUNT;i++) {
         ring.setPixelColor(i,color_ring[i][0],color_ring[i][1],color_ring[i][2]);
         ring.show();
        }
        delay(500);
        for (int i=0; i < PIXEL_COUNT;i++) {
         ring.setPixelColor(i,0,0,0);
         ring.show();
        }
        delay(500);
    #endif
    
    Mesh.publish("DHT_CMDC",devname);
}

void loop() {
        read_dht();
        delay(5000);
        //jsonify(watermark);
}

void update_dht() {
    #ifdef HAS_OLED == 1
        oled.clearDisplay();
        display_dht();
    #endif
 
    #ifdef HAS_LIGHTRING == 1
        lightring_display_dht();
    #endif
    publish_dht();
}

void display_dht() { 
    
    double voltage = analogRead(BATT) * 0.0011224; 
    unsigned long ts = 0;
    char h_str[17]="";
    char t_str[20]="";
    char d_str[20]="";
    char b_str[20]="";
    
    read_dht();
    
    snprintf(h_str,sizeof(h_str),"HUMIDITY: %2.2f ",dht_data[0]);
    snprintf(t_str,sizeof(t_str),"TEMP:%3.2f ",dht_data[1]);
    snprintf(d_str,sizeof(d_str),"DEW_PT: %3.2f ",dht_data[2]);
    snprintf(b_str,sizeof(b_str),"BATT: %2.2f ",voltage);
     
    ts = System.millis(); 
    String tsstr = String(ts);
    
     oled.clearDisplay();
     
     display_msg(h_str,0);
     display_msg(t_str,8);
     display_msg(d_str,16);
     display_msg(b_str,24);
     display_msg(tsstr,30);
}

/*
void publish_dht() { 
    double voltage = analogRead(BATT) * 0.0011224; 
    char h_str[17]="";
    char t_str[20]="";
    char d_str[20]="";
    char b_str[20]="";
   
     snprintf(h_str,sizeof(h_str),"H: %2.2f ",dht_data[0]);
     snprintf(t_str,sizeof(t_str),"T: %3.2f ",dht_data[1]);
     snprintf(d_str,sizeof(d_str),"D: %3.2f ",dht_data[2]);
     snprintf(b_str,sizeof(b_str),"B: %2.2f ",voltage);
        
        String dht_msg = String(devname);
        dht_msg.concat(h_str);
        dht_msg.concat(t_str);
        dht_msg.concat(d_str);
        dht_msg.concat(b_str);
        Particle.publish("DHT_DATA",dht_msg);
        Mesh.publish("DHT_DATA",dht_msg);
}
*/
    void publish_dht() { 
    
    char dhtstr[96];
      //publish initial battery status
    double voltage = analogRead(BATT) * 0.0011224; 
  
   
    //snprintf(dhtstr, "{\"device\":%s,\"humidity\":%.2f,\"airtemp\":%.2f,\"dewpoint\":%.2f,\"batt\":%.2f}",devname,dht_data[0],dht_data[1],dht_data[2],voltage);
     sprintf(dhtstr, "{\"device\":%s,\"H\":%.2f,\"T\":%.2f,\"D\":%.2f,\"B\":%.2f}",devname,dht_data[0],dht_data[1],dht_data[2],voltage);

    Mesh.publish("DHT_DATA",dhtstr);
}

void display_msg(String txt,int rowpos) { 
    oled.setCursor(0,rowpos);
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.println(txt);
    oled.display();
}

void lightring_display_dht() {
 // There's supposed to be a handler here. 

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

void devname_handler(const char *topic, const char *data) {
    
   // strcpy(devname,data);
    
    devname=String(data);
    devname.concat(": ");
}

void dht_cmdHandler(const char *topic, const char *data) {
 
 //Apparently ParticleOS 12.x doesn't like C-str functions and we have to use Strings inside handlers, or at least not C-strings. 
 //And this is another reason we cannot have nice things. 
 String cmdstr = String(data);
 
}
