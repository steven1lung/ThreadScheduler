#include "os2021_thread_api.h"

struct itimerval Signaltimer;
ucontext_t dispatch_context;
ucontext_t timer_context;


int currentID;

size_t n_threads=0;
char* thread_name[20];
int thread_function[20];
char thread_priority[20];
int thread_mode[20];

typedef struct context_t
{
    int TID;
    char name[100];
    char state[15];
    char b_prior[3];
    char c_prior[3];
    int q_time;
    int w_time;
    int cancel_mode;
    int wait_time;
    int time_left;
    int wait;
    int signal_canceled;
    int deleted;
    ucontext_t context;
    struct context_t* next;
    struct context_t* previous;
} context_t;

context_t  *hqueue_head;
context_t *hqueue_rear;
size_t hqueue_total=0;
context_t *mqueue_head;
context_t *current;
context_t *mqueue_rear;
size_t mqueue_total=0;
context_t *lqueue_head;
context_t *lqueue_rear;
size_t lqueue_total=0;
size_t queue_total=0;
context_t hqueue[20];
context_t queue[20];
context_t mqueue[20];
context_t lqueue[20];
context_t waiting[50];
int head=0;
int wait_num=0;
int wait_head=0;

int OS2021_ThreadCreate(char *job_name, char *p_function, char* priority, int cancel_mode)
{
    // printf("create %s be : %d\n",job_name,queue_total);

    context_t* tmp = (context_t*)malloc(sizeof(context_t));
    strcpy(tmp->b_prior,priority);
    strcpy(tmp->c_prior,priority);

    if(!strcmp(tmp->c_prior,"H")) tmp->TID=queue_total;
    else if(!strcmp(tmp->c_prior,"M")) tmp->TID=queue_total;
    else tmp->TID=queue_total;
    strcpy(tmp->name,job_name);
    strcpy(tmp->state,"READY");

    tmp->q_time=0;
    tmp->w_time=0;
    tmp->wait_time=0;
    tmp->cancel_mode=cancel_mode;
    if (!strcmp(tmp->b_prior,"H")) tmp->time_left=1;
    else if (!strcmp(tmp->b_prior,"M")) tmp->time_left=2;
    else tmp->time_left=3;
    tmp->wait=0;
    tmp->signal_canceled=0;
    tmp->deleted=0;
    if(!strcmp(p_function,"Function1")) CreateContext(&tmp->context,NULL,&Function1);
    else if(!strcmp(p_function,"Function2")) CreateContext(&tmp->context,NULL,&Function2);
    else if(!strcmp(p_function,"Function3")) CreateContext(&tmp->context,NULL,&Function3);
    else if(!strcmp(p_function,"Function4")) CreateContext(&tmp->context,NULL,&Function4);
    else if(!strcmp(p_function,"Function5")) CreateContext(&tmp->context,NULL,&Function5);
    else if(!strcmp(p_function,"ResourceReclaim")) CreateContext(&tmp->context,NULL,&ResourceReclaim);
    else return -1;

    if(!strcmp(tmp->c_prior,"L"))
    {
        if(lqueue_total==0)
        {
            lqueue_head=tmp;
            lqueue_rear=tmp;
            tmp->previous=NULL;
            tmp->next=NULL;
        }
        else
        {
            context_t* tmp2 = lqueue_rear;
            tmp2->next=tmp;
            tmp->previous=tmp2;
            tmp->next=NULL;
            lqueue_rear=tmp;
        }


        lqueue_total++;
    }
    else if(!strcmp(tmp->c_prior,"H"))
    {
        if(hqueue_total==0)
        {
            hqueue_head=tmp;
            hqueue_rear=tmp;
            tmp->previous=NULL;
            tmp->next=NULL;
        }
        else
        {
            context_t* tmp2 = hqueue_rear;
            tmp2->next=tmp;
            tmp->previous=tmp2;
            tmp->next=NULL;
            hqueue_rear=tmp;
        }


        hqueue_total++;

    }
    else
    {
        if(mqueue_total==0)
        {
            mqueue_head=tmp;
            mqueue_rear=tmp;
            tmp->previous=NULL;
            tmp->next=NULL;
        }
        else
        {
            context_t* tmp2 = mqueue_rear;
            tmp2->next=tmp;
            tmp->previous=tmp2;
            tmp->next=NULL;
            mqueue_rear=tmp;
        }


        mqueue_total++;
    }



    queue_total++;
    // print_info();

    return tmp->TID;


    // queue[head].TID=head;
    // strcpy(queue[head].name,job_name);
    // strcpy(queue[head].state,"READY");

    // strcpy(queue[head].b_prior,priority);
    // strcpy(queue[head].c_prior,priority);

    // if (!strcmp(queue[head].b_prior,"H")) queue[head].time_left=1;
    // else if (!strcmp(queue[head].b_prior,"M")) queue[head].time_left=2;
    // else queue[head].time_left=3;
    // queue[head].w_time=0;
    // queue[head].wait=0;
    // queue[head].signal_canceled=0;
    // queue[head].cancel_mode=cancel_mode;
    // queue[head].deleted=0;

    // // CreateContext(&queue[head].context,NULL,&Function1);
    // if(!strcmp(p_function,"Function1")) CreateContext(&queue[head].context,NULL,&Function1);
    // else if(!strcmp(p_function,"Function2")) CreateContext(&queue[head].context,NULL,&Function2);
    // else if(!strcmp(p_function,"Function3")) CreateContext(&queue[head].context,NULL,&Function3);
    // else if(!strcmp(p_function,"Function4")) CreateContext(&queue[head].context,NULL,&Function4);
    // else if(!strcmp(p_function,"Function5")) CreateContext(&queue[head].context,NULL,&Function5);
    // else if(!strcmp(p_function,"ResourceReclaim")) CreateContext(&queue[head].context,NULL,&ResourceReclaim);
    // else return -1;

    // return queue[head++].TID;

}

