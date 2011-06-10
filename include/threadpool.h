/*
 * Copyright 2011 Robert C. Curtis. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBERT C. CURTIS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ROBERT C. CURTIS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Robert C. Curtis.
 */

#include <ll.h>

#include <pthread.h>
#include <time.h>

#ifndef I__THREADPOOL_H__
	#define I__THREADPOOL_H__

#ifdef __cplusplus
extern "C" {
#endif

struct poolthread; /* Forward Declaration */

/*
 * Main Threadpool Data Structure
 */
struct threadpool {
	/*
	 * Configuration
	 *
	 * These members need to be filled by the owner of the thread pool.
	 * They should be filled after the call to threadpool_init() since
	 * it zeros the structure.
	 */
	struct {
		unsigned int max_threads;
		unsigned int idle_secs;
		struct list_head *(*work_func)(struct poolthread *thread,
				struct list_head *work);
	} config;

	/************************** Private Members *************************/
	/* Pool */
	struct list_head pool_threads;
	volatile unsigned int pool_thread_count;
	pthread_mutex_t pool_lock;
	pthread_cond_t pool_change;

	/* Work Queue */
	struct list_head work_queue;
	volatile unsigned int work_count;
	volatile unsigned int idle_threads;
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_wake;

	/* Control Threads */
	struct list_head ctrl_threads;
	pthread_mutex_t ctrl_lock;
	pthread_cond_t ctrl_change;

	volatile unsigned int shutdown;
};

/*
 * Thread Data Structures
 */
struct poolthread {
	/* Parent Pool */
	struct threadpool *pool;

	/* Pool Connectivity */
	struct list_head pool_node;

	/* Timeout Info */
	time_t idle_start;

	/* Thread */
	pthread_t thread;
};

struct ctrlthread {
	/* Parent Pool */
	struct threadpool *pool;

	/* Pool Connectivity */
	struct list_head ctrl_node;

	/* Thread */
	pthread_t thread;
	void (*ctrl_func)(struct ctrlthread *thread);
	void *priv;
	volatile unsigned int shutdown;
};

/* FUNCTION:    threadpool_init
 *
 * + DESCRIPTION:
 *   - initialize a threadpool structure
 *
 * + PARAMETERS:
 *   + struct threadpool *pool
 *
 * + RETURNS: int
 *   - 0 on success
 */
int threadpool_init(struct threadpool *pool);

/* FUNCTION:    threadpool_shutdown
 *
 * + DESCRIPTION:
 *   - shutdown a threadpool
 *
 * + PARAMETERS:
 *   + struct threadpool *pool
 *
 * + RETURNS: int
 *   - 0 on success
 */
int threadpool_shutdown(struct threadpool *pool);

/* FUNCTION:    threadpool_queue_work
 *
 * + DESCRIPTION:
 *   - queue a piece of work
 *
 * + PARAMETERS:
 *   + struct threadpool *pool
 *     - pool
 *   + struct list_head *work
 *     - list_head node in the piece of work
 */
void threadpool_queue_work(struct threadpool *pool, struct list_head *work);

/* FUNCTION:    threadpool_is_work_done
 *
 * + DESCRIPTION:
 *   - check if work queue is empty and all threads are idle
 *
 * + PARAMETERS:
 *   + struct threadpool *pool
 *     - pool
 *
 * + RETURNS: int
 *   - boolean
 */
int threadpool_is_work_done(struct threadpool *pool);

/* FUNCTION:    threadpool_start_control_thread
 *
 * + DESCRIPTION:
 *   - Starts a threadpool control thread. A control thread is independent of
 *   the work queue, but is signaled to shutdown with the rest of the
 *   threadpool. It's function is called in a loop until a shutdown is
 *   requested. The function should not loop itself, and should properly
 *   handle yielding (either explicitly or calling blocking calls). It can
 *   shut itself down by setting its own shutdown flag.
 *
 * + PARAMETERS:
 *   + struct threadpool *pool
 *     - pool
 *   + void (*ctrl_func)(struct ctrlthread *thread)
 *     - control thread function
 *   + void *priv
 *     - private data
 *
 * + RETURNS: int
 *   - 0 on success
 */
int threadpool_start_control_thread(struct threadpool *pool,
	void (*ctrl_func)(struct ctrlthread *thread), void *priv);

#ifdef __cplusplus
}
#endif
#endif /* I__THREADPOOL_H__ */
