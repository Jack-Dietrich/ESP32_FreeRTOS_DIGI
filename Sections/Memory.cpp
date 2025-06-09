#include <Arduino.h>
#include <stdlib.h> //for atoi

#if CONFIG_FREERTOS_UNICORE  // this is just limiting the cpu to 1 core for demonstration purposes
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif



//Pins
static const int led_pin = LED_BUILTIN;

//setting
static const uint8_t buf_len = 255;//so user can type in 255 chars max

//global variables
//pointer
static char *ptr = nullptr;
static volatile uint8_t msg_flag = 0;

void readSerial(void *parameter){ //this function monitors for user input to change blinking rate
    char c; //character
    char buf[buf_len]; //create an array of char to store our message
    uint8_t idx = 0; //start index at 0

    memset(buf,0,buf_len); //set everything in buffer to 0

    while (1)
    {
    if(Serial.available() > 0){ //if serial is available
        c = Serial.read(); //get a char from serial
        
        if(idx < buf_len-1){//if we are not at the end of our buffer, copy in the character
            buf[idx] = c;
            idx++;
        }
        
        if (c == '\n') { //if we are  at the end of the string

            buf[idx-1] = '\0'; //make the string null terminated

            if(msg_flag == 0 ){
                ptr = (char *)pvPortMalloc(idx * sizeof(char));//if we have not created a message yet, dynamically allocate the memory for it.

                configASSERT(ptr); //make sure the pointer is not empy, this would mean we have run out of memory

                memcpy(ptr,buf,idx);//copy into dynamic memory our buffer

                msg_flag = 1; //notify other task that we are ready
            }

                //we are here if the message flag is 1
                memset(buf, 0, buf_len); //set the buffer back to 0
                idx = 0; //reset index

            } 
            

        }
        else{
           
        }
    }

}

void PrintMSG(void *parameter){//this task should print back the dynamically allocated message then free it.

    while(1){
        if(msg_flag == 1){
            Serial.println(ptr);


            vPortFree(ptr);//free the message pointer
            ptr = nullptr;
            msg_flag = 0;
        }
    }

}



// put function declarations here:

void setup() {

    Serial.begin(115200);



    //wait for a bit
    vTaskDelay(1000/ portTICK_PERIOD_MS); //delay for 1 second just to be safe



    xTaskCreatePinnedToCore(PrintMSG,
        "Print MSG",
        1024,   //how big the stack should be 
        NULL,   //parameters
        1,      //priority
        NULL, //no need to create a handle as we are not controlling 
        app_cpu); //last param is core


    xTaskCreatePinnedToCore(readSerial,
        "readSerial",
        1024,   //how big the stack should be 
        NULL,   //parameters
        1,      //priority
        NULL,   //handle
        app_cpu); //last param is core

        vTaskDelete(NULL); //passing null causes the calling task to be deleted, so once we are finished setup, the only tasks will be the serial read and blinkled


    }


void loop() {

}