void OS2021_ThreadCancel(char *job_name)
{
    // printf("cancel %s\n",job_name);
    pauseTimer();
    context_t* tmp = hqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->name,job_name) && tmp->cancel_mode==0) strcpy(tmp->state,"TERMINATED");
        else if (!strcmp(tmp->name,job_name) && tmp->cancel_mode==1)
        {

            tmp->signal_canceled=1;
            ResetTimer();
            return;
        }


        tmp=tmp->next;
    }
    tmp = mqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->name,job_name) && tmp->cancel_mode==0) strcpy(tmp->state,"TERMINATED");
        else if (!strcmp(tmp->name,job_name) && tmp->cancel_mode==1)
        {

            tmp->signal_canceled=1;
            ResetTimer();
            return;
        }


        tmp=tmp->next;
    }
    tmp = lqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->name,job_name) && tmp->cancel_mode==0) strcpy(tmp->state,"TERMINATED");
        else if (!strcmp(tmp->name,job_name) && tmp->cancel_mode==1)
        {

            tmp->signal_canceled=1;
            ResetTimer();
            return;
        }


        tmp=tmp->next;
    }



    // for(int i=0;i<head;++i){
    //     if(!strcmp(job_name,queue[i].name) && queue[i].cancel_mode==0) strcpy(queue[i].state,"TERMINATED");
    //     else if (!strcmp(job_name,queue[i].name) && queue[i].cancel_mode==1) queue[i].signal_canceled=1;
    // }
    ResetTimer();
}

