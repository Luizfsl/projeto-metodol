#include "contiki.h"

/*---------------------------------------------------------------------------*/

#include "dev/battery-sensor.h"
#include "dev/light-sensor.h"
#include "dev/temperature-sensor.h"
#include "dev/humidity-sensor.h"
#include "dev/pressure-sensor.h"
#include "dev/magneto-sensor.h"
#include "dev/acceleration-sensor.h"
#include "dev/gyroscope-sensor.h"
/*---------------------------------------------------------------------------*/

#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Hello, world\n");
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
