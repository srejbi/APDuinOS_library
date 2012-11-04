// todo deprecate this file, let display-enabled versions define their own ways of displaying
#ifndef __APD_SCREENS_H
#define __APD_SCREENS_H
const char splash_screen[][16] ={ 
                  "APDuinOS",
                  "", 
                  "initializing   ",
                  "",             
                  "",
                  ""};
                         
// what a waste of resources, given the current direction...                         
const char app_screens[][8][31] = { {
                  "* Sensor Status *",
                  "",
                  "",
                  "",
                  "", 
                  "",        
                  "" }, {
                  "* APDuino Status *",
                  " Time:",
                  " Debug:",
                  " Uptime:",
                  " Free RAM:",
                  " Internal Temp:        C      ",
                  " Web Clients:"}, {
                  "* APDuino *",
                  "            APDuino Project   ",
                  "            v.", 
                  "            http://apduino.org",
                  "",
                  "             Arduino Mega 2560",
                  "                GLCD: HJ19264A" } };
#endif