void OS2021_ThreadWaitEvent(int event_id)
{
    printf("%s wants to wait for event %d\n",current->name,event_id);
    current->wait=event_id;
    strcpy(current->state,"WAITING");
    if(!strcmp(current->c_prior,"L"))
    {
        printf("The priority of thread %s has changed from L to M\n",current->name);
        strcpy(current->c_prior,"M");
        current->time_left=2;
        context_t* tmp = lqueue_head;
        if(lqueue_total==1)
        {
            lqueue_head=NULL;
            lqueue_rear=NULL;
            lqueue_total--;
        }
        else
        {
            while(tmp->next->TID!=current->TID) tmp=tmp->next;
            tmp->next=current->next;
            if(current->next!=NULL)
                current->next->previous=tmp;
            lqueue_total--;


        }

        if(mqueue_total==0)
        {
            mqueue_head=current;
            mqueue_rear=current;
            current->previous=NULL;
            current->next=NULL;
        }
        else
        {
            context_t* tmp2 = mqueue_rear;
            tmp2->next=current;
            current->previous=tmp2;
            current->next=NULL;
            mqueue_rear=current;
        }
        mqueue_total++;
        //     strcpy(queue[currentID].c_prior,"M");
        //     queue[currentID].time_left=2;
    }
    else if(!strcmp(current->c_prior,"M"))
    {
        printf("The priority of thread %s has changed from M to H\n",current->name);
        strcpy(current->c_prior,"H");
        current->time_left=1;
        context_t* tmp = mqueue_head;
        printf("gogo");
        if(mqueue_total==1)
        {
            mqueue_head=NULL;
            mqueue_rear=NULL;
            mqueue_total--;
        }
        else
        {
            while(tmp->next->TID!=current->TID) tmp=tmp->next;
            tmp->next=current->next;
            if(current->next!=NULL)
                current->next->previous=tmp;
            mqueue_total--;


        }

        if(hqueue_total==0)
        {
            hqueue_head=current;
            hqueue_rear=current;
            current->previous=NULL;
            current->next=NULL;
        }
        else
        {
            context_t* tmp2 = hqueue_rear;
            tmp2->next=current;
            current->next->previous=tmp2;
            current->next=NULL;
            hqueue_rear=current;
        }
        hqueue_total++;
    }
    else
    {
        current->time_left=1;
        hqueue_head=current->next;
        hqueue_head->previous=NULL;
        hqueue_rear->next=current;
        current->next=NULL;
        current->previous=hqueue_rear;
        hqueue_rear=current;
    }
    // else if(!strcmp(queue[currentID].c_prior,"M")) {
    //     printf("The priority of thread %s has changed from M to H\n",queue[currentID].name);
    //     strcpy(queue[currentID].c_prior,"H");
    //     queue[currentID].time_left=1;
    // }
    // else queue[currentID].time_left=1;


    // waiting[wait_num]=queue[currentID];
    // wait_num++;
    scheduler();

}

void OS2021_ThreadSetEvent(int event_id)
{
    pauseTimer();
    printf("set %d \n",event_id);
    context_t* tmp = hqueue_head;
    while(tmp!=NULL)
    {
        if(tmp->wait==event_id && !strcmp(tmp->state,"WAITING"))
        {
            printf("%s changes the status of %s to READY\n",current->name,tmp->name);
            strcpy(tmp->state,"READY");
            ResetTimer();
            return;
        }
        tmp=tmp->next;
    }
    tmp = mqueue_head;
    while(tmp!=NULL)
    {
        if(tmp->wait==event_id && !strcmp(tmp->state,"WAITING"))
        {
            printf("%s changes the status of %s to READY\n",current->name,tmp->name);
            strcpy(tmp->state,"READY");
            ResetTimer();
            return;
        }
        tmp=tmp->next;
    }
    tmp = lqueue_head;
    while(tmp!=NULL)
    {
        if(tmp->wait==event_id && !strcmp(tmp->state,"WAITING"))
        {
            printf("%s changes the status of %s to READY\n",current->name,tmp->name);
            strcpy(tmp->state,"READY");
            ResetTimer();
            return;
        }
        tmp=tmp->next;
    }




    // int i=wait_head;
    // for(i=0;i<wait_num;i++){
    //     if(waiting[i].wait==event_id && !strcmp(waiting[i].state,"WAITING") && !strcmp(waiting[i].c_prior,"H")){
    //         printf("%s changes the status of %s to READY\n",queue[currentID].name,waiting[i].name);
    //         strcpy(queue[waiting[i].TID].state,"READY");
    //         strcpy(waiting[i].state,"READY");
    //         wait_num--;
    //         return;
    //     }
    // }
    // for(i=0;i<wait_num;i++){
    //     if(waiting[i].wait==event_id && !strcmp(waiting[i].state,"WAITING") && !strcmp(waiting[i].c_prior,"M")){
    //         printf("%s changes the status of %s to READY\n",queue[currentID].name,waiting[i].name);
    //         strcpy(queue[waiting[i].TID].state,"READY");
    //         strcpy(waiting[i].state,"READY");
    //         wait_num--;
    //         return;
    //     }
    // }
    // for(i=0;i<wait_num;i++){
    //     if(waiting[i].wait==event_id && !strcmp(waiting[i].state,"WAITING") && !strcmp(waiting[i].c_prior,"L")){
    //         printf("%s changes the status of %s to READY\n",queue[currentID].name,waiting[i].name);
    //         strcpy(queue[waiting[i].TID].state,"READY");
    //         strcpy(waiting[i].state,"READY");
    //         wait_num--;
    //         return;
    //     }
    // }


}

