#ifndef _TARGET_H_
#define _TARGET_H_

#include <FreeRTOS.h>
#include <task.h>

#define TLSF_MLOCK_T            void *
#define TLSF_CREATE_LOCK(l)     do{}while(0)
#define TLSF_DESTROY_LOCK(l)    do{}while(0)
#define TLSF_ACQUIRE_LOCK(l)    vTaskSuspendAll()
#define TLSF_RELEASE_LOCK(l)    xTaskResumeAll()

#endif
