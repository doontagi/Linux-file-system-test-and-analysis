/**
 *  author : hanul lee
 *  date: 2019.11.01
 */

struct block_information { // struct for containing block information
    unsigned long long block_number;
    long sec; // seconds
    long usec; //micro seconds
    char *fs_name; // file system name
};

#define CIRCULAR_QUEUE_MAX_SIZE 1024
int circular_queue_front = 0; // place of first log
EXPORT_SYMBOL(circular_queue_front);
int circular_queue_rear = -1; // place of last log
EXPORT_SYMBOL(circular_queue_rear);
int circular_queue_size = 0; // The number of log stored in queue
EXPORT_SYMBOL(circular_queue_size);
struct block_information circular_queue[CIRCULAR_QUEUE_MAX_SIZE];
EXPORT_SYMBOL(circular_queue);

/**
 * circular_queue_enqueue - push block information to circular queue.
 */
void circular_queue_enqueue(unsigned long long block_number, struct timeval *t, char *fs_name)
{
    if (circular_queue_front == (circular_queue_rear + 1) % CIRCULAR_QUEUE_MAX_SIZE && circular_queue_size > 0) {
        // If the queue is full, move the front one step to make a space.
        circular_queue_front = (circular_queue_front + 1) % CIRCULAR_QUEUE_MAX_SIZE;
        circular_queue_size--;
    }
    circular_queue_rear = (circular_queue_rear + 1) % CIRCULAR_QUEUE_MAX_SIZE;
    circular_queue[circular_queue_rear].block_number = block_number;
    circular_queue[circular_queue_rear].sec = t-> tv_sec;
    circular_queue[circular_queue_rear].usec = t-> tv_usec;
    circular_queue[circular_queue_rear].fs_name = fs_name;
    circular_queue_size++;
}
EXPORT_SYMBOL(circular_queue_enqueue);

blk_qc_t submit_bio(int rw, struct bio *bio)
{
    bio->bi_rw |= rw;
    /*
    * If it's a regular read/write or a barrier with data attached,
    * go through the normal accounting stuff before submission.
    */
    if (bio_has_data(bio)) {
        unsigned int count;
        if (unlikely(rw & REQ_WRITE_SAME))
            count = bdev_logical_block_size(bio->bi_bdev) >> 9;
        else
            count = bio_sectors(bio);

        if (rw & WRITE) {
            struct timeval current_time;
            if(bio != NULL) {
                do_gettimeofday(&current_time); // get current time
                if (bio->bi_bdev->bd_super != NULL) {
                    // check if the file system is f2fs.
                    if(strcmp(bio->bi_bdev->bd_super->s_type->name, "f2fs") == 0) {
                        // push block number, time and fs name.
                        circular_queue_enqueue((unsigned long long) bio->bi_iter.bi_sector, &current_time,
                                               bio->bi_bdev->bd_super->s_type->name);
                    }
                }
            }
            count_vm_events(PGPGOUT, count);
        }
        else {
            task_io_account_read(bio->bi_iter.bi_size);
            count_vm_events(PGPGIN, count);
        }

        if (unlikely(block_dump)) {
            char b[BDEVNAME_SIZE];
            printk(KERN_DEBUG "%s(%d): %s block %Lu on %s (%u sectors)\n",
                    current->comm, task_pid_nr(current),
                    (rw & WRITE) ? "WRITE" : "READ",
                    (unsigned long long)bio->bi_iter.bi_sector,
                    bdevname(bio->bi_bdev, b),
                    count);
        }
    }

    return generic_make_request(bio);
}
EXPORT_SYMBOL(submit_bio);