void OS2021_ThreadWaitTime(int msec)
{
    strcpy(current->state,"WAITING");
    current->wait_time=msec*10;
    scheduler();
}

void OS2021_DeallocateThreadResource()
{

    pauseTimer();
    context_t* tmp = hqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"TERMINATED"))
        {
            printf("The memory space of %s has been released\n",tmp->name);
            if(hqueue_total==1)
            {
                hqueue_head=NULL;
                hqueue_rear=NULL;

                hqueue_total--;
                printf("The memory space of %s has been released\n",tmp->name);
                ResetTimer();
                return;
            }
            else if(tmp==hqueue_rear)
            {
                tmp = hqueue_rear->previous;
                tmp->next=NULL;
                hqueue_total--;
                printf("The memory space of %s has been released\n",tmp->name);
                ResetTimer();
                return;
            }
            else if(hqueue_total>1)
            {
                printf("The memory space of %s has been released\n",tmp->name);
                context_t* tmp2 = tmp->previous;
                printf("a\n");
                tmp2->next=tmp->next;
                printf("a\n");
                if(tmp->next!=NULL)
                    tmp->next->previous=tmp2;
                lqueue_total--;
                print_info();
                ResetTimer();
                return;
            }

        }

        tmp=tmp->next;
    }
    tmp = mqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"TERMINATED"))
        {
            // printf("The memory space of %s has been released\n",tmp->name);
            if(hqueue_total==1)
            {
                mqueue_head=NULL;
                mqueue_rear=NULL;

                mqueue_total--;
                printf("The memory space of %s has been releaseda\n",tmp->name);
                ResetTimer();
                return;
            }
            else if(tmp==mqueue_rear)
            {
                tmp = mqueue_rear->previous;
                tmp->next=NULL;
                mqueue_total--;
                printf("The memory space of %s has been releasedaa\n",tmp->name);
                ResetTimer();
                return;
            }
            else if(hqueue_total>1)
            {
                printf("The memory space of %s has beendd released\n",tmp->name);
                context_t* tmp2 = tmp->previous;
                printf("a\n");
                tmp2->next=tmp->next;
                printf("a\n");
                if(tmp->next!=NULL)
                    tmp->next->previous=tmp2;
                lqueue_total--;
                print_info();
                ResetTimer();
                return;
            }


        }
        tmp=tmp->next;
    }
    tmp = lqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"TERMINATED"))
        {
            printf("%d hehe\n",lqueue_total);

            if(lqueue_total==1)
            {
                lqueue_head=NULL;
                lqueue_rear=NULL;

                lqueue_total--;
                printf("The memory space of %s has been releasezzzz\n",tmp->name);
                ResetTimer();
                return;
            }
            else if(lqueue_total>1)
            {
                if(tmp==lqueue_head)
                {
                    context_t* tmp2 =tmp->next;

                    tmp2->previous=NULL;
                    lqueue_head=tmp2;
                    lqueue_total--;
                    printf("The memory space of %s has been releaseddd\n",tmp->name);
                    ResetTimer();
                    return;
                }
                else if (tmp==lqueue_rear)
                {
                    context_t* tmp2 = lqueue_rear->previous;
                    tmp2->next=NULL;
                    lqueue_rear=tmp2;
                    lqueue_total--;
                    printf("The memory space of %s has been releaseabc\n",tmp->name);
                    ResetTimer();
                    return;
                }
                else
                {
                    context_t* tmp2 = tmp->previous;
                    printf("a\n");
                    tmp2->next=tmp->next;
                    printf("a\n");

                    tmp->next->previous=tmp2;
                    lqueue_total--;
                    print_info();
                    printf("The memory space of %s has beenasd released\n",tmp->name);
                    ResetTimer();
                    return;
                }




            }





        }
        tmp=tmp->next;
    }

    // for(int i=0;i<20;++i){
    //     if(!strcmp(queue[i].state,"TERMINATED")){
    //         // free(&queue[i].context);
    //         queue[i].deleted=1;
    //         printf("The memory space of %s has been released\n",queue[i].name);
    //     }
    // }
    ResetTimer();
}

