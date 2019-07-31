//
// Created by Pavan Kumar  Paluri  on 2019-07-22.
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#define NUM_ENTRIES 3

// Global Inits
static int arr[NUM_ENTRIES]; // init A_val with number of schedule entries
int counter;


// Structure Definitions
typedef struct {
    int id;
    int wcet; // getAAF_numerator
    int period; // getAAF_denominator
}sched_entry_t;

// Maintain a single linked list of timeslices
struct node{
    int ts;
    struct node *next;
};


// Function Prototypes
void swap(int,int);
int lcm(int, int);
int hyper_period(sched_entry_t *);
sched_entry_t* dom_comp(sched_entry_t *);
struct node* load_timeslices(struct node*,int );
void getA_calc(sched_entry_t *, int);
void Partition_single(sched_entry_t*,int,struct node*);

// Append timeslice at the end of the ts list
void append(struct node** header, int ts)
{
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    struct node *last = *header;

    // Assignment(s)
    new_node->ts = ts;
   // printf("time slice added:%d\n",new_node->ts);
    // Make next of this new_node to NULL since its being appended at the end of the list
    new_node->next=NULL;

    // If list is empty, make the head as last node
    if(*header == NULL)
    {
        *header = new_node;
       // printf("Head's timeslice:%d\n",(*header)->ts);
        return;
    }
    //If list is not empty, traverse the entire list to append the ts at the end
    while(last->next !=NULL)
        last = last->next;
    last->next = new_node;
   // printf("Tail Timeslice:%d\n",last->ts);
    return;

}

// Print contents of linked list
void print_list(struct node* noder)
{
    while(noder!= NULL)
    {
        printf("timeslice:%d\n",noder->ts);
        noder = noder->next;
    }
    printf("\n");
}

// Get the dynamic length of the list of timeslices
int list_length(struct node *header)
{
    int counter=0;
    while(header != NULL)
    {
        counter++;
        header = header->next;
    }
    return counter;
}


// Remove a certain node from the list given the node
void delete_entry(struct node **head, int position)
{
    // If linked list is empty
    if(*head == NULL)
        return;

    struct node* temp = *head;

    //If only a single entry in the list
    if(position == 0)
    {
        *head = temp->next;
        free(temp);
        return;
    }

    for(int i=0; temp!=NULL && i<position-1;i++)
    {
        temp = temp->next;
    }

    if(temp == NULL || temp->next == NULL)
        return;

    struct node *next = temp->next->next;

    free(temp->next);

    temp->next = next;

}

// Search an element by its value to check its presence
/* Checks whether the value x is present in linked list */
bool search(struct node* head, int x)
{
    struct node* current = head;  // Initialize current
    while (current != NULL)
    {
        if (current->ts == x)
            return true;
        current = current->next;
    }
    return false;
}


// get the ith entry of a linked list
int get_i(struct node* head, int index)
{
    struct node* current = head;
    int count = 0; /* the index of the node we're currently
                  looking at */
    while (current != NULL)
    {
        if (count == index)
            return(current->ts);
        count++;
        current = current->next;
    }

    /* if we get to this line, the caller function was asking
       for a non-existent element so we assert fail */
    assert(0);
}

// Copy elements of one linked list into another
struct node *copy(struct node *org, struct node *new)
{
    new = NULL;
    struct node **tail = &new;

    for( ;org; org = org->next) {
        *tail = malloc (sizeof **tail );
        (*tail)->ts = org->ts;
        (*tail)->next = NULL;
        tail = &(*tail)->next;
    }
    return new;
}

