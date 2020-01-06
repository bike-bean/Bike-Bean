/*
  Version 1.2
  Autor: Mirko Buhrandt, Uwe Gerhard
  Date: 06.01.2020
  www.bike-bean.de
*/

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include <avr/power.h>
#include <string.h>

SoftwareSerial gsmSerial(PD6, PD5); //RX ,TX
SoftwareSerial wifiSerial(4, 17); //RX, TX

int Batterylowsent = 0;
char mytelephonenumberarray[20] = ""; //+49XXXXXXXXXXXX
char *mytelephonenumber = mytelephonenumberarray; 
char sendsmstextarray[161] = ""; //used for gsm towers, wifi towers and getsmstext
int interval = 1;
boolean iswifion = false;
int arraysize = 300;
char dataarray[300];

////////////////300 bytes RAM required//////////////////

void setup() {
  wdt_disable();  //recommended to do this in case of low probability event
  Serial.begin(9600);
  wifiOff();
}

void loop(){ 

  gsmOn();
  Config();

  flushgsm(2000); 
  byte battpercent = getBattPercent();
  byte battpercent2 = 120;
  int batcounter = 0;

  // checks battery level after 2 secondes until it`s equal. Sometimes there was an issue before
  do{ 
    battpercent2 = battpercent;
    flushgsm(2000);
    battpercent = getBattPercent();
    batcounter++;
  }while((battpercent != battpercent2) && batcounter < 5);  
  
  if (battpercent > 0 && battpercent != 111) { 

  waitFor("+CMTI", 15000);      

    int i = 1;
    char *unread = GetUnread(i); 
    while (strcasestr(unread, "REC") != NULL) {

      if (battpercent < 20 && mytelephonenumber != 0) { 
        if(Batterylowsent == 0 && battpercent > 10){
          char *sendsmstext = sendsmstextarray; 
          memset(sendsmstextarray, NULL, 161);   
          strcat(sendsmstext, "BATTERY LOW!\nBATTERY STATUS: "); 
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%"); 
          SendSMS(mytelephonenumber, sendsmstext); 
          Batterylowsent = 1;
        }else 
          if(battpercent <= 10 && battpercent > 0 && Batterylowsent == 1){
            char *sendsmstext = sendsmstextarray; 
            memset(sendsmstextarray, NULL, 161);   
            strcat(sendsmstext, "BATTERY LOW!\nBATTERY STATUS: "); 
            char battpercentage[3] = ""; 
            itoa(battpercent,battpercentage,10); 
            strcat(sendsmstext, battpercentage);
            strcat(sendsmstext, "%"); 
            strcat(sendsmstext, "\nInterval set to 24h");
            SendSMS(mytelephonenumber, sendsmstext); 
            interval = 24;
            Batterylowsent = 2;
          }
      }//Battery Warning SMS

       if (strcasestr(unread, "UNREAD") != NULL) {
       char *checksmstext = GetSMSText(i);
       /* doesn`t work anymore...
        
        if(strcasestr(checksmstext, "pos") != NULL && strlen(checksmstext) == 3){
          char *sendernumber = SenderNumber(i);   
          char *sendsmstext = sendsmstextarray; 
          getLocation();
          strcat(sendsmstext, "\nBattery Status: ");
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);          
          free(sendernumber);
          DeleteSMS(i);
        }
        */        
        if(strcasestr(checksmstext, "pos") != NULL && strlen(checksmstext) == 3){
          char *sendernumber = SenderNumber(i);          
          char *sendsmstext = sendsmstextarray; 
          char battpercentage[3] = "";     
          delay(20000);
          getLocationApp();
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, "Battery Status: ");
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);          
          free(sendernumber);
          DeleteSMS(i);
        }else
        if(strcasestr(checksmstext, "status") != NULL && strlen(checksmstext) == 6){ 
          char* sendsmstext = sendsmstextarray; 
          char intervallarray[3]="";
          itoa(interval,intervallarray,10); 
          memset(sendsmstextarray, NULL, 161);
          char *sendernumber = SenderNumber(i); 
          strcat(sendsmstext, "Warningnumber: ");
          if(mytelephonenumber == NULL){
            strcat(sendsmstext, "no number set\n");
          }else{
            strcat(sendsmstext, mytelephonenumber);
            strcat(sendsmstext, "\n");
          }//else       
          strcat(sendsmstext,"Interval: ");
          strcat(sendsmstext,intervallarray);
          strcat(sendsmstext, "h\n");
          strcat(sendsmstext,"Wifi Status: ");  
          if(iswifion == true){
            strcat(sendsmstext,"on\n");
          }else{
            strcat(sendsmstext,"off\nBattery Status: ");
          }
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);          
          free(sendernumber);
          DeleteSMS(i);
        }else
        if((strcasestr(checksmstext, "int1") != NULL && strlen(checksmstext) == 4) || (strcasestr(checksmstext, "int 1") != NULL && strlen(checksmstext) == 5)){
          interval = 1;
          char *sendernumber = SenderNumber(i);          
          char *sendsmstext = sendsmstextarray; 
          memset(sendsmstextarray, NULL, 161);
          strcat(sendsmstext, "GSM will be switched on every 1 hour.\nBattery Status: ");
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);          
          free(sendernumber);
          DeleteSMS(i);
        }else
        if((strcasestr(checksmstext, "int2") != NULL && strlen(checksmstext) == 4) || ((strcasestr(checksmstext, "int 2") != NULL && strlen(checksmstext) == 5))){
          interval = 2;
          char *sendernumber = SenderNumber(i);          
          char *sendsmstext = sendsmstextarray; 
          memset(sendsmstextarray, NULL, 161);
          strcat(sendsmstext, "GSM will be switched on every 2 hours.\nBattery Status: ");
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);          
          free(sendernumber);
          DeleteSMS(i);        
        }else
        if((strcasestr(checksmstext, "int4") != NULL && strlen(checksmstext) == 4) || (strcasestr(checksmstext, "int 4") != NULL && strlen(checksmstext) == 5)){
          interval = 4;
          char *sendernumber = SenderNumber(i);          
          char *sendsmstext = sendsmstextarray; 
          memset(sendsmstextarray, NULL, 161);
          strcat(sendsmstext, "GSM will be switched on every 4 hours.\nBattery Status: ");
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);          
          free(sendernumber);
          DeleteSMS(i);   
        }else
        if((strcasestr(checksmstext, "int8") != NULL && strlen(checksmstext) == 4) || (strcasestr(checksmstext, "int 8") != NULL && strlen(checksmstext) == 5)){
          interval = 8;
          char *sendernumber = SenderNumber(i);          
          char *sendsmstext = sendsmstextarray; 
          memset(sendsmstextarray, NULL, 161);
          strcat(sendsmstext, "GSM will be switched on every 8 hours.\nBattery Status: ");
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);          
          free(sendernumber);
          DeleteSMS(i); 
        }else
        if((strcasestr(checksmstext, "int12") != NULL && strlen(checksmstext) == 5) || (strcasestr(checksmstext, "int 12") != NULL && strlen(checksmstext) == 6)){
          interval = 12;
          char *sendernumber = SenderNumber(i);          
          char *sendsmstext = sendsmstextarray; 
          memset(sendsmstextarray, NULL, 161);
          strcat(sendsmstext, "GSM will be switched on every 12 hours.\nBattery Status: ");
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);          
          free(sendernumber);
          DeleteSMS(i); 
        }else
        if((strcasestr(checksmstext, "int24") != NULL && strlen(checksmstext) == 5) || (strcasestr(checksmstext, "int 24") != NULL && strlen(checksmstext) == 6)){
          interval = 24;
          char *sendernumber = SenderNumber(i);          
          char *sendsmstext = sendsmstextarray; 
          memset(sendsmstextarray, NULL, 161);
          strcat(sendsmstext, "GSM will be switched on every 24 hours.\nBattery Status: ");
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);          
          free(sendernumber);
          DeleteSMS(i); 
        }else
        if(strcasestr(checksmstext, "wapp") != NULL && strlen(checksmstext) == 4){ //wlan daten + GSM daten für app
          char *sendernumber = SenderNumber(i);          
          char *sendsmstext = sendsmstextarray; 
          gsmOff();
          delay(100);
          wifiOn();
          GetWifisApp();
          flushwifi(100);
          wifiOff();
          gsmOn();
          Config();
          waitFor("+CMTI", 15000); 
          flushgsm(5000);       
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          SendSMS(sendernumber, sendsmstext);              
          delay(20000);
          getLocationApp();
          SendSMS(sendernumber, sendsmstext);          
          free(sendernumber);
          DeleteSMS(i);
        }else
        if(strcasestr(checksmstext, "warningnumber") != NULL && strlen(checksmstext) == 13){
          mytelephonenumber = SenderNumber(i);
          char *sendernumber = mytelephonenumber;          
          char *sendsmstext = sendsmstextarray; 
          memset(sendsmstextarray, NULL, 161);  
          strcat(sendsmstext,"Battery Status Warningnumber has been changed to "); 
          strcat(sendsmstext, mytelephonenumber);
          strcat(sendsmstext,"\nBattery Status: ");
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);              
          free(sendernumber);
          DeleteSMS(i);    
        }else
        if(strcasestr(checksmstext, "wifi on") != NULL && strlen(checksmstext) == 7){
          char *sendernumber = SenderNumber(i);          
          char *sendsmstext = sendsmstextarray; 
          memset(sendsmstextarray, NULL, 161);
          gsmOff();
          delay(100);
          wifiOn();
          iswifion = true;
          GetWifis();
          flushwifi(100);
          wifiOff();
          gsmOn();
          Config();
          waitFor("+CMTI", 15000); 
          flushgsm(5000);  
          strcat(sendsmstext, "\nWifi is on!\n");     
          strcat(sendsmstext,"Battery Status: ");
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);              
          free(sendernumber);
          DeleteSMS(i);   
       }else
        if(strcasestr(checksmstext, "wifi off") != NULL && strlen(checksmstext) == 8){
          char *sendernumber = SenderNumber(i);          
          char *sendsmstext = sendsmstextarray; 
          memset(sendsmstextarray, NULL, 161);
          iswifion = false;
          strcat(sendsmstext,"Wifi Off!"); 
          strcat(sendsmstext,"\nBattery Status: ");
          char battpercentage[3] = ""; 
          itoa(battpercent,battpercentage,10); 
          strcat(sendsmstext, battpercentage);
          strcat(sendsmstext, "%");
          SendSMS(sendernumber, sendsmstext);              
          free(sendernumber);
          DeleteSMS(i);   
        }       
      }

      i++;
      free(unread);
      unread = GetUnread(i);

      if(strcasestr(unread, "REC") == NULL && iswifion == true){
        free(unread);
        i = 1;
        gsmOff();
        wifiOn();
        for (int counter3 = 0; counter3 < 450; counter3++){ ////sleep 15 min (450)
          delayWDT(WDTO_2S);   
        }
        wifiOff();
        gsmOn();
        Config();
        waitFor("+CMTI", 15000); 
        flushgsm(5000);
        unread = GetUnread(i);

        battpercent = getBattPercent();
        battpercent2 = 120;
        batcounter = 0;

        // checks battery level after 2 secondes until it`s equal. Sometimes there was an issue before
        do{ 
          battpercent2 = battpercent;
          flushgsm(2000);
          battpercent = getBattPercent();
          batcounter++;
        }while((battpercent != battpercent2) && batcounter < 5);  
        
      }//if wifi on
      
    }//while
  free(unread);

  }//Battery > 0?
  gsmOff();

  for (int counter2 = 0; counter2 < (interval * (3600/2)); counter2++) { //sleep X h
    delayWDT(WDTO_2S);   // deep sleep
  }
}

