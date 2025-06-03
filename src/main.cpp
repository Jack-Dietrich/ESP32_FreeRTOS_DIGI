#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE  // this is just limiting the cpu to 1 core for demonstration purposes
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif


//Pins 
static const int led_pin = LED_BUILTIN;

//The task

void toggleLED(void *param){

    while (1)
    {
        digitalWrite(led_pin,HIGH);
        vTaskDelay(500/portTICK_PERIOD_MS);
        digitalWrite(led_pin,LOW);
        vTaskDelay(500/portTICK_PERIOD_MS);

    }
    

}


//challenge task
void toggleLED2(void *param){

    while (1)
    {
        digitalWrite(led_pin,HIGH);
        vTaskDelay(200/portTICK_PERIOD_MS);
        digitalWrite(led_pin,LOW);
        vTaskDelay(200/portTICK_PERIOD_MS);

    }
    

}


// put function declarations here:

void setup() {
    pinMode(led_pin,OUTPUT);


    xTaskCreatePinnedToCore(
        toggleLED,
        "Toggle LED",
        1024,
        NULL, 
        1,
        NULL,
        app_cpu);

    xTaskCreatePinnedToCore(
        toggleLED2,
        "Toggle LED2",
        1024,
        NULL, 
        1,
        NULL,
        app_cpu);        

}



void loop() {
  // put your main code here, to run repeatedly:
}

