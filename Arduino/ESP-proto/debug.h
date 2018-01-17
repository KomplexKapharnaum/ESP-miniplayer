#define DEBUG

#ifdef DEBUG
 #define LOGSETUP()     Serial.begin(115200)
 #define LOGINLINE(x)   Serial.print (x)
 #define LOGDEC(x)      Serial.print (x, DEC)
 #define LOGF(x, y)     Serial.printf (x, y)
 #define LOGF2(x, y1, y2)     Serial.printf (x, y1, y2)
 #define LOG(x)         Serial.println (x)
#else
 #define LOGSETUP()     
 #define LOGINLINE(x)   
 #define LOGDEC(x)
 #define LOGF(x, y)
 #define LOGF2(x, y1, y2)       
 #define LOG(x)         
#endif
