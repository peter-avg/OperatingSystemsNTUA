struct input_data { 
    spinlock_t lock;
    wait_queue_head_t wq; 
    uint32_t cnt;

#define CIRC_BUF_SIZE (1024 * sizeof(measurement_t))
    char circ_buffer[CIRC_BUF_SIZE];

} input_data;

void intr(void) {
    struct input_data *inp = &input_data;
    spin_lock_irq(&inp->lock);
    memcpy(&inp->circ_buffer[inp->cnt % CIRC_BUF_SIZE], 
            device_memory, sizeof(measurement_t));
    // Το cnt χρησιμοποιείται σαν μετρητής για τον αριθμό 
    // των μετρήσεων, ανάλογο με το σύνολο το μετρήσεων που 
    // εχουν μετρηθεί η επόμενη μετρηση θα τοποθετείται
    // σε συγκεκριμένη θέση στο CIRC_BUFFER σαν LRU σχεδόν
    inp->cnt += sizeof(measurement_t);
    spin_unlock_irq(&inp->lock);
    // Ξύπνα τα processes που περιμένουν data
    wake_up_interruptable(&inp->wq);
}

struct chrdev_state {
    struct semaphore lock;
    struct input_data *inp;
    // Χρειάζομαι κάτι άλλο εδώ;
}

#define wait_event_interruptible(waitqueue, condition)
unsigned long copy_to_user(void __user *dst, const void *src, unsigned long len);

static int chrdev_open(struct inode *inode, struct file *filp) {
    struct chrdev_state *state = kmalloc(sizeof(*state), GFP_KERNEL);
    filp->private_data = state;
    // Συμπλήρωσε
    int ret = -ENODEV;

    if ((ret = nonseekable_open(inode,filp)) < 0) {
        kfree(state);
        return ret;
    }
    sema_init(&state->lock,1);
out:
    return 0;
}

static ssize_t chrdev_read(struct file *filp, char __user *usrbuf, 
        size_t cnt, loff_t *f_pos) {
    struct chrdev_state *state = filp->private_data;
    struct input_data *inp = state->inp;
    // Συμπλήρωσε
    uint32_t bytes_to_copy = sizeof(measurement_t);
    uint32_t available = inp->cnt;
    
    if (down_interruptible(&state->lock)) {
        return -ERESTARTSYS;
    }

    while(cnt > 0) {
        if (inp->cnt == 0) {
            if (wait_event_interruptible(inp->wq, inp->cnt > 0)) {
                up(&state->lock);
                return -ERESTARTSYS;
            }
        }

        if (copy_to_user(usrbuf, &inp->circ_buffer[(CIRC_BUF_SIZE - available) % CIRC_BUF_SIZE],
                    bytes_to_copy)){
            return -EFAULT;
        }

        *f_pos += bytes_to_copy;
        usrbuf += bytes_to_copy;
        cnt -= bytes_to_copy;
        available -= bytes_to_copy;


    }

    up(&state->lock);
    return 0;
}