void OS2021_TestCancel()
{
    if(current->signal_canceled==1  )
    {
        // printf("%s test cancel\n",current->name);
        strcpy(current->state,"TERMINATED");
        return;
    }



    // if(queue[currentID].signal_canceled==1 ) {
    //     strcpy(queue[currentID].state,"TERMINATED");
    //     scheduler();
    // }
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
void pauseTimer()
{
    Signaltimer.it_value.tv_sec = 0;
    Signaltimer.it_value.tv_usec = 0;
    if(setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
    {
        printf("ERROR SETTING TIME SIGALRM!\n");
    }
}

void ResetTimer()
{
    Signaltimer.it_value.tv_sec = 0;
    Signaltimer.it_value.tv_usec = 10;
    if(setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
    {
        printf("ERROR SETTING TIME SIGALRM!\n");
    }
}

void add_qtime()
{
    context_t* tmp = hqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"WAITING"))
        {
            tmp->w_time+=10;
            if(tmp->wait_time>0)
            {
                tmp->wait_time-=10;
                if(tmp->wait_time<=10)
                {
                    strcpy(tmp->state,"READY");
                    tmp->wait_time=0;
                }
            }
        }
        else if(!strcmp(tmp->state,"READY")) tmp->q_time+=10;
        tmp=tmp->next;
    }

    tmp = mqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"WAITING"))
        {
            tmp->w_time+=10;
            if(tmp->wait_time>0)
            {
                tmp->wait_time-=10;
                if(tmp->wait_time<=10)
                {
                    strcpy(tmp->state,"READY");
                    tmp->wait_time=0;
                }
            }
        }
        else if(!strcmp(tmp->state,"READY")) tmp->q_time+=10;
        tmp=tmp->next;
    }

    tmp = lqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"WAITING"))
        {
            tmp->w_time+=10;
            if(tmp->wait_time>0)
            {
                tmp->wait_time-=10;
                if(tmp->wait_time<=10)
                {
                    strcpy(tmp->state,"READY");
                    tmp->wait_time=0;
                }
            }
        }
        else if(!strcmp(tmp->state,"READY")) tmp->q_time+=10;
        tmp=tmp->next;
    }

    current->time_left-=1;
    if(current->time_left<=0)
    {
        strcpy(current->state,"READY");
        if(!strcmp(current->c_prior,"M"))
        {
            strcpy(current->c_prior,"L");
            current->time_left=1;

            context_t* tmp2 = mqueue_head;
            if(mqueue_total==1)
            {
                mqueue_head=NULL;
                mqueue_rear=NULL;
                mqueue_total--;
            }
            else
            {
                while(tmp2->next->TID!=current->TID) tmp2=tmp2->next;
                tmp2->next=current->next;
                if(current->next!=NULL)
                    current->next->previous=tmp2;

                mqueue_total--;
            }

            if(lqueue_total==0)
            {
                lqueue_head=current;
                lqueue_rear=current;
                current->previous=NULL;
                current->next=NULL;
            }
            else
            {
                context_t* tmp2 = lqueue_rear;
                tmp2->next=current;
                current->previous=tmp2;
                current->next=NULL;
                lqueue_rear=current;
            }
            lqueue_total++;
        }
        else if(!strcmp(current->c_prior,"H") )
        {
            strcpy(current->c_prior,"M");
            current->time_left=2;

            context_t* tmp2 = hqueue_head;
            if(hqueue_total==1)
            {
                hqueue_head=NULL;
                hqueue_rear=NULL;
                hqueue_total--;
            }
            else
            {
                while(tmp2->next->TID!=current->TID) tmp2=tmp2->next;
                tmp2->next=current->next;
                if(current->next!=NULL)
                    current->next->previous=tmp2;

                hqueue_total--;
            }

            if(mqueue_total==0)
            {
                mqueue_head=current;
                mqueue_rear=current;
                current->previous=NULL;
                current->next=NULL;
            }
            else
            {
                context_t* tmp2 = mqueue_rear;
                tmp2->next=current;
                current->previous=tmp2;
                current->next=NULL;
                mqueue_rear=current;
            }
            mqueue_total++;
        }
        else
        {
            current->time_left=3;
            lqueue_head=current->next;
            lqueue_rear->next=current;
            current->previous=lqueue_rear;
            current->next=NULL;
            lqueue_rear=current;
        }
        scheduler();
    }



