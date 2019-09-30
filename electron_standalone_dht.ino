// This #include statement was automatically added by the Particle IDE.
#include <PietteTech_DHT.h>

#define DHTTYPE  DHT22       // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   D5    

// Electron-specific version of code to read a DHT sensor and report back. 

int blink_count = 0;

double dht_data[3] = {0.0,0.0,0.0};

int watermark=0;

int battAlertLvl=20;

int sleepInterval=60;

char devname[12]="storage_dht";
char dhtstr[96];


FuelGauge fuel;

PietteTech_DHT DHT(DHTPIN,DHTTYPE);

void setup() {
    
    pinMode(D7,OUTPUT); // Running light LED
    pinMode(D6,OUTPUT); // DHT11 sensor power
    
    pinMode(D5,INPUT_PULLUP); // DHT11 sensor

    fuel.quickStart();
    
    DHT.begin();
    Serial.begin(115200);
    
    //Check power via PowerShield, ignore for Electron
    //int batt_flag = digitalRead(D2);
    
    Particle.variable("dht_data",dhtstr);
    fuel.setAlertThreshold(battAlertLvl);
    
    read_dht();
    publish_dht();
    delay(10000);
    System.sleep(SLEEP_MODE_DEEP,360);
}

void loop() {
         //No reason to be more complex than this since this particular device spends most of its time asleep
          Serial.println("TOP OF LOOP");
}

void publish_dht() { 
    
      //publish initial battery status
    float voltage = fuel.getVCell();
    // Read the State of Charge of the LiPo
    float charge = fuel.getSoC();
    
    sprintf(dhtstr, "{\"device\":%s,\"humidity\":%.2f,\"airtemp\":%.2f,\"dewpoint\":%.2f,\"batt\":%.2f}",devname,dht_data[0],dht_data[1],dht_data[2],voltage);
    Particle.publish("DHT_DATA",dhtstr);
}


float * batt_stat() { 
    
    float batt_levels[2]={0.0,0.0};
    
        //publish initial battery status
    batt_levels[0] =fuel.getVCell();
    // Read the State of Charge of the LiPo
    batt_levels[1]=fuel.getSoC();
    
    return batt_levels;
}

int read_analog_sensor() { 
    int val=0;
    digitalWrite(D6,HIGH);
      delay(1000);
      val = analogRead(A0);
      delay(100);
      digitalWrite(D6,LOW);
    return val;
}

int read_digital_sensor(int sensorpin, int sensor_power_pin) { 
    int val=0;// Yellow Status LED - Battery Power
    digitalWrite(sensor_power_pin,HIGH);
    delay(1000);
    val = digitalRead(sensorpin);
    delay(100);
    digitalWrite(sensor_power_pin,LOW);
    return val;
}

void read_dht() {
    int dht_result = DHT.acquireAndWait(1000); // wait up to 1 sec (default indefinitely)
    
    if ( dht_result != DHTLIB_OK) {
    
    /*
    String dht_str =String("DHLIB_ERROR:");
    switch (dht_result) {
  
  case DHTLIB_ERROR_CHECKSUM:
    dht_str.concat("Error\n\r\tChecksum error");
    break;
  case DHTLIB_ERROR_ISR_TIMEOUT:
    dht_str.concat("Error\n\r\tISR time out error");
    break;
  case DHTLIB_ERROR_RESPONSE_TIMEOUT:
    dht_str.concat("Error\n\r\tResponse time out error");
    break;
  case DHTLIB_ERROR_DATA_TIMEOUT:
    dht_str.concat("Error\n\r\tData time out error");
    break;
  case DHTLIB_ERROR_ACQUIRING:
    dht_str.concat("Error\n\r\tAcquiring");
    break;
  case DHTLIB_ERROR_DELTA:
    dht_str.concat("Error\n\r\tDelta time to small");
    break;
  case DHTLIB_ERROR_NOTSTARTED:
    dht_str.concat("Error\n\r\tNot started");
    break;
  default:
    dht_str.concat("Unknown error");
    break;
   }
   */
   Particle.publish("DHT_ERROR","Error reading Sensor");
 } else {
        dht_data[0] = DHT.getHumidity();
        dht_data[1] = DHT.getCelsius();
        dht_data[2] = DHT.getDewPoint();
    }
}
