#include "contiki.h"
#include "shell.h"
#include "serial-shell.h"
#include "collect-view.h"
#include "dev/leds.h"

#include "dev/button-sensor.h"
/*---------------------------------------------------------------------------*/
PROCESS(collect_view_shell_process, "Contiki Collect View Shell");
PROCESS(blink_process, "Blink");
AUTOSTART_PROCESSES(&collect_view_shell_process,&blink_process);

/*---------------------------------------------------------------------------*/
/* Collect View procees declaration*/
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(collect_view_shell_process, ev, data)
{
   PROCESS_BEGIN();
   //SENSORS_ACTIVATE(button_sensor);
   //PROCESS_WAIT_EVENT_UNTIL(ev==sensors_event && data==&button_sensor);  
   serial_shell_init();
   shell_blink_init();

   shell_reboot_init();
   shell_rime_init();
   shell_rime_netcmd_init();

   shell_powertrace_init();

   shell_text_init();
   shell_time_init();


 #if CONTIKI_TARGET_SKY
   shell_sky_init();
 #endif 

   shell_collect_view_init();
   PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* Blink LED procees declaration*/
/*---------------------------------------------------------------------------*/
 PROCESS_THREAD(blink_process, ev, data)
{
 PROCESS_EXITHANDLER(goto exit;)
 PROCESS_BEGIN();
 
 while(1) 
 {
  static struct etimer et;
  etimer_set(&et, CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  leds_on(LEDS_ALL);
  etimer_set(&et, CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  leds_off(LEDS_ALL);
 }

 exit:
   leds_off(LEDS_ALL);
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