void flushgsm(uint16_t timeout) {
  unsigned long t = millis();
  while (millis() < t + timeout)
    if (gsmSerial.available()) {
      gsmSerial.read();
    }
}

void flushwifi(uint16_t timeout) {
  unsigned long t = millis();
  while (millis() < t + timeout)
    if (wifiSerial.available()) {
      wifiSerial.read();
    }
}

char *SenderNumber(int SMSindex) { 

  gsmSerial.print(F("AT+CMGR="));
  gsmSerial.print(SMSindex);
  gsmSerial.print(F(",1"));
  gsmSerial.print(F("\r"));

  char *ptr;
  byte count = 0;
  char *p = readData(5000); //Max response time 5 sec per sim800 AT manual
  ptr = strtok(p, "\"");
  while (count <= 2) {
    ptr = strtok(NULL, "\"");
    count++;
  }
    char *sendernumber = (char*)malloc(20); 
    strncpy(sendernumber, ptr, 20);
    return sendernumber;
}

char *GetUnread(int SMSindex) {

  gsmSerial.print("AT+CMGR=");
  gsmSerial.print(SMSindex);
  gsmSerial.print(",1");    //do not change SMS status
  gsmSerial.print("\r");

  char *ptr;
  byte count = 0;
  char *p = readData(5000); //Max response time 5 sec per sim800 AT manual
  ptr = strtok(p, "\"");
  while (count < 1) { 
    ptr = strtok(NULL, "\"");
    count++;
  }
  char *unread = (char*)malloc(11);
  memset(unread, NULL, 11);
  strncpy(unread, ptr, 11);
  return unread;

}

