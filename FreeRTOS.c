#include "FreeRTOS.h"
#include "task.h"


//закомментировать для первого варианта
#define USE_PARAMETERS_IN_TASKS

extern void vSendString(const char * const pcString);

#define LAB_A               2
#define LAB_B               9
#define BASE_TICKS          200
#define MAX_LEDS_AVAILABLE  5

#define LED_COUNT  ((MAX_LEDS_AVAILABLE > (2 + (LAB_A % 4))) ? \
                    MAX_LEDS_AVAILABLE : (2 + (LAB_A % 4)))

#define LED1_PERIOD   (LAB_A * BASE_TICKS)
#define LED2_PERIOD   (LED1_PERIOD + LAB_B * BASE_TICKS)
#define LED3_PERIOD   (LED2_PERIOD + LAB_B * BASE_TICKS)
#define LED4_PERIOD   (LED3_PERIOD + LAB_B * BASE_TICKS)
#define LED5_PERIOD   (LED4_PERIOD + LAB_B * BASE_TICKS)

#define LED1_HALF     (LED1_PERIOD / 2)
#define LED2_HALF     (LED2_PERIOD / 2)
#define LED3_HALF     (LED3_PERIOD / 2)
#define LED4_HALF     (LED4_PERIOD / 2)
#define LED5_HALF     (LED5_PERIOD / 2)

#define TASK_STACK_SIZE   512
#define TASK_PRIORITY     2

#ifndef USE_PARAMETERS_IN_TASKS

static void led_task_1(void *pvParameters)
{
    (void) pvParameters;

    for (;;)
    {
        vSendString("LED1 ON\r\n");
        vTaskDelay(LED1_HALF);

        vSendString("LED1 OFF\r\n");
        vTaskDelay(LED1_HALF);
    }
}

static void led_task_2(void *pvParameters)
{
    (void) pvParameters;

    for (;;)
    {
        vSendString("LED2 ON\r\n");
        vTaskDelay(LED2_HALF);

        vSendString("LED2 OFF\r\n");
        vTaskDelay(LED2_HALF);
    }
}

static void led_task_3(void *pvParameters)
{
    (void) pvParameters;

    for (;;)
    {
        vSendString("LED3 ON\r\n");
        vTaskDelay(LED3_HALF);

        vSendString("LED3 OFF\r\n");
        vTaskDelay(LED3_HALF);
    }
}

static void led_task_4(void *pvParameters)
{
    (void) pvParameters;

    for (;;)
    {
        vSendString("LED4 ON\r\n");
        vTaskDelay(LED4_HALF);

        vSendString("LED4 OFF\r\n");
        vTaskDelay(LED4_HALF);
    }
}

static void led_task_5(void *pvParameters)
{
    (void) pvParameters;

    for (;;)
    {
        vSendString("LED5 ON\r\n");
        vTaskDelay(LED5_HALF);

        vSendString("LED5 OFF\r\n");
        vTaskDelay(LED5_HALF);
    }
}

#else

typedef struct
{
    int led_num;
    TickType_t half_period;
    const char *on_text;
    const char *off_text;
} LedTaskData;

static LedTaskData led_data[LED_COUNT];

static void led_task_common(void *pvParameters)
{
    LedTaskData *config = (LedTaskData *) pvParameters;

    for (;;)
    {
        vSendString(config->on_text);
        vTaskDelay(config->half_period);

        vSendString(config->off_text);
        vTaskDelay(config->half_period);
    }
}

#endif

int main_tasks(void)
{
    vSendString("FreeRTOS LED lab started\r\n");
    vSendString("Variant: a=2, b=9, t=200, n=5\r\n");

#ifndef USE_PARAMETERS_IN_TASKS

    xTaskCreate(led_task_1, "LED1_Task", TASK_STACK_SIZE, NULL, TASK_PRIORITY, NULL);
    xTaskCreate(led_task_2, "LED2_Task", TASK_STACK_SIZE, NULL, TASK_PRIORITY, NULL);
    xTaskCreate(led_task_3, "LED3_Task", TASK_STACK_SIZE, NULL, TASK_PRIORITY, NULL);
    xTaskCreate(led_task_4, "LED4_Task", TASK_STACK_SIZE, NULL, TASK_PRIORITY, NULL);
    xTaskCreate(led_task_5, "LED5_Task", TASK_STACK_SIZE, NULL, TASK_PRIORITY, NULL);

#else

    led_data[0].led_num = 1;
    led_data[0].half_period = LED1_HALF;
    led_data[0].on_text = "LED1 ON\r\n";
    led_data[0].off_text = "LED1 OFF\r\n";

    led_data[1].led_num = 2;
    led_data[1].half_period = LED2_HALF;
    led_data[1].on_text = "LED2 ON\r\n";
    led_data[1].off_text = "LED2 OFF\r\n";

    led_data[2].led_num = 3;
    led_data[2].half_period = LED3_HALF;
    led_data[2].on_text = "LED3 ON\r\n";
    led_data[2].off_text = "LED3 OFF\r\n";

    led_data[3].led_num = 4;
    led_data[3].half_period = LED4_HALF;
    led_data[3].on_text = "LED4 ON\r\n";
    led_data[3].off_text = "LED4 OFF\r\n";

    led_data[4].led_num = 5;
    led_data[4].half_period = LED5_HALF;
    led_data[4].on_text = "LED5 ON\r\n";
    led_data[4].off_text = "LED5 OFF\r\n";

    xTaskCreate(led_task_common, "LED1_Task", TASK_STACK_SIZE, &led_data[0], TASK_PRIORITY, NULL);
    xTaskCreate(led_task_common, "LED2_Task", TASK_STACK_SIZE, &led_data[1], TASK_PRIORITY, NULL);
    xTaskCreate(led_task_common, "LED3_Task", TASK_STACK_SIZE, &led_data[2], TASK_PRIORITY, NULL);
    xTaskCreate(led_task_common, "LED4_Task", TASK_STACK_SIZE, &led_data[3], TASK_PRIORITY, NULL);
    xTaskCreate(led_task_common, "LED5_Task", TASK_STACK_SIZE, &led_data[4], TASK_PRIORITY, NULL);

#endif

    vTaskStartScheduler();

    for (;;)
    {
    }
}

void main_blinky(void)
{
    main_tasks();
}
