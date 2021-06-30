/*
    Jubayer Rahman    - 06
    Farhan Fuad Haque - 35
    Nafisa Naznin     - 41
*/

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>

#include "invest_assignment.h"
#include "investor_producer.h"

/*
 * **********************************************************************
 * YOU ARE FREE TO CHANGE THIS FILE BELOW THIS POINT AS YOU SEE FIT
 *
 */
static struct semaphore * sem_queue;                // locks the req_serv_item queue.  
static struct semaphore * sem_producer_take_order;  //stops the producer thread from producing
static struct semaphore * sem_customer;             //locks the customers variable. 
static struct semaphore * sem_bank_account;         //locks the bank account array. 
static struct semaphore * sem_producer_req_item;    // locks the producers from producing same request
static struct semaphore * sem_consume[NCUSTOMER];   // ensures that the customers previous orders are served before placing new order
static struct semaphore * sem_total_served;         //locks the total_served array

struct item * current_item;

int customers = NCUSTOMER; 
int total_served[NCUSTOMER]; // total item served per customer. 

//////////////////////////////////******************************************////////////////////
//SELF MADE FUNCTION
/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY CUSTOMER THREADS
 * **********************************************************************
 */
/*
 * order_item()
 *
 * Takes one argument specifying the item produces. The function
 * makes the item order available to producer threads and then blocks until the producers
 * have produced the item with that appropriate for the customers.
 *
 * The item itself contains the number of ordered items.
 */

void order_item(void * itm) 
{
    (void) itm; // to avoid warning
    P(sem_queue);

    struct item * curr = req_serv_item;
    struct item * temp = itm;
    while (curr -> next != NULL) 
    {
        curr = curr -> next;
    }

    int i;
    for (i = 0; i < 10; i++) 
    {
        struct item * new_item = (struct item * ) kmalloc(sizeof(struct item));
        new_item -> item_quantity = temp -> item_quantity;
        new_item -> i_price = temp -> i_price;
        new_item -> requestedBy = temp -> requestedBy;
        new_item -> servBy = temp -> servBy;
        new_item -> order_type = REQUEST;
        new_item -> next = NULL;

        if (req_serv_item == NULL) 
        {
            curr = new_item;
            req_serv_item = new_item;
        } 
        else 
        {
            curr -> next = new_item;
            curr = curr -> next;
        }
        temp++;

        V(sem_producer_take_order);
    }

    V(sem_queue);

    //panic("You need to write some code!!!!");
}

/**
 * consume_item() 
 * Customer consume items which were served by the producers.
 * affected variables in the order queue, on item quantity, order type, requested by, served by
 * customer should keep records for his/her spending in shopping
 * and update spending account
 **/
void consume_item(unsigned long customernum) 
{
    (void) customernum; // avoid warning 

    P(sem_queue);
    P(sem_consume[customernum]);

    struct item * curr = req_serv_item;

    while (1) 
    {
        if (curr -> next == NULL) 
        {
            break;
        }
        if ((curr -> next) -> order_type == SERVICED && (curr -> next) -> requestedBy == (long) customernum) 
        {
            customer_spending_amount[customernum] += ((curr -> next) -> item_quantity) * ((curr -> next) -> i_price);
            curr -> next = (curr -> next) -> next;
        } 
        else
        {
            curr = curr -> next;
        }
    }
    V(sem_queue);
    
    //panic("You need to write some code!!!!");

}

/*
 * end_shoping()
 *
 * This function is called by customers when they go home. It could be
 * used to keep track of the number of remaining customers to allow
 * producer threads to exit when no customers remain.
 */
void end_shoping() 
{
    P(sem_customer);
    customers--;
    V(sem_customer);

    if (customers <= 0) 
    {
        for (int i = 0; i < NPRODUCER; i++) 
        {
            V(sem_producer_take_order);
        }
        return;
    }
    V(sem_producer_take_order);

    return;

    //panic("You need to write some code!!!!");
}

/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY ITEM PRODUCER THREADS
 * **********************************************************************
 */

/*
 * take_order()
 *
 * This function waits for a new order to be submitted by
 * customers. When submitted, it records the details and returns a
 * pointer to something representing the order.
 *
 * The return pointer type is void * to allow freedom of representation
 * of orders.
 *
 * The function can return NULL to signal the producer thread it can now
 * exit as there are no customers nor orders left.
 */

void * take_order() 
{
    P(sem_producer_take_order);
    P(sem_producer_req_item);
    struct item * requested_item;
    while(1) 
    {
        if(customers == 0) break;
        if(current_item == NULL)
        {
            current_item = req_serv_item;
        }
        if(current_item -> order_type == REQUEST) 
        {
            requested_item = current_item;
            current_item = requested_item -> next;
            V(sem_producer_req_item);
            return requested_item;
        }
        else
        {
          current_item = current_item -> next;
        }

    }
    V(sem_producer_req_item);

    return NULL;
    
    //panic("You need to write some code!!!!");

}

