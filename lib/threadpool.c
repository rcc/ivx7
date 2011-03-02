/*
 * threadpool.c
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
#include <threadpool.h>
#include <logging.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

static void *threadpool_thread(struct poolthread *t)
{
	struct list_head *work;
	struct timeval tv;
	struct timespec ts;

	logverbose("start %p\n", t);

	t->idle_start = time(NULL);

	pthread_mutex_lock(&t->pool->queue_lock);
	t->pool->idle_threads++;
	while(!t->shutdown && ((time(NULL) - t->idle_start) < 60)) {
		if(!list_empty(&t->pool->work_queue)) {
			/* get the work from the work_queue */
			work = t->pool->work_queue.next;
			list_del(work);
			/* set self as non-idle */
			t->pool->idle_threads--;
			/* do the work */
			pthread_mutex_unlock(&t->pool->queue_lock);
			if(t->pool->work_func)
				t->pool->work_func(t, work);
			pthread_mutex_lock(&t->pool->queue_lock);
			/* set self as idle again */
			t->pool->idle_threads++;
			t->idle_start = time(NULL);
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

	pthread_mutex_lock(&t->pool->pool_lock);
	list_del(&t->pool_node);
	t->pool->thread_count--;
	pthread_mutex_unlock(&t->pool->pool_lock);
	pthread_cond_broadcast(&t->pool->pool_change);

	logverbose("end %p\n", t);
	free(t);

	return NULL;
}

static void threadpool_start_new_thread(struct threadpool *pool)
{
	struct poolthread *t = malloc(sizeof(*t));

	if(t == NULL) {
		logerror("%s: could not allocate pool thread\n", __FUNCTION__);
		return;
	}

	t->pool = pool;
	t->shutdown = 0;
	if(pthread_create(&t->thread, NULL,
				(void *(*)(void *))threadpool_thread, t) != 0) {
		logerror("%s: could not start pool thread\n", __FUNCTION__);
		free(t);
		return;
	}
	pthread_detach(t->thread);
	list_add(&t->pool_node, &pool->thread_pool);
	pool->thread_count++;
}

int threadpool_init(struct threadpool *pool)
{
	memset(pool, 0, sizeof(*pool));

	INIT_LIST_HEAD(&pool->thread_pool);
	pthread_mutex_init(&pool->pool_lock, NULL);
	pthread_cond_init(&pool->pool_change, NULL);

	INIT_LIST_HEAD(&pool->work_queue);
	pthread_mutex_init(&pool->queue_lock, NULL);
	pthread_cond_init(&pool->queue_wake, NULL);

	return 0;
}

int threadpool_shutdown(struct threadpool *pool)
{
	int status = -1;
	struct poolthread *pos, *n;
	struct timeval tv;
	struct timespec ts;

	pthread_mutex_lock(&pool->pool_lock);
	list_for_each_entry_safe(pos, n, &pool->thread_pool, pool_node) {
		pos->shutdown = 1;
	}

	do {
		if(list_empty(&pool->thread_pool)) {
			logdebug("threadpool shutdown successful\n");
			status = 0;
			break;
		}
		gettimeofday(&tv, NULL);
		ts.tv_sec = tv.tv_sec + 30;
		ts.tv_nsec = 0;
	} while(pthread_cond_timedwait(&pool->pool_change,
				&pool->pool_lock, &ts) == 0);
	pthread_mutex_unlock(&pool->pool_lock);

	pthread_mutex_destroy(&pool->pool_lock);
	pthread_cond_destroy(&pool->pool_change);
	pthread_mutex_destroy(&pool->queue_lock);
	pthread_cond_destroy(&pool->queue_wake);

	return status;
}

void threadpool_queue_work(struct threadpool *pool, struct list_head *work)
{
	/* Queue work and signal pool */
	pthread_mutex_lock(&pool->queue_lock);
	list_add_tail(work, &pool->work_queue);
	pthread_mutex_unlock(&pool->queue_lock);
	pthread_cond_signal(&pool->queue_wake);

	/* If work is waiting, try to start a new thread */
	pthread_mutex_lock(&pool->queue_lock);
	if(!pool->idle_threads && !list_empty(&pool->work_queue)) {
		pthread_mutex_lock(&pool->pool_lock);
		if(pool->thread_count < pool->max_threads) {
			logdebug("starting new pool thread\n");
			threadpool_start_new_thread(pool);
		}
		pthread_mutex_unlock(&pool->pool_lock);
	}
	pthread_mutex_unlock(&pool->queue_lock);
}

int threadpool_is_work_done(struct threadpool *pool)
{
	int ret;
	pthread_mutex_lock(&pool->queue_lock);
	ret = list_empty(&pool->work_queue);
	pthread_mutex_unlock(&pool->queue_lock);
	return ret;
}

#ifdef THREADPOOLTESTCMD
#include <prjutil.h>
#include <cmds.h>
#include <unistd.h>

struct test_work_data {
	unsigned int sec;
	struct list_head node;
};

static void test_work_func(struct poolthread *thread, struct list_head *work)
{
	struct test_work_data *d =
		list_entry(work, struct test_work_data, node);
	logdebug("%p : waiting for %d seconds\n", thread, d->sec);
	sleep(d->sec);
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
