#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <DHT.h>

#define DHTPIN 7
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(8,9,5,4,3,2);  //lines used by LCD (cannot use 10, 11, 12, 13, as these are used by the ethernet)
int lcdbacklight = 6;  //LCD backlight on pin 6
int backlightdelay = 0;
int x = 0;
int y = 0;
int t = 0;
int h = 0;
int startdelay = 20; //default is 250
// int irled = 13;  //sets IR LED to pin 13


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   //physical mac address
byte ip[] = { 10, 1, 1, 30 };                          // ip in lan (that's what you need to use in your browser. ("192.168.1.178")
byte gateway[] = { 10, 1, 1, 1 };                      // internet access via router
byte subnet[] = { 255, 255, 255, 0 };                  //subnet mask
EthernetServer server(11220);                          //server port     
String readString;

void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.clear();
  pinMode(lcdbacklight, OUTPUT);
  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW);

  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  dht.begin();
  startroutine();  //calls startroutine sub
}


void loop() {
  
  get_DHT_data();  //runs the routine for fetching data from the DHT module and printing it to the LCD
  htmlroutine();  //runs the html routine for printing data to webpage
  
}


void startroutine() {  //flashy startup routine makes it look pretty
  
  for(int y = 0; y <5; y++) {
  digitalWrite(lcdbacklight, LOW);
  delay(50);
  digitalWrite(lcdbacklight, HIGH);
  delay(100);
  }
  
  
  lcd.clear();
  delay(startdelay*8);
  lcd.print("Remote Climate");
  lcd.setCursor(0,1);
  lcd.print("Control System");
  delay(startdelay*16);
  
  lcd.clear();
  lcd.print("SW ver 1.3");
  lcd.setCursor(0,1);
  lcd.print("Grant Caban 2014");
  delay(startdelay*16);
  
  lcd.clear();
  lcd.print("Initialising");
  delay(startdelay*6);
  lcd.setCursor(0,1);
    for(int x = 0; x < 16; x++){
      lcd.print((char)219);
      delay(startdelay);   
    if (x==4){
      delay(startdelay*6);
    }
    if (x==10){
      delay(startdelay*8);
    }
    }
    delay(startdelay*8);
  
    lcd.clear();
    lcd.print(Ethernet.localIP());
    lcd.setCursor(0,1);
    lcd.print("59.167.157.55");
    delay(startdelay*12);
    lcd.clear();
    lcd.print("Port");
    lcd.setCursor(0,1);
    lcd.print("11220");
    delay(startdelay*8);
    
    
  
 
    lcd.clear();
    lcd.print("Temperature: ");
    lcd.setCursor(0,1);
    lcd.print("Humidity:    ");
    delay(1000);
}

void htmlroutine()
{
  // Create a client connection
  EthernetClient client = server.available();
  
  if (client)
  {
    while (client.connected())
    {   
      if (client.available())
      {
        
        char c = client.read();
        //read char by char HTTP request
          if (readString.length() < 100)
          {
            //store characters to string
            readString += c;
            //Serial.print(c);
          }

         //if HTTP request has ended
         if (c == '\n')
         {          
           Serial.println(readString); //print to serial monitor for debuging
              
           if (readString.indexOf("?button1on") >0){  //if button ON condition is met
               AC_ON_routine();
               
               client.println("GET / HTTP/1.0");
               client.println();
           }

           else if (readString.indexOf("?button1off") >0){  //if button OFF condition is met
               AC_OFF_routine();
               
               client.println("GET / HTTP/1.0");
               client.println();
           }
           else
           {
               client.println("HTTP/1.1 200 OK"); //send new page
               client.println("Content-Type: text/html");
               client.println();     
               client.println("<HTML>");
               client.println("<HEAD>");
               client.println("<meta name='apple-mobile-web-app-capable' content='yes' />");
               client.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
               client.println("<link rel='stylesheet' type='text/css' href='http://randomnerdtutorials.com/ethernetcss.css' />");
               client.println("<TITLE>Remote Climate Control System - Grant Caban</TITLE>");
               client.println("</HEAD>");
               client.println("<BODY>");
               client.println("<H1>Grants Remote Climate Control System</H1>");
               client.println("<hr />");
               client.println("<br />");
               client.println("<H2>Aircon is set to 25&#176</H2>");
               client.println("<br />");  
               client.print("<H2>Current indoor temperature is ");
               client.print(int(dht.readTemperature()));
               client.print("&#176</H2>");  //degrees symbol
               client.println("<H2>Current indoor humidity is ");
               client.print(int(dht.readHumidity()));
               client.print("&#37</H2>");  //percentage symbol
               client.println("<br />"); 
               client.println("<a href=\"/?button1on\"\">Turn On Aircon</a>");
               client.println("<a href=\"/?button1off\"\">Turn Off Aircon</a><br />");   
               client.println("<br />");     
               client.println("<br />"); 
               client.println("<br />"); 
               client.println("</BODY>");
               client.println("</HTML>");
           }
           
           delay(100);
           //stopping client
           client.stop();
           
           //clearing string for next read
           readString="";  
         }
       }
    }
  }
}

