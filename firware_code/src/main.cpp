#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>

struct Pot_data{
  uint8_t temp;
  uint8_t humidity;
  uint8_t door_lvl;
  uint8_t water_lvl;
  uint8_t soil_moi;
  bool    led_state;
  bool    sun_exist;
} pot_data;

struct Pot_control{
  uint8_t auto_en;
  uint8_t sprinkle_en;
  uint8_t req_soil_moi;
  uint8_t sun_led_en;
}pot_control;
 struct Crop_db{
  uint8_t temp;
  uint8_t humidity;
  uint8_t soil_mosi;
  bool    sun_required;
 } strawberry;
 
 enum {
  DOOR_INVALID, //no notificaion is required
  DOOR_OPEN,   //door needs to be opened
  DOOR_CLOSE,  //door needs to be closed
 } door_state;

  enum {
  SYS_AUTO = 0,  //closed
  SYS_MANUAL,
  SYS_INVALID,      //fully open
 }system_mode;

#define blink_led()                do{digitalWrite(BUILTIN_LED,!digitalRead(BUILTIN_LED)); delay(300);}while(0)

#define WIFI_SSID                 "VF_DEVICE"
#define WIFI_PASS                 "123456789"
#define API_KEY                   "AIzaSyDdQZ7k-ZGqIBxL4jkmWG-_44tCmrh3sHU"
#define RTDB_URL                  "https://greenhouseproj-da15c-default-rtdb.europe-west1.firebasedatabase.app/"
#define DB_SAVE_REFRESH_RATE      (5000)
#define DB_CROP_R_REFRESH_RATE    (10000)
#define DB_DEFAULT_R_REFRESH_RATE (5000)

#define BREAK_ERR(x)               if(!x){break;}
#define BREAK_TYPE(fb_data,type)   if(fb_data != type){break;}
#define PERCENT_CONTRAIN(value)    do{value = constrain(value,0,100);}while (0)

#define APP_SUCCESS             (0)
#define ERR_NULL                (1)
#define ERR_DP_ERR              (2)
#define ERR_INVALID_ARGU        (255)

#define DHT_PIN                 (15)
#define TEMP_HUMI_SCAN_INTERVAL (2500) //Scan every 2.5 seconds 

#define SOIL_ADC_PIN            (36)  //VP pin
#define SOIL_SAMPLES_TO_AVG     (32)   //number of soil samples to take their average
#define SENSORS_SCAN_INTERVAL   (500) //Scan every 2.5 seconds

#define SPRINKLE_PIN            (17)
#define MINI_WATERING_INTRVAL   (10000)      //6*3600*1000
#define ULTRA_SONIC_ECHO_PIN    (4)
#define ULTRA_SONIC_TRIG_PIN    (16)
#define ULTRA_SONIC_MAX_DIS     (28)
#define FULL_TANK_DISTANCE      (18)
#define TANK_SENSOR_OFFSET      (ULTRA_SONIC_MAX_DIS-FULL_TANK_DISTANCE)

#define TEMP_HUM_OFFSET         (5)
#define DOOR_MOTOR_PIN          (23)
#define DOOR_REQ_INTERVAL       (10000)        //1*3600*1000

#define SUN_LED_PIN             (18)
#define SUN_LIGHT_LDR_THRESHOLD (900)
#define LDR_PIN                 (39)

#define POT_COVER_LIMIT_PIN     (5)
/**********************************************/
/*****************GOLBALS*********************/
/********************************************/
FirebaseData   fbd;
FirebaseAuth   fb_auth;
FirebaseConfig fb_config;

DHT dht(DHT_PIN,DHT11);

//NewPing tank_sensor = NewPing(ULTRA_SONIC_TRIG_PIN,ULTRA_SONIC_ECHO_PIN,50);

bool is_fb_signed_up         = false;
bool door_change_requested   = false;
bool watering_started        = false;
bool data_updated_one_time   = false;
bool read_crop_first_time    = true;
bool is_pot_cover_closed     = false;
 

long watering_end_time      = 0;
long chng_door_end_time     = 0;
uint32_t last_send_time     = 0;
uint32_t last_read_time     = 0;
uint32_t last_crop_read_time= 0;
uint8_t  g_door_level;
uint8_t  required_soil_mosi =0;
void idicate_wifi_status(void)
{
  static bool b_is_connected = false;
  if(WiFi.status() == WL_CONNECTED && b_is_connected == false)
  {
    digitalWrite(BUILTIN_LED,HIGH);
    b_is_connected = true;
    Serial.println("[WIFI]: -- Connected -- ");
    Serial.print("[WIFI]: SSID = ");
    Serial.print(WiFi.SSID());
    Serial.print(" ,strength = ");
    Serial.print(WiFi.RSSI());
    Serial.print(" ,Local IP = ");
    Serial.println(WiFi.localIP());
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    blink_led();
    if(b_is_connected == true)
    {
      b_is_connected = false;
      Serial.println("[WIFI]: -- Disconnected -- ");
    }
  }
}

