// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_DHT_Particle.h>

// Example testing sketch for various DHT humidity/temperature sensors, based on the original Adafruit DHT examples

#define DHTPIN D2     // what pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11		// DHT 11 
#define DHTTYPE DHT22		// DHT 22 (AM2302)
//#define DHTTYPE DHT21		// DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

char devname[13]="storage_dht";
char dht_msg[96];

FuelGauge fuel;

int loopCount;

void setup() {
	Serial.begin(9600); 
	Serial.println("DHTxx test!");
	Particle.publish("state", "DHTxx test start");
	Particle.variable("dht_data",dht_msg);

	dht.begin();
	fuel.quickStart();
	loopCount = 0;
	delay(2000);
}

void loop() {
// Wait a few seconds between measurements.
//	delay(2000);

// Reading temperature or humidity takes about 250 milliseconds!
// Sensor readings may also be up to 2 seconds 'old' (its a 
// very slow sensor)
	float h = dht.getHumidity();
	delay(500);
// Read temperature as Celsius
	float t = dht.getTempCelcius();
	delay(500);
	 float dp = dht.getDewPoint();
// Read temperature as Farenheit
	//float f = dht.getTempFarenheit();
	
	float v = fuel.getVCell();
  
// Check if any reads failed and exit early (to try again).
	if (isnan(h) || isnan(t) || isnan(dp)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}

    sprintf(dht_msg, "{\"device\":%s,\"humidity\":%.2f,\"airtemp\":%.2f,\"dewpoint\":%.2f,\"batt\":%.2f}",devname,h,t,dp,v);
    Particle.publish("DHT_DATA",String(dht_msg));
    delay(10000);
    System.sleep(SLEEP_MODE_DEEP,21600);
}