char *GetSMSText(int SMSindex) { 

  gsmSerial.print(F("AT+CMGR="));
  gsmSerial.print(SMSindex);
  gsmSerial.print(F(",0"));
  gsmSerial.print(F("\r"));
  char *ptr;
  char *p = readData(5000); //Max response time 5 sec per sim800 AT manual
  ptr = strtok(p, "\n");
  ptr = strtok(NULL, "\n");
  ptr = strtok(NULL, "\n");
  char *smstext = sendsmstextarray;
  memset(sendsmstextarray, NULL, 161);
  strncpy(smstext, ptr, 160);
  sendsmstextarray[strlen(smstext)-1]='\0';
  strlwr(smstext);
  //remove space after sms text
  while(sendsmstextarray[strlen(smstext)-1] == ' '){ 
    sendsmstextarray[strlen(smstext)-1] = '\0';
  } 
  //remove space in front of sms text
  if(sendsmstextarray[0] == ' '){ 
    int i=0;
    for (; i < strlen(smstext) - 1 ; i++){
       sendsmstextarray[i] = sendsmstextarray[i+1];
    }
    sendsmstextarray[i] = '\0';
  }
  return smstext;
}

bool SendSMS(const char* number, char* text) { 

  if (CallReady(30) == false) { //checks if connected
    return false;
  }
  if (strlen(number) < 10 || strlen(number) > 20){
    return false;
  }

  gsmSerial.print(F("AT+CMGS=\""));
  gsmSerial.print(number);
  gsmSerial.print(F("\""));
  gsmSerial.println();
  delay(1000);  
  gsmSerial.print(text); 
  delay(100);
  gsmSerial.println((char)26); //the ASCII code of the ctrl+z is 26 (final command to send out SMS)
  delay(100);
  gsmSerial.println();
  if (waitFor("OK", 20000) == true) {
    return true;
  } else {
    return false;
  }
}