char upload_data_to_fb(Pot_data* ptr_pot_data)
{
  char ret_val = APP_SUCCESS;

  if(NULL != ptr_pot_data)
  {
    do
    {
      BREAK_ERR(Firebase.RTDB.setIntAsync(&fbd,"device_aa/pot_data/temp",(int)ptr_pot_data->temp));
      BREAK_ERR(Firebase.RTDB.setIntAsync(&fbd,"device_aa/pot_data/humidity",(int)ptr_pot_data->humidity));
      BREAK_ERR(Firebase.RTDB.setIntAsync(&fbd,"device_aa/pot_data/water_lvl",(int)ptr_pot_data->water_lvl));
      BREAK_ERR(Firebase.RTDB.setIntAsync(&fbd,"device_aa/pot_data/door_lvl",(int)ptr_pot_data->door_lvl));
      if(ptr_pot_data->door_lvl != DOOR_INVALID)  ptr_pot_data->door_lvl = DOOR_INVALID;
      BREAK_ERR(Firebase.RTDB.setIntAsync(&fbd,"device_aa/pot_data/soil_moi",(int)ptr_pot_data->soil_moi));
      Serial.printf("!!! Attemping to write sun exist = %d\r\n",ptr_pot_data->sun_exist);
      BREAK_ERR(Firebase.RTDB.setBoolAsync(&fbd,"device_aa/pot_data/sun_exist",ptr_pot_data->sun_exist));
      BREAK_ERR(Firebase.RTDB.setBoolAsync(&fbd,"device_aa/pot_data/led_state",ptr_pot_data->led_state));

      Serial.printf("[FireBase]: Data saved successfully\r\n");
      Serial.printf("[FireBase]: Temp = %d C\tHumidity = %d %\tWater Level = %d %\tDoor Level = %d %\t \
      Soil Moisture = %d %\tLed State  = %d\r\n",ptr_pot_data->temp,ptr_pot_data->humidity, ptr_pot_data->water_lvl, \
                    ptr_pot_data->door_lvl, ptr_pot_data->soil_moi, ptr_pot_data->led_state);
      return APP_SUCCESS;

    }while(0);
    ret_val = ERR_DP_ERR;
    Serial.printf("[FireBase]: Failed saving, error = %s",fbd.errorReason().c_str());
    Serial.printf("Data path on failure = %s\r\n",fbd.dataPath().c_str());
  }
  else
  {
    ret_val = ERR_NULL;
  }
  return ret_val;
}

char read_cntrl_data_from_fb(Pot_control* ptr_pot_control)
{
  char ret_val = APP_SUCCESS;
  if(NULL != ptr_pot_control)
  {
    do
    {
      BREAK_ERR(Firebase.RTDB.getBool(&fbd,"device_aa/pot_control/auto"));
      BREAK_TYPE(fbd.dataType(),"boolean");
      ptr_pot_control->auto_en = fbd.boolData();

      BREAK_ERR(Firebase.RTDB.getBool(&fbd,"device_aa/pot_control/sprinkle"));
      BREAK_TYPE(fbd.dataType(),"boolean");
      ptr_pot_control->sprinkle_en = fbd.boolData();

      BREAK_ERR(Firebase.RTDB.getBool(&fbd,"device_aa/pot_control/sun_led"));
      BREAK_TYPE(fbd.dataType(),"boolean");
      ptr_pot_control->sun_led_en = fbd.boolData();

      BREAK_ERR(Firebase.RTDB.getInt(&fbd,"device_aa/pot_control/req_soil_moi"));
      BREAK_TYPE(fbd.dataType(),"int");
      ptr_pot_control->req_soil_moi = fbd.intData();
      PERCENT_CONTRAIN(ptr_pot_control->req_soil_moi);

      Serial.printf("[FireBase]: --Control-- Read successfully\r\n");
      Serial.printf("LED enable = %d\t Auto = %d\t sprinkle_enable = %d\t required soil moisure = %d%\r\n", \
                    ptr_pot_control->sun_led_en,ptr_pot_control-> auto_en , ptr_pot_control->sprinkle_en, ptr_pot_control->req_soil_moi);
      data_updated_one_time = true;
      return APP_SUCCESS;

    }while(0);
    ret_val = ERR_DP_ERR;
    Serial.printf("[FireBase]: Failed Reading control data, error = %s\r\n",fbd.errorReason().c_str());
    Serial.printf("Data path on failure = %s\r\n",fbd.dataPath().c_str());
  }
  else
  {
    ret_val = ERR_NULL;
  }
  return ret_val;
}

