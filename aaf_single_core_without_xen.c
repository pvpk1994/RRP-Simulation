/* RRP SINGLE CORE Schedule Generator (Magic7) without Xen environment needed
 * Author:: Pavan Kumar Paluri, Guangli Dai
 * Copyright 2019-2020 - RTLAB UNIVERSITY OF HOUSTON */
//
// Created by Pavan Kumar  Paluri  on 2019-07-22.
//
//Last updated: April-8, 2020
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
    int id;
    struct node *next;
};


// Function Prototypes
void swap(int,int);
int lcm(int, int);
int hyper_period(sched_entry_t *);
sched_entry_t* dom_comp(sched_entry_t *);
struct node* load_timeslices(struct node*,int );
void getA_calc(sched_entry_t *, int);
struct node* partition_single(sched_entry_t*,int,struct node*);
struct node* SortedMerge(struct node* , struct node* );
void HalfSplit(struct node*, struct node**,
               struct node**);
// Perform merge sort on unsorted ts-dom pair based on ts Complexity : O(nlogn)
struct node* MergeSort(struct node ** HeadRef)
{
    struct node* head = *HeadRef;
    // Create 2 nodes a and b for splitting
    struct node* a;
    struct node* b;

    // Trivial case test: what if the there is only 1 element in the list?
    if((head->next == NULL) || (head == NULL))
        return NULL;

    // Split the unsorted list into 2 halves
    HalfSplit(head, &a, &b);

    // Sort the halves recursively
    MergeSort(&a);
    MergeSort(&b);

    // Merge the sorted halves and point it back to headref
    *HeadRef = SortedMerge(a,b);
    return *HeadRef;

}

// Sorted Merge to sort the already sorted halves
struct node* SortedMerge(struct node* a, struct node* b)
{
    // init a temp node
    struct node* result = NULL;

    // Trivial checks
    // if only on half is present, no need to sort, just return that valid half
    if(a == NULL)
        return b;
    else if(b == NULL)
        return a;

    // Actual sort happens here, sort it in ascending order
    if(a->ts <= b->ts)
    {
        result = a;
        result->next = SortedMerge(a->next, b);
    }
    else
    {
        result = b;
        result->next = SortedMerge(a,b->next);
    }
    return result;
}

// Now, all we are left with is the completion of HalfSplit(...)
void HalfSplit(struct node* header, struct node** first_half,
                                    struct node** second_half)
{
    struct node* fast_divider;
    struct node* slow_divider;
    slow_divider = header;
    fast_divider = header->next;
    // while slow_divider increments one node at a time, fast_divider advances 2 nodes at a time
    // thereby staying ahead and checking if list is not null
    while(fast_divider != NULL)
    {
        fast_divider = fast_divider->next;
        if(fast_divider != NULL)
        {
            slow_divider = slow_divider->next;
            fast_divider = fast_divider->next;
        }
    }

    // By the time control is out of above while loop, fast_divider should have reached
    // the end of list while slow_divider should have reached node before midpoint
    *first_half = header;
    *second_half = slow_divider->next;
    slow_divider->next = NULL;
}


// Append timeslice at the end of the ts list
void append(struct node** header, int ts, int id)
{
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    struct node *last = *header;

    // Assignment(s)
    new_node->ts = ts;
    new_node->id = id;
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
        printf("DomID:%d\n",noder->id);
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
    struct node *head_1, *head_2;

    head_1 = load_timeslices(head, hyper_period(scheduler));
    //print_list(load_timeslices(head, hyper_period(scheduler)));
    // Returns an unsorted timeslice-domain Pair
    // Need to sort it based on the timeslices....
    head_2 = partition_single(scheduler, hyper_period(scheduler), head_1);
    head_2= MergeSort(&head_2);
    printf("\nSorted list\n");
    print_list(head_2);


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
        append(&head,i,0);
    return head;
}

/* ********* check_delta ************
 * @param: node* avail_set: A list of time slices available now.
 * @param: int * standard_p: An array representing T(p, q, 0)
 * @param: int wcet: The length of standard_p.
 * @param: int delta: The right shifted value of the standar_p to be tested.
 * @param: int period: The period of the current partition.
 * @return: Returns true if T(p, q, delta) is available in the avail_set,
 *          Otherwise, return false.
 * *********************************/