// //dd
//     for(int i=0;i<head;++i) {
//         if(!strcmp(queue[i].state,"WAITING")) queue[i].w_time+=10;
//         else if(!strcmp(queue[i].state,"READY")) queue[i].q_time+=10;

//     }
//     queue[currentID].time_left-=1;
//     if(queue[currentID].time_left<=0) {

//         if(!strcmp(queue[currentID].c_prior,"H")) {
//             strcpy(queue[currentID].c_prior,"M");
//             queue[currentID].time_left=2;
//         }
//         else if(!strcmp(queue[currentID].c_prior,"M")) {
//             strcpy(queue[currentID].c_prior,"L");
//             queue[currentID].time_left=3;
//         }
//         else{
//             queue[currentID].time_left=3;

//         }

//         strcpy(queue[currentID].state,"READY");


    // strcpy(queue[head].name,queue[currentID].name);
    // strcpy(queue[head].state,"READY");
    // strcpy(queue[head].b_prior,queue[currentID].b_prior);
    // strcpy(queue[head].c_prior,queue[currentID].c_prior);
    // queue[head].time_left=queue[currentID].time_left;

    // queue[head].w_time=queue[currentID].w_time;
    // queue[head].wait=queue[currentID].wait;
    // queue[head].signal_canceled=queue[currentID].signal_canceled;
    // queue[head].cancel_mode=queue[currentID].cancel_mode;
    // queue[head].TID=queue[currentID].TID;
    // queue[head].context=queue[currentID].context;
    // head++;

    // scheduler();



}

void print_info()
{
    // printf("%d nodes\n",queue_total);
    printf("\n************************************************************************************************************\n");
    printf("*%15s%15s%15s%15s%15s%15s%15s *\n","TID","Name","State","B_Priority","C_Priority","Q_Time","W_Time");
    // printf("*\tTID\tName\tState\tB_Priority\tC_Priority\tQ_Time\tW_Time  *\n");
    context_t* tmp=hqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"TERMINATED"))
        {
            tmp=tmp->next;
            continue;
        }
        printf("*%15d%15s%15s%15s%15s%15d%15d *\n",tmp->TID, tmp->name, tmp->state, tmp->b_prior, tmp->c_prior, tmp->q_time, tmp->w_time);

        tmp=tmp->next;
    }
    tmp=mqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"TERMINATED"))
        {
            tmp=tmp->next;
            continue;
        }
        printf("*%15d%15s%15s%15s%15s%15d%15d *\n",tmp->TID, tmp->name, tmp->state, tmp->b_prior, tmp->c_prior, tmp->q_time, tmp->w_time);

        tmp=tmp->next;
    }
    tmp=lqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"TERMINATED"))
        {
            tmp=tmp->next;
            continue;
        }
        printf("*%15d%15s%15s%15s%15s%15d%15d *\n",tmp->TID, tmp->name, tmp->state, tmp->b_prior, tmp->c_prior, tmp->q_time, tmp->w_time);

        tmp=tmp->next;
    }
    // for(int i=0;i<head;i++){
    //     if(queue[i].deleted==1) continue;
    //     if(!strcmp(queue[i].state,"TER")) continue;
    //     printf("*%15d%15d%15s%15s%15s%15s%15d%15d *\n",queue[i].TID,queue[i].time_left, queue[i].name, queue[i].state, queue[i].b_prior, queue[i].c_prior, queue[i].q_time, queue[i].w_time);
    // }
    printf("************************************************************************************************************\n");
}