byte getBattPercent() { 

  //This function is use for getting battery percentage
  //It will return a value between 0 - 100
  // Error Code 111
  char *ptr;
  
  gsmSerial.println(("AT+CBC"));
  char *p = readData(1000);
  ptr = strtok(p, ",");
  ptr = strtok(NULL, ",");

  byte chargestatus = atoi(ptr); 
  // converts the result to interger
  if(strlen(ptr)>3){
    return 111;
  }
  return chargestatus;
}

char *readData(int timer) { 

  memset(dataarray, NULL, arraysize);
  int a = 0;
  int timeout = 0;
  int wait = 0;

  while (!gsmSerial.available() && timeout < (timer / 10)) {
    delay(10);
    timeout++;
  }

  while (wait <= 20 && a < arraysize-1) { 
    if (!gsmSerial.available()) {
      wait++;
      delay(5);
    } else {
      dataarray[a] = gsmSerial.read();
      a++;
      wait = 0;
    }
  }
  if (a == arraysize-1) { //if more data then memory - data will be deleted here
    flushgsm(1000);
  }
  dataarray[a] = '\0';
  return dataarray;

}

void wifiOff() {

  pinMode(PD3, INPUT); //CH_EN Pin off
  wifiSerial.begin(9600);
  wifiSerial.println(F("AT"));
  waitForWifi("OK", 4000);
  wifiSerial.println(F("AT+GSLP=0"));
  waitForWifi("OK", 4000);
  wifiSerial.end();

}

