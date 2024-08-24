// Create a device driver that computes sparse to dense vectors and the other way around
// user space connects to the device via cvec_ioctl
// statuses for each slot in the buffer of the device: FREE, OCCUPIED, PROCESSED 
// the driver only processes buffer slots that are OCCUPIED and an interrupt happens when 
// the processing of a buffer slot is done to change the status of the slot to PROCESSED
// if there is no FREE slot in the buffer, than the process sleeps until there is one 
#define DENSE_TO_SPARSE 0
#define SPARSE_TO_DENSE 0

// =======================
// Αρχικοποίηση του device 
// =======================
//
// static int __init cvec_device_init(void) {
//     int i;
//     printk(KERN_INFO "Initializing cvec device\n");
//
//     // Initialize the device lock
//     mutex_init(&cvec_dev.lock);
//
//     // Initialize the wait queue
//     init_waitqueue_head(&cvec_dev.wq);
//
//     // Initialize buffer elements
//     for (i = 0; i < BUF_LEN; i++) {
//         cvec_dev.buffer[i].cvdesc = NULL;
//         cvec_dev.buffer[i].status = FREE;
//         cvec_dev.buffer[i].conversion_mode = DENSE_TO_SPARSE; // Default conversion mode
//     }
//
//     return 0;
// }
//
// module_init(cvec_device_init);

// Assume these two are implemented 
int get_free_slot(struct cvec_device *cvdev);
int get_processed_slot(struct cvec_device *cvdev);

struct cvec_state {
    // TODO: what kind of lock here? 
    struct semaphore lock;
    // TODO: done
    int conversion_mode;
}

struct cvec_descriptor{
    int len;
    int *input;
    int *output;
}

struct cvec_device {
    #define BUF_LEN 1024
    struct {
        cvec_descriptor *cvdesc;
        int conversion_mode;
        int status;
    } buffer[BUF_LEN];
    // TODO: what kind of lock here? 
    spinlock_t lock;
    // TODO: done
    wait_queue_head_t wq;
} cvec_dev;

void open(struct inode *inode, struct file *filp) {
    int ret = 0;
    struct cvec_state *state; 
    struct cvec_device *cvdev = &cvec_dev;
    if ((ret = nonseekable_open(inode,filp)) < 0) {
        ret = -ENODEV;
        goto out;
    }
    state = kmalloc(sizeof(struct cvec_state), GFP_KERNEL);
    // TODO: αρχικοποίηση των structs
    state->conversion_mode = DENSE_TO_SPARSE; // Default από εκφώνηση
    filp->private_data = state;
    sema_init(&cvec_state->lock,1);
    // TODO: Done 
    
    return ret;
}

void intr(unsigned int intr_mask) {
    struct cvec_device *cvdev = &cvec_dev;
    // TODO: lock?
    spin_lock_irq(&cvdev->lock);
    // TODO: change status to PROCESSED in slot that was recently processed
    int slot = get_processed_slot(cvdev);
    if (slot != -1) {
        cvdev->buffer[slot].status = PROCESSED;
    }
    // TODO: unlock?
	spin_unlock_irq(&cvdev->lock);
    wake_up_interruptable(&cvdev->wq);
}

static ssize_t cvec_ioctl(struct file *filp, unsigned int cmd, unsigned long uarg) {
    struct cvec_device *cvdev = &cvec_dev;
    struct cvec_descriptor *cvdesc;
    // TODO: standard ioctl stuff
	// File descriptor not associated with character special device, or the request does not apply
	// to the kind of object the file descriptor references. (ENOTTY)
    int ret = -ENOTTY;
    int retry = -ERESTARTSYS;
	if(_IOC_TYPE(cmd) != CVEC_IOC_MAGIC) return ret;
	if(_IOC_NR(cmd) > CVEC_IOC_MAXNR) return ret;
    // TODO: Done with standard ioctl stuff

    switch(cmd){
        case CONVERT_VECTOR:
            cvdesc = kzalloc(sizeof(*cvdesc), GFP_KERNEL);

            // TODO: case of CONVERT_VECTOR
            // .. initialize structs & copy data from user space
            struct cvec_state *state;
            state = filp->private_data;
            if (down_interruptible(&state->lock)) return retry;
            if (copy_from_user(&cvdesc, (cvec_descriptor __user *) uarg, sizeof(cvec_descriptor))) {
                return -EFAULT;
            }

            // .. check if there is a free slot in the buffer otherwise sleep 
            int slot = get_free_slot(cvdev);
            if (slot == -1) {
                wait_event_interruptible(cvdev->wq, get_free_slot(cdev));
                slot = get_free_slot(cdev);
            }
            up(&state->lock);
            // .. submit the computation and sleep until it finishes
            spin_lock_irq(&cdev->lock);
            cvdev->buffer[slot].cvdesc = cvdesc;
            cvdev->buffer[slot].status = OCCUPIED;
            spin_unlock_irq(&cdev->lock);
            wait_event_interruptible(cvdev->wq, cvdev->buffer[slot].status == PROCESSED);
            // .. copy data to user space
            if (copy_to_user((char *)cvdesc.output, cvdesc.output, cvdesc.len)) {
                return -EFAULT;
            }
            // .. update the buffer 
            break;
        case SET_CONVERSION:
            // TODO: change conversion mode
            struct cvec_state *state = filp->private_data;
            if (down_interruptible(&state->lock)) return retry;
            if (state->conversion_mode == DENSE_TO_SPARSE) {
                state->conversion_mode = SPARSE_TO_DENSE;
            } else {
                state->conversion_mode = DENSE_TO_SPARSE;
            }
            up(&state->lock);
            break;
        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}
