#include "WString.h"
#ifndef TextGroup_h
#define TextGroup_h
#include "Arduino.h" 
class TextGroup {
public:
  TextGroup(int id, int x, int y );
  void New(int id);
private:
  int _id;
};
#endif