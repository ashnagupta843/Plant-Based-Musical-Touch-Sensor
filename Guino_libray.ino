#include <EasyTransfer.h>
#include <EEPROM.h>


#define guino_executed -1
#define guino_init 0
#define guino_addSlider 1
#define guino_addButton 2
#define guino_iamhere 3
#define guino_addToggle 4
#define guino_addRotarySlider 5
#define guino_saveToBoard 6
#define guino_setFixedGraphBuffer 8
#define guino_clearLabel 7
#define guino_addWaveform 9
#define guino_addColumn 10
#define guino_addSpacer 11
#define guino_addMovingGraph 13
#define guino_buttonPressed 14
#define guino_addChar 15
#define guino_setMin 16
#define guino_setMax 17
#define guino_setValue 20
#define guino_addLabel 12
#define guino_large 0
#define guino_medium 1
#define guino_small 2
#define guino_setColor  21

boolean guidino_initialized = false;

//This function will write a 2 byte integer to the eeprom at the specified address and address + 1
void EEPROMWriteInt(int p_address, int p_value)
{
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);

  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);
}

//This function will read a 2 byte integer from the eeprom at the specified address and address + 1
unsigned int EEPROMReadInt(int p_address)
{
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

//create object
EasyTransfer ET; 

struct SEND_DATA_STRUCTURE
{
  //pvariable definitions here for the data
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO CODE
  char cmd;
  char item;
  int value;
};

// Find a way to dynamically allocate memory
int guino_maxGUIItems = 100;
int guino_item_counter = 0;
int *guino_item_values[100]; 
int gTmpInt = 0; // temporary int for items without a variable
boolean internalInit = true; // boolean to initialize before connecting to serial

// COMMAND STRUCTURE

//give a name to the group of data
SEND_DATA_STRUCTURE guino_data;
int eepromKey = 1234;

//This updates the guino with the new values
void guino_update()
{

  while(Serial.available())
  {
    if(ET.receiveData())
    {
      switch (guino_data.cmd) 
      {
      case guino_init:

        guino_item_counter = 0;
        guidino_initialized = true;
        gInit();
        break;
      case guino_setValue:
        *guino_item_values[guino_data.item] = guino_data.value;
        guino_data.cmd = guino_executed;
        break;
      case guino_buttonPressed:
        gButtonPressed(guino_data.item);
        break;
      case guino_saveToBoard:
        {
          
          gInitEEprom();
          for (int i =0; i < guino_item_counter;i++)
          {
            EEPROMWriteInt(i*2+2, *guino_item_values[i]);
          }
        }
        break;
      }
    }
  }
}

//Initialized the EEProm, EEProm is a form of memory in the arduino that is kept even when the arduino turns off
void gInitEEprom()
{
  if(EEPROMReadInt(0) != eepromKey)
  {
    EEPROMWriteInt(0, eepromKey);
    for (int i =1; i <guino_maxGUIItems;i++)
    {
      EEPROMWriteInt(i*2+2, -3276);
    }
  }

}

//This gets a value from a variable in EEProm
void gGetSavedValue(int item_number, int *_variable)
{
   
  if(EEPROMReadInt(0) == eepromKey && internalInit)
  {
   
    int tmpVar =  EEPROMReadInt((item_number)*2+2);
    if(tmpVar != -3276)
      *_variable = tmpVar;
  }

}

//This initializes the guino dashboard
void gBegin(int _eepromKey)
{

  // Sets all pointers to a temporary value just to make sure no random memory pointers.
  for(int i = 0; i < guino_maxGUIItems; i++)
  {
    guino_item_values[i] = &gTmpInt;
  }
  eepromKey = _eepromKey;

  gInit();
  gInit(); // this one needs to run twice, only way to work without serial connection.
  internalInit = false;
  Serial.begin(115200);
  ET.begin(details(guino_data), &Serial);
  gSendCommand(guino_executed, 0, 0);
  gSendCommand(guino_executed, 0, 0);
  gSendCommand(guino_executed, 0, 0);
  gSendCommand(guino_iamhere, 0, 0); 

}

//This allows me to add a button to the guino dashboard
//I am currently not adding any buttons to my dashboard
int gAddButton(char * _name)
{
  if(guino_maxGUIItems > guino_item_counter)
  {
    gSendCommand(guino_addButton,(byte)guino_item_counter,0);
    for (int i = 0; i < strlen(_name); i++){
      gSendCommand(guino_addChar,(byte)guino_item_counter,(int)_name[i]);
    }
    guino_item_counter++;
    return guino_item_counter-1;
  }
  return -1;
}

//This allows me to create multiple columns in the dashboard
//Currently I only have a single column
void gAddColumn()
{

  gSendCommand(guino_addColumn,0,0);

}

//This allows me to change the background color of the guino dashboard
void gSetColor(int _red, int _green, int _blue)
{
  gSendCommand(guino_setColor, 0, _red);
  gSendCommand(guino_setColor, 1, _green);
  gSendCommand(guino_setColor, 2, _blue);
}

//This allows me to add text to the dashboard
int gAddLabel(char * _name, int _size)
{
  if(guino_maxGUIItems > guino_item_counter)
  { 
    gSendCommand(guino_addLabel,(byte)guino_item_counter,_size);

    for (int i = 0; i < strlen(_name); i++){
      gSendCommand(guino_addChar,(byte)guino_item_counter,(int)_name[i]);
    }

    guino_item_counter++;

    return guino_item_counter-1;
  }
  return -1;


} 

//This allows me to add a line and an enter to the dashboard
int gAddSpacer(int _size)
{
  if(guino_maxGUIItems > guino_item_counter)
  {
    gSendCommand(guino_addSpacer,(byte)guino_item_counter,_size);

    guino_item_counter++;
    return guino_item_counter-1;
  }
  return -1;

}   


//This allows me to add a toggle to the dashboard
//I am currently not using a toggle on my dashboard
int gAddToggle(char * _name, int * _variable)
{
  if(guino_maxGUIItems > guino_item_counter)
  {
    guino_item_values[guino_item_counter] =_variable ;
    gGetSavedValue(guino_item_counter, _variable);
    gSendCommand(guino_addToggle,(byte)guino_item_counter,*_variable);

    for (int i = 0; i < strlen(_name); i++){
      gSendCommand(guino_addChar,(byte)guino_item_counter,(int)_name[i]);
    }

    guino_item_counter++;

    return guino_item_counter-1;


  }
  return -1;
}   

//This allows me to add a fixed graph to the dashboard
int gAddFixedGraph(char * _name,int _min,int _max,int _bufferSize, int * _variable, int _size)
{
  if(guino_maxGUIItems > guino_item_counter)
  {
    gAddLabel(_name,guino_small);
    guino_item_values[guino_item_counter] =_variable ;
    gGetSavedValue(guino_item_counter, _variable);
    gSendCommand(guino_addWaveform,(byte)guino_item_counter,_size);
    gSendCommand(guino_setMax,(byte)guino_item_counter,_max);
    gSendCommand(guino_setMin,(byte)guino_item_counter,_min);
    gSendCommand(guino_setFixedGraphBuffer,(byte)guino_item_counter,_bufferSize);


    guino_item_counter++;

    return guino_item_counter-1;
  }
  return -1;
}

//This allows me to add a graph that moves to the dashboard
//I am currently not using a moving graph in my dashboard
int gAddMovingGraph(char * _name,int _min,int _max, int * _variable, int _size)
{
  if(guino_maxGUIItems > guino_item_counter)
  {
    gAddLabel(_name,guino_small);
    guino_item_values[guino_item_counter] =_variable ;
    gGetSavedValue(guino_item_counter, _variable);
    gSendCommand(guino_addMovingGraph,(byte)guino_item_counter,_size);
    gSendCommand(guino_setMax,(byte)guino_item_counter,_max);
    gSendCommand(guino_setMin,(byte)guino_item_counter,_min);


    guino_item_counter++;

    return guino_item_counter-1;
  }
  return -1;


}   

//This allows me to update any text in my dashboard
int gUpdateLabel(int _item, char * _text)
{

  gSendCommand(guino_clearLabel,(byte)_item,0);
  for (int i = 0; i < strlen(_text); i++){
    gSendCommand(guino_addChar,(byte)_item,(int)_text[i]);
  }



}


//This allows me to add a slider shaped like a circle called a rotary slider
//Currently not using rotary sliders
int gAddRotarySlider(int _min,int _max, char * _name, int * _variable)
{
  if(guino_maxGUIItems > guino_item_counter)
  {
    guino_item_values[guino_item_counter] =_variable ;
    gGetSavedValue(guino_item_counter, _variable);
    gSendCommand(guino_addRotarySlider,(byte)guino_item_counter,*_variable);
    gSendCommand(guino_setMax,(byte)guino_item_counter,_max);
    gSendCommand(guino_setMin,(byte)guino_item_counter,_min);
    for (int i = 0; i < strlen(_name); i++){
      gSendCommand(guino_addChar,(byte)guino_item_counter,(int)_name[i]);
    }

    guino_item_counter++;

    return guino_item_counter-1;
  }
  return -1;

}

//This allows me add sliders to my dashboard
int gAddSlider(int _min,int _max, char * _name, int * _variable)
{
  if(guino_maxGUIItems > guino_item_counter)
  {
    guino_item_values[guino_item_counter] =_variable ;
    gGetSavedValue(guino_item_counter, _variable);
    gSendCommand(guino_addSlider,(byte)guino_item_counter,*_variable);
    gSendCommand(guino_setMax,(byte)guino_item_counter,_max);
    gSendCommand(guino_setMin,(byte)guino_item_counter,_min);
    for (int i = 0; i < strlen(_name); i++){
      gSendCommand(guino_addChar,(byte)guino_item_counter,(int)_name[i]);
    }

    guino_item_counter++;

    return guino_item_counter-1;
  }
  return -1;

}

//This allows me to update the value of an item
void gUpdateValue(int _item)
{

  gSendCommand(guino_setValue,_item, *guino_item_values[_item]); 
}

//This allows me to update the value by taking in a variable
//This calls the previous gupdateValue function
void gUpdateValue(int * _variable)
{

  int current_id = -1;
  for(int i = 0; i < guino_item_counter; i++)
  {

    if(guino_item_values[i] == _variable)
    {

      current_id = i;
      gUpdateValue(current_id);
    }
  }
  // if(current_id != -1)

}


//This sends a command to the guino software and is used by all other functions in this document
void gSendCommand(byte _cmd, byte _item, int _value)
{
  if(!internalInit && (guidino_initialized || guino_executed || _cmd == guino_iamhere)  )
  {
    guino_data.cmd = _cmd;
    guino_data.item = _item;
    guino_data.value = _value;
    ET.sendData();
  }

}