// MAIN
int main()
{
    sched_entry_t schedule[NUM_ENTRIES], *scheduler;
    int A_val[NUM_ENTRIES];

    printf("Enter the schedule WCET:");
    for(int i=0; i<NUM_ENTRIES; i++)
    {
        scanf("%d",&schedule[i].wcet);
    }
    printf("Enter the schedule's Periods:");
    for(int i=0; i<NUM_ENTRIES; i++)
    {
        scanf("%d",&schedule[i].period);
    }

    for(int i=0; i< NUM_ENTRIES; i++)
    {
        schedule[i].id = i;
        printf("WCET of schedule[%d] is %d\n ",i, schedule[i].wcet);
        printf("Period of schedule[%d] is %d\n",i, schedule[i].period);
    }

    int wcet[2] ={13,15};
    printf("Return Val:%d\n",domain_handle_comp(wcet[0],wcet[1]));
    int a = 34;
    int b = 45;
    int *avail_ts;
    swap(a,b);
    scheduler = dom_comp(schedule);
    for(int i=0;i <NUM_ENTRIES;i++) {
        printf(" schedule WCET[%d]= %d\n", i, scheduler[i].wcet);
        printf(" schedule Period[%d]= %d\n", i, scheduler[i].period);
    }
    getA_calc(scheduler, hyper_period(scheduler));
    for( int i=0;i< NUM_ENTRIES;i++)
        printf("arr[%d]:%d\n",i,arr[i]);
    struct node *head = NULL;
    struct node *head_1;
    head_1 = load_timeslices(head, hyper_period(scheduler));
    //print_list(load_timeslices(head, hyper_period(scheduler)));
    Partition_single(scheduler, hyper_period(scheduler), head_1);



    // Deleting an entry
  //  head =delete_entry(head_1,0);
    return 0;
}

int domain_handle_comp(int wcet1, int wcet2)
{
    return wcet1 > wcet2 ? 0 : 1;
}

void swap(int a, int b)
{
    int temp;
    temp = a;
    a = b;
    b = temp;
    printf(" Value of a and b are: %d %d respectively\n", a,b);

}

sched_entry_t *dom_comp(sched_entry_t sched[])
{
    int i,j;
    for(i=0;i<NUM_ENTRIES;i++) {
        for (j = i + 1; j < NUM_ENTRIES; j++) {
            int k;
            k = domain_handle_comp(sched[i].wcet, sched[j].wcet);
            if (k != 0) {
               // swap(sched[i].wcet, sched[j].wcet);
               int temp,temp1,temp2;
               temp = sched[i].wcet;
               sched[i].wcet = sched[j].wcet;
               sched[j].wcet = temp;
                //return sched;
                temp1 = sched[i].period;
                sched[i].period = sched[j].period;
                sched[j].period = temp1;

                temp2 = sched[i].id;
                sched[i].id = sched[j].id;
                sched[j].id = temp2;
            }
        }
    }
    return sched;
}


int lcm(int num1, int num2)
{
  int minMultiple;

  minMultiple = (num1 > num2) ? num1 : num2;

  while(1)
  {
      if( minMultiple%num1==0 && minMultiple%num2==0 )
      {
          printf("The LCM of %d and %d is %d.\n", num1, num2,minMultiple);
          break;
      }
      ++minMultiple;
  }
   return minMultiple;
}


int hyper_period(sched_entry_t *sched)
{
    int final_val=1;
    for (int i=0; i< NUM_ENTRIES; i++)
    {

            final_val = lcm(final_val, sched[i].period);
            printf("Final Value: %d\n", final_val);

    }
    return final_val;
}


void getA_calc(sched_entry_t *sched, int hp)
{
    int i;


    for(i=0;i< NUM_ENTRIES;i++)
    {
        arr[i] = (int)(hp/sched[i].period)*(sched[i].wcet);
    }
}

struct node* load_timeslices(struct node* head, int hp)
{
    for(int i=0;i<hp;++i)
        append(&head,i);
    return head;
}

void Partition_single(sched_entry_t *sched, int hp, struct node* Node)
{

    // iterate through sorted schedule entry list
    for(int i=0;i< NUM_ENTRIES;i++)
    {
        // init hp_now to the current size of timeslice list
        int hp_now = list_length(Node);
       // printf("SIZE:%d\n",hp_now);

        // Init a new list
        struct node *occupied_time_index=NULL;
        for(int k=0;k <arr[i];k++)
        {
         // Assignment of calculated timeslices to a partition
         int index_now = (int)(floor(k*hp_now/arr[i]))%hp_now;
         append(&occupied_time_index,index_now);
         //printf("Index Now:%d\n",index_now);

         printf("Assigning %d to %d\n",get_i(Node,index_now),sched[i].id);

         // Assign timeslices calculated above to Partition_i
         //  under progress...

        }
        struct node *temp=NULL;
        for(int i=0;i<hp_now;i++)
        {
            if(search(occupied_time_index,i)== false)
                // insert the ith element of linked list with head "Node"
                append(&temp,get_i(Node,i));
        }
        // copy(old, new) {old->new}
        //print_list(temp);
        Node=copy(temp,Node);
        //print_list(Node);


    }

}
