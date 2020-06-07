#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>


/*constants*/
#define MAX_CUSTOMER 20       /* Limit of work for customers */
#define HAIRCUT_TIME 5          /* Time for a haircut */

/* semaphores */
sem_t f_barbers;                /* semaphore for barbers */
sem_t m_barbers;
sem_t u_barbers;
sem_t mutex;                    /* provides mutual exclusive access to the barber chair */

/* functions */
void female_barber_process(void* number);
void male_barber_process(void* number);
void unisex_barber_process(void* number);
void customer_process(void* number);
void time_between_customers();

/* variables */
int number_of_daily_customers;
int customer_chair_amount;         /* number of chairs in the waiting room */
int number_of_f_barbers;
int number_of_m_barbers;
int number_of_u_barbers;
int number_of_empty_customer_chairs;      /* number of empty seats in the waiting room */
int* chair;                    /* for identity exchange between the barber and the customer */
int f_customer_queue[MAX_CUSTOMER];
int m_customer_queue[MAX_CUSTOMER];
int f_head_of_q = 0;
int m_head_of_q = 0;
int f_tail_of_q = 0;
int m_tail_of_q = 0;
int number_of_customers_served = 0;

//-----------------------------------------------------------------------------------

int main(int argc, char** args)
{
    if (argc != 6)
    {
        printf("\nUsage error!\nUsage:\t sleeping-barber <Number of Customers> <Number of Waiting Room Chairs> <Women Barber Count> <Men Barber Count> <Unisex Barber Count>\n\n");
        return EXIT_FAILURE;
    }

    number_of_daily_customers = atoi(args[1]);
    customer_chair_amount = atoi(args[2]);
    number_of_f_barbers = atoi(args[3]);
    number_of_m_barbers = atoi(args[4]);
    number_of_u_barbers = atoi(args[5]);
    number_of_empty_customer_chairs = customer_chair_amount;
    chair = (int*) malloc(sizeof(int) * customer_chair_amount);

    for(int j=0; j<customer_chair_amount; j++)
        chair[j] = 0;

    for(int j=0; j < MAX_CUSTOMER; j++) /* female queue */
        f_customer_queue[j] = 0;

    for(int j=0; j < MAX_CUSTOMER; j++) /* male queue */
        m_customer_queue[j] = 0;

    if (number_of_daily_customers > MAX_CUSTOMER)
    {
        printf("\nMaximum customer: %d\n\n", MAX_CUSTOMER);
        printf("\nUsage error!\nUsage:\t sleeping-barber <Number of Customers> <Number of Waiting Room Chairs> <Women Barber Count> <Men Barber Count> <Unisex Barber Count>\n\n");
        return EXIT_FAILURE;
    }

    printf("\n\nToday we are going to serve %d customers.", number_of_daily_customers);
    printf("\nWe currently have %d waiting room chairs.", customer_chair_amount);
    printf("\nTotal number of barbers on duty is %d.\n\n", number_of_f_barbers + number_of_m_barbers + number_of_u_barbers);
    

    pthread_t f_barber_thread[number_of_f_barbers], m_barber_thread[number_of_m_barbers], u_barber_thread[number_of_u_barbers], customer_thread[number_of_daily_customers]; /* identify a thread */

    /* creating semaphores */
    sem_init(&f_barbers, 0, 0);
    sem_init(&m_barbers, 0, 0);
    sem_init(&u_barbers, 0, 0);
    sem_init(&mutex, 0, 1);

    printf("\nBarber shop is opened.\n\n");

    /* creation of female barber processes */
    for (int i = 1; i <= number_of_f_barbers; i++)
    {
        pthread_create(&f_barber_thread[i], NULL, (void*)female_barber_process, (void*)&i);
        sleep(1);
    }

    /* creation of male barber processes */
    for (int i = 1; i <= number_of_m_barbers; i++)
    {
        pthread_create(&m_barber_thread[i], NULL, (void*)male_barber_process, (void*)&i);
        sleep(1);
    }

    /* creation of unisex barber processes */
    for (int i = 1; i <= number_of_u_barbers; i++)
    {
        pthread_create(&u_barber_thread[i], NULL, (void*)unisex_barber_process, (void*)&i);
        sleep(1);
    }


    /* creation of customer processes */
    for (int i = 1; i <= number_of_daily_customers; i++)
    {
        pthread_create(&customer_thread[i], NULL, (void*)customer_process, (void*)&i);
        time_between_customers();    /* to create customers at random intervals */
    }

    while(number_of_customers_served != number_of_daily_customers)
        sleep(1);

    printf("\nAll customers were served. Barber shop is closed. Barbers left the shop.\n\n");

    return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------------

void female_barber_process(void* number)
{
    int s = *(int*)number;
    int customer_id;
    int sem_count1;
    int sem_count2;


    printf("[Female barber: %d]\tentered the shop.\n", s);

    while (1)
    {
        if(f_head_of_q == f_tail_of_q)
        {
            sem_getvalue(&f_barbers, &sem_count1);
            if(sem_count1 == 0)
            {
               printf("[Female barber: %d]\twent to sleep.\n\n", s);
            }
            //sem_getvalue(&f_barbers, &sem_count1);
            //printf("Debug 1 %d\n", sem_count1);

            sem_wait(&f_barbers);   /* join the sleeping barbers */

            //sem_getvalue(&f_barbers, &sem_count2);
            //printf("Debug 2 %d\n", sem_count2);
        }

        else
        {
            sem_wait(&mutex);       /* lock access to seat */
            customer_id = f_customer_queue[f_head_of_q]; /* selection of the customer to be served among the waiting */
            f_head_of_q += 1;
        
            for(int j=0; j<customer_chair_amount; j++)
            {
                if(customer_id == chair[j])
                   chair[j] = 0;
            }

          
            number_of_empty_customer_chairs++;

            sem_post(&mutex);       /* unlock access to seat */

            printf("[Female barber: %d]\tstarted cutting %d. female customer's hair.\n\n", s, customer_id);
            sleep(HAIRCUT_TIME);
            number_of_customers_served++;
            printf("[Female barber: %d]\tfinished %d. female customer's hair.\n\n", s, customer_id);
        }
    }
}

//-----------------------------------------------------------------------------------

void male_barber_process(void* number)
{
    int s = *(int*)number;
    int customer_id;
    int sem_count1;
    int sem_count2;


    printf("[Male barber: %d]\tentered the shop.\n", s);

    while (1)
    {
        if (m_head_of_q == m_tail_of_q)
        {
            sem_getvalue(&m_barbers, &sem_count1);
            if(sem_count1 == 0)
            {
               printf("[Male barber: %d]\twent to sleep.\n\n", s);
            }

            //sem_getvalue(&m_barbers, &sem_count1);
            //printf("Debug 3 %d\n", sem_count1);

            sem_wait(&m_barbers);   /* join the sleeping barbers */

            //sem_getvalue(&m_barbers, &sem_count2);
            //printf("Debug 4 %d\n", sem_count2);
        }

        else
        {
            sem_wait(&mutex);       /* lock access to seat */
            customer_id = m_customer_queue[m_head_of_q]; /* selection of the customer to be served among the waiting */
            m_head_of_q += 1;
        
            for(int j=0; j<customer_chair_amount; j++)
            {
                if(customer_id == chair[j])
                   chair[j] = 0;
            }

          
            number_of_empty_customer_chairs++;

            sem_post(&mutex);       /* unlock access to seat */

            printf("[Male barber: %d]\tstarted cutting %d. male customer's hair.\n\n", s, customer_id);
            sleep(HAIRCUT_TIME);
            number_of_customers_served++;
            printf("[Male barber: %d]\tfinished %d. male customer's hair.\n\n", s, customer_id);
        }
    }
}

//-----------------------------------------------------------------------------------

void unisex_barber_process(void* number)
{
    int s = *(int*)number;
    int customer_id;
    int sem_count1;
    int sem_count2;


    printf("[Unisex barber: %d]\tentered the shop.\n", s);

    while (1)
    {
        if ((f_head_of_q == f_tail_of_q) && (m_head_of_q == m_tail_of_q))
        {
        sem_getvalue(&u_barbers, &sem_count1);
        if(sem_count1 == 0)
        {
            printf("[Unisex barber: %d]\twent to sleep.\n\n", s);
        }
            //sem_getvalue(&u_barbers, &sem_count1);
            //printf("Debug 5 %d\n", sem_count1);

            sem_wait(&u_barbers);   /* join the sleeping barbers */

            //sem_getvalue(&u_barbers, &sem_count2);
            //printf("Debug 6 %d\n", sem_count2);
        }

        else
        {
            if(f_head_of_q != f_tail_of_q)
            {
               sem_wait(&mutex);       /* lock access to seat */
               customer_id = f_customer_queue[f_head_of_q]; /* selection of the customer to be served among the waiting */
               f_head_of_q += 1;
        
               for(int j=0; j<customer_chair_amount; j++)
               {
                   if(customer_id == chair[j])
                      chair[j] = 0;
               }

          
               number_of_empty_customer_chairs++;

               sem_post(&mutex);       /* unlock access to seat */

               printf("[Unisex barber: %d]\tstarted cutting %d. female customer's hair.\n\n", s, customer_id);
               sleep(HAIRCUT_TIME);
               number_of_customers_served++;
               printf("[Unisex barber: %d]\tfinished %d. female customer's hair.\n\n", s, customer_id);
            }
            else
            {
               sem_wait(&mutex);       /* lock access to seat */
               customer_id = m_customer_queue[m_head_of_q]; /* selection of the customer to be served among the waiting */
               m_head_of_q += 1;
        
               for(int j=0; j<customer_chair_amount; j++)
               {
                   if(customer_id == chair[j])
                      chair[j] = 0;
               }

          
               number_of_empty_customer_chairs++;

               sem_post(&mutex);       /* unlock access to seat */

               printf("[Unisex barber: %d]\tstarted cutting %d. male customer's hair.\n\n", s, customer_id);
               sleep(HAIRCUT_TIME);
               number_of_customers_served++;
               printf("[Unisex barber: %d]\tfinished %d. male customer's hair.\n\n", s, customer_id);
            }
        }
    }
}

//-----------------------------------------------------------------------------------

void customer_process(void* number)
{
    int s = *(int*)number;
    char sex;
    int seated_chair;
    int sem_count1;
    int sem_count2;

    time_t t;

    /* Intializes random number generator */
    srand((unsigned) time(&t));
    if(rand() % 2 >= 1 )
    {
       sex = 'f';
       printf("[Customer: %d]\t female customer entered the shop.\n", s);
    }
    else 
    {
       sex = 'm';
       printf("[Customer: %d]\t male customer entered the shop.\n", s);
     }

    sem_wait(&mutex);   /* Lock access to keep seat */

    /* if there are empty chairs in the waiting room */
    if (number_of_empty_customer_chairs > 0)
    {
        

        printf("[Customer: %d]\twaits in waiting room.\n\n", s);

        /* choose a chair from the waiting room and sit (p get v release) */
        for(int i=0; i < customer_chair_amount; i++)
        { 
          if (chair[i] == 0){
          	chair[i] = s;
                seated_chair = i;
		number_of_empty_customer_chairs--;
		break;
          }
	 }

        if(sex == 'f')
        {
           f_customer_queue[f_tail_of_q] = s;
           f_tail_of_q = (++f_tail_of_q);
           sem_post(&mutex);           /* unlock access to seat */  
   
            //sem_getvalue(&f_barbers, &sem_count1);
            //printf("GET VALUE1 %d \n\n", sem_count1);
            //printf("Debug p7\n");

           sem_post(&f_barbers);           /* wake up the available barber (error code) */

            //printf("Debug p8\n");

           usleep(1000);

            //printf("Debug p9\n");

           sem_post(&u_barbers);

            //printf("Debug p10 \n");
        }
        else
        {
           m_customer_queue[m_tail_of_q] = s;
           m_tail_of_q = (++m_tail_of_q);
           sem_post(&mutex);           /* unlock access to seat */

            //sem_getvalue(&m_barbers, &sem_count2);
            //printf("GET VALUE2 %d \n\n", sem_count2);
            //printf("Debug p11\n");

           sem_post(&m_barbers);           /* wake up the available barber (error code) */

            //printf("Debug p12\n");

           usleep(1000);

            //printf("Debug p13\n");

           sem_post(&u_barbers);

            //printf("Debug p14\n");
        }

    }
    else
    {
        sem_post(&mutex);           /* unlock access to seat */
        number_of_customers_served++;
        printf("[Customer: %d]\tcould not find a seat in waiting room, leaving the barber shop.\n\n", s);
    }

    while(chair[seated_chair] == s) /* wait until customer is taken from the seat by a barber */
        sleep(1);

    pthread_exit(0);
}

//-----------------------------------------------------------------------------------

void time_between_customers()
{
    srand((unsigned int)time(NULL));
    usleep(rand() % (250000 - 50000 + 1) + 50000); /* 50000 - 250000 ms */
}


