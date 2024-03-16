/*
 * lunix-chrdev.c
 *
 * Implementation of character devices
 * for Lunix:TNG
 *
 * < Your name here >
 *
 */

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include "lunix.h"
#include "lunix-chrdev.h"
#include "lunix-lookup.h"

/*
 * Global data
 */
// struct cdev: Η εσωτερική δομή του πυρήνα που εκφράζει τη συσκευή
struct cdev lunix_chrdev_cdev;

/*
 * Just a quick [unlocked] check to see if the cached
 * chrdev state needs to be updated from sensor measurements.
 */
/*
 * Declare a prototype so we can define the "unused" attribute and keep
 * the compiler happy. This function is not yet used, because this helpcode
 * is a stub.
 */

// State need Refresh
// Πρέπει να γίνει refresh στο timestamp γιατι έχουμε άλλο από αυτό που θέλουμε
static int lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *state)
{
	struct lunix_sensor_struct *sensor;

	WARN_ON ( !(sensor = state->sensor));
	/* ? */
	debug("exiting state need refresh");
	return state->buf_timestamp != sensor->msr_data[state->type]->last_update;
}

/*
 * Updates the cached state of a character device
 * based on sensor data. Must be called with the
 * character device state lock held.
 */


// Update State
static int lunix_chrdev_state_update(struct lunix_chrdev_state_struct *state)
{
	struct lunix_sensor_struct *sensor = state->sensor;
	long int proper_data;
	uint32_t raw_time = sensor->msr_data[state->type]->last_update;
	uint32_t raw_value = sensor->msr_data[state->type]->values[0];
	char sign;
	debug("entering state update");


	// debug("leaving\n");

	/*
	 * Grab the raw data quickly, hold the
	 * spinlock for as little as possible.
	 */
	/* ? */
	spin_lock_irq(&sensor->lock);
	raw_time = sensor->msr_data[state->type]->last_update;
	raw_value = sensor->msr_data[state->type]->values[0];
	spin_unlock_irq(&sensor->lock);
	/* Why use spinlocks? See LDD3, p. 119 */

	/*
	 * Any new data available?
	 */
	/* ? */
	if (lunix_chrdev_state_needs_refresh(state)) {
		spin_lock_irq(&sensor->lock);
		raw_time = sensor->msr_data[state->type]->last_update;
		raw_value = sensor->msr_data[state->type]->values[0];
		spin_unlock_irq(&sensor->lock);
	}
	else { 
		// -EAGAIN = no data available right now, try again later
		return -EAGAIN;
	}

	/*
	 * Now we can take our time to format them,
	 * holding only the private state semaphore
	 */

	switch(state->type){
		case BATT:
			proper_data = lookup_voltage[raw_value]; 
			break;
		case TEMP:
			proper_data = lookup_temperature[raw_value];
			break;
		case LIGHT:
			proper_data = lookup_light[raw_value];
			break;
		default:
			return -EAGAIN;
	}

	if (proper_data >= 0) { 
		sign='+';
	}
	else {
		sign='-';
		proper_data = (-1)*proper_data;
	}


	state->buf_lim = snprintf(state->buf_data,
			LUNIX_CHRDEV_BUFSZ, "%c%ld,%ld",
			sign, proper_data/1000,
			proper_data%1000);
	state->buf_timestamp = raw_time;

	/* ? */

	debug("leaving\n");
	return 0;
}

/*************************************
 * Implementation of file operations
 * for the Lunix character device
 *************************************/

