typedef struct {} wait_queue_head_t;
// έχω ένα status για ένα dca_request
// μπορεί να έιναι FREE, RESERVED, FINISHED
typedef enum {
    DCA_REQ_FREE,
    DCA_REQ_RESERVED,
    DCA_REQ_FINISHED,
} dca_status_t;

// Μέγεθος buffer λογικά για input και output
#define MAX_DATA_SZ (4 << 20)
typedef struct {
    int input[MAX_DATA_SZ];
    int result[MAX_DATA_SZ];
    dca_status_t status;
    wait_queue_head_t request_wq;
} dca_request_t;

#define MAX_REQS (64*sizeof(dca_request_t))
typedef struct {
    spinlock_t lock;
    wait_queue_head_t slots_wq;
    dca_request_t reqs[MAX_REQS];
} dca_dev_t;

dca_dev_t dca_dev;

// Συνάρτηση για αλλαγή του state σε ένα request
void dca_req_set_status(dca_request_t *req, dca_status_t status) {
    req->status = status;
}

dca_request_t find_free_request(dca_dev_t *cdev) {
    int i;

    for (i=0; i<MAX_REQS; i++) {
        if (cdev->reqs[i].status == DCA_REQ_FREE) {
            return cdev->reqs[i];
        }
    }
    return NULL;
}

dca_request_t *dca_is_req_finished(dca_request_t *req) {
    return (req->status == DCA_REQ_FINISHED);
}

// Implemented elsewhere
void dca_notify_device(dca_request_t *req);

void dca_intr(void) {
    dca_dev_t *dev = &dca_dev;
    int i;
    // .. ? .. lock
    spin_lock_irq(&dev->lock);
    for (i=0; i<MAX_REQS; i++) {
        if (dca_is_req_finished(&dca_dev->reqs[i])) {
            wake_up_interruptable(&cdev->reqs[i].request_wq);
            dca_req_set_status(&cdev->req[i], DCA_REQ_FREE);
        }
    }
    spin_unlock_irq(&dev->lock);
    // .. ? .. unlock
}

static int dca_chrdev_open(struct inode *inode, struct file *filp) {
    int ret = 0;
    dca_dev_t *dev = &dca_dev;

    if((ret = nonseekable_open(inode,filp)) < 0) {
        ret = -ENODEV;
        goto out;
    }

    filp->private_data = dev;
    return ret;
}

typedef struct {
    uint8_t input[MAX_DATA_SZ];
    uint8_t result[MAX_DATA_SZ];
} dca_user_request_t;

#define DCA_MAGIC 'D'
#define DCA_SUBMIT_REQ _IORW(DCA_MAGIC, 0, cda_user_request_t);

static long dca_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    dca_user_request_t __user *argp = (dca_user_request_t __user *) arg;
    dca_dev_t *dev = filp->private_data;
    int ret = -ENOTTY;
    int retry = -ERESTARTSYS;
	if(_IOC_TYPE(cmd) != DCA_MAGIC) return ret;

    switch(cmd) {
        case DCA_SUBMIT_REQ:

            // Εύρεση επόμενου ελεύθερου slot στο device, αναμονή αν δεν υπάρχει ελεύθερη θέση
            spin_lock_irq(&dev->lock);
            dca_request_t req = find_free_request(&dev);
            if (!req) { 
                wait_event_interruptible(dev->slots_wq,find_free_request(&dev));
                req = find_free_request(&dev);
            }

            // αντιγραφή δεδομένων από user space για το request
            if (copy_from_user(&req->input, &argp->input, sizeof(argp->input))) {
                return -EFAULT;
            }

            // θέσε το status του request ως reserved για το request του user
            dca_req_set_status(req, DCA_REQ_RESERVED);
            spin_unlock_irq(&dev->lock);

            // ενημέρωσε τη συσκευή
            dca_notify_device(req);
            // αναμονή μέχρι το request να γίνει finished
            wait_event_interruptible(req->request_wq, req->status == DCA_REQ_FINISHED);
            if (copy_to_user(argp->result, req->result, sizeof(req->result))) {
                return -EFAULT;
            }
            break;

        default:
            return -EINVAL;
    }

}


