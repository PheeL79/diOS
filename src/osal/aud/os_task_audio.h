/***************************************************************************//**
* @file    task_audio.h
* @brief   Audio daemon task.
* @author  A. Filyanov
*******************************************************************************/
#ifndef _OS_TASK_AUDIO_H_
#define _OS_TASK_AUDIO_H_

#if (1 == OS_AUDIO_ENABLED)
#define OS_DAEMON_NAME_AUDIO        "AudioD"
#endif //(1 == OS_AUDIO_ENABLED)

#endif // _OS_TASK_AUDIO_H_