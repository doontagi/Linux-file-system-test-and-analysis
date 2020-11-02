obj-m += block_log_lkm.o
PWD = /home/p4/lkm

KDIR = /home/p4/linux-4.4

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order
