/* %W% %I% */
/********************************************************************/
/*                      IBM Micro Broker                            */
/* IBM Confidential                                                 */
/*                                                                  */
/* OCO Source Materials                                             */
/*                                                                  */
/* 5724-K75                                                         */
/*                                                                  */
/* (C) Copyright IBM Corp. 2009                                     */
/*                                                                  */
/* The source code for this program is not published or otherwise   */
/* divested of its trade secrets, irrespective of what has been     */
/* deposited with the U.S. Copyright Office.                        */
/*                                                                  */
/*                                                                  */
/********************************************************************/

#if !defined(THREAD_H)
#define THREAD_H

#if defined(WIN32)
	#include <Windows.h>
	#define thread_type HANDLE 
	#define thread_return_type DWORD WINAPI
	#define thread_fn LPTHREAD_START_ROUTINE
	#define mutex_type HANDLE 
#else
	#include <pthread.h>
	#define thread_type pthread_t
	#define thread_return_type void*
	typedef thread_return_type (*thread_fn)(void*);
	#define mutex_type pthread_mutex_t*
#endif

thread_type Thread_start(thread_fn, void*);
mutex_type Thread_create_mutex();
int Thread_lock_mutex(mutex_type);
int Thread_unlock_mutex(mutex_type);
void Thread_destroy_mutex(mutex_type);

#endif
