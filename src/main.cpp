
#include <Arduino.h>
#include "radio.h"
#include "web_ui.h"

static TaskHandle_t  Core0TaskHnd ;
static void Core0Task(void *parameter);

inline void Core0TaskInit()
{
    xTaskCreatePinnedToCore(Core0Task, "CoreTask0", 8000, NULL, 999, &Core0TaskHnd, 0);
}

void setup()
{
    radio_setup();
    Core0TaskInit();
    web_ui_setup();
}

void loop()
{
    radio_loop();
    web_ui_loop();
}

static void Core0Task(void *parameter)
{
    dsp_task_setup();
    while (true)
    {
        dsp_task_loop();
        // delay(1);
        yield();
    }
}