// Open System Call
static int lunix_chrdev_open(struct inode *inode, struct file *filp)
{
	/* Declarations */
	/* ? */
	// Στο lunix-chrdev.h υπάρχει ένα struct 
	// Το οποίο χρησιμοποιείται για όταν μία
	// συσκευή είναι ανοιχτή επομένως θα 
	// πρέπει να ορίσουμε ένα τέτοιο struct
	struct lunix_chrdev_state_struct *lunix_chrdev_state;
	int ret;
	int minor = iminor(inode);
	int sensor = minor >> 3;
	int type = minor%8; //?? 

	debug("entering\n");
	ret = -ENODEV;
	if ((ret = nonseekable_open(inode, filp)) < 0)
		goto out;

	/*
	 * Associate this open file with the relevant sensor based on
	 * the minor number of the device node [/dev/sensor<NO>-<TYPE>]
	 */

	/* Allocate a new Lunix character device private state structure */
	/* ? */
	// Θα πρέπει να κάνουμε allocate μνήμη στον πυρήνα με ασφαλή τρόπο
	// για το struct state μας
	lunix_chrdev_state = kzalloc(sizeof(*lunix_chrdev_state), GFP_KERNEL);
	// Το GFP_KERNEL flag δηλώνει ότι το allocation γίνεται για ένα process
	// που τρέχει στο χώρο του πυρήνα
	// Θα πρέπει το struct που δημιούργησα να το αλλάξω με τέτοιο τρόπο ώστε
	// να συνδεθεί στη minor θέση που θέλω 
	// Το struct μας χρειάζεται:
	//          -type {BATT, TEMP, LIGHT, N_LUNIX_MSR}
	//          -sensor, τον οποίο θα βρούμε μέσα στη λίστα των sensors
	//          με αριθμό minor/8.
	//          -buf_lim
	//          -buf_data
	//          -lock
	//          -buf_timestamp
	//          -raw_data

	lunix_chrdev_state->sensor = &lunix_sensors[sensor];
	lunix_chrdev_state->type = type;
	lunix_chrdev_state->buf_lim = 0;
	lunix_chrdev_state->buf_timestamp = 0;
	// struct file: void *private_data;
	// The open system call sets this pointer to NULL before calling the open method for the driver.
	// The driver is free to make its own use of the field or to ignore it.
	// The driver can use the field to point to allocated data, but then must free memory 
	// in the release method before the file structure is destroyed by the kernel.
	// private_data is a useful resource for preserving state information across system calls
	// and is used by most of our sample modules.
	filp->private_data = lunix_chrdev_state;

	// Initialize lock
	sema_init(&lunix_chrdev_state->lock,1);
out:
	debug("leaving, with ret = %d\n", ret);
	return ret;
}

// Release System Call (free allocated memory of file)
static int lunix_chrdev_release(struct inode *inode, struct file *filp)
{
	/* ? */
	kfree(filp->private_data);
	return 0;
}

// Device Specific Commands
static long lunix_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	// File descriptor not associated with character special device, or the request does not apply
	// to the kind of object the file descriptor references. (ENOTTY)
	int ret = -ENOTTY;
	int retry = -ERESTARTSYS;
	struct lunix_chrdev_state_struct *state;

	// https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch05.html
	if(_IOC_TYPE(cmd) != LUNIX_IOC_MAGIC) return ret;
	if(_IOC_NR(cmd) > LUNIX_IOC_MAXNR) return ret;
	state = filp->private_data;

	switch(cmd) {
		case LUNIX_IOC_EXAMPLE:
			if(down_interruptible(&state->lock)) return retry;
			debug("in LUNIX_IOC_EXAMPLE area");
			up(&state->lock);
			break;
		default: return ret;
	}
	/* Why? */
	// return -EINVAL; Invalid Argument
	debug("ioctl done my guy ");
	return 0;
}

