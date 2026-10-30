#include "contiki.h"
#include "net/rime/rime.h"
#include "dev/leds.h"
#include "powertrace.h"
#include "random.h"
#include <stdio.h>
#include <string.h>

/* Modos de Transmissão */
#define MODE_BURST 1
#define MODE_BATCH 2

#ifndef TRANSMISSION_MODE
#define TRANSMISSION_MODE MODE_BURST
#endif

/* Quantidade de pacotes por ciclo (injetada dinamicamente pelo Makefile/Script) */
#ifndef PACKET_COUNT
#define PACKET_COUNT 5
#endif

/* Estrutura para Envio em Lote (Batching) */
typedef struct {
  uint16_t seq;
  uint16_t value;
} sensor_data_t;

typedef struct {
  uint8_t total_samples;
  sensor_data_t samples[32]; // Suporta lotes de até 32 amostras
} batch_packet_t;

PROCESS(node_process, "Processo No IoT");
AUTOSTART_PROCESSES(&node_process);

/* Callback de recepção unicast */
static void recv_uc(struct unicast_conn *c, const linkaddr_t *from) {
  leds_toggle(LEDS_RED);
  #if TRANSMISSION_MODE == MODE_BATCH
    batch_packet_t *batch = (batch_packet_t *)packetbuf_dataptr();
    printf("<-- [SINK %d.%d] Lote recebido de %d.%d com %d amostras\n",
           linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
           from->u8[0], from->u8[1], batch->total_samples);
  #else
    char *msg = (char *)packetbuf_dataptr();
    printf("<-- [SINK %d.%d] Pacote recebido de %d.%d: '%s'\n",
           linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
           from->u8[0], from->u8[1], msg);
  #endif
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;

PROCESS_THREAD(node_process, ev, data)
{
  static struct etimer periodic_timer;
  static uint16_t seq_num = 0;

  PROCESS_EXITHANDLER(unicast_close(&uc);)

  PROCESS_BEGIN();

  /* Inicia o monitoramento de energia a cada 2 segundos no log */
  powertrace_start(CLOCK_SECOND * 2);

  unicast_open(&uc, 146, &unicast_callbacks);

  /* LÓGICA DE PAPÉIS */
  if(linkaddr_node_addr.u8[0] == 1) {
    /* NÓ 1: SINK / RECEPTOR */
    printf("[NÓ SINK %d.%d] Aguardando transmissões...\n",
           linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
    while(1) {
      PROCESS_YIELD();
    }
  } else {
    /* NÓS > 1: EMISSORES */
    printf("[NÓ EMISSOR %d.%d] Modo: %s | Pacotes/Amostras por ciclo: %d\n",
           linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
           (TRANSMISSION_MODE == MODE_BURST) ? "RAJADA (BURST)" : "LOTE (BATCH)",
           PACKET_COUNT);

    /* Atraso inicial para estabilização */
    etimer_set(&periodic_timer, CLOCK_SECOND * 5 + (random_rand() % (CLOCK_SECOND * 2)));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    etimer_set(&periodic_timer, CLOCK_SECOND * 10); // Intervalo de ciclo

    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

      linkaddr_t target_addr;
      target_addr.u8[0] = 1; // Endereço do Sink
      target_addr.u8[1] = 0;

      #if TRANSMISSION_MODE == MODE_BURST
        /* --- ENVIO EM RAJADA (Vários pacotes pequenos em sequência rápida) --- */
        int i;
        for(i = 0; i < PACKET_COUNT; i++) {
          char msg[32];
          snprintf(msg, sizeof(msg), "BURST seq=%u sub=%d", seq_num++, i + 1);
          
          packetbuf_copyfrom(msg, strlen(msg) + 1);
          unicast_send(&uc, &target_addr);
          
          leds_toggle(LEDS_GREEN);
          printf("--> [EMISSOR %d.%d] Disparo Rajada %d/%d enviado.\n",
                 linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], i + 1, PACKET_COUNT);
          
          clock_delay_usec(2000); // Pequeno atraso na rajada
        }

      #elif TRANSMISSION_MODE == MODE_BATCH
        /* --- ENVIO EM LOTE (1 pacote único grande com todos os dados agrupados) --- */
        batch_packet_t batch;
        batch.total_samples = PACKET_COUNT;
        
        int i;
        for(i = 0; i < PACKET_COUNT; i++) {
          batch.samples[i].seq = seq_num++;
          batch.samples[i].value = 200 + (random_rand() % 50); // Dado simulado
        }

        packetbuf_copyfrom(&batch, sizeof(batch_packet_t));
        unicast_send(&uc, &target_addr);

        leds_toggle(LEDS_GREEN);
        printf("--> [EMISSOR %d.%d] Pacote Lote com %d amostras enviado (%u bytes).\n",
               linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
               batch.total_samples, (unsigned int)sizeof(batch_packet_t));
      #endif

      etimer_reset(&periodic_timer);
    }
  }

  PROCESS_END();
}
