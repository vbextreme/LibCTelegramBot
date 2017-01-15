#define _GNU_SOURCE

#include <tgb/easythread.h>
#include <tgb/terminale.h>

#define sizeal(TY) (sizeof(TY) + sizeof(TY) % sizeof(int_t)) 
#define tooffset(M,OF) ((uint8_t*)(M) + (OF))

/**************/
/** SUPPORT ***/
/**************/

/*
static double _bch_get()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}

static inline double _bch_clc(double st, double en)
{
    return en-st;
}
*/

static cpu_set_t* _setcpu(uint_t mcpu)
{
    dbg("");
    
	static cpu_set_t cpu;
	CPU_ZERO(&cpu);
	
	if (mcpu == 0 ) return &cpu;
	
	uint_t s;
	while ( (s = mcpu % 10) )
	{
		CPU_SET(s - 1,&cpu);
		mcpu /= 10;
	}
	
	return &cpu;
}
/*****************/
/*** SEMAPHORE ***/
/*****************/

err_t thr_semaphore_init(semaphore_h* s, uint_t stval)
{
    dbg("");
    iassert( s != NULL );
    
    if ( sem_init(s, 0, stval) ) 
    { 
        dbg("error: semaphore return error");
        return -1;
    }
    return 0;
}

int_t thr_semaphore_get(semaphore_h* s)
{
    dbg("");
    iassert( s != NULL );
	
    int_t ret;
    #if ASSERT_ENABLE == 1
		err_t err = sem_getvalue(s, &ret);
		iassert( err == 0 );
	#else
		sem_getvalue(s, &ret);
	#endif
	
	return ret;
}

/*************/
/*** MUTEN ***/
/*************/

