#include <Arduino.h>

//this is just limiting the cpu to 1 core for demonstration purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Globals
static int shared_var = 0;
static SemaphoreHandle_t mutex;


static const int led_pin = LED_BUILTIN;


//*****************************************************************************
// Tasks




void blinkLed(void *parameter){//this task should toggle led at rate given by user

    //copy param into local var
    int led_delay = *(int *)parameter;

    Serial.print("Recieved: ");
    Serial.println(led_delay);

    pinMode(led_pin,OUTPUT);

    while (1)//blink
    {
        digitalWrite(led_pin,HIGH);
        vTaskDelay(led_delay/portTICK_PERIOD_MS);
        digitalWrite(led_pin,LOW);
        vTaskDelay(led_delay/portTICK_PERIOD_MS);
    }
    

}








//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  long int delay_arg;

  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("FreeRTOS Mutex program");
  Serial.println("Please enter a number to delay the led blink for (miliseconds)");


  mutex = xSemaphoreCreateMutex();//create a mutex and assign it to the handle we made earlier

  while (Serial.available () <= 0);

  //now serial should be available
  delay_arg = Serial.parseInt();//
  Serial.print("Sending: ");
  Serial.println(delay_arg);

  // Start task 1
  xTaskCreatePinnedToCore(blinkLed,
                          "blinkLed",
                          1024,
                          (void *)&delay_arg, //passing in a delay arg this points to the value read in by the serial terminal from the user(stack mem)
                          1,
                          NULL,
                          app_cpu);

  Serial.println("DONE!");


}

void loop() {
    //just yeild to other tasks
    vTaskDelay(1000/portTICK_PERIOD_MS);
}