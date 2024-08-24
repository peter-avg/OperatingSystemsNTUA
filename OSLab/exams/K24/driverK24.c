// Write a character device driver with the structure below and these specifications
// Each write needs to return bytes equal to a single sizeof(patient_data)
// Sequential reads need to return sequential measurements only from when open happened and on 
// If for any reason sequential reads can't happen, then the second read should return EOF
// after an EOF, for a new stream of data, the process needs to close() and open() again
// if a process reads all measurements, it sleeps and waits for new measurements through the interrupt 

struct medical_dev {
    // DONE: Lock type ? 
    spinlock_t lock; 

    waitqueue_t wq;
    uint128_t cnt; /* Initialised to zero and will never wrap */

#define CIRC_BUF_SIZE (1024 * sizeof(struct patient_data))
    char circ_buffer[CIRC_BUF_SIZE];
} medical_dev;

void intr(void) { 
    struct medical_dev *dev = &medical_dev;
    struct patient_data pd;

    get_patient_data_from_hw(&pd); /* Get data from real device */

    spin_lock_irq(&medical_dev->lock);
    memcpy(&dev->circ_buffer[dev->cnt % CIRC_BUF_SIZE],
       &pd, sizeof(struct patient_data));
    dev->cnt += sizeof(struct patient_data);
    spin_unlock_irq(&medical_dev->lock);

    wake_up_interruptable(&dev->wq); 
}

struct chrdev_state { 
    // DONE: Lock type ? 
    struct semaphore lock;

    struct medical_dev *medical_dev;

    char local_buf[sizeof(struct patient_data)];
    uint128_t local_cnt; /* Suppose it will never wrap */
}

static int medical_chrdev_open(struct inode *inode, struct file *filp) { 
    struct chrdev_state *state;
    struct medical_dev *dev = &medical_dev;
     
    if ((ret = nonseekable_open(inode, filp)) < 0) {
        kfree(state);
        return -ENODEV;
    }

    state = kmalloc(sizeof(chrdev_state), GFP_KERNEL);
    if (!state) {
        return -ENOMEM;
    }

    state->local_cnt = dev->cnt;
    state->medical_dev = dev;
    filp->private_data = state;
    sema_init(&state->lock,1);
    
    return ret; 
}

static ssize_t medical_chrdev_read(struct file *filp, char __user *usrbuf,
        size_t cnt, loff_t *f_pos) {

    struct chrdev_state *state;
    struct medical_dev *dev;
    
    state = filp->private_data;
    dev = state->medical_dev;

    uint32_t bytes_to_copy = sizeof(patient_data);

    if (down_interruptible(&state->lock)) {
        return -ERESTARTSYS;
    }
    
    // Do we need to fetch a new measurement
    if (dev->cnt == state->local_cnt) {
        if (wait_event_interruptable(dev->wq, dev->cnt > state->local_cnt) > 0) {
            up(&state->lock);
            return -ERESTARTSYS;
        }
    }

    if (*f_pos == 0) {
        if (dev->cnt - state->local_cnt >= CIRC_BUF_SIZE) {
            f_pos = 0;
            return 0;
        }
    }

    // Copy data from circular buffer
    memcpy(state->local_buf,
            &dev->circ_buffer[state->local_cnt % CIRC_BUF_SIZE],
            bytes_to_copy);

    // Send the data to user
    if (copy_to_user(usrbuf, state->local_buf, bytes_to_copy)) {
        up(&state->lock);
        return -EFAULT;
    }

    state->local_cnt += bytes_to_copy;
    *f_pos += bytes_to_copy;

    up(&state->lock);
    return bytes_to_copy;
}
        

