#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/spinlock.h> 	
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
     
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Madhuparna Bhowmik <madhuparnabhowmik04@gmail.com>");
MODULE_DESCRIPTION("Readers Writers Problem using kthreads and RCU");

static spinlock_t mylock = __SPIN_LOCK_UNLOCKED();

struct my_list{
struct list_head rw_list;      
int data_item;
};

struct list_head rw_head;

static struct task_struct *t1;
static struct task_struct *t2;
static struct task_struct *t3;
static struct task_struct *t4;

/*
	The reader function reads the entire list and prints the data_item
	after every 2 secs.
*/
static int reader(void *data)
{
	while(!kthread_should_stop())
	{
		rcu_read_lock();
		struct my_list *entry;
		list_for_each_entry_rcu(entry,&rw_head,rw_list)
		{
			printk(KERN_INFO "List item: %d Read by: %d \t ", entry->data_item,current->pid);
		}
		rcu_read_unlock();
		msleep(2000);
	}
	return 0;
	
}
/*
	The writer function modifies the list by adding an element to the list
	after every 5 secs. Each writer thread adds atmost 50 new entries to the list.
*/
static int writer(void *data)
{
	int count=0;
	while(!kthread_should_stop()){
		count++;
		if(count>50)
		{
			msleep(5000);
			continue;
		}
		struct my_list *el;

		el=kmalloc(sizeof(struct my_list),GFP_KERNEL);
		if(el == NULL)
			{
				count--;
				msleep(5000);
				continue;
			}
		el->data_item = count%7;

		spin_lock(&mylock);
		list_add_rcu(&el->rw_list,&rw_head);
		spin_unlock(&mylock);

		msleep(5000);
	}
	return 0;
}

static int __init start_init(void){
	
	INIT_LIST_HEAD_RCU(&rw_head);


	struct my_list *one,*two,*three;
	one=kmalloc(sizeof(struct my_list),GFP_KERNEL);
	two=kmalloc(sizeof(struct my_list),GFP_KERNEL);
	three=kmalloc(sizeof(struct my_list),GFP_KERNEL);
	
	if(!one || !two || !three)
	{
		printk(KERN_INFO "\n Kmalloc failed  \t ");
		return 0;
	}
	one->data_item = 1;
	two->data_item = 2;
	three->data_item = 3;

	//Adding thre entries to the list initially.
	list_add_rcu(&one->rw_list,&rw_head);
	list_add_rcu(&two->rw_list,&rw_head);
	list_add_rcu(&three->rw_list,&rw_head);


	t1 = kthread_create(reader,NULL,"reader1");
	t2 = kthread_create(reader,NULL,"reader2");
    t3 = kthread_create(writer,NULL,"writer1");
	t4 = kthread_create(writer,NULL,"writer2");

	if(t1 && t2 && t3 && t4){

		printk(KERN_INFO "All threads created successfully! \n");
		wake_up_process(t1);
		wake_up_process(t2);
		wake_up_process(t3);
		wake_up_process(t4);
	
	}
	else{
		printk(KERN_ALERT "Thread Creation Failed\n");
	}
	
	return 0;
}

static void __exit end_exit(void){


	printk(KERN_INFO "Cleaning up started...\n");

	int ret1 = kthread_stop(t3);
	int ret2 = kthread_stop(t4);
	int ret3 = kthread_stop(t1);
	int ret4 = kthread_stop(t2);

	if(!ret1 && !ret2 && !ret3 & !ret4)
		printk(KERN_INFO "Threads stopped\n");
 	
	
	spin_lock(&mylock);
	//Deleting entries from the list
	while(1)
	{
		struct list_head *pt;
		struct my_list *temp;
		pt = list_next_rcu(&rw_head);
		if(pt==&rw_head)
			break;
		temp = list_entry_rcu(pt,struct my_list,rw_list);
		list_del_rcu(&temp->rw_list);
		synchronize_rcu();
		kfree(temp);
	}
	
	spin_unlock(&mylock);
	
	printk(KERN_INFO " Clean Up Done! ");
	
}

module_init(start_init)
module_exit(end_exit)
