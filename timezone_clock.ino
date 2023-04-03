#include <RTClib.h>
#include <Wire.h>
#include "Waveshare_LCD1602_RGB.h" // https://www.waveshare.com/wiki/LCD1602_RGB_Module
#include <Timezone.h>    // https://github.com/JChristensen/Timezone

// LCD
Waveshare_LCD1602_RGB lcd(16,2);  //16 characters and 2 lines of show
int r,g,b,t=0; //LCD colors

// RTC clock
RTC_DS3231 rtc;
char time_str[32]; //TIME STR

// Temp
char temp_str[10];

//Timezones west to east

//https://en.wikipedia.org/wiki/Pacific_Time_Zone#Daylight_time
//Los Angeles
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};   //UTC - 8 hours //Standard time
TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};  //UTC - 7 hours //Daylight saving
Timezone usPacific(usPDT, usPST);

//https://en.wikipedia.org/wiki/Eastern_Time_Zone
//Boston
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //UTC - 5 hours
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 4 hours
Timezone usEastern(usEDT, usEST);

//Argentina
//Argentina is not observing daylight saving time (DST) and the entire country stays on UTC-03:00.
TimeChangeRule artSTD = {"ART", Last, Sun, Oct, 2, -180};  //UTC - 3 hours
Timezone artTime(artSTD, artSTD);

//London
TimeChangeRule ukGMT = {"GMT", Last, Sun, Oct, 2, 0};  //UTC - 0 hours
TimeChangeRule ukBST = {"BST", Last, Sun, Mar, 1, 60};   //UTC + 1 hours
Timezone ukTime(ukBST, ukGMT);

//Luxembourg
TimeChangeRule euCET = {"CET", Last, Sun, Oct, 3, 60};  //UTC + 1 hours
TimeChangeRule euCEST = {"CEST", Last, Sun, Mar, 2, 120};   //UTC + 2 hours
Timezone luxTime(euCEST, euCET);

//Luxembourg
TimeChangeRule ltEET = {"EET", Last, Sun, Oct, 3, 120};  //UTC + 2 hours
TimeChangeRule ltEEST = {"EEST", Last, Sun, Mar, 2, 180};   //UTC + 3 hours
Timezone ltTime(ltEEST, ltEET);

DateTime getLastSundayFromMonth(int year, int month){
  DateTime date = DateTime(year, month+1, 1, 0, 0, 0); // 1st of Month
  int weekday = date.dayOfTheWeek(); // 0 Sunday... 6 Saturday
  int dayDiff = weekday;
  if(weekday == 0){ //If the day is sunday then assign 7
    dayDiff = 7;
  }

  uint32_t new_unix_time = date.unixtime() - (dayDiff*86400); //seconds in day
  return DateTime(new_unix_time);
}

DateTime getUTCtime(int std_offset_secs){ //default 1 hour = 3600 seconds
  DateTime current_time = rtc.now(); // retrieve time
  DateTime summer_date = getLastSundayFromMonth(current_time.year(), 3);

  DateTime winter_date = getLastSundayFromMonth(current_time.year(), 10);

  uint32_t now_t = current_time.unixtime();
  uint32_t summer_t = summer_date.unixtime();
  uint32_t winter_t = winter_date.unixtime();


  uint32_t utc_t = 0;

  //if on Daylight
  if(now_t > summer_t && now_t < winter_t){
    utc_t = now_t - (std_offset_secs + 3600);
  }else{ //standard time
    utc_t = now_t - std_offset_secs;
  }

  DateTime utc = DateTime(utc_t);

  return utc;
}

void printTimeInTimezone(Timezone tz, String city){
  lcd.clear();
  //PRINT CITY
  lcd.setCursor(0,0);
  lcd.send_string(city.c_str());
  //PRINT TIME
  lcd.setCursor(0,1);
  uint32_t utc_time = getUTCtime(3600).unixtime();
  uint32_t city_t = tz.toLocal(utc_time);
  DateTime date = DateTime(city_t);
  sprintf(time_str, "%02d:%02d %02d/%02d/%02d",  date.hour(), date.minute(), date.day(), date.month(), date.year());
  Serial.println(time_str);
  lcd.send_string(time_str);
}

void setup() 
{
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();

  //LCD
  lcd.init(); 
  lcd.setRGB(255,255,255); //white
  lcd.setCursor(0,0);
  lcd.send_string("Hola!");
  lcd.setCursor(0,1);
  lcd.send_string("Moien!");
  delay(1000);

  // Uncomment below block to set time. 7 seconds added to bear in mind the compilation + upload time
  // uint32_t unixtime = DateTime(F(__DATE__),F(__TIME__)).unixtime() + 7; // create DateTime as now + 7 sec
  // rtc.adjust(DateTime(unixtime)); // set the clock on time 
  // DateTime now = rtc.now(); // retrieve time
  // sprintf(t, "%02d:%02d:%02d %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());  // format time
  // Print time
  // Serial.print(F("Setup. Date/Time: ")); 
  // Serial.println(t);
}

unsigned int ctr = 0;

void loop()
{ 
  int wait = 1000*10; //10 secs
  printTimeInTimezone(luxTime, "Luxembourg");
  delay(wait);
  printTimeInTimezone(artTime, "Argentina");
  delay(wait);
  printTimeInTimezone(usEastern, "Boston");
  delay(wait);
  printTimeInTimezone(usPacific, "Los Angeles");
  delay(wait);
  printTimeInTimezone(ltTime, "Lithuania");
  delay(wait);

  ctr+=1;
  switch(ctr % 3){
    case 0:
      lcd.setRGB(64,255,0); //green
      break;
    case 1:
      lcd.setRGB(255,0,0); //red
      break;
    case 2:
      lcd.setRGB(255,255,255); //white
    default:
      break;
  }
}