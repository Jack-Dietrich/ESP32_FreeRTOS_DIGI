#include <Arduino.h>

/*
You can type in the terminal to turn on the led, once the timer expires due to inactivity, the led will turn off.

*/

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Globals
static TimerHandle_t one_shot_timer = NULL;
static TimerHandle_t auto_reload_timer = NULL;
static const int led_pin = LED_BUILTIN;

//Settings

static const uint8_t buf_len = 255;//so user can type in 255 chars max


//*****************************************************************************
// Callbacks

// Called when one of the timers expires
void myTimerCallback(TimerHandle_t xTimer) {

  // Print message if timer 0 expired
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 0) {
    //turn led off
    digitalWrite(led_pin,LOW);
  }

}

//** TASKS

void readSerial(void *parameter){ //this function monitors for user input to change blinking rate
    char c; //character

    while (1)
    {
    if(Serial.available() > 0){ //if serial is available
        c = Serial.read(); //get a char from serial
        Serial.print(c); //echo back character
        //turn on led since we are typing
        digitalWrite(led_pin,HIGH);
        xTimerStart(one_shot_timer, portMAX_DELAY);
        }

    }

}




//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  // Configure Serial
  Serial.begin(115200);

  pinMode(led_pin,OUTPUT);//set led as output


  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Timer Demo---");

  // Create a one-shot timer
  one_shot_timer = xTimerCreate(
                      "One-shot timer",           // Name of timer
                      5000 / portTICK_PERIOD_MS,  // Period of timer (in ticks). In this case its 5 seconds
                      pdFALSE,                    // Auto-reload
                      (void *)0,                  // Timer ID
                      myTimerCallback);           // Callback function





     xTaskCreatePinnedToCore(readSerial,
        "readSerial",
        1024,   //how big the stack should be 
        NULL,   //parameters
        1,      //priority
        NULL, //no need to create a handle as we are not controlling 
        app_cpu); //last param is core
        
        
  // Create an auto-reload timer


  // Check to make sure timers were created
  if (one_shot_timer == NULL) {
    Serial.println("Could not create one of the timers");
  } else {
    
    // Wait and then print out a message that we're starting the timers
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println("Starting timers...");

    // Start timers (max block time if command queue is full)
  }

  // Delete self task to show that timers will work with no user tasks
  vTaskDelete(NULL);
}


void loop() {
  // Execution should never get here
}