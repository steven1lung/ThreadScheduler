#include "os2021_thread_api.h"

struct itimerval Signaltimer;
ucontext_t dispatch_context;
ucontext_t timer_context;

int OS2021_ThreadCreate(char *job_name, char *p_function, int priority, int cancel_mode)
{
    return -1;
}

void OS2021_ThreadCancel(char *job_name)
{

}

void OS2021_ThreadWaitEvent(int event_id)
{

}

void OS2021_ThreadSetEvent(int event_id)
{

}

void OS2021_ThreadWaitTime(int msec)
{

}

void OS2021_DeallocateThreadResource()
{

}

void OS2021_TestCancel()
{

}

void CreateContext(ucontext_t *context, ucontext_t *next_context, void *func)
{
    getcontext(context);
    context->uc_stack.ss_sp = malloc(STACK_SIZE);
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = next_context;
    makecontext(context,(void (*)(void))func,0);
}

void ResetTimer()
{
    Signaltimer.it_value.tv_sec = 0;
    Signaltimer.it_value.tv_usec = 0;
    if(setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
    {
        printf("ERROR SETTING TIME SIGALRM!\n");
    }
}

void Dispatcher()
{

}

void StartSchedulingSimulation()
{
    /*Set Timer*/
    Signaltimer.it_interval.tv_usec = 0;
    Signaltimer.it_interval.tv_sec = 0;
    ResetTimer();
    /*Create Context*/
    CreateContext(&dispatch_context, &timer_context, &Dispatcher);
    setcontext(&dispatch_context);
}