static ssize_t lunix_chrdev_read(struct file *filp, char __user *usrbuf, size_t cnt, loff_t *f_pos)
{
	ssize_t ret = 0;
	int retry = -ERESTARTSYS;

	struct lunix_sensor_struct *sensor;
	struct lunix_chrdev_state_struct *state = filp->private_data;
	WARN_ON(!state);

	sensor = state->sensor;
	WARN_ON(!sensor);

	/* Lock? */
    // Αν ο semaphore δεν ειναι διαθέσιμος τότε το process μπαινει στο waitqueue
	if(down_interruptible(&state->lock)){
		debug("down interruptable failed in read");
		return retry;
	}
	/*
	 * If the cached character device state needs to be
	 * updated by actual sensor data (i.e. we need to report
	 * on a "fresh" measurement, do so
	 */
	if (*f_pos == 0) {
		// Nothing to read
		while (lunix_chrdev_state_update(state) == -EAGAIN) {
			/* ? */
			/* The process needs to sleep */
			/* See LDD3, page 153 for a hint */
			up(&state->lock);
			if (wait_event_interruptible(sensor->wq, lunix_chrdev_state_needs_refresh(state))){
				debug("wait interruptable failed in read");
				return -ERESTARTSYS;
			}
			if (down_interruptible(&state->lock)){
				debug("down interruptable failed in read");
				return -ERESTARTSYS;
			}
		}
	}

	/* End of file */
	/* ? */

	/* Determine the number of cached bytes to copy to userspace */
	/* ? */
	// Αν ο user ζητήσει περισσότερα από όσα έχω να δώσω στο αρχείο, πρέπει να τα 
	// επεξεργαστώ 
	if (*f_pos + cnt >= state->buf_lim) { 
		cnt = state->buf_lim - *f_pos;
	}

	if (copy_to_user(usrbuf, state->buf_data, cnt)) {
		ret=-EFAULT;
		goto out;
	}

	// προσθέτουμε τον αριθμό που διαβάσαμε στο offset
	*f_pos += cnt;
	ret = cnt;


	/* Auto-rewind on EOF mode? */
	/* ? */
	if (*f_pos == state->buf_lim){*f_pos = 0;}

out:
	/* Unlock? */
	up(&state->lock);
	return ret;
}

// Mmap 
static int lunix_chrdev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return -EINVAL;
}

// Λειτουργίες της δομής cdev 
static struct file_operations lunix_chrdev_fops = 
{
	.owner          = THIS_MODULE,
	.open           = lunix_chrdev_open,
	.release        = lunix_chrdev_release,
	.read           = lunix_chrdev_read,
	.unlocked_ioctl = lunix_chrdev_ioctl,
	.mmap           = lunix_chrdev_mmap
};

// Initialization
int lunix_chrdev_init(void)
{
	/*
	 * Register the character device with the kernel, asking for
	 * a range of minor numbers (number of sensors * 8 measurements / sensor)
	 * beginning with LINUX_CHRDEV_MAJOR:0
	 */
	int ret;
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;

	// Αριθμός των sensors * 2^3 είναι ο συνολικός αριθμός των minor numbers

	debug("initializing character device\n");
	cdev_init(&lunix_chrdev_cdev, &lunix_chrdev_fops);
	// Αρχικοποίηση μίας δομής cdev με τις λειτουργίες της
	lunix_chrdev_cdev.owner = THIS_MODULE;

	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	// Δημιουργία device ID 
	/* ? */
	/* register_chrdev_region? */
	// Θέλουμε να δηλώσουμε μία σειρά από device numbers για τον οδηγό
	// Πρέπει να γνωρίζουμε από που να ξεκινήσουμε(ίσως απο τον Major Number),
	// το σύνολο των συσκευών, και το όνομα του οδηγού 
	ret = register_chrdev_region(dev_no, lunix_minor_cnt, "lunix");
	if (ret < 0) {
		debug("failed to register region, ret = %d\n", ret);
		goto out;
	}	
	/* ? */
	/* cdev_add? */
	// Πρόσθεσε την συσκευή στο σύστημα με cdev_add
	// Παράμετροι: δομή cdev, τον πρώτο αριθμό για τον οποίο είναι υπεύθυνη η συσκευή
	// τον αριθμό των minor numbers.
	ret = cdev_add(&lunix_chrdev_cdev, dev_no, lunix_minor_cnt);
	if (ret < 0) {
		debug("failed to add character device\n");
		goto out_with_chrdev_region;
	}
	debug("completed successfully\n");
	return 0;

out_with_chrdev_region:
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
out:
	return ret;
}

// Destroy
void lunix_chrdev_destroy(void)
{
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;

	debug("entering\n");
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	cdev_del(&lunix_chrdev_cdev);
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
	debug("leaving\n");
}
