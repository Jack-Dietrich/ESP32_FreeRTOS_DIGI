#include <Arduino.h>

/*
Using semaphores and mutex to control producer and consumer tasks. The end result prints 3 of each number of producer task, since there are 5, the output will bee 3 0's 3 1's up to 3 4's.
Not necicarrily in order due to scheduling (this is why we are using semaphores and mutex's in the first place)

*/



// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
enum {BUF_SIZE = 5};                  // Size of buffer array
static const int num_prod_tasks = 5;  // Number of producer tasks
static const int num_cons_tasks = 2;  // Number of consumer tasks
static const int num_writes = 3;      // Num times each producer writes to buf

// Globals
static int buf[BUF_SIZE];             // Shared buffer
static int head = 0;                  // Writing index to buffer
static int tail = 0;                  // Reading index to buffer
static SemaphoreHandle_t bin_sem;     // Waits for parameter to be read
static SemaphoreHandle_t emp_sem;     //Semaphore for counting empty slots in circular buffer
static SemaphoreHandle_t ful_sem;     //counting full slots in circular buffer
static SemaphoreHandle_t mutex;       //for accessing circular buffer


//*****************************************************************************
// Tasks

// Producer: write a given number of times to shared buffer
void producer(void *parameters) {

  // Copy the parameters into a local variable
  int num = *(int *)parameters;

  // Release the binary semaphore
  xSemaphoreGive(bin_sem);

  // Fill shared buffer with task number
  for (int i = 0; i < num_writes; i++) {

    // Critical section (accessing shared buffer)
    
    xSemaphoreTake(emp_sem,portMAX_DELAY); //remove from empty sem since we are adding something. This waits for empty slot

    xSemaphoreTake(mutex,portMAX_DELAY);
    buf[head] = num;
    head = (head + 1) % BUF_SIZE;
    xSemaphoreGive(mutex);

    //also need to adjust the semaphore counts of full and empty 
    xSemaphoreGive(ful_sem); //add to the full sem
  }

  // Delete self task
  vTaskDelete(NULL);
}

// Consumer: continuously read from shared buffer
void consumer(void *parameters) {

  int val;

  // Read from buffer
  while (1) {


    xSemaphoreTake(ful_sem,portMAX_DELAY);//wait for at least 1 to be in full slot


    // Critical section (accessing shared buffer and Serial)
    xSemaphoreTake(mutex,portMAX_DELAY);
    
    val = buf[tail];
    tail = (tail + 1) % BUF_SIZE;
    Serial.println(val);

    xSemaphoreGive(mutex);//give mutex back once outputted to serial



    xSemaphoreGive(emp_sem);//add to empty as we read/consumed 1
  }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  char task_name[12];
  
  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("FreeRTOS Semaphore ");

  // Create mutexes and semaphores before starting tasks
  bin_sem = xSemaphoreCreateBinary();
  ful_sem = xSemaphoreCreateCounting(5,0);//start at 0 because initially we have no full slots in circular queue
  emp_sem = xSemaphoreCreateCounting(5,5);//start at 5 because all are initlaly empty
  mutex = xSemaphoreCreateMutex();//for accessing serial to print


  // Start producer tasks (wait for each to read argument)
  for (int i = 0; i < num_prod_tasks; i++) {
    sprintf(task_name, "Producer %i", i);
    xTaskCreatePinnedToCore(producer,
                            task_name,
                            1024,
                            (void *)&i,
                            1,
                            NULL,
                            app_cpu);
    xSemaphoreTake(bin_sem, portMAX_DELAY);
  }

  // Start consumer tasks
  for (int i = 0; i < num_cons_tasks; i++) {
    sprintf(task_name, "Consumer %i", i);
    xTaskCreatePinnedToCore(consumer,
                            task_name,
                            1024,
                            NULL,
                            1,
                            NULL,
                            app_cpu);
  }

  // Notify that all tasks have been created
  Serial.println("All tasks created");
}

void loop() {
  
  // Do nothing but allow yielding to lower-priority tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}