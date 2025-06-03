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
static const uint8_t buf_len = 20;

//global variables
static int led_delay = 500; //default for led if no other value is set

void readSerial(void *parameter){ //this function monitors for user input to change blinking rate
    char c; //character
    char buf[buf_len]; //create an array of char 20 chars long
    uint8_t idx = 0; //start index at 0

    memset(buf,0,buf_len); //set everything in buffer to 0

    while (1)
    {
    if(Serial.available() > 0){ //if serial is available
        c = Serial.read(); //get a char from serial
        if (c == '\n') { //if we are  at the end of the string

            led_delay = atoi(buf); //convert the string we have created to an integer
            Serial.print("Updated LED delay to: ");
            Serial.println(led_delay);
            memset(buf, 0, buf_len); //set the buffer back to 0
            idx = 0; //reset index
            } else {

            // Only append if index is not over message limit
                if (idx < buf_len - 1) { //if we are not yet at the end of the string, append the integer
                    buf[idx] = c;
                    idx++; //may not be right
                }
            }
        }
    }

}

void blinkLed(void *parameter){//this task should toggle led at rate given by user

    while (1)
    {
        digitalWrite(led_pin,HIGH);
        vTaskDelay(led_delay/portTICK_PERIOD_MS);
        digitalWrite(led_pin,LOW);
        vTaskDelay(led_delay/portTICK_PERIOD_MS);
    }
    

}


// put function declarations here:

void setup() {

    Serial.begin(115200);

    pinMode(led_pin,OUTPUT);

    //wait for a bit
    vTaskDelay(1000/ portTICK_PERIOD_MS); //delay for 1 second just to be safe

    Serial.println("Multi-task LED Demo");
    Serial.println("Enter a number in milliseconds to change the LED delay.");


    xTaskCreatePinnedToCore(blinkLed,
        "blinkLed",
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

