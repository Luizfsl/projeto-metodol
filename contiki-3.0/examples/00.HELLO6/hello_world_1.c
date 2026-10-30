#include <stdio.h>
#include "contiki.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"

#include "lib/random.h"
#include "net/rime/rime.h"
#include "net/rime/collect.h"
#include "net/netstack.h"

/*----------------------------------------------------------------------------------*/
static struct collect_conn tc;
static void recv(const linkaddr_t *originator, uint8_t seqno, uint8_t hops)
{
  if(hops>0){
  	printf("1 recebe '%s' de  %d (HOPS %d)\n",
         (char *)packetbuf_dataptr(),
         originator->u8[0],
         hops);
  }
}

/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/


PROCESS(button_process, "Test Button");
AUTOSTART_PROCESSES(&button_process);

/*--------------------------COLLECT VIEW-------------------------------------*/
#include "shell.h"
#include "serial-shell.h"
#include "collect-view.h"

static void get_info(void)
{
  //--INICIALIZAR DADOS COLLECTview
  serial_shell_init();
  shell_blink_init();
  shell_reboot_init();
  shell_rime_init();
  shell_rime_netcmd_init();
  shell_powertrace_init();
  shell_text_init();
  shell_time_init();
  shell_rime_ping_init();
  //shell_sendtest_init();
  //NAO COMPATIVEL COM TMOTESKY
  //shell_download_init();
  //shell_rime_sendcmd_init();
  //shell_ps_init();
  //shell_rime_debug_init();
  //shell_rime_debug_runicast_init();
  //shell_base64_init();

#if CONTIKI_TARGET_SKY
  shell_sky_init();
#endif /* CONTIKI_TARGET_SKY */
    
 //--ATIVAR COLLECTview
 shell_collect_view_init();
}

static uint8_t flag;
PROCESS_THREAD(button_process,ev,data)
{

  static struct etimer periodic;
  static struct etimer et;

	PROCESS_BEGIN();
	flag=0;
	SENSORS_ACTIVATE(button_sensor);

	collect_open(&tc, 130, COLLECT_ROUTER, &callbacks);

	  if(linkaddr_node_addr.u8[0] == 1 &&
	     linkaddr_node_addr.u8[1] == 0) {
	    printf("I am sink\n");
	    collect_set_sink(&tc, 1);
	  }

	  /* Allow some time for the network to settle. */
	  etimer_set(&et, 120 * CLOCK_SECOND);
	  PROCESS_WAIT_UNTIL(etimer_expired(&et));

	
	while(1)
	{
		PROCESS_WAIT_UNTIL(etimer_expired(&et));
		leds_toggle(LEDS_BLUE);

		if(!flag)
		{
			leds_off(LEDS_BLUE);
			printf("LED BLUE ON %d\n",leds_get());
			printf("COLLECT VIEW (DESATIVADO)\n");

			/* Send a packet every 30 seconds. */
			    etimer_set(&periodic, CLOCK_SECOND * 30);
			    etimer_set(&et, random_rand() % (CLOCK_SECOND * 30));

			    PROCESS_WAIT_UNTIL(etimer_expired(&et));
			    {
			      static linkaddr_t oldparent;
			      const linkaddr_t *parent;

			      printf("Sending\n");
			      packetbuf_clear();
			      packetbuf_set_datalen(sprintf(packetbuf_dataptr(),
						            "%s", "Hello") + 1);
			      collect_send(&tc, 15);

			      parent = collect_parent(&tc);
			      if(!linkaddr_cmp(parent, &oldparent)) {
				if(!linkaddr_cmp(&oldparent, &linkaddr_null)) {
				  printf("#L %d 0\n", oldparent.u8[0]);
				}
				if(!linkaddr_cmp(parent, &linkaddr_null)) {
				  printf("#L %d 1\n", parent->u8[0]);
				}
				linkaddr_copy(&oldparent, parent);
			      }
			    }
}
		
		else 
		{
			printf("LED BLUE OFF %d\n",leds_get());
			leds_on(LEDS_BLUE);
			printf("COLLECT VIEW (ATIVO)\n");
			get_info();
		}

		PROCESS_WAIT_EVENT_UNTIL(ev==sensors_event && data==&button_sensor);	{	
			flag ^= 1;
		}
	}
	
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