void get_DHT_data() {
  
  int h = dht.readHumidity();
  int t = dht.readTemperature();

  if (isnan(t) || isnan(h)) dhterror();      //if pin 7 in not a number, dhterror sub is called
  if (h+t==0) dhterror();                    //if h+t (humidity + temp) = 0, dhterror sub is called
  else {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
    lcd.clear();
    lcd.print("Temperature: ");
    lcd.print(t);
    lcd.print((char)223);
    lcd.setCursor(0,1);
    lcd.print("Humidity:    ");
    lcd.print(h);
    lcd.print("%");
    delay(2000);
  }
}


void dhterror() {  //Prints "System error, check DHT module" when no data is received on pin 7 to LCD module
  
    Serial.println("Failed to read from DHT");
     lcd.clear();
     lcd.print("System error");
     lcd.setCursor(0,1);
     lcd.print("Check DHT module"); 
 
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {   
      if (client.available()) {
        char c = client.read();
     
        //read char by char HTTP request
        if (readString.length() < 100) {
          //store characters to string
          readString += c;
          //Serial.print(c);
         }

         //if HTTP request has ended
         if (c == '\n') {              
           client.println("HTTP/1.1 200 OK"); //send new page
           client.println("Content-Type: text/html");
           client.println();     
           client.println("<HTML>");
           client.println("<HEAD>");
           client.println("<meta name='apple-mobile-web-app-capable' content='yes' />");
           client.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
           client.println("<link rel='stylesheet' type='text/css' href='http://randomnerdtutorials.com/ethernetcss.css' />");
           client.println("<TITLE>Remote Climate Control System - Grant Caban</TITLE>");
           client.println("</HEAD>");
           client.println("<BODY>");
           client.println("<H1>Grants Remote Climate Control System</H1>");
           client.println("<hr />");
           client.println("<br />");
           client.println("<H2>System offline</H2>");
           client.println("<br />");  
           client.println("<H2>no data received from DHT module");
           client.println("</BODY>");
           client.println("</HTML>");
    delay(100);
           //stopping client
           client.stop();
        
            //clearing string for next read
            readString="";  
           
         }
       }
    }
}
} 

void AC_ON_routine() {

    lcd.clear();
    lcd.print("Remote access");
    lcd.setCursor(0,1);
    lcd.print("Aircon ON");

  for(int a = 0; a <10; a++) {
      digitalWrite(A0, LOW);
      delay(50);
      digitalWrite(A0, HIGH);
      delay(50);
    }
delay(3000);
}


void AC_OFF_routine() {

    lcd.clear();
    lcd.print("Remote access");
    lcd.setCursor(0,1);
    lcd.print("Aircon OFF");

  for(int a = 0; a <4; a++) {
      digitalWrite(A0, HIGH);
      delay(200);
      digitalWrite(A0, LOW);
      delay(200);
    }
delay(3000);
}
  