void delayWDT(byte timer) { 

  sleep_enable(); //enable the sleep capability
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //set the type of sleep mode. Default is Idle
  ADCSRA &= ~(1 << ADEN); //Turn off ADC before going to sleep (set ADEN bit to 0)
  SPCR = 0;  //turn off SPI
  power_all_disable(); //turn off SPI and ADC
  WDTCSR |= 0b00011000;    //Set the WDE bit and then clear it when set the prescaler, WDCE bit must be set if changing WDE bit
  WDTCSR =  0b01000000 | timer; //Or timer prescaler byte value with interrupt selectrion bit set
  wdt_reset(); //Reset the WDT
  sleep_cpu(); //enter sleep mode. Next code that will be executed is the ISR when interrupt wakes Arduino from sleep
  sleep_disable(); //disable sleep mode
  SPCR = 1; //turn on SPI
  power_all_enable(); //turn on SPI and ADC
  ADCSRA |= (1 << ADEN); //Turn the ADC back on

}

ISR (WDT_vect) {

  wdt_disable();

}

void gsmOff() { //switch gsm off
  gsmSerial.println("AT+CPOWD=1");
  waitFor("NORMAL POWER DOWN", 5000);
  gsmSerial.end();
  pinMode(PD6, INPUT); //RX
  pinMode(PD5, INPUT); //TX
}

bool waitFor(const char* searchtext, int timer) {

  int i = 0;
  int timercounter = 0;
  int arraysize = 200;
  char incomingByte[arraysize]; 
  memset(incomingByte, NULL, arraysize);
  while (timercounter < (timer / 5) && i < arraysize) {

    if (!gsmSerial.available()) {
      delay(5);
      timercounter++;
      continue;
    }
    char buffer = gsmSerial.read();
    if (buffer == 0) {
      continue;
    }
    incomingByte[i] = buffer;  
    i++;
    if (strcasestr(incomingByte, searchtext)) { 
      flushgsm(10);
      return true;
    }
  }

  if (arraysize == 240) { //more data then memory will be deleted here
    flushgsm(1000);
  }
  return false;

}

void Config() { //configures the SIM module after start

  gsmSerial.print("ATE0\r"); //turns off the Echo of GSM shield
  waitFor("OK", 2000);
  gsmSerial.print("AT+CMGF=1\r"); //converts msg style to text
  waitFor("OK", 4000);

}

void gsmOn() { //switch gsm on 

  pinMode(PD5, OUTPUT);
  gsmSerial.begin(9600);
  delay(1000);
  gsmSerial.println("AT");

  if (waitFor("OK", 1000)) {
    return;
  }

  // Pull PWRKEY low
  pinMode(PD2, OUTPUT);
  digitalWrite(PD2, LOW);
  // For more then one second
  delay(1100);
  // pull high again
  pinMode(PD2, INPUT);
  // Send data to the the Sim800 so it auto bauds
  delayWDT(WDTO_2S);
  delayWDT(WDTO_2S);
  gsmSerial.print(("\r\n\r\n\r\n"));
  waitFor("SMS Ready", 15000);

}

void wifiOn() { 

  pinMode(A2, OUTPUT); 
  digitalWrite(A2, LOW);
  delay(500);
  digitalWrite(A2, HIGH);
  delay(500);
  digitalWrite(A2, LOW);
  delay(500);
  digitalWrite(A2, HIGH);
  delay(500);
  wifiSerial.begin(9600);
  wifiSerial.println(F("AT"));
  waitForWifi("OK", 4000);
  wifiSerial.println(F("AT+CWMODE=3"));
  waitForWifi("OK", 4000);
  wifiSerial.println(F("AT+CWSAP=\"BikeBean.de\",\"12345678\",7,3"));
  waitForWifi("OK", 4000);
  delay(5000); //wait until currentpeaks are gone before further commands will be executed

}