/*
 * produce_item()
 *
 * This function produce an item if the investment is available for the product
 *
 */

void produce_item(void * v) 
{
    (void) v;
    struct item * curr = v;
    int total_price = curr->item_quantity * ITEM_PRICE;
    curr -> i_price = total_price + (total_price * (PRODUCT_PROFIT + BANK_INTEREST)) / 100;
    
    //panic("You need to write some code!!!!");
}

/*
 * serve_order()
 *
 * Takes a produced item and makes it available to the waiting customer.
 */

void serve_order(void * v, unsigned long producernumber) 
{
    (void) v;
    (void) producernumber;

    struct item * curr = v;
    P(sem_total_served);

    if (curr -> order_type != SERVICED) 
    {
        curr -> order_type = SERVICED;
        curr -> servBy = producernumber;
        total_served[curr -> requestedBy]++;
    }

    if (total_served[curr -> requestedBy] >= 10) 
    {
        total_served[curr -> requestedBy] = 0;
        V(sem_consume[curr -> requestedBy]);
    }


    V(sem_total_served);
    
    //panic("You need to write some code!!!!");
}

/**
 * calculate_loan_amount()
 * Calculate loan amount
 */
long int calculate_loan_amount(void * itm) 
{
    (void) itm;
    struct item * curr = itm;
    int total = ITEM_PRICE * curr -> item_quantity;
    return total;

    //panic("You need to write some code!!!!");
}

/**
 * bank 
 * void loan_request()
 * Request for loan from bank
 */
void loan_request(void * amount, unsigned long producernumber) 
{
    (void) amount;
    (void) producernumber;

    long int requested_ln = * (long int * ) amount;
    int bank = random() % NBANK;
    P(sem_bank_account);
    bank_account[bank].acu_loan_amount += requested_ln;
    bank_account[bank].prod_loan[producernumber] += requested_ln;
    bank_account[bank].remaining_cash -= requested_ln;
    V(sem_bank_account);
    
    //panic("You need to write some code!!!!");
}

/**
 * loan_reimburse()
 * Return loan amount and service charge
 */
void loan_reimburse(void * loan, unsigned long producernumber) 
{
    (void) loan;
    (void) producernumber;

    long int debt = * (long int * ) loan;
    long int service_charge = (debt / 100) * BANK_INTEREST;
    P(sem_bank_account);
    int i;
    for (i = 0; i < NBANK; i++) 
    {
        if (bank_account[i].prod_loan[producernumber] != 0) 
        {
            bank_account[i].interest_amount += service_charge;
            bank_account[i].remaining_cash += debt + service_charge;
            bank_account[i].prod_loan[producernumber] -= debt;
            break;
        }
    }
    producer_income[producernumber] += (debt / 100) * PRODUCT_PROFIT;
    V(sem_bank_account);

    //panic("You need to write some code!!!!");
    
}

/*
 * **********************************************************************
 * INITIALISATION AND CLEANUP FUNCTIONS
 * **********************************************************************
 */

/*
 * initialize()
 *
 * Perform any initialization you need before opening the investor-producer process to
 * producers and customers
 */

void initialize() 
{

    memset(total_served, 0, NCUSTOMER * sizeof(int));
    memset(customer_spending_amount, 0, NCUSTOMER * sizeof(long int));
    memset(producer_income, 0, NPRODUCER * sizeof(long int));
    
    //creating semaphores 
    sem_queue = sem_create("sem_queue", 1);
    sem_producer_take_order = sem_create("sem_producer_take_order", 0);
    sem_producer_req_item = sem_create("sem_producer_req_item", 1);
    sem_customer = sem_create("sem_customer", 1);
    sem_bank_account = sem_create("sem_bank_account", 1);
    sem_total_served = sem_create("sem_total_served", 1);

    //sem_consume initializing
    for (int i = 0; i < NCUSTOMER; i++) 
    {
        sem_consume[i] = sem_create("sem_consume", 0);
    }
    req_serv_item = (struct item * ) kmalloc(sizeof(struct item));
    req_serv_item -> item_quantity = 2;
    req_serv_item -> i_price = 35;
    req_serv_item -> servBy = 80;
    req_serv_item -> requestedBy = 119;
    req_serv_item -> order_type = 4;
    req_serv_item -> next = NULL;
    current_item = req_serv_item;
    
    // panic("You need to write some code!!!!");
}

/*
 * finish()
 *
 * Perform any cleanup investor-producer process after the finish everything and everybody
 * has gone home.
 */

void finish() 
{
    //destroying semaphores
    sem_destroy(sem_customer);
    sem_destroy(sem_queue);
    sem_destroy(sem_producer_req_item);
    sem_destroy(sem_bank_account);
    sem_destroy(sem_producer_take_order);
    for (int i = 0; i < NCUSTOMER; i++) 
    {
        sem_destroy(sem_consume[i]);
    }
     
    kfree(req_serv_item);
    
    //panic("You need to write some code!!!!");

}