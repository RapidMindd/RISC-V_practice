#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

extern void vSendString(const char * const pcString);

#define LAB_A               1
#define LAB_B               4
#define BASE_TICKS          200
#define MAX_LEDS_AVAILABLE  2

#define LED_COUNT  ((MAX_LEDS_AVAILABLE > (2 + ((LAB_A + 1) % 4))) ? \
                    MAX_LEDS_AVAILABLE : (2 + ((LAB_A + 1) % 4)))

#define LED0_PERIOD   (LAB_A * BASE_TICKS + LAB_B * BASE_TICKS * 0)
#define LED1_PERIOD   (LAB_A * BASE_TICKS + LAB_B * BASE_TICKS * 1)
#define LED2_PERIOD   (LAB_A * BASE_TICKS + LAB_B * BASE_TICKS * 2)
#define LED3_PERIOD   (LAB_A * BASE_TICKS + LAB_B * BASE_TICKS * 3)

#define LED0_HALF     (LED0_PERIOD / 2)
#define LED1_HALF     (LED1_PERIOD / 2)
#define LED2_HALF     (LED2_PERIOD / 2)
#define LED3_HALF     (LED3_PERIOD / 2)

#define MAX_ACTIVE_LEDS   (1 + (LAB_B % (LED_COUNT / 2)))

#define TASK_STACK_SIZE   512
#define TASK_PRIORITY     2

typedef struct
{
    int led_num;
    TickType_t half_period;
    const char *on_text;
    const char *off_text;
    const char *fail_text;
} LedTaskData;

static SemaphoreHandle_t xLedMutex = NULL;
static LedTaskData led_data[LED_COUNT];

static void led_mutex_task(void *pvParameters)
{
    LedTaskData *config = (LedTaskData *) pvParameters;

    for (;;)
    {

        if (xSemaphoreTake(xLedMutex, 0) == pdTRUE)
        {
            vSendString(config->on_text);


            vTaskDelay(config->half_period);

            vSendString(config->off_text);

            xSemaphoreGive(xLedMutex);
        }
        else
        {

            vSendString(config->fail_text);
        }


        vTaskDelay(config->half_period);
    }
}

int main_tasks(void)
{
    vSendString("FreeRTOS mutex lab started\r\n");
    vSendString("Variant: a=1, b=4, k=2, n=4\r\n");
    vSendString("Max active LEDs = 1\r\n");

    xLedMutex = xSemaphoreCreateMutex();

    if (xLedMutex == NULL)
    {
        vSendString("Mutex creation failed\r\n");
        for (;;)
        {
        }
    }

    led_data[0].led_num = 0;
    led_data[0].half_period = LED0_HALF;
    led_data[0].on_text = "LED0 ON\r\n";
    led_data[0].off_text = "LED0 OFF\r\n";
    led_data[0].fail_text = "LED0 SKIP\r\n";

    led_data[1].led_num = 1;
    led_data[1].half_period = LED1_HALF;
    led_data[1].on_text = "LED1 ON\r\n";
    led_data[1].off_text = "LED1 OFF\r\n";
    led_data[1].fail_text = "LED1 SKIP\r\n";

    led_data[2].led_num = 2;
    led_data[2].half_period = LED2_HALF;
    led_data[2].on_text = "LED2 ON\r\n";
    led_data[2].off_text = "LED2 OFF\r\n";
    led_data[2].fail_text = "LED2 SKIP\r\n";

    led_data[3].led_num = 3;
    led_data[3].half_period = LED3_HALF;
    led_data[3].on_text = "LED3 ON\r\n";
    led_data[3].off_text = "LED3 OFF\r\n";
    led_data[3].fail_text = "LED3 SKIP\r\n";

    xTaskCreate(led_mutex_task, "LED0_Task", TASK_STACK_SIZE, &led_data[0], TASK_PRIORITY, NULL);
    xTaskCreate(led_mutex_task, "LED1_Task", TASK_STACK_SIZE, &led_data[1], TASK_PRIORITY, NULL);
    xTaskCreate(led_mutex_task, "LED2_Task", TASK_STACK_SIZE, &led_data[2], TASK_PRIORITY, NULL);
    xTaskCreate(led_mutex_task, "LED3_Task", TASK_STACK_SIZE, &led_data[3], TASK_PRIORITY, NULL);

    vTaskStartScheduler();

    for (;;)
    {
    }
}

void main_blinky(void)
{
    main_tasks();
}