char read_crop_data_from_fb(Crop_db* ptr_crop_param)
{
  char ret_val = APP_SUCCESS;
  if(NULL != ptr_crop_param)
  {
    do
    {
      BREAK_ERR(Firebase.RTDB.getBool(&fbd,"crops_db/strawberry/sun_required"));
      BREAK_TYPE(fbd.dataType(),"boolean");
      ptr_crop_param->sun_required = fbd.boolData();

      BREAK_ERR(Firebase.RTDB.getInt(&fbd,"crops_db/strawberry/soil_moi"));
      BREAK_TYPE(fbd.dataType(),"int");
      ptr_crop_param->soil_mosi = fbd.intData();
      PERCENT_CONTRAIN(ptr_crop_param->soil_mosi);

      BREAK_ERR(Firebase.RTDB.getInt(&fbd,"crops_db/strawberry/temp"));
      BREAK_TYPE(fbd.dataType(),"int");
      ptr_crop_param->temp = fbd.intData();
      PERCENT_CONTRAIN(ptr_crop_param->temp);

      BREAK_ERR(Firebase.RTDB.getInt(&fbd,"crops_db/strawberry/humidity"));
      BREAK_TYPE(fbd.dataType(),"int");
      ptr_crop_param->humidity = fbd.intData();
      PERCENT_CONTRAIN(ptr_crop_param->humidity);

      Serial.printf("[FireBase]: Read Crop paramters From Database\r\n");
      Serial.printf("Sun required = %d\t Temp = %d\t Humidity = %d%\t Soil moisture = %d%\r\n", \
                    ptr_crop_param->sun_required , ptr_crop_param->temp, ptr_crop_param->humidity, ptr_crop_param->soil_mosi);

      return APP_SUCCESS;

    }while(0);
    ret_val = ERR_DP_ERR;
    Serial.printf("[FireBase]: Failed Reading crop data, error = %s",fbd.errorReason().c_str());
    Serial.printf("Data path on failure = %s\r\n",fbd.dataPath().c_str());
  }
  else
  {
    ret_val = ERR_NULL;
  }
  return ret_val;
}
//------------------------------------------------
//----------------- Soil Moisture ----------------
//------------------------------------------------
uint8_t get_soil_moisture(uint16_t pot_num, uint16_t samples_num)
{
  uint8_t  adc_channel      = 0;
  uint16_t adc_resolution   = 0;
  uint8_t  pot_soil_moisure = 0;
  switch (pot_num)
  {
  case 1:
    adc_channel    = SOIL_ADC_PIN; //measure from VP GPIO 36
    adc_resolution = 4095; //12-bit resolution
    break;
  
  default:
    Serial.printf("[Soil]: unsupported Pot\r\n");
    return ERR_INVALID_ARGU;
  }
  uint32_t adc_sample = 0;
  for(uint16_t i; i< samples_num; i++)
  {
    adc_sample += analogRead(adc_channel);
  }
  //calculate the average
  adc_sample /= samples_num;
  pot_soil_moisure = (uint8_t) (((adc_resolution - adc_sample) * 100) / adc_resolution);
  //make sure it is contrained 
  PERCENT_CONTRAIN(pot_soil_moisure);
  Serial.printf("[Soil]: Measure Soil Moisture = %d%\r\n",pot_soil_moisure);

  return pot_soil_moisure;
}

