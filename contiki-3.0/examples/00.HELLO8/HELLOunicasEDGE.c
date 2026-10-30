#include "contiki.h"
#include "net/rime/rime.h"
#include <stdio.h>

#define DESTINO 3

/********************libSENSORES*********************/
#include "dev/sht11/sht11-sensor.h"
#include "dev/battery-sensor.h"
#include "dev/light-sensor.h"


static char* get_light(void)
{
  return (10 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) / 7);

}


/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
	printf("EU sou NO edge");
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{

  PROCESS_EXITHANDLER(unicast_close(&uc);)
    
  PROCESS_BEGIN();

  //--LIGA SENSORES
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(battery_sensor);
  SENSORS_ACTIVATE(sht11_sensor);

  unicast_open(&uc, 146, &unicast_callbacks);

  while(1) {
    static struct etimer et;
    linkaddr_t addr;
    
    etimer_set(&et, CLOCK_SECOND);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    packetbuf_copyfrom(get_light(), 10);

    addr.u8[0] = DESTINO;
    addr.u8[1] = 0;
    if(!linkaddr_cmp(&addr, &linkaddr_node_addr)) {
      unicast_send(&uc, &addr);
    }

  }

  //--LIGA SENSORES
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(battery_sensor);
  SENSORS_ACTIVATE(sht11_sensor);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