err_t thr_muten_init(muten_s* h, char_t* filenamespace, char_t* name)
{	
	///printf("Page size: 4096\nmx+att = %d\n",sizeal(pthread_mutex_t) + sizeal(pthread_mutexattr_t));
	///printf("max muten = 4096 / mx+att+name = %d\n",4096 / (sizeal(pthread_mutex_t) + sizeal(pthread_mutexattr_t) + THR_MUTEN_NAME_MAX));
	
    dbg("");
    iassert( h != NULL );
    iassert( filenamespace != NULL );
    iassert( name != NULL );
    
	FILE* tst = fopen(filenamespace,"r");
	if ( !tst )
	{
		tst = fopen(filenamespace,"w");
        if ( !tst )
        {
            dbg("error: can't open filenamespace");
            return -1;
        }
        
		fprintf(tst,"named mutex %s\n%d", filenamespace, THR_MUTEN_MAX);
		fclose(tst);
	}
	
	key_t k = ftok(filenamespace,'M');
		if ( k < 0 ) {return -1;}
	
	uint_t sz = sizeal(pthread_mutex_t) + sizeal(pthread_mutexattr_t);
	uint_t ofch = sz;
	sz += THR_MUTEN_MAX * THR_MUTEN_NAME_MAX;
	uint_t ofmu = sz;
	sz += THR_MUTEN_MAX * ( sizeal(pthread_mutex_t) + sizeal(pthread_mutexattr_t) );
	
	bool_t nw = FALSE;
	if ( (h->memid = shmget(k,sz,IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) < 0 )
	{
		if ( (h->memid = shmget(k,sz,S_IRUSR | S_IWUSR)) < 0 ) 
        { 
            dbg( "syscall shmget return error");
            return -2;
        } 
	}
	else
		nw = TRUE;
	
	h->membase = shmat(h->memid, (void *)0, 0);
		if (h->membase == (char *)(-1)) {return -3;}
		
	pthread_mutex_t* lkm = (pthread_mutex_t*) h->membase;
	pthread_mutexattr_t* alkm = (pthread_mutexattr_t*) tooffset(h->membase,sizeal(pthread_mutex_t));
	
	char_t* pn;
	int_t i;
	
	if ( nw )
	{
		pthread_mutexattr_init(alkm);
		pthread_mutexattr_setpshared(alkm, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(lkm,alkm);
		
		pn = (char_t*) tooffset(h->membase,ofch);
		for (i = 0; i < THR_MUTEN_MAX; ++i, pn += THR_MUTEN_NAME_MAX)
			*pn = '\0';
	}
	pthread_mutex_lock(lkm);
	
	for (pn = (char_t*) tooffset(h->membase,ofch), i = 0; i < THR_MUTEN_MAX && strcmp(pn,name); ++i, pn += THR_MUTEN_NAME_MAX);
	
	if ( i >= THR_MUTEN_MAX )
	{
		for (pn = (char_t*) tooffset(h->membase,ofch), i = 0; i < THR_MUTEN_MAX && *pn; ++i, pn += THR_MUTEN_NAME_MAX);
		if ( i >= THR_MUTEN_MAX ) { return -4;}
		
		strcpy(pn,name);
		h->memname = pn;
		h->mx =(pthread_mutex_t*) tooffset(h->membase,ofmu + ((sizeal(pthread_mutex_t) + sizeal(pthread_attr_t)) * i));
		h->att = (pthread_mutexattr_t*) tooffset(h->mx,sizeal(pthread_mutex_t));
		pthread_mutexattr_init(h->att);
		pthread_mutexattr_setpshared(h->att, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(h->mx,h->att);
	}
	else
	{
		h->mx =(pthread_mutex_t*) tooffset(h->membase,ofmu + ((sizeal(pthread_mutex_t) + sizeal(pthread_attr_t)) * i));
		h->att = (pthread_mutexattr_t*) tooffset(h->mx,sizeal(pthread_mutex_t));
	}
	pthread_mutex_unlock(lkm);
	
	return 0;
}

void thr_muten_lock(muten_s* m)
{
    dbg("");
    iassert( m != NULL );
    
    pthread_mutex_lock(m->mx);
}

void thr_muten_unlock(muten_s* m)
{
    dbg("");
    iassert( m != NULL );
    
    pthread_mutex_unlock(m->mx);
}

void thr_muten_destroy(char_t* filenamespace, muten_s* m)
{
    dbg("");
    iassert( filenamespace != NULL );
    iassert( m != NULL );
    
	int_t memid = m->memid;
	thr_muten_deatach(m);
	shmctl(memid,IPC_RMID,0);
	unlink(filenamespace);
}

void thr_muten_deatach(muten_s* m)
{
    dbg("");
    iassert( m != NULL );
    
	pthread_mutex_destroy(m->mx);
	pthread_mutexattr_destroy(m->att);
	m->memname = '\0';
	shmdt(m->membase);
}

/*************/
/*** EVENT ***/
/*************/

void thr_event_init(event_s* e, int_t autoenter,int_t autoexit,int_t resumeall,int_t value)
{
    dbg("");
    iassert( e != NULL );
    
    e->autoenter = autoenter;
    e->autoexit = autoexit;
    e->resumeall = resumeall;
    e->value = value;
    thr_mutex_init(&e->mtx);
    pthread_cond_init(&e->condition, NULL);
}

void thr_event_destroy(event_s* e)
{
    dbg("");
    iassert( e != NULL );
    
    thr_mutex_destroy(&e->mtx);
    pthread_cond_destroy(&e->condition);
}

void thr_event_enter(event_s* e)
{
    dbg("");
    iassert( e != NULL );
    
    if (e->autoenter) return;
    thr_mutex_lock(&e->mtx);
}

void thr_event_exit(event_s* e)
{
    dbg("");
    iassert( e != NULL );
    
    if (e->autoexit) return;
    thr_mutex_unlock(&e->mtx);
}

int_t thr_event_wait(event_s* e,int_t timeoutms)
{
    dbg("");
    iassert( e != NULL );
    
    if (e->autoenter)
        thr_mutex_lock(&e->mtx);

    if (!e->value)
    {
        if (!timeoutms)
        {
            pthread_cond_wait(&e->condition, &e->mtx);
        }
        else
        {
            struct timespec   ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += timeoutms * 1000;

            int_t ret = pthread_cond_timedwait(&e->condition, &e->mtx, &ts);
            if (ret)
            {
                ret = 0;
                thr_mutex_unlock(&e->mtx);
                return 0;
            }
        }

    }

    if (e->autoexit)
    {
        thr_mutex_unlock(&e->mtx);
    }

    return 1;
}

void thr_event_raise(event_s* e)
{
    dbg("");
    iassert( e != NULL );
    
    thr_mutex_lock(&e->mtx);
    e->value = 1;

    if (e->resumeall)
        pthread_cond_broadcast(&e->condition);
    else
        pthread_cond_signal(&e->condition);

    thr_mutex_unlock(&e->mtx);
}

void thr_event_reset(event_s* e)
{
    dbg("");
    iassert( e != NULL );
    
    thr_mutex_lock(&e->mtx);
    e->value = 0;
    thr_mutex_unlock(&e->mtx);
}

/// /////// ///
/// MESSAGE ///
/// /////// ///

void thr_message_init(message_s* m, thrmessage_e type, void* data, int_t autofree)
{
    dbg("");
    iassert( m != NULL );
    
    m->type = type;
    m->msg = data;
    m->autofree = autofree;
    m->next = NULL;
}

message_s* thr_message_new(thrmessage_e type, void* data, int_t autofree)
{
    dbg("");
    
    message_s* m = NEW(message_s);
    iassert( m != NULL );
    
    thr_message_init(m, type, data, autofree);
    return m;
}

void thr_message_destroy(message_s* m)
{
    dbg("");
    iassert( m != NULL );
    
    if ( m->autofree && m->msg != NULL ) free(m->msg);
    m->msg = NULL;
    m->next = NULL;
}

void thr_message_free(message_s* m)
{
    dbg("");
    iassert( m != NULL );
    
    thr_message_destroy(m);
    free(m);
}

thrmessage_e thr_message_gettype(message_s* m) 
{ 
    dbg("");
    iassert( m != NULL );
    return m->type; 
}

void* thr_message_getmsg(message_s* m) 
{ 
    dbg("");
    iassert( m != NULL );
    return m->msg; 
}

int_t thr_message_getautofree(message_s* m) 
{ 
    dbg("");
    iassert( m != NULL );
    return m->autofree; 
}

/****************/
/*** MSGQUEUE ***/
/****************/

void thr_queue_init(msgqueue_s* q)
{
    dbg("");
    iassert( q != NULL );
    
    q->szcoda = 0;
    q->first = NULL;
    q->last = NULL;
    q->current = NULL;

    thr_mutex_init(&q->safeins);
    thr_event_init(&q->havemsg, 1, 1, 0, 0);
}


msgqueue_s* thr_queue_new(void)
{
    dbg("");
    
    msgqueue_s* q = NEW(msgqueue_s);
    iassert( q != NULL );
    
    thr_queue_init(q);
    return q;
}

void thr_queue_destroy(msgqueue_s* q)
{
    dbg("");
    iassert( q != NULL );
    
    for ( ; q->first != NULL ; q->first = q->current)
    {
        q->current = q->first->next;
        thr_message_free(q->first);
    }
    thr_mutex_destroy(&q->safeins);
    thr_event_destroy(&q->havemsg);
}

void thr_queue_free(msgqueue_s* q)
{
    dbg("");
    iassert( q != NULL );
    
    thr_queue_destroy(q);
    free(q);
}

int_t thr_queue_getsize(msgqueue_s* q)
{
    dbg("");
    iassert( q != NULL );
    
    return q->szcoda;
}

err_t thr_queue_add(msgqueue_s* q, message_s* m)
{
    dbg("");
    iassert( q != NULL );
    iassert( m != NULL );
    
    thr_mutex_lock(&q->safeins);

    if (q->first == NULL)
    {
        q->first = m;
        q->last = m;
    }
    else
    {
        q->last->next = m;
        q->last = m;
    }


    ++q->szcoda;

    thr_mutex_unlock(&q->safeins);
    thr_event_raise(&q->havemsg);
    return 0;
}

message_s* thr_queue_getmessage(msgqueue_s* q, uint_t waitms)
{
    dbg("");
    iassert( q != NULL );
    
    if (waitms && q->szcoda <= 0)
    {
        thr_event_wait(&q->havemsg, waitms);
        thr_event_reset(&q->havemsg);
    }

    thr_mutex_lock(&q->safeins);

    message_s* r;

    if (q->first == NULL)
    {
        r = NULL;
        q->szcoda = 0;
    }
    else
    {
        r = q->first;
        if (q->first == q->last)
        {
            q->first = NULL;
            q->last = NULL;
        }
        else
        {
            q->first = q->first->next;
        }
        --q->szcoda;
    }
    thr_mutex_unlock(&q->safeins);

    return r;
}

/*****************/
/*** GRESOURCE ***/
/*****************/

err_t thr_resource_init(gresource_s* rs)
{
	dbg("");
	rs->n = 0;
	thr_mutex_init(&rs->syncN);
	return 0;
}

err_t thr_resource_employ(gresource_s* rs)
{
	dbg("");
	thr_mutex_lock(&rs->syncN);
	++rs->n;
	thr_mutex_unlock(&rs->syncN);
	return 0;
}

err_t thr_resource_release(gresource_s* rs)
{
	dbg("");
	thr_mutex_lock(&rs->syncN);
	--rs->n;
	if ( 0 == rs->n ) return 0;
	thr_mutex_unlock(&rs->syncN);
	return rs->n;
}

err_t thr_resource_complete(gresource_s* rs)
{
	dbg("");
	rs->n = 0;
	thr_mutex_unlock(&rs->syncN);
	return 0;
}

err_t thr_resource_destroy(gresource_s* rs)
{
	dbg("");
	thr_mutex_destroy(&rs->syncN);
	return 0;
}

/**************/
/*** THREAD ***/
/**************/

err_t thr_init(thread_s*  thr, thrcall_f thrcall, uint_t stksz, int_t runsuspend, uint_t oncpu)
{
    dbg("");
    iassert( thr != NULL );
    
    thr->stato = T_CREATE;
    thr->fnc = thrcall;

    err_t err = pthread_attr_init(&thr->att);
    if ( err )
    {
		dbg("error: init attribute %d", err);
		return err;
	}
	
    if ( stksz > 0 && (err = pthread_attr_setstacksize (&thr->att, stksz)) ) 
    {
		dbg("error: set stack %d", err);
		return err;
	}
	
	if ( oncpu > 0 )
	{
		cpu_set_t* ncpu = _setcpu(oncpu);
		if ( (err = pthread_attr_setaffinity_np(&thr->att, CPU_SETSIZE, ncpu)) )
		{
			dbg("error: set cpu %d", err);
			return err;
		}
	}
	
    thr->runsuspend = runsuspend;
    thr_event_init(&thr->suspend, 1, 1, 0, 0);
    thr_event_init(&thr->finish, 1, 1, 0, 0);
    return 0;
}


thread_s* thr_new(thrcall_f thrcall, uint_t stksz, int_t runsuspend, uint_t oncpu)
{
    dbg("");
    
    thread_s* thr = NEW(thread_s);
    iassert( thr != NULL );
    
    thr_init(thr, thrcall, stksz, runsuspend, oncpu);
    return thr;
}

err_t thr_run(thread_s* t, void* param)
{
    dbg("");
    iassert( t != NULL );
    
    if (t->stato > T_CREATE && t->stato < T_END) return 0;

    t->stato = T_RUN;
    t->param = param;
    thr_event_reset(&t->suspend);
    thr_event_reset(&t->finish);
    err_t ret = pthread_create(&t->id, &t->att, t->fnc, (void*)t);
    if ( ret )
    {
		dbg("error: pthread fail %d", ret);
		return ret;
	}
	
	
    return 0;
}

void thr_changecpu(thread_s* t, uint_t oncpu)
{
    dbg("");
    iassert( t != NULL );
    
	if ( oncpu == 0 ) return;
	
    cpu_set_t* ncpu = _setcpu(oncpu);
    pthread_attr_setaffinity_np(&t->att, CPU_SETSIZE, ncpu);
}

void* thr_getparam(thread_s* t)
{
    dbg("");
    iassert( t != NULL );
    
    return t->param;
}

void* thr_waitthr(thread_s* t)
{
    dbg("");
    iassert( t != NULL );
    
    void* ret;
    pthread_join(t->id, &ret);
    return ret;
}

void thr_requestwait(thread_s* t)
{
    dbg("");
    iassert( t != NULL );
    
    if (t->stato != T_RUN) return;
    t->stato = T_PAUSE;
}

void thr_resume(thread_s* t)
{
    dbg("");
    iassert( t != NULL );
    
    if (t->stato != T_PAUSE) return;
    thr_event_raise(&t->suspend);
}

void thr_startsuspend(thread_s* t)
{
    dbg("");
    iassert( t != NULL );
    
    if (!t->runsuspend) return;
    thr_suspendme(t);
}

void thr_suspendme(thread_s* t)
{
    dbg("");
    iassert( t != NULL );
    
    t->stato = T_PAUSE;

    thr_event_wait(&t->suspend,0);
    thr_event_reset(&t->suspend);
    t->stato = T_RUN;
}

void thr_chkpause(thread_s* t)
{
    dbg("");
    iassert( t != NULL );
    
    if (t->stato != T_PAUSE) return;
    thr_suspendme(t);
}

int_t thr_chkrequestend(thread_s* t)
{
    dbg("");
    iassert( t != NULL );
    
    if (t->stato != T_REQUESTEXIT) return 0;
    return 1;
}

int_t thr_sleep(double sleep_time)
{
    dbg("");
    
	struct timespec tv;

	tv.tv_sec = (time_t) sleep_time;
	tv.tv_nsec = (long) ((sleep_time - tv.tv_sec) * 1e+9);

	while (1)
	{
		int rval = nanosleep(&tv, &tv);
		if (rval == 0)
			return 0;
		else if (errno == EINTR)
			continue;
		else
			return rval;
	}
	return 0;
}

int_t thr_msleep(uint_t ms)
{
    dbg("");
    
	double rs = (double)(ms) / 1000.0;
    return thr_sleep(rs);
}

int_t thr_nsleep(uint_t ns)
{
    dbg("");
    
	double rs = (double)(ns) / 1000000.0;
    return thr_sleep(rs);
}

int_t thr_signal(int_t enable)
{
	dbg("");
	static sigset_t old;
	
	if ( !enable )
	{
		sigset_t ss;
		sigfillset(&ss);
		if ( pthread_sigmask(SIG_BLOCK, &ss, &old) )
		{
			dbg("error: block signal");
			return -1;
		}
	}
	else
	{
		if ( pthread_sigmask(SIG_SETMASK, &old, NULL) )
		{
			dbg("error: unblock signal");
			return -1;
		}
	}
	return 0;
}

int_t thr_stop(thread_s* t, uint_t ms, int_t forceclose)
{
    dbg("");
    iassert( t != NULL );
    
    if (t->stato == T_END) return 1;
    //if (t->stato != T_RUN) return 0;
    thrmode_e old = t->stato;
    t->stato = T_REQUESTEXIT;
    int_t tr = thr_event_wait(&t->finish, ms);
    if (!tr)
    {
        if (forceclose)
        {
            pthread_cancel(t->id);
            t->stato = T_END;
        }
        else
        {
            t->stato = old;
            return 0;
        }
    }
    thr_event_reset(&t->finish);
    return 1;
}

void thr_exit(thread_s* t,void* ret)
{
    dbg("");
    iassert( t != NULL );
    
    thr_event_raise(&t->finish);
    t->stato=T_END;
    pthread_exit(ret);
}

uint_t thr_ncore()
{
	int_t ncore = sysconf( _SC_NPROCESSORS_ONLN );
	return (ncore <= 0 ) ? 1 : ncore;
}

void thr_destroy(thread_s* t, uint_t ms)
{
    dbg("");
    iassert( t != NULL );
    
    thr_stop(t, ms, 1);
    thr_event_destroy(&t->suspend);
    thr_event_destroy(&t->finish);
    pthread_attr_destroy(&t->att);
}

void thr_free(thread_s* t, uint_t ms)
{
    dbg("");
    iassert( t != NULL );
    
    thr_destroy(t, ms);
    free(t);
}

/***********/
/*** JOB ***/
/***********/

void thr_job_init(job_s* j, int_t nthread, thrcall_f thrcall, uint_t stksz)
{
    dbg("");
    iassert( j != NULL );
    
    j->n = nthread;
    j->j = (thread_s*)malloc(sizeof(thread_s) * j->n);
    int_t i;
    for (i = 0 ; i < j->n; ++i)
    {
        thr_init(&j->j[i], thrcall, stksz, 1, 0);
        j->j[i].param = NULL;
    }
}

job_s* thr_job_new(int_t nthread, thrcall_f thrcall, uint_t stksz)
{
    dbg("");
    
    job_s* j = NEW(job_s);
    iassert( j != NULL );
    
    thr_job_init(j, nthread, thrcall, stksz);
    return j;
}

void thr_job_run(job_s* j)
{
    dbg("");
    iassert( j != NULL );
    
    int_t i;
    for (i = 0; i < j->n; i++)
    {
        thr_run(&j->j[i], j->j[i].param);
    }
}

void thr_job_destroy(job_s* j, uint_t ms)
{
    dbg("");
    iassert( j != NULL );
    
    int_t i;
    for (i = 0 ; i < j->n ; i++)
    {
        thr_destroy(&j->j[i], ms);
    }
    free(j->j);
    j->j = NULL;
}

void thr_job_free(job_s* j, uint_t ms)
{
    dbg("");
    iassert( j != NULL );
    
    thr_job_destroy(j, ms);
    free(j);
}

void thr_job_wait(job_s* j)
{
    dbg("");
    iassert( j != NULL );
    
    int_t i;
    for (i = 0; i < j->n; i++)
    {
        thr_waitthr(&j->j[i]);
    }
}

int_t thr_job_stop(job_s* j, uint_t ms, int_t forceclose)
{
    dbg("");
    iassert( j != NULL );
    
    int_t i;
    for (i = 0; i < j->n ; ++i)
    {
        if (!thr_stop(&j->j[i], ms, forceclose))
            return 0;
    }
    return 1;
}

void thr_job_setparam(job_s* j, uint_t index, void* p)
{
    dbg("");
    iassert( j != NULL );
    
    if ((int_t)index >= j->n) return;
    j->j[index].param = p;
}