void sigroutine(int signo)
{
    if(signo == SIGTSTP)
    {
        print_info();
    }
    else if (signo == SIGALRM)
    {
        // printf("alarm\n");
        add_qtime();
        // print_info();
    }

    ResetTimer();
}


void Dispatcher()
{

    FILE *fp;
    fp = fopen("init_threads.json","r");
    char rdname[100];
    char rdfunc[100];
    char rdpriority[100];
    char rdmode[100];
    char rdline[100];
    fgets(rdline,100,fp);
    fgets(rdline,100,fp);

    while(1)
    {

        fgets(rdline,100,fp);

        fgets(rdname,100,fp);
        char* name = strtok(rdname,":");
        name = strtok(NULL,":");
        name = strtok(name,"\"");
        name = strtok(NULL,"\"");

        fgets(rdfunc,100,fp);
        char* func = strtok(rdfunc,":");
        func = strtok(NULL,":");
        func = strtok(func,"\"");
        func = strtok(NULL,"\"");

        fgets(rdpriority,100,fp);
        char priority = rdpriority[24];




        fgets(rdmode,100,fp);
        char mode = rdmode[27];

        fgets(rdline,100,fp);
        // printf("%s\n%s\n%c\n%c\n",name,func,priority,mode);
        thread_name[n_threads]=name;
        thread_function[n_threads]=func[8]-'0';
        thread_priority[n_threads]=priority;
        thread_mode[n_threads]=mode-'0';
        // printf("%s%d%c%d\n",thread_name[n_threads],thread_function[n_threads],thread_priority[n_threads],thread_mode[n_threads]);
        char tfunc[20];
        strcpy(tfunc,"Function");
        strncat(tfunc,&func[8],1);
        char pr[3];
        strncpy(pr,&priority,1);
        printf("thread func: %s\n",tfunc);
        OS2021_ThreadCreate(name,tfunc,pr,mode-'0');
        ++n_threads;
        if(rdline[9]!=',') break;
        memset(rdline,0,100);
        memset(rdname,0,100);
        memset(rdfunc,0,100);
        memset(rdpriority,0,100);
        memset(rdmode,0,100);
        memset(tfunc,0,20);
        memset(pr,0,3);

    }
    printf("Finish parse. There are total %ld threads\n",n_threads);
    fclose(fp);



}



