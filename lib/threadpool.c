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

#include <threadpool.h>
#include <logging.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>

/***************************** Thread Functions *****************************/
static void *threadpool_thread(struct poolthread *t)
{
	struct list_head *work;
	struct timeval tv;
	struct timespec ts;

	logverbose("start %p\n", t);

	t->idle_start = time(NULL);

	pthread_mutex_lock(&t->pool->queue_lock);
	t->pool->idle_threads++;
	while(!t->pool->shutdown &&
			((time(NULL) - t->idle_start) < t->pool->idle_secs)) {
		if(!list_empty(&t->pool->work_queue)) {
			/* get the work from the work_queue */
			work = t->pool->work_queue.next;
			list_del(work);
			t->pool->work_count--;
			/* set self as non-idle */
			t->pool->idle_threads--;
			/* do the work */
			pthread_mutex_unlock(&t->pool->queue_lock);
			if(t->pool->work_func)
				work = t->pool->work_func(t, work);
			pthread_mutex_lock(&t->pool->queue_lock);
			/* set self as idle again */
			t->pool->idle_threads++;
			t->idle_start = time(NULL);
			/* Add work back to queue if it was passed back from
			 * the work_func */
			if(work) {
				list_add_tail(work, &t->pool->work_queue);
				t->pool->work_count++;
			}
		} else {
			/* idle, wait for queue signal */
			gettimeofday(&tv, NULL);
			ts.tv_sec = tv.tv_sec + 1;
			ts.tv_nsec = 0;
			pthread_cond_timedwait(&t->pool->queue_wake,
					&t->pool->queue_lock, &ts);
		}
	}
	t->pool->idle_threads--;
	pthread_mutex_unlock(&t->pool->queue_lock);

	/* Remove self from pool */
	pthread_mutex_lock(&t->pool->pool_lock);
	list_del(&t->pool_node);
	t->pool->pool_thread_count--;
	pthread_mutex_unlock(&t->pool->pool_lock);
	pthread_cond_broadcast(&t->pool->pool_change);

	logverbose("end %p\n", t);
	free(t);

	return NULL;
}

static void *ctrlthread_thread(struct ctrlthread *t)
{
	logverbose("start %p\n", t);

	while(!t->shutdown && !t->pool->shutdown) {
		t->ctrl_func(t);
	}

	/* Remove self from control thread list */
	pthread_mutex_lock(&t->pool->ctrl_lock);
	list_del(&t->ctrl_node);
	pthread_mutex_unlock(&t->pool->ctrl_lock);
	pthread_cond_broadcast(&t->pool->ctrl_change);

	logverbose("end %p\n", t);
	free(t);

	return NULL;
}

/****************************** Local Functions *****************************/
static void threadpool_start_new_thread(struct threadpool *pool)
{
/* It's assumed that you own the pool_lock before entering this function */
	struct poolthread *t = malloc(sizeof(*t));

	if(t == NULL) {
		logerror("could not allocate pool thread: %s\n",
				strerror(errno));
		return;
	}

	t->pool = pool;
	if(pthread_create(&t->thread, NULL,
				(void *(*)(void *))threadpool_thread, t) != 0) {
		logerror("could not start pool thread\n");
		free(t);
		return;
	}
	pthread_detach(t->thread);
	list_add(&t->pool_node, &pool->pool_threads);
	pool->pool_thread_count++;
}

/******************************* API Functions ******************************/
int threadpool_init(struct threadpool *pool)
{
	memset(pool, 0, sizeof(*pool));

	pool->max_threads = 10;
	pool->idle_secs = 60;

	INIT_LIST_HEAD(&pool->pool_threads);
	pthread_mutex_init(&pool->pool_lock, NULL);
	pthread_cond_init(&pool->pool_change, NULL);

	INIT_LIST_HEAD(&pool->work_queue);
	pthread_mutex_init(&pool->queue_lock, NULL);
	pthread_cond_init(&pool->queue_wake, NULL);

	INIT_LIST_HEAD(&pool->ctrl_threads);
	pthread_mutex_init(&pool->ctrl_lock, NULL);
	pthread_cond_init(&pool->ctrl_change, NULL);

	return 0;
}

