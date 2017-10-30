//
// Created by enric on 27/10/2017.
//
#include "stdio.h"
#include "contiki.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "constants.h"
#include "net/rime/rime.h"
#include "addresses.h"
#include "dev/sht11/sht11-sensor.h"


static struct etimer alarmTimer;
static struct etimer temperatureTimer;

static unsigned char alarm = 0;
static unsigned char ledStatus = 0;

static float temperatures[5];
static int lastTemperatureIndex = 0;

static process_event_t alarm_toggled_event;

void command_switch(int);

PROCESS(door_node_main, "Door Node Main Process");


static void recv_runicast(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno)
{
    int receivedCommand = *((int*)packetbuf_dataptr());
    printf("runicast message received from %d.%d, seqno %d, message: %d\n", from->u8[0], from->u8[1], seqno, receivedCommand);

    command_switch(receivedCommand);
}

static void sent_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){}


static void timedout_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions){}

static const struct runicast_callbacks runicast_calls = {recv_runicast, sent_runicast, timedout_runicast};
static struct runicast_conn runicast;

void command_switch(int command)
{
    switch(command)
    {
        case ALARM_TOGGLE_COMMAND: //Alarm toggle
            //TODO:alarm toggle
            process_post(&door_node_main, alarm_toggled_event, NULL);
            break;
        case GATELOCK_TOGGLE_COMMAND: //Gate lock toggle
            //TODO:gate lock toggle
            printf("Gate Lock Toggled\n");
            break;
        case DOORS_OPEN_COMMAND: //Open doors
            //TODO:open doors
            printf("Doors opened\n");
            break;
        case AVERAGE_TEMPERATURE_COMMAND: //Average temp
            //TODO:average temp
            printf("Average temp\n");
            break;
        case LIGHT_VALUE_COMMAND: //Light value
            //TODO: light value
            printf("Light Value\n");
            break;
        default: //error, no command
            printf("There is no command with id %d\n", command);
            break;
    }
}

void toggleAlarm()
{
    printf("Alarm Toggled\n");
    if(alarm == 0)
    {
        alarm = 1;
        ledStatus = leds_get();
        leds_on(LEDS_ALL);
        etimer_set(&alarmTimer, ALARM_LED_PERIOD*CLOCK_SECOND);
    }
    else
    {
        alarm = 0;
        leds_set(ledStatus);
        etimer_stop(&alarmTimer);
    }
}

AUTOSTART_PROCESSES(&door_node_main);

PROCESS_THREAD(door_node_main, ev, data)
{
    PROCESS_BEGIN();

                setNodesAddresses();

                SENSORS_ACTIVATE(sht11_sensor);

                etimer_set(&temperatureTimer, TEMPERATURE_MEASURE_PERIOD*CLOCK_SECOND);

                printf("My address is %d.%d\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);

                runicast_open(&runicast, 144, &runicast_calls);

                while(1) {

                    //printf("I wait for event\n");

                    PROCESS_WAIT_EVENT();

                    //printf("Event arrived\n");

                    if (ev == sensors_event && data == &button_sensor) {

                    } else if (ev == PROCESS_EVENT_TIMER) {
                        if (alarm && etimer_expired(&alarmTimer)) {
                            leds_toggle(LEDS_ALL);
                            etimer_reset(&alarmTimer);
                        }
                        if (etimer_expired(&temperatureTimer))
                        {
                            int measuredTemperature = sht11_sensor.value(SHT11_SENSOR_TEMP);
                            printf("Temp measured %d\n",measuredTemperature);
                            etimer_reset(&temperatureTimer);
                        }
                    } else if (ev == alarm_toggled_event)
                    {
                        toggleAlarm();
                    }
                }

    PROCESS_END();
}