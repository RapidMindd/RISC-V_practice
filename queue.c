#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdint.h>

extern void vSendString(const char * const pcString);


#define LAB_A 1
#define LAB_B 4

#define SEQUENCE_LEN   (5 + (LAB_A % 6))
#define FIRST_VALUE    (LAB_A)
#define SEQ_COUNT      11

#define TASK_STACK_SIZE  768
#define TASK_PRIORITY    2
#define QUEUE_LENGTH     16

typedef struct
{
    uint8_t seq_id;
    uint8_t index;
    uint8_t value;
} SequenceItem;


static QueueHandle_t xQueues[SEQUENCE_LEN - 1];
static SemaphoreHandle_t xPrintMutex = NULL;


static void append_char(char *buf, int *pos, char c)
{
    if (*pos < 95)
    {
        buf[*pos] = c;
        (*pos)++;
        buf[*pos] = '\0';
    }
}

static void append_str(char *buf, int *pos, const char *str)
{
    while (*str != '\0' && *pos < 95)
    {
        buf[*pos] = *str;
        (*pos)++;
        str++;
    }
    buf[*pos] = '\0';
}

static void append_uint(char *buf, int *pos, unsigned value)
{
    char tmp[10];
    int i = 0;
    int j;

    if (value == 0)
    {
        append_char(buf, pos, '0');
        return;
    }

    while (value > 0 && i < 10)
    {
        tmp[i] = (char)('0' + (value % 10));
        value /= 10;
        i++;
    }

    for (j = i - 1; j >= 0; j--)
    {
        append_char(buf, pos, tmp[j]);
    }
}

static void send_line(const char *text)
{
    if (xPrintMutex != NULL)
    {
        xSemaphoreTake(xPrintMutex, portMAX_DELAY);
        vSendString(text);
        xSemaphoreGive(xPrintMutex);
    }
    else
    {
        vSendString(text);
    }
}

static void print_init(uint8_t seq_id, uint8_t f0, uint8_t f1)
{
    char buf[96];
    int pos = 0;

    buf[0] = '\0';

    append_str(buf, &pos, "Init: s=");
    append_uint(buf, &pos, seq_id);
    append_str(buf, &pos, " f[0]=");
    append_uint(buf, &pos, f0);
    append_str(buf, &pos, " f[1]=");
    append_uint(buf, &pos, f1);
    append_str(buf, &pos, "\r\n");

    send_line(buf);
}

static void print_generated(uint8_t seq_id, uint8_t index, uint8_t value)
{
    char buf[96];
    int pos = 0;

    buf[0] = '\0';

    append_str(buf, &pos, "Gen: s=");
    append_uint(buf, &pos, seq_id);
    append_str(buf, &pos, " f[");
    append_uint(buf, &pos, index);
    append_str(buf, &pos, "]=");
    append_uint(buf, &pos, value);
    append_str(buf, &pos, "\r\n");

    send_line(buf);
}

static void print_result(uint8_t seq_id, uint8_t ks8)
{
    char buf[96];
    int pos = 0;

    buf[0] = '\0';

    append_str(buf, &pos, "Result: s=");
    append_uint(buf, &pos, seq_id);
    append_str(buf, &pos, " KS8=");
    append_uint(buf, &pos, ks8);
    append_str(buf, &pos, "\r\n");

    send_line(buf);
}

static void queue_init_task(void *pvParameters)
{
    uint8_t s;

    (void) pvParameters;

    for (;;)
    {
        for (s = 0; s < SEQ_COUNT; s++)
        {
            SequenceItem item0;
            SequenceItem item1;
            uint8_t f0 = (uint8_t) FIRST_VALUE;
            uint8_t f1 = (uint8_t) (LAB_A + LAB_B + s);

            item0.seq_id = s;
            item0.index = 0;
            item0.value = f0;

            item1.seq_id = s;
            item1.index = 1;
            item1.value = f1;

            xQueueSendToBack(xQueues[0], &item0, portMAX_DELAY);
            xQueueSendToBack(xQueues[0], &item1, portMAX_DELAY);

            print_init(s, f0, f1);


            vTaskDelay(200);
        }

        send_line("----- cycle restart -----\r\n");
        vTaskDelay(1000);
    }
}

static void queue_forward_task(void *pvParameters)
{
    int stage = (int) (uintptr_t) pvParameters;

    for (;;)
    {
        int count;
        uint8_t prev2 = 0;
        uint8_t prev1 = 0;
        uint8_t seq_id = 0;


        for (count = 0; count < stage + 2; count++)
        {
            SequenceItem item;

            xQueueReceive(xQueues[stage], &item, portMAX_DELAY);
            xQueueSendToBack(xQueues[stage + 1], &item, portMAX_DELAY);

            seq_id = item.seq_id;
            prev2 = prev1;
            prev1 = item.value;
        }


        {
            SequenceItem next_item;
            uint8_t next_value = (uint8_t) ((prev1 + prev2) & 0xFF);

            next_item.seq_id = seq_id;
            next_item.index = (uint8_t) (stage + 2);
            next_item.value = next_value;

            print_generated(seq_id, next_item.index, next_value);
            xQueueSendToBack(xQueues[stage + 1], &next_item, portMAX_DELAY);
        }
    }
}

static void queue_result_task(void *pvParameters)
{
    (void) pvParameters;

    for (;;)
    {
        int count;
        uint16_t sum = 0;
        uint8_t seq_id = 0;

        for (count = 0; count < SEQUENCE_LEN; count++)
        {
            SequenceItem item;

            xQueueReceive(xQueues[SEQUENCE_LEN - 2], &item, portMAX_DELAY);

            seq_id = item.seq_id;
            sum = (uint16_t) (sum + item.value);
        }

        {
            uint8_t ks8 = (uint8_t) (((uint8_t) sum) ^ 0xFF);
            print_result(seq_id, ks8);
        }
    }
}

int main_tasks(void)
{
    int i;

    xPrintMutex = xSemaphoreCreateMutex();
    if (xPrintMutex == NULL)
    {
        for (;;)
        {
        }
    }

    send_line("FreeRTOS queue lab started\r\n");

    for (i = 0; i < SEQUENCE_LEN - 1; i++)
    {
        xQueues[i] = xQueueCreate(QUEUE_LENGTH, sizeof(SequenceItem));
        if (xQueues[i] == NULL)
        {
            send_line("Queue creation failed\r\n");
            for (;;)
            {
            }
        }
    }

    xTaskCreate(queue_init_task,
                "Init_Task",
                TASK_STACK_SIZE,
                NULL,
                TASK_PRIORITY,
                NULL);

    for (i = 0; i < SEQUENCE_LEN - 2; i++)
    {
        xTaskCreate(queue_forward_task,
                    "Forward_Task",
                    TASK_STACK_SIZE,
                    (void *) (uintptr_t) i,
                    TASK_PRIORITY,
                    NULL);
    }

    xTaskCreate(queue_result_task,
                "Result_Task",
                TASK_STACK_SIZE,
                NULL,
                TASK_PRIORITY,
                NULL);

    vTaskStartScheduler();

    for (;;)
    {
    }
}

void main_blinky(void)
{
    main_tasks();
}
