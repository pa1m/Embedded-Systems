 #include<EEPROM.h>
#include<string.h>
#include<Time.h>

#define MAX_NUM_OF_ENTRIES 20

struct memoryLocation
{
  boolean flag;
  
  String lat;
  String lon;
  
  int hr;
  int mn;

  String what;
} memLoc[MAX_NUM_OF_ENTRIES];


void setup()
{
  Serial.begin(115200);
  while(!Serial);
  
  for(int i=0; i< MAX_NUM_OF_ENTRIES; i++)
    memLoc[i].flag = 0;

  append_db("10","10",10,10,"10");
  append_db("10","10",20,20,"10");
 //delete_db(0);
 
}

void loop()
{
//  for(int i=0; i < MAX_NUM_OF_ENTRIES; i++)
//    if(memLoc[i].flag==1)
//    {
//      Serial.println(memLoc[i].hr);
//    }
}

void append_db(String lat, String lon, int hr, int mn, String what)
{
  for(int i=0;i<MAX_NUM_OF_ENTRIES;i++)
    if(memLoc[i].flag==0)
      {
        memLoc[i].lat = lat;
        memLoc[i].lon = lon;
        memLoc[i].hr = hr;
        memLoc[i].mn = mn;
        memLoc[i].what = what;
        memLoc[i].flag = 1;
        break;
      }
}

void delete_db(int to_be_deleted)
{
  memLoc[to_be_deleted].flag=0;
}

//void read_db()