void first_scheduler()
{


    context_t* tmp = hqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"READY"))
        {
            strcpy(tmp->state,"RUNNING");
            currentID=tmp->TID;
            current=tmp;
            setcontext(&tmp->context);
            return;
        }
        tmp=tmp->next;
    }
    tmp = mqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"READY"))
        {
            strcpy(tmp->state,"RUNNING");
            currentID=tmp->TID;
            current=tmp;
            setcontext(&tmp->context);
            return;
        }
        tmp=tmp->next;
    }
    tmp = lqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"READY"))
        {
            strcpy(tmp->state,"RUNNING");
            currentID=tmp->TID;
            current=tmp;
            setcontext(&tmp->context);
            return;
        }
        tmp=tmp->next;
    }


    // for(int i=0;i<3;++i){
    //     for(int j=0;j<head;++j){
    //         if(i==0){
    //             if(!strcmp(queue[j].c_prior,"H")&& !strcmp(queue[j].state,"READY")) {


    //                 strcpy(queue[j].state,"RUNNING");
    //                 currentID=queue[j].TID;
    //                 setcontext(&queue[j].context);
    //                 return;

    //             }
    //         }
    //         else if(i==1){
    //             if(!strcmp(queue[j].c_prior,"M")&& !strcmp(queue[j].state,"READY")) {

    //                 strcpy(queue[j].state,"RUNNING");



    //                 currentID=queue[j].TID;
    //                 setcontext(&queue[j].context);
    //                 return;


    //             }
    //         }
    //         else if(i==2){
    //             if(!strcmp(queue[j].c_prior,"L")&& !strcmp(queue[j].state,"READY")) {
    //                 strcpy(queue[j].state,"RUNNING");



    //                 currentID=queue[j].TID;
    //                 setcontext(&queue[j].context);
    //                 return;


    //             }


    //         }
    //     }
    // }
}
void scheduler()
{
    context_t* tmp = hqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"READY"))
        {
            strcpy(tmp->state,"RUNNING");
            // printf("swap from %d to %d\n",current->TID,tmp->TID);
            int temp=currentID;
            ResetTimer();
            context_t* tmp2 = current;
            current = tmp;
            swapcontext(&tmp2->context,&tmp->context);

            return;
        }
        tmp=tmp->next;
    }
    tmp = mqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"READY"))
        {
            strcpy(tmp->state,"RUNNING");
            // printf("swap from %d to %d\n",current->TID,tmp->TID);
            int temp=currentID;
            ResetTimer();
            context_t* tmp2 = current;
            current = tmp;
            swapcontext(&tmp2->context,&tmp->context);

            return;
        }
        tmp=tmp->next;
    }
    tmp = lqueue_head;
    while(tmp!=NULL)
    {
        if(!strcmp(tmp->state,"READY"))
        {
            strcpy(tmp->state,"RUNNING");
            // printf("swap from %d to %d\n",current->TID,tmp->TID);
            int temp=currentID;
            ResetTimer();
            context_t* tmp2 = current;
            current = tmp;
            swapcontext(&tmp2->context,&tmp->context);

            return;
        }
        tmp=tmp->next;
    }



    // printf("schedule now %d \n",currentID);
    // int j=currentID+1;

    // while(1){

    //     if(!strcmp(queue[j].c_prior,"H")&& !strcmp(queue[j].state,"READY")) {

    //             strcpy(queue[j].state,"RUNNING");

    //             printf("swap from %d to %d\n",currentID,queue[j].TID);
    //             int temp=currentID;
    //             currentID=queue[j].TID;
    //             ResetTimer();
    //             swapcontext(&queue[temp].context,&queue[j].context);
    //             return;

    //     }
    //     j++;
    //     if(j==20) j=0;
    //     if(j==currentID) break;
    // }

    // j=currentID+1;
    // while(1){

    //     if(!strcmp(queue[j].c_prior,"M")&& !strcmp(queue[j].state,"READY")) {

    //             strcpy(queue[j].state,"RUNNING");

    //             printf("swap from %d to %d\n",currentID,queue[j].TID);
    //             int temp=currentID;
    //             currentID=queue[j].TID;
    //             ResetTimer();
    //             swapcontext(&queue[temp].context,&queue[j].context);
    //             return;

    //     }
    //     j++;
    //     if(j==20) j=0;
    //     if(j==currentID) break;
    // }
    // j=currentID+1;
    // while(1){

    //     if(!strcmp(queue[j].c_prior,"L")&& !strcmp(queue[j].state,"READY")) {

    //             strcpy(queue[j].state,"RUNNING");

    //             printf("swap from %d to %d\n",currentID,queue[j].TID);
    //             int temp=currentID;
    //             currentID=queue[j].TID;
    //             ResetTimer();
    //             swapcontext(&queue[temp].context,&queue[j].context);
    //             return;

    //     }
    //     j++;
    //     if(j==20) j=0;
    //     if(j==currentID) break;
    // }


}

//H 100
//M 200
//L 300
void StartSchedulingSimulation()
{
    signal(SIGTSTP,sigroutine);
    signal(SIGALRM,sigroutine);
    printf("start simulate\n");
    /*Set Timer*/
    OS2021_ThreadCreate("reclaimer","ResourceReclaim","L",1);
    ResetTimer();
    print_info();
    first_scheduler();

}
