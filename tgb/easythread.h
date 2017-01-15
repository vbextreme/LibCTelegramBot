#ifndef _EASYTHREAD_H_INCLUDED_
#define _EASYTHREAD_H_INCLUDED_

#include <tgb/stdextra.h>

#define THREAD_START(VOIDPARAMNAME,TYPE,NAMEVAR)    thread_s* _this = (thread_s*) VOIDPARAMNAME;\
                                                    thr_startsuspend(_this);\
                                                    TYPE NAMEVAR = (TYPE) thr_getparam(_this)


#define THREAD_END(VOIDRETVAL)     thr_exit(_this,VOIDRETVAL);\
                                   return NULL

#define THREAD_REQUEST()    if (thr_chkrequestend(_this)) goto _LRQEXIT;\
                                thr_chkpause(_this)

#define THREAD_GOEXIT   goto _LRQEXIT

#define THREAD_ONEXIT   _LRQEXIT: \
                        do{}while(0)

#define REQUEST_EXIT() thr_chkrequestend(_this)

#define THR_AUTOCPU 0

#define THR_MUTEN_NAME_MAX 64
#define THR_MUTEN_MAX 43

///il ritorno > 0 è l'attesa in millisecondi
#define THR_WORK_COMPLETE 0
#define THR_WORK_CONTINUE -1
#define THR_WORK_COMPLETE_EXIT -2
#define THR_WORK_SKIP -7
#define THR_WORK_PRIORITY_END -3

#define THR_TALK_MSG_SZ 1024

/*
typedef struct muten muten_s;
typedef struct event event_s;
typedef struct _message message_s;
typedef struct msgqueue msgqueue_s;
typedef struct thread thread_s;
typedef struct job job_s;
*/

typedef void*(*thrcall_f)(void*);

typedef enum thrmode{T_CREATE = 0,T_RUN,T_PAUSE,T_END,T_REQUESTEXIT} thrmode_e;
typedef enum thrmessage{M_CHAR = 0,M_UCHAR,M_PCHAR,M_INT16,M_UINT16,M_PINT16,M_INT32,M_UINT32,M_PINT32,M_DOUBLE,M_PDOUBLE,M_USER} thrmessage_e;

typedef sem_t semaphore_h;
typedef pthread_mutex_t mutex_h;

typedef struct muten
{
	int_t memid;
	char_t* membase;
	char_t* memname;
	pthread_mutex_t* mx;
	pthread_mutexattr_t* att;
}muten_s;

typedef pthread_barrier_t barrier_h;

typedef struct event
{
    mutex_h mtx;
    pthread_cond_t condition;
    int_t autoenter;
    int_t autoexit;
    int_t value;
    int_t resumeall;
}event_s;

typedef struct _message
{
    thrmessage_e type;
    void* msg;
    int_t autofree;
    struct _message* next;
}message_s;

typedef struct msgqueue
{
    message_s* first;
    message_s* last;
    message_s* current;

    mutex_h safeins;
    event_s havemsg;
    int_t szcoda;
}msgqueue_s;

typedef struct gresource
{
	int_t n;
	mutex_h syncN;
}gresource_s;

typedef struct thread
{
    pthread_t id;
    pthread_attr_t att;
    thrcall_f fnc;
    int_t runsuspend;
    event_s suspend;
    event_s finish;
    void* param;
    thrmode_e stato;
}thread_s;

typedef struct job
{
    thread_s* j;
    int_t n;
}job_s;


/*** SEMAPHORE ***/
err_t thr_semaphore_init(semaphore_h* s, uint_t stval);
#if DEBUG_ENABLED == 1
	#define thr_semaphore_wait(SEM) do{dbg("semaphore_wait"); int_t ret = sem_wait(SEM);dbg("semaphore_end_wait::%d", ret);}while(0)
#else
	#define thr_semaphore_wait(SEM) do{dbg("semaphore_wait"); sem_wait(SEM); dbg("semaphore_end_wait");}while(0)
#endif
#define thr_semaphore_post(SEM) do{dbg("semaphore_release"); sem_post(SEM);}while(0)
int_t thr_semaphore_get(semaphore_h* s);
int_t thr_semaphore_count_wait(semaphore_h* s);
#define thr_semaphore_destroy(SEM) do{dbg("semaphore_destroy"); sem_destroy(SEM);}while(0)

