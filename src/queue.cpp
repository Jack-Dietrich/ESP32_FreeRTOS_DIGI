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

//settings
static const uint8_t buf_len = 255;//so user can type in 255 chars max
static const uint8_t msg_queue_len = 5;
static const char command[] = "delay "; //this is so we can remove it to get the led delay when parsing thru given command.
static const uint8_t delay_queue_len = 5;
static const uint8_t blink_max = 100; //number of times we blink before we send a message

typedef struct { //this is our struct of the message
    char body[20];
    int count;
} Message;


//handles
static QueueHandle_t delay_queue; //handle to queue 1
static QueueHandle_t msg_queue; //handle to queue 2


//task A
void CLI(void *parameter){ //this function monitors for user input to change blinking rate
    
    Message rcv_msg; //using our struct to create a message
    char c; //character
    char buf[buf_len]; //create an array of char 255 char long
    uint8_t idx = 0; //start index at 0
    uint8_t cmd_len = strlen(command);//how long is the command.
    int led_delay;

    memset(buf,0,buf_len); //set everything in buffer to 0 (clear buffer)

    while (1)
    {

        //first we want to see if there is a message in the queue
    if(xQueueReceive(msg_queue, (void *)&rcv_msg, 0) == pdTRUE){//true if read from queue
        Serial.print(rcv_msg.body); //print out the body of the message
        Serial.println(rcv_msg.count); 
    }


    if(Serial.available() > 0){ //if serial is available
        c = Serial.read(); //get a char from serial
        
        if(idx < buf_len-1){//if we are not at the end of our buffer, copy in the character
            buf[idx] = c;
            idx++;
        }
        
        if ((c == '\n') || (c == '\r')) { //if we are  at the end of the string (newline or carrage return?)

            Serial.print("\r\n");

            //now we need to check if string if of the format delay xxx
            if(memcmp(buf,command,cmd_len) == 0) {//here we compare the buffer(what we read in) with the command
                
                //if they are the same, we may have a delay xxx comand.

                char * tail = buf - cmd_len; //makes a pointer to the start of the string that we want to read.
                led_delay = atoi(tail);
                led_delay = abs(led_delay); 


                if (xQueueSend(delay_queue, (void *)&led_delay, 10) != pdTRUE) {//just invoking the left half of this check will send it to queue, and if it returns false that is when the error handling happens.
                    Serial.println("ERROR: Could not put item on delay queue.");
                }


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

void blinkLED(void *parameter){//this task should print back the dynamically allocated message then free it.
    
    Message msg; //create a message
    int led_delay = 500; //default LED delay
    uint8_t counter = 0;

    pinMode(LED_BUILTIN,OUTPUT);
    
    while (true){//forever loop


    if(xQueueReceive(delay_queue, (void *)&led_delay, 0) == pdTRUE){//true if read from queue

        strcpy(msg.body, "Message received ");
        msg.count = 1;
        xQueueSend(msg_queue,(void *)&msg, 10);


    }


    //blink the led at the specified rate

    digitalWrite(led_pin,HIGH);
    vTaskDelay(led_delay/portTICK_PERIOD_MS);
    digitalWrite(led_pin,LOW);
    vTaskDelay(led_delay/portTICK_PERIOD_MS);

    counter++;

    if(counter >= blink_max){

        //create message to send
        strcpy(msg.body, "Blinked: ");
        msg.count = counter;
        xQueueSend(msg_queue,(void *)&msg,10);

        counter = 0;
    }

    

    }

}



// put function declarations here:

void setup() {

    Serial.begin(115200);



    //wait for a bit
    vTaskDelay(1000/ portTICK_PERIOD_MS); //delay for 1 second just to be safe

    //printing
    Serial.println("Queue Testing \n Enter the command 'delay xxx' where xxx is an integer to switch the blinking rate of the led\n LED blink time in miliseconds:");



    //create queue 1 and queue 2
    delay_queue = xQueueCreate(delay_queue_len,sizeof(int));//queue 1 is storing integers, so each I tem only needs to be 1 int size.
    msg_queue = xQueueCreate(msg_queue_len,sizeof(Message));//queue 2 is storing strings, with max len of 255



    xTaskCreatePinnedToCore(CLI,
        "CLI",
        1024,   //how big the stack should be 
        NULL,   //parameters
        1,      //priority
        NULL, //no need to create a handle as we are not controlling 
        app_cpu); //last param is core


    xTaskCreatePinnedToCore(blinkLED,
        "blinkLED",
        1024,   //how big the stack should be 
        NULL,   //parameters
        1,      //priority
        NULL,   //handle
        app_cpu); //last param is core

        vTaskDelete(NULL); //passing null causes the calling task to be deleted, so once we are finished setup, the only tasks will be the serial read and blinkled


    }


void loop() {

}

