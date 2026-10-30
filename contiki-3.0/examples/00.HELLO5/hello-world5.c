#include "contiki.h"

/*****************COLLECTview************************/
#include "shell.h"
#include "serial-shell.h"
#include "collect-view.h"
/*****************************************************/

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "HELLO VIEW");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(hello_world_process, ev, data)
{

  PROCESS_BEGIN();
  
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
  shell_sendtest_init();
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

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