bool waitForWifi(const char* searchtext, int timer) {

  int i = 0;
  int arraysize = 200;
  int timercounter = 0;
  char incomingByte[arraysize]; 
  memset(incomingByte, NULL, arraysize);

  while (timercounter < (timer / 5) && i < arraysize) {

    if (!wifiSerial.available()) {
      delay(5);
      timercounter++;
      continue;
    }
    char buffer = wifiSerial.read();
    if (buffer == 0) {
      // Ignore NULL character
      continue;
    }
    incomingByte[i] = buffer;   
    i++;
    if (strcasestr(incomingByte, searchtext)) { 
      flushwifi(10);
      return true;
    }
  }

  if (arraysize == 240) { //more data then memory will be deleted here
    flushwifi(1000);
  }

  return false;
}

char *readwifiData(int timer) { 

  memset(dataarray, NULL, arraysize);
  int a = 0;
  int timeout = 0;
  int wait = 0;

  while (!wifiSerial.available() && timeout < (timer / 10)) {
    delay(10);
    timeout++;
  }

  while (wait <= 500 && a < arraysize-1) { 
    if (!wifiSerial.available()) {
      wait++;
      delay(5);
    } else {
      dataarray[a] = wifiSerial.read();
      a++;
      wait = 0;
    }
  }

  if (a == arraysize-1) { //more data then memory will be deleted here
    flushwifi(1000);
  }

  dataarray[a] = '\0'; 
  return dataarray;

}

/*
void enableBearerProfile(){
  // This function enable and set the bearer profile
  gsmSerial.print(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n")); 
  waitFor("OK", 10000);
  gsmSerial.println(F("AT+SAPBR=3,1,\"APN\",\"RCMNET\""));
  waitFor("OK", 10000);
  gsmSerial.print(F("AT+SAPBR=1,1\r\n")); 
  waitFor("OK", 10000);
  gsmSerial.print(F("AT+SAPBR=2,1\r\n")); 
  waitFor("OK", 10000);
}
*/

/*
void disableBearerProfile() {

  // Disable the bearer profile
  gsmSerial.print(F("AT+SAPBR=0,1\r\n"));
  waitFor("OK", 10000);
}
*/

void GetWifis() { 

  wifiSerial.println(F("AT"));
  char* smstext = sendsmstextarray; 
  if (waitForWifi("OK", 4000) == true) {
    wifiSerial.println(F("AT+CWLAP"));  //get wifis
    char *ptr;
    memset(sendsmstextarray, NULL, 161); 
    byte count = 0;
    byte count2 = 0;
    char *p = readwifiData(5000); // ~4 wifis fit into this array

    ptr = strtok(p, "\"");
    while (strlen(smstext) < 92 && ptr != NULL ) { 
      ptr = strtok(NULL, "\""); 
      count++;
      strcat(smstext, ptr);
      if (count % 2 == 0) {
        while (count2 < 4 && ptr != NULL ) {
          ptr = strtok(NULL, ",");
          count2++;
        }
        sendsmstextarray[strlen(smstext)-1]='\n';
        count2 = 0;
      }
    }//while
    if (count < 3) {
      memset(smstext, NULL, 161);
      strcpy(smstext, "no wifi available"); 
    }
  }else{
    memset(smstext, NULL, 161);
    strcpy(smstext, "ERROR"); 
  }
}

/*
void getLocation() { 

  enableBearerProfile();
  gsmSerial.print(F("AT+CIPGSMLOC=1,1\r\n"));
  char *longitude;
  char *lat;
  char *p = readData(5000);
  char *mapslink = sendsmstextarray;
  memset(sendsmstextarray, NULL, 161);
  
  disableBearerProfile();

  if (strlen(p) > 35) {
    longitude = strtok(p, ",");
    longitude = strtok(NULL, ",");
    lat = strtok(NULL, ",");
    strcpy(mapslink, "https://www.google.de/maps/search/");
    strncat(mapslink, lat, 10);
    strcat(mapslink, ",");
    strncat(mapslink, longitude, 10);
    strcat(mapslink, "/@");
    strncat(mapslink, lat, 10);
    strcat(mapslink, ",");
    strncat(mapslink, longitude, 10);
    strcat(mapslink, ",16.5z");
  } else {
    memset(mapslink, NULL, 161);
    strcpy(mapslink, "ERROR"); 
  }
}
*/

