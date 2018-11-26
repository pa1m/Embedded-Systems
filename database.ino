// The database and ETA calculation
#include<EEPROM.h>
#include<string.h>
#include<Time.h>

// CONVERT TO LC... DON'T FORGET.

#define MAX_NUM_OF_ENTRIES 20
#define VELOCITY_CONSTANT 1462

char* Places[11] = {"Classroom", "Department", "Hostel", "Store", "Beach", "Temple" "Auditorium", "Ground", "Mess", "Bus stop", "Library"};
float LONGITUDE[11] = {};
float LATITUDE[11] = {};
int no_of_places = 11;
char* Works[9] = {"Date", "Meeting", "Lecture", "Class", "Eat", "Work", "Shop", "Buy", "Go"};
int no_of_works = 9;

struct memoryLocation
{
  boolean flag;
  
  float lat;
  float lon;
  
  int hr;
  int mn;

  char* what;
} memLoc[MAX_NUM_OF_ENTRIES];


void setup()
{
  Serial.begin(115200);
  while(!Serial);
  
  for(int i=0; i< MAX_NUM_OF_ENTRIES; i++)
    memLoc[i].flag = 0;

  append_db(10.0, 10.0,10,10,"10");
  append_db(10.0, 10.0,20,20,"10");
 //delete_db(0);
 
}

void loop()
{
}

void append_db(float lat, float lon, int hr, int mn, char* what)
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

void parse_db()
{
//  for(int i=0; i < MAX_NUM_OF_ENTRIES; i++)
//    if(memLoc[i].flag==1)
//    {
//      Serial.println(memLoc[i].hr);
//    }

// If reminder sent, delete the record

/* Do GPS shiz once every 10 or 15 mins.
 * Get current location.
 * Update ETA based on current location and places API
 * Do what we discussed. Based on ETA, Time of event and polling interval, current time. Decision is reminder or no reminder.
 */
}

void get_GPS()
{
  
}

void get_information(char* transcript)
{ 

    //char transcript_words[10][15];
    //int i=0;
    char* token = strtok(transcript, " "); 

    int flag_place = 0;
    int flag_time = 0;
    int flag_work = 0;

    memoryLocation temp;
    
    while (token != NULL) { 
        //strcpy(transcript_words[i], token);
        //i=i+1;
        if(flag_place==0)
        {
          for(int place_iterator=0; place_iterator<no_of_places;place_iterator++)
          {
            if(strcmp(token,Places[place_iterator])==0)
            {
              temp.lon = LONGITUDE[place_iterator];
              temp.lat = LATITUDE[place_iterator];
              flag_place=1;
            }
          }
        }

        else if(flag_time==0)
        {
          // Find the first character of a token and check if number
          // Tokenize the string based on ':' to get hr and mn
          // Using next token from transcript string decide whether to add 12 or not.
          // Update flag.

          
          if(token[0] >= 48 && token[0] <= 57)
          {
            int hou = atoi(strtok(token, ":"));
            int minu = atoi(strtok(NULL, ":"));
            temp.mn=minu;
            temp.hr = hou;
  
          }

          else if(strcmp(token,"p.m")==0)
          {
            temp.hr = temp.hr+12;
            flag_time=1;
          }
         
        }

        else if(flag_work==0)
        {
          for(int work_iterator=0; work_iterator<no_of_works;work_iterator++)
          {
            if(strcmp(token, Works[work_iterator])==0)
            {
              strcpy(temp.what, token);
              flag_work=1;
            }
          }
          
        }

        else
          append_db(temp.lat, temp.lon, temp.hr, temp.mn, temp.what);
          break;
        
        token = strtok(NULL, " "); 
    } 
    
}


float ETA(float lat1, float lon1, float lat2, float lon2)
{
  return (abs(lat1-lat2) + abs(lon1-lon2))*VELOCITY_CONSTANT;
}
