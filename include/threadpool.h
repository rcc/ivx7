/*
 * threadpool.h
 *
 * Copyright (C) 2010 Robert C. Curtis
 *
 * threadpool is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * threadpool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with threadpool.  If not, see <http://www.gnu.org/licenses/>.
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
	/* Configuration */
	unsigned int max_threads;
	void (*work_func)(struct poolthread *thread, struct list_head *work);

	/* Pool */
	volatile unsigned int thread_count;
	struct list_head thread_pool;
	pthread_mutex_t pool_lock;
	pthread_cond_t pool_change;

	/* Work Queue */
	struct list_head work_queue;
	volatile unsigned int idle_threads;
	pthread_cond_t queue_wake;
	pthread_mutex_t queue_lock;
};

/*
 * Thread Data Structure
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
 *   - check if work queue is empty
 *
 * + PARAMETERS:
 *   + struct threadpool *pool
 *     - pool
 *
 * + RETURNS: int
 *   - boolean
 */
int threadpool_is_work_done(struct threadpool *pool);

#ifdef __cplusplus
}
#endif
#endif /* I__THREADPOOL_H__ */