//------------------------------------------------
//--------------- Water Tank Data ----------------
//------------------------------------------------
uint16_t ultrasonic_read_cm()
{
  digitalWrite(ULTRA_SONIC_TRIG_PIN, LOW);  
	delayMicroseconds(2);  
	digitalWrite(ULTRA_SONIC_TRIG_PIN, HIGH);  
	delayMicroseconds(10);  
	digitalWrite(ULTRA_SONIC_TRIG_PIN, LOW); 
  uint32_t duration = pulseIn(ULTRA_SONIC_ECHO_PIN, HIGH);
  uint16_t  distance = (duration*.0343)/2;
  
  return distance;   

}
uint8_t get_water_tank_lvl (void)
{
  uint8_t tank_percentage   = 0;
  uint8_t tank_len_reading  = 0;

  tank_len_reading = ultrasonic_read_cm();
  Serial.printf("Ultrasonic reading = %d\r\n",tank_len_reading);
  //limit the readings
  tank_len_reading = constrain(tank_len_reading,0,FULL_TANK_DISTANCE);
  if(tank_len_reading<= TANK_SENSOR_OFFSET) tank_len_reading = 0;
  tank_percentage  = ((FULL_TANK_DISTANCE - tank_len_reading)*100)/FULL_TANK_DISTANCE;
  PERCENT_CONTRAIN(tank_percentage);
  
  Serial.printf("Tank reading after limits= %d\r\n",tank_len_reading);
  Serial.printf("Tank percentage = %d\r\n",tank_percentage);

  return tank_percentage;
}

bool did_sun_raise(void)
{
  uint32_t ldr_value = 0;

  for(uint8_t i; i< 32; i++)
  {
    ldr_value += analogRead(LDR_PIN);
  }
  //calculate the average
  ldr_value /= 32;
  Serial.printf("[LDR]: light value = %d\r\n",ldr_value);
  return (ldr_value > SUN_LIGHT_LDR_THRESHOLD);
}
void global_var_init()
{
  memset(&strawberry,0,sizeof(strawberry));
  memset(&pot_control,0,sizeof(pot_control));
  memset(&pot_data,0,sizeof(pot_data));

  data_updated_one_time = false;

  digitalWrite(SPRINKLE_PIN,HIGH);
  digitalWrite(SUN_LED_PIN,HIGH);
  digitalWrite(POT_COVER_LIMIT_PIN,HIGH);
}
void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(SPRINKLE_PIN,OUTPUT);
  pinMode(SUN_LED_PIN,OUTPUT);
  pinMode(POT_COVER_LIMIT_PIN,INPUT);

  pinMode(ULTRA_SONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRA_SONIC_ECHO_PIN, INPUT);

  Serial.begin(921600);
  global_var_init();
  //----------DHT initializing 
  Serial.printf("[DHT]: init start!");
  dht.begin();

  //----------WiFi Module--------
  WiFi.begin(WIFI_SSID,WIFI_PASS);
  //wait until the wifi is connected 
  Serial.println("ESP Started !!");
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.printf("[WIFI]: Scanning ...\n");
    idicate_wifi_status();
  }

  fb_config.api_key = API_KEY;
  fb_config.database_url = RTDB_URL;
  if(Firebase.signUp(&fb_config,&fb_auth,"",""))
  {
    Serial.printf("[FireBase] Signed in\r\n");
    is_fb_signed_up = true;
  }
  else
  {
    Serial.printf("error signing up code = %s\r\n",fb_config.signer.signupError.message.c_str());
    is_fb_signed_up = false;
  }
  fb_config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&fb_config,&fb_auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  //refresh rates 
  static uint32_t data_reading_rfrsh = 0; //refresh rate for data sampling except temperature and humidity duo sensor limit 
  static uint32_t temp_humi_refrsh   = 0; 
  // put your main code here, to run repeatedly:
  idicate_wifi_status();
  if(Firebase.ready() && is_fb_signed_up && (millis() - last_send_time >= DB_SAVE_REFRESH_RATE))
  {
      uint8_t api_return_val = 0;
      api_return_val = read_crop_data_from_fb(&strawberry) ;
      if(api_return_val == APP_SUCCESS)
      {
        api_return_val = upload_data_to_fb(&pot_data);
        if(api_return_val == APP_SUCCESS)
        {
          last_send_time = millis();
        }
        else
        {
          /* Don't update the send timer as we need to try to save data again*/
        }
      }  
  }
  if(Firebase.ready() && is_fb_signed_up && ((millis() - last_crop_read_time >= DB_CROP_R_REFRESH_RATE) || read_crop_first_time == true))
  {
    if(APP_SUCCESS ==  read_crop_data_from_fb(&strawberry))
    {
      last_crop_read_time = millis();
      read_crop_first_time = false;
    }
    else
    {
      /* Don't update the send timer as we need to retry to read data */
    }
    
  }

  if(Firebase.ready() && is_fb_signed_up && (millis() - last_read_time >= DB_DEFAULT_R_REFRESH_RATE))
  { 
    if(APP_SUCCESS ==  read_cntrl_data_from_fb(&pot_control))
    {
      last_read_time = millis();
    }
    else
    {
      /* Don't update the send timer as we need to retry to read data */
    }
  }