bool check_delta(struct node* avail_set, int * standard_p, int wcet, int delta, int period)
{
    for(int i=0; i < wcet; i++)
    {
        int t_now = (standard_p[i] + delta)%period;
        if(search(avail_set, t_now)==false)
            return false;
    }
    return true;
}

/* ********* find_delta ************
 * @param: node* avail_set: A list of time slices available now.
 * @param: int period: The period of the current partition.
 * @param: int wcet: The wcet of the current partition.
 * @param: int wcet_left: The wcet_left after this partition is allocated.
 * @return: Returns the delta1 so that T(period, wcet, delta1) U T(period, wcet_left, delta2) = avail_set
            and the two partitions do not overlap. If no such delta1 and delta2 can be found, return -1.
 * *********************************/
int find_delta(struct node* avail_set, int period, int wcet, int wcet_left)
{
    int *standard_p1 = malloc(sizeof(int)*period);
    for(int i=0; i < period; i++)
    {
        standard_p1[i] = (int)(floor(i*period/wcet))%period;
    }
    int *standard_p2 = malloc(sizeof(int)*period);
    for(int i=0; i < period; i++)
    {
        standard_p2[i] = (int)(floor(i*period/wcet_left))%period;
    }
    for(int delta1=0; delta1 < period; delta1++)
    {
        if(check_delta(avail_set, standard_p1, wcet, delta1, period))
        {
            for(int delta2=0; delta2 < period; delta2++)
            {
                if(check_delta(avail_set, standard_p2, wcet_left, delta2, period))
                    return delta1;
            }
        }
    }
    return -1;
}

/* ********* partition_single ************
 * @param: sched_entry_t * partitions: An array of sched_entry_t, stores the partitions' information.
 * @param: int hp: The hyper-period calculated based on partitions' approximate availability factors.
 * @param:  node* avail: Available time slices now, ranges from 0 to hp - 1.
 * @return: returns array of struct pcpus
 * *********************************/
struct node *partition_single(sched_entry_t *partitions, int hp, struct node* avail)
{
    //Initialize the time slice list
    struct node *result = NULL;
    // iterate through sorted schedule entry list (partitions)
    for(int i=0;i< NUM_ENTRIES;i++)
    {
        // Init a new list to record the time slices allocated.
        struct node *occupied_time_index=NULL;
        //allocate time slices based on different wcet of the current partition
        if(partitions[i].wcet!=1)
        {
            //find available delta first
            int avail_length = list_length(avail);
            int delta1 = find_delta(avail, partitions[i].period, partitions[i].wcet, (int)(avail_length * partitions[i].period / hp) - partitions[i].wcet);
            if(delta1 == -1)
            {
                printf("Unschedulable partitions!\n");
                return NULL;
            }
            //utilize the delta found to allocate time slices to partitions[i]
            for(int l=0; l < hp/partitions[i].period; l++)
            {
                for(int k=0; k < partitions[i].wcet; k++)
                {
                    int index_now = (int)(floor(k*partitions[i].period/partitions[i].wcet) + delta1)
                                    %partitions[i].period + l * partitions[i].period;
                    if(search(avail, index_now)==false)
                    {
                        printf("Time slice %d is allocated redundantly!\n", index_now);
                        return NULL;
                    }
                    append(&occupied_time_index, index_now, 0);
                    append(&result, index_now, partitions[i].id);
                }
            }
        }
        else
        {
            //retrieve the smallest time slice available now
            int index = get_i(avail, 0);
            //check whether time is in feasible range.
            if(index >= partitions[i].period)
            {
                printf("Unschedulable partitions!\n");
                return NULL;
            }
            //allocate time slices to partition[i]
            for(int l= 0;l<hp/partitions[i].period;l++)
            {
                int index_now = index + l * partitions[i].period;
                append(&occupied_time_index, index_now, 0);
                append(&result, index_now, partitions[i].id);
            }
        }
        //update time slices left

        struct node *temp=NULL;
        for(int i=0;i<hp;i++)
        {
            if(search(avail,i)==true && search(occupied_time_index,i)== false)
                // insert the ith element of linked list with head "Node"
                append(&temp,i,0);
        }
        //print_list(temp);
        avail=copy(temp,avail);
        //print_list(Node);


    }
    print_list(result);
    return result;
}
