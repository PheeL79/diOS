/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __SYS_RTXC_H__
#define __SYS_RTXC_H__

//#include "cmsis_os.h"
#include "os_common.h"
#include "os_semaphore.h"
#include "os_memory.h"
#include "os_mutex.h"
#include "os_queue.h"
#include "os_task.h"

#define malloc(size)    OS_MallocEx(size, OS_NETWORK_MEM_HEAP)
#define calloc(size)    OS_MallocEx(size, OS_NETWORK_MEM_HEAP)
#define free(p)         OS_FreeEx(p, OS_NETWORK_MEM_HEAP)

#define SYS_MBOX_NULL (OS_QueueHd)OS_NULL
#define SYS_SEM_NULL  (OS_SemaphoreHd)OS_NULL
#define SYS_DEFAULT_THREAD_STACK_DEPTH	OS_STACK_SIZE_MIN

typedef OS_SemaphoreHd  sys_sem_t;
typedef OS_MutexHd      sys_mutex_t;
typedef OS_QueueHd      sys_mbox_t;
typedef OS_TaskHd       sys_thread_t;

typedef struct _sys_arch_state_t
{
	// Task creation data.
	char cTaskName[OS_TASK_NAME_LEN];
	unsigned short nStackDepth;
	unsigned short nTaskCount;
} sys_arch_state_t;

sys_prot_t  sys_arch_protect(void);
void        sys_arch_unprotect(sys_prot_t pval);
void        sys_assert(const char *msg);

//extern sys_arch_state_t s_sys_arch_state;

//void sys_set_default_state();
//void sys_set_state(signed char *pTaskName, unsigned short nStackSize);

/* Message queue constants. */
#define archMESG_QUEUE_LENGTH	( OS_NETWORK_DAEMON_QUEUE_LEN )
#endif /* __SYS_RTXC_H__ */