boolean CallReady(int waittime) {
  for (int connectioncounter = 0; connectioncounter < waittime; connectioncounter++) {
    gsmSerial.println(F("AT+CCALR?"));
    if ((waitFor(": 1", 1000)) == true) {
      return true;
    }
  }
  return false;
}

void DeleteSMS(int SMSindex) { //delete SMS with index X and all sent SMS
  int i=0;
  do{
    gsmSerial.print(F("AT+CMGD="));
    gsmSerial.print(SMSindex);
    gsmSerial.print(F(",0\r"));
    i++;
  }while(waitFor("OK", 4000) != true && i < 3);
  gsmSerial.println(F("AT+CMGDA=\"DEL SENT\"")); //delete all sent SMS
  waitFor("OK", 4000);
}

void getLocationApp() { 

  gsmSerial.print(F("AT+CENG=3\r\n"));
  waitFor("OK",1000);
  gsmSerial.print(F("AT+CENG?\r\n"));
  char *p = readData(5000);
  char *smstext = sendsmstextarray;
  memset(sendsmstextarray, NULL, 161);
  char *ptr;
  byte count = 0;

  ptr = strtok(p, "\"");
  while (strlen(smstext) < 140 && ptr != NULL && strcasestr(smstext, "CENG") == NULL && strcasestr(smstext, "FFFF") == NULL && strcasestr(smstext, "0000") == NULL ) { 
    count++;
    ptr = strtok(NULL, ","); 
    strcat(smstext, ptr);
    strcat(smstext, ",");
    if(count % 4 == 0){
      ptr = strtok(NULL, ","); 
      ptr = strtok(NULL, "\"");
      strcat(smstext, ptr);
      strcat(smstext, "\n");
      count = 0;
      ptr = strtok(NULL, "\"");
     }     
   }//while

   count=0;
   while((strcasestr(smstext, "CENG") != NULL || strcasestr(smstext, "FFFF") != NULL || strcasestr(smstext, ",,") != NULL || strcasestr(smstext, "0000") != NULL) && count < 8){
     count++;
     char *c2 = strrchr(sendsmstextarray, '\n');
     *(c2+1) = '\0';   
   }
  
   if(strlen(smstext)<15 && strlen(smstext)>150){
     memset(smstext, NULL, 161);
     strcpy(smstext, "ERROR");
   }
}

void GetWifisApp() { 
  wifiSerial.println(F("AT"));
  char* smstext = sendsmstextarray; 
  if (waitForWifi("OK", 4000) == true) {
    wifiSerial.println(F("AT+CWLAP"));  //get wifis
    char *ptr;
    memset(sendsmstextarray, NULL, 161); 
    byte count = 0;
    char *p = readwifiData(5000);
    ptr = strtok(p, ",");
    ptr = strtok(NULL, ","); 
    while (strlen(smstext) < 139 && ptr != NULL ) { 
      ptr = strtok(NULL, ","); 
      strcat(smstext, ptr+1);
      ptr = strtok(NULL, ":");
          strcat(smstext, ptr+1);
        while (count < 4 && ptr != NULL ) {
          ptr = strtok(NULL, ":");
          strcat(smstext, ptr);
          count++;
        }
        count = 0;
        ptr = strtok(NULL, "\"");
        strcat(smstext, ptr);
        strcat(smstext, "\n"); //neu
        ptr = strtok(NULL, ",");
        ptr = strtok(NULL, ",");
        ptr = strtok(NULL, ",");
        ptr = strtok(NULL, ",");
    }//while
    if (strlen(smstext) < 10) {
      memset(sendsmstextarray, NULL, 161); 
      strcpy(smstext, "no wifi available"); 
    }
  }else{
    memset(sendsmstextarray, NULL, 161); 
    strcpy(smstext, "ERROR"); 
  }
}