/*** MUTEX ***/
#define thr_mutex_init(MUX) do{dbg("mutex_init"); pthread_mutex_init((MUX), NULL);}while(0)
#define thr_mutex_lock(MUX) do{dbg("mutex_lock"); pthread_mutex_lock(MUX);}while(0)
#define thr_mutex_trylock(MUX) pthread_mutex_trylock(MUX)
#define thr_mutex_unlock(MUX) do{dbg("mutex_unlock"); pthread_mutex_unlock((MUX));}while(0)
#define thr_mutex_destroy(MUX) do{dbg("mutex_destroy"); pthread_mutex_destroy((MUX));}while(0)

/*** MUTEN ***/
err_t thr_muten_init(muten_s* h, char_t* filenamespace, char_t* name);
void thr_muten_lock(muten_s* m);
void thr_muten_unlock(muten_s* m);
void thr_muten_destroy(char_t* filenamespace, muten_s* m);
void thr_muten_deatach(muten_s* m);

/*** BARRIER ***/
#define thr_barrier_init(BAR, NTH) do{dbg("barrier_init"); pthread_barrier_init((BAR),NULL,NTH);}while(0)
#define thr_barrier_enter(BAR) do{dbg("barrier_enter"); pthread_barrier_wait((BAR));}while(0)
#define thr_barrier_destroy(BAR) do{dbg("barrier_destroy"); pthread_barrier_destroy((BAR));}while(0)

/*** EVENT ***/
void thr_event_init(event_s* e, int_t autoenter,int_t autoexit,int_t resumeall,int_t value);
void thr_event_destroy(event_s* e);
void thr_event_free(event_s* e);
void thr_event_enter(event_s* e);
void thr_event_exit(event_s* e);
int_t thr_event_wait(event_s* e,int_t timeoutms);
void thr_event_raise(event_s* e);
void thr_event_reset(event_s* e);

/*** MESSAGE ***/
void thr_message_init(message_s* m, thrmessage_e type, void* data, int_t autofree);
message_s* thr_message_new(thrmessage_e type, void* data, int_t autofree);
void thr_message_destroy(message_s* m);
void thr_message_free(message_s* m);
thrmessage_e thr_message_gettype(message_s* m);
void* thr_message_getmsg(message_s* m);
int_t thr_message_getautofree(message_s* m);

/*** MSGQUEUE ***/
void thr_queue_init(msgqueue_s* q);
msgqueue_s* thr_queue_new(void);
void thr_queue_destroy(msgqueue_s* q);
void thr_queue_free(msgqueue_s* q);
 int_t thr_queue_getsize(msgqueue_s* q);
err_t thr_queue_add(msgqueue_s* q, message_s* m);
message_s* thr_queue_getmessage(msgqueue_s* q, uint_t waitms);

/*** GRESOURCE ***/
err_t thr_resource_init(gresource_s* rs);
err_t thr_resource_employ(gresource_s* rs);
err_t thr_resource_release(gresource_s* rs);
err_t thr_resource_complete(gresource_s* rs);
err_t thr_resource_destroy(gresource_s* rs);

/*** THREAD ***/
err_t thr_init(thread_s*  thr, thrcall_f thrcall, uint_t stksz, int_t runsuspend, uint_t oncpu);
thread_s* thr_new(thrcall_f thrcall, uint_t stksz, int_t runsuspend, uint_t oncpu);
err_t thr_run(thread_s* t, void* param);
void thr_changecpu(thread_s* t, uint_t oncpu);
void* thr_getparam(thread_s* t);
void* thr_waitthr(thread_s* t);
void thr_requestwait(thread_s* t);
void thr_resume(thread_s* t);
void thr_startsuspend(thread_s* t);
void thr_suspendme(thread_s* t);
void thr_chkpause(thread_s* t);
int_t thr_chkrequestend(thread_s* t);
int_t thr_sleep(double sleep_time);
int_t thr_msleep(uint_t ms);
int_t thr_nsleep(uint_t ns);
int_t thr_signal(int_t enable);
int_t thr_stop(thread_s* t, uint_t ms, int_t forceclose);
void thr_exit(thread_s* t,void* ret);
uint_t thr_ncore();
void thr_destroy(thread_s* t, uint_t ms);
void thr_free(thread_s* t, uint_t ms);

/*** JOB ***/
void thr_job_init(job_s* j, int_t nthread, thrcall_f thrcall, uint_t stksz);
job_s* thr_job_new(int_t nthread, thrcall_f thrcall, uint_t stksz);
void thr_job_run(job_s* j);
void thr_job_destroy(job_s* j, uint_t ms);
void thr_job_free(job_s* j, uint_t ms);
void thr_job_wait(job_s* j);
int_t thr_job_stop(job_s* j, uint_t ms, int_t forceclose);
void thr_job_setparam(job_s* j, uint_t index, void* p);


#endif // EASYTHREAD_H_INCLUDED