int threadpool_shutdown(struct threadpool *pool)
{
	int status = 0;
	struct timeval tv;
	struct timespec ts;

	if(!threadpool_is_work_done(pool)) {
		logwarn("shutting down threadpool with active work\n");
	}

	/* Set the global shutdown flag */
	pool->shutdown = 1;

	/* Wait for pool threads to exit */
	pthread_mutex_lock(&pool->pool_lock);
	do {
		if(list_empty(&pool->pool_threads)) {
			logdebug("pool threads shutdown successful\n");
			status++;
			break;
		}
		gettimeofday(&tv, NULL);
		ts.tv_sec = tv.tv_sec + 30;
		ts.tv_nsec = 0;
	} while(pthread_cond_timedwait(&pool->pool_change,
				&pool->pool_lock, &ts) == 0);
	pthread_mutex_unlock(&pool->pool_lock);

	/* Wait for control threads to exit */
	pthread_mutex_lock(&pool->ctrl_lock);
	do {
		if(list_empty(&pool->ctrl_threads)) {
			logdebug("control threads shutdown successful\n");
			status++;
			break;
		}
		gettimeofday(&tv, NULL);
		ts.tv_sec = tv.tv_sec + 30;
		ts.tv_nsec = 0;
	} while(pthread_cond_timedwait(&pool->ctrl_change,
				&pool->ctrl_lock, &ts) == 0);
	pthread_mutex_unlock(&pool->ctrl_lock);

	pthread_mutex_destroy(&pool->pool_lock);
	pthread_cond_destroy(&pool->pool_change);
	pthread_mutex_destroy(&pool->queue_lock);
	pthread_cond_destroy(&pool->queue_wake);
	pthread_mutex_destroy(&pool->ctrl_lock);
	pthread_cond_destroy(&pool->ctrl_change);

	return (status != 2);
}

void threadpool_queue_work(struct threadpool *pool, struct list_head *work)
{
	/* Queue work */
	pthread_mutex_lock(&pool->queue_lock);
	list_add_tail(work, &pool->work_queue);
	pool->work_count++;

	/* Check if a new thread is needed */
	if(pool->work_count > pool->idle_threads) {
		pthread_mutex_lock(&pool->pool_lock);
		if(pool->pool_thread_count < pool->max_threads) {
			logdebug("starting new pool thread\n");
			threadpool_start_new_thread(pool);
		}
		pthread_mutex_unlock(&pool->pool_lock);
	}
	pthread_mutex_unlock(&pool->queue_lock);

	/* Signal pool for new work */
	pthread_cond_signal(&pool->queue_wake);
}

int threadpool_is_work_done(struct threadpool *pool)
{
	int ret;
	pthread_mutex_lock(&pool->pool_lock);
	pthread_mutex_lock(&pool->queue_lock);
	ret = (!pool->work_count &&
			(pool->idle_threads == pool->pool_thread_count));
	pthread_mutex_unlock(&pool->queue_lock);
	pthread_mutex_unlock(&pool->pool_lock);
	return ret;
}

int threadpool_start_control_thread(struct threadpool *pool,
	void (*ctrl_func)(struct ctrlthread *thread), void *priv)
{
	int status = 0;
	struct ctrlthread *t;

	if((ctrl_func == NULL) || (pool == NULL))
		return EINVAL;

	if((t = malloc(sizeof(*t))) == NULL) {
		logerror("could not allocate control thread: %s\n",
				strerror(errno));
		return errno;
	}

	t->pool = pool;
	t->ctrl_func = ctrl_func;
	t->priv = priv;
	t->shutdown = 0;

	pthread_mutex_lock(&t->pool->ctrl_lock);
	if((status = pthread_create(&t->thread, NULL,
				(void *(*)(void *))ctrlthread_thread, t))
			!= 0) {
		logerror("could not start control thread\n");
		pthread_mutex_unlock(&t->pool->ctrl_lock);
		free(t);
		return status;
	}
	pthread_detach(t->thread);
	list_add(&t->ctrl_node, &pool->ctrl_threads);
	pthread_mutex_unlock(&t->pool->ctrl_lock);

	return status;
}

#ifdef THREADPOOLTESTCMD
#include <prjutil.h>
#include <cmds.h>
#include <unistd.h>

struct test_work_data {
	unsigned int sec;
	struct list_head node;
};

static struct list_head *test_work_func(struct poolthread *thread,
		struct list_head *work)
{
	struct test_work_data *d =
		list_entry(work, struct test_work_data, node);
	logdebug("%p : waiting for %d seconds\n", thread, d->sec);
	sleep(d->sec);

	return NULL;
}

static struct threadpool testpool;
static struct test_work_data test_data[10];

CMDHANDLER(threadpool_test)
{
	int i;

	loginfo("Testing Thread Pool...\n");

	if(threadpool_init(&testpool) != 0) {
		return -1;
	}
	testpool.max_threads = 3;
	testpool.work_func = &test_work_func;
	for(i = 0; i < ARRAY_SIZE(test_data); i++) {
		test_data[i].sec = i;
		threadpool_queue_work(&testpool, &test_data[i].node);
	}

	while(!threadpool_is_work_done(&testpool));
	threadpool_shutdown(&testpool);

	loginfo("Done Testing Thread Pool\n");
	return 0;
}
APPCMD(threadpool, &threadpool_test, "test threadpool", "usage: threadpool",
		NULL);
#endif /* THREADPOOLTESTCMD */
