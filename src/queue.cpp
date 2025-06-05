#include <Arduino.h>
#include <stdlib.h> //for atoi

//should not need stdlib anymore

#if CONFIG_FREERTOS_UNICORE  // this is just limiting the cpu to 1 core for demonstration purposes
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif



//Pins
static const int led_pin = LED_BUILTIN;

//setting
static const uint8_t buf_len = 255;//so user can type in 255 chars max
static const uint8_t msg_queue_len = 5;

//global variables
//pointer


//handles
static QueueHandle_t queue1; //handle to queue 1
static QueueHandle_t queue2; //handle to queue 2


//task A
void TaskA(void *parameter){ //this function monitors for user input to change blinking rate
    char c; //character
    char buf[buf_len]; //create an array of char 255 char long
    uint8_t idx = 0; //start index at 0
    char msg[buf_len];//this will store the message from queue 2
    int cmd_len = 6;
    int led_delay;

    memset(buf,0,buf_len); //set everything in buffer to 0

    while (1)
    {

        //first we want to see if there is a message in the queue
    if(xQueueReceive(queue2, (void *)&msg, 0) == pdTRUE){//true if read from queue
        Serial.print(msg); //print out the item we read in
    }


    if(Serial.available() > 0){ //if serial is available
        c = Serial.read(); //get a char from serial
        
        if(idx < buf_len-1){//if we are not at the end of our buffer, copy in the character
            buf[idx] = c;
            idx++;
        }
        
        if (c == '\n' || (c == '\r')) { //if we are  at the end of the string (newline or carrage return?)

            buf[idx-1] = '\0'; //make the string null terminated

            Serial.print("\r\n");

            //now we need to check if string if of the format delay xxx
            if(strstr(msg, "delay " ) != NULL){
                //if the message we input has delay followed by a space we want to read the number that comes after(this is prone to error if someone were to enter letters)
                //also need to convert to integer
                char * tail = buf - cmd_len;  //possible source of error
                led_delay = atoi(tail);
                led_delay = abs(led_delay);

            }


            if (xQueueSend(queue2, (void *)&led_delay, 10) != pdTRUE) {//just invoking the left half of this check will send it to queue, and if it returns false that is when the error handling happens.
            Serial.println("ERROR: Could not put item on delay queue.");
            }
               
            memset(buf, 0, buf_len); //set the buffer back to 0
            idx = 0; //reset index
            } 
            
            else{
                Serial.print(c);//if we are not at end of line, just echo back what was written
            }

        }
    }

}

void TaskB(void *parameter){//this task should print back the dynamically allocated message then free it.
    
    int item; //for storing the item retrived from the queue
    int time; //time to blink led for
    int blinkCount = 0;
    
    while (true){
    if(blinkCount == 100){
        xQueueSend(queue2,"Blinked",0);//sending Blinked to Queue2
    }

    if(xQueueReceive(queue2, (void *)&item, 0) == pdTRUE){//true if read from queue
        Serial.print(item); //print out the item we read in
    }

    time = item;

    //blink the led at the specified rate

    digitalWrite(led_pin,HIGH);
    vTaskDelay(time/portTICK_PERIOD_MS);
    digitalWrite(led_pin,LOW);
    vTaskDelay(time/portTICK_PERIOD_MS);



    blinkCount++;
    }

}



// put function declarations here:

void setup() {

    Serial.begin(115200);



    //wait for a bit
    vTaskDelay(1000/ portTICK_PERIOD_MS); //delay for 1 second just to be safe


    //create queue 1 and queue 2
    queue1 = xQueueCreate(msg_queue_len,sizeof(int));//queue 1 is storing integers, so each I tem only needs to be 1 int size.
    queue2 = xQueueCreate(msg_queue_len,sizeof(char)*buf_len);//queue 2 is storing strings, with max len of 255



    xTaskCreatePinnedToCore(TaskA,
        "TaskA",
        1024,   //how big the stack should be 
        NULL,   //parameters
        1,      //priority
        NULL, //no need to create a handle as we are not controlling 
        app_cpu); //last param is core


    xTaskCreatePinnedToCore(TaskB,
        "TaskB",
        1024,   //how big the stack should be 
        NULL,   //parameters
        1,      //priority
        NULL,   //handle
        app_cpu); //last param is core

        vTaskDelete(NULL); //passing null causes the calling task to be deleted, so once we are finished setup, the only tasks will be the serial read and blinkled


    }


void loop() {

}