#if 1
  //--------------Temperature + Humidity -----------
  if((millis() - temp_humi_refrsh) >= TEMP_HUMI_SCAN_INTERVAL)
  {
    pot_data.temp     = (char)((int)dht.readTemperature());
    pot_data.humidity = (char)((int)dht.readHumidity());
    Serial.printf("Temperature = %d C ,, Humidity = %d %\r\n",pot_data.temp,pot_data.humidity);
    temp_humi_refrsh = millis();
  }
  
//-------------- Soil Reading ------------------------
  if((millis() - data_reading_rfrsh) >= SENSORS_SCAN_INTERVAL)
  {
    pot_data.soil_moi = get_soil_moisture(1,SOIL_SAMPLES_TO_AVG);
    pot_data.water_lvl= get_water_tank_lvl();
    pot_data.sun_exist= did_sun_raise();
    data_reading_rfrsh = millis();
    is_pot_cover_closed = digitalRead(POT_COVER_LIMIT_PIN) == LOW?true:false;
  }
//------------- Executing auto tasks -----------------

if(pot_control.auto_en == true && data_updated_one_time == true)
{
  system_mode = SYS_AUTO;
  pot_control.sun_led_en  = false;
  pot_control.sprinkle_en = false;
  required_soil_mosi = strawberry.soil_mosi;
  //------------Automatic System Mode ---------
#if 1
  //Temperature + humidity 7
  if((pot_data.humidity < (strawberry.humidity - TEMP_HUM_OFFSET) || pot_data.temp < (strawberry.temp - TEMP_HUM_OFFSET)))
  {
    if(!is_pot_cover_closed)
    {
      pot_data.door_lvl = DOOR_CLOSE;
    }
  }
  else if((pot_data.humidity > (strawberry.humidity + TEMP_HUM_OFFSET) || pot_data.temp > (strawberry.temp + TEMP_HUM_OFFSET)))
  {
    if(is_pot_cover_closed)
    {
      pot_data.door_lvl = DOOR_OPEN;
    }
  }
  else
  {
    //Temp and Humidty is good no need to close the cover
    if(is_pot_cover_closed)
    {
      pot_data.door_lvl = DOOR_OPEN;
    }
  }

  if(pot_data.sun_exist == true && strawberry.sun_required == true)
  {
    if(is_pot_cover_closed)
    {
      pot_data.door_lvl = DOOR_OPEN;
    }
    digitalWrite(SUN_LED_PIN,HIGH);
  }
  else if(pot_data.sun_exist == false && strawberry.sun_required == true)
  {
    digitalWrite(SUN_LED_PIN,LOW);
  }
  else 
  {
    digitalWrite(SUN_LED_PIN,HIGH);
  }
#endif
}
else if(pot_control.auto_en == false && data_updated_one_time == true)
{
  data_updated_one_time = false;
}
else if(pot_control.auto_en == false && data_updated_one_time == false)
{
  //Manual Mode
  system_mode = SYS_MANUAL;
  pot_data.door_lvl = DOOR_INVALID;

  digitalWrite(SUN_LED_PIN,!pot_control.sun_led_en);
  digitalWrite(SPRINKLE_PIN,!pot_control.sprinkle_en);
  
  if(pot_control.sprinkle_en == false)
  {
    required_soil_mosi = pot_control.req_soil_moi;
    watering_started = false;
  }
  else
  {
    //return to ower value from data base.
    required_soil_mosi = strawberry.soil_mosi;
  }
}
//------------ Sprinkle Control ---------
if((system_mode == SYS_AUTO || (system_mode == SYS_MANUAL && pot_control.sprinkle_en == false)))
{
  if(pot_data.soil_moi < required_soil_mosi-5 && ((millis() - watering_end_time > MINI_WATERING_INTRVAL) || watering_started == false))
  {
    watering_started = true;
    digitalWrite(SPRINKLE_PIN,LOW);
    watering_end_time = millis();   
  }
  if(pot_data.soil_moi > required_soil_mosi+5 && watering_started == true)
  {
    digitalWrite(SPRINKLE_PIN,HIGH);
  }
  if((millis() - watering_end_time > MINI_WATERING_INTRVAL) && watering_started == true)
  {
    watering_end_time = millis(); 
    watering_started = false;
  }
}
else if ((system_mode == SYS_MANUAL && pot_control.sprinkle_en == true))
{
  watering_end_time = millis();
  watering_started = true;
}
else
{
  watering_end_time = millis();
  watering_started = false;
}
#endif
}
