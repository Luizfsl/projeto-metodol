/*---------------------------------------------------------------------------*/
/*-------------------------------LIBS----------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lib/random.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//transmission
#include "net/rime/rime.h"
#include "net/rime/collect.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "net/netstack.h"

//battery
#include "powertrace.h"
#include "dev/battery-sensor.h"

//SENSORES
#include "dev/sht11/sht11-sensor.h"
#include "dev/battery-sensor.h"
#include "dev/light-sensor.h"

//VARIAVEIS DO TMOTE SKY
#define _Nb 90 //tamanho do pacote
static uint8_t _Dist = 245;

//DADOS DE TRABNSMISSAO
static struct collect_conn tc;

/*--------------------------SENSORIAMENTO------------------------------------*/
//light-sensor
static int get_light(void)
{
  return (10 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) / 7);
}

static int get_sol(void){
return (light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR));
}

//sht11-sensor
static int get_temp(void)
{
  return ((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10;
}

static int get_hum(void)
{
  return (sht11_sensor.value(SHT11_SENSOR_HUMIDITY));
}

/*
static int get_bat1(void){
 return (sht11_sensor.value(SHT11_SENSOR_BATTERY_INDICATOR));
}
*/

//battery-sensor (TENSÃO DA BATERIA)
static int get_bat(void){
 	uint32_t battery;
	int fbatterycharge = 2500*2; //2500mV== 2,5V
	int fbatterychargeADC = 4096; //12 bit ADC, 2^12 = 4096
	
	battery=battery_sensor.value(0);
	
	battery *= fbatterycharge; 
	battery /= fbatterychargeADC;
 	
 	return (battery%1000);
}


/*---------------------------transmission------------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS(example_collect_process, "Test collect process");
AUTOSTART_PROCESSES(&example_collect_process);
/*---------------------------------------------------------------------------*/
static void recv(const linkaddr_t *originator, uint8_t seqno, uint8_t hops)
{

    if(hops > 0){	
	uint8_t d = _Dist/hops;		
 	/*
	Eihop: Consumo Energetico Por i saltos
	i: numero de saltos de uma transmissão
	d: distância entre os nós
	R: taxa de Transmissão
	Nb: tamanho do Pacote
	P0: potência de transmissão

	Orig: no origem	
	Batt: status bateria
	alt: altura da planta (medida por sensor de radar)
        */

	//Orig, i, d, umi, temp 
	printf(	     
		"%d,%u,%u,%s \n",
		originator->u8[0],
		hops,
		d, 		
		(char *)packetbuf_dataptr()
	) ;
    }

}

/*---------------------------------------------------------------------------*/
/*----------------------------MOTE OPERATION---------------------------------*/
/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_collect_process, ev, data)
{
  static struct etimer periodic;
  static struct etimer et;
  char *packet = malloc(_Nb);

  //--LIGA SENSORES
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(battery_sensor);
  SENSORS_ACTIVATE(sht11_sensor);

  //CRIACAO DO PACOTE
  sprintf(packet,"%u,%u,%u,%u,%u",  		
		get_sol(),
		get_light(),
		get_temp(),
		get_hum(),
		get_bat()
         );

  PROCESS_BEGIN();

  collect_open(&tc, 130, COLLECT_ROUTER, &callbacks);

  if(linkaddr_node_addr.u8[0] == 1 &&
     linkaddr_node_addr.u8[1] == 0) {
     //printf("I am sink\n");
    collect_set_sink(&tc, 1);
  }

  //Aguarde algum tempo para que a rede se estabilize.
  etimer_set(&et, 120 * CLOCK_SECOND);
  PROCESS_WAIT_UNTIL(etimer_expired(&et));

  while(1) {

    //Envio de pacote a cada 30 segundos.
    etimer_set(&periodic, CLOCK_SECOND * 30);
    etimer_set(&et, random_rand() % (CLOCK_SECOND * 30));

    PROCESS_WAIT_UNTIL(etimer_expired(&et));
    {
      static linkaddr_t oldparent;
      const linkaddr_t *parent;

      //printf("Sending\n");
      //packetbuf_set_datalen(_Nb);
      packetbuf_clear();
      packetbuf_set_datalen(sprintf(packetbuf_dataptr(),
                                    "%s", packet) + 1);                                    	

      energest_flush();	                        
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
    
    PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
    
    free(packet);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
