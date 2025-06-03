#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE  // this is just limiting the cpu to 1 core for demonstration purposes
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

//a string to print
const char msg[] = "yo ho yo ho A pirate's life for me";

//task handles
static TaskHandle_t task_1 = NULL; //this just initializes the task handles to null, they are set later by xTaskCreatePinnedToCore
static TaskHandle_t task_2 = NULL;

//The task

void startTask1(void *parameter){
    int msg_len = strlen(msg);

    while(1){
        Serial.println();
        for (int i = 0 ; i < msg_len; i++){

            Serial.print(msg[i]);
        }
        Serial.println();
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void startTask2(void *parameter){
    while(1){
        Serial.print('*');
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}


// put function declarations here:

void setup() {

    Serial.begin(300);

    //wait for a bit
    vTaskDelay(1000/ portTICK_PERIOD_MS);
    Serial.println();
    Serial.println("FreeRTOS task demo");

    Serial.print("setup and loop task running on core: ");
    Serial.print(xPortGetCoreID());
    Serial.print(" With Priority ");
    Serial.println(uxTaskPriorityGet(NULL));


    xTaskCreatePinnedToCore(startTask1,
        "Task 1",
        1024,   //how big the stack should be 
        NULL,   //parameters
        1,      //priority
        &task_1, //this is passing in the task handle we made ealier
        app_cpu); //last param is core


    xTaskCreatePinnedToCore(startTask2,
        "Task 2",
        1024,   //how big the stack should be 
        NULL,   //parameters
        2,      //priority
        &task_2, //this is passing in the task handle we made ealier
        app_cpu); //last param is core
}


void loop() {
  // put your main code here, to run repeatedly:

  for(int i = 0; i < 3; i++){
    vTaskSuspend(task_2); 
    vTaskDelay(2000/portTICK_PERIOD_MS);
    vTaskResume(task_2);
    vTaskDelay(2000/portTICK_PERIOD_MS);
  }

  //after suspending it 3 times, we delete the task

  if(task_1 != NULL){
    vTaskDelete(task_1);
    task_1=NULL;
  }
}

