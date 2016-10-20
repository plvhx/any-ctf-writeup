#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#define ISSPACE(c)  ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define ISASCII(c)  (((c) & ~0x7f) == 0)
#define ISUPPER(c)  ((c) >= 'A' && (c) <= 'Z')
#define ISLOWER(c)  ((c) >= 'a' && (c) <= 'z')
#define ISALPHA(c)  (ISUPPER(c) || ISLOWER(c))
#define ISDIGIT(c)  ((c) >= '0' && (c) <= '9')

static unsigned long local_strtoul( char *nstr, char **endptr, int base ) {
#if !( defined(__KERNEL__) )
	return strtoul(nstr, endptr, base);    /* user mode */
#else
	char *s = nstr;
	unsigned long acc;
	unsigned char c;
	unsigned long cutoff;
	int neg = 0, any, cutlim;

	do {
		c = *s++;
	} while ( ISSPACE(c) );

	if ( c == '-' ) {
		neg = 1;
		c = *s++;
	}
	else if ( c == '+' )
		c = *s++;

	if ( (base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X') ) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if ( base == 0 )
		base = c == '0' ? 8 : 10;

	cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
	cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;

	for ( acc = 0, any = 0; ; c = *s++ ) {
		if ( !ISASCII(c) )
			break;

		if ( ISDIGIT(c) )
			c -= '0';
		else if ( ISALPHA(c) )
			c -= ISUPPER(c) ? 'A' - 10 : 'a' - 10;
		else
			break;

		if ( c >= base )
			break;

		if ( any < 0 || acc > cutoff || (acc == cutoff && c > cutlim) )
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}

	if ( any < 0 ) {
		acc = INT_MAX;
	}
	else if ( neg )
		acc = -acc;

	if ( endptr != 0 )
		*((const char **)endptr) = any ? s - 1 : nstr;

	return ( acc );
#endif
}

static dev_t first;	// Global variable for the first device number
static struct cdev c_dev;	// Global variable for the character device structure
static struct class *cl;	// Global variable for the device class

struct tostring_s {
	unsigned int pointer;
	unsigned int pointer_max;
	unsigned long long int *tostring_stack;
	ssize_t ( *tostring_read )( struct file *f, char __user *buf, size_t len, loff_t *off );
};

static struct tostring_s *tostring;
static unsigned int taille = 2;

module_param( taille, int, 1 );

MODULE_PARM_DESC( taille, "Stack size in Ko" );


static int tostring_open( struct inode *i, struct file *f ) {
	printk(KERN_INFO "Tostring: open()\n");

	return ( 0 );
}

static int tostring_close( struct inode *i, struct file *f ) {
	printk(KERN_INFO "Tostring: close()\n");

	return ( 0 );
}

static ssize_t tostring_read( struct file *f, char __user *buf, size_t len, loff_t *off ) {
	printk(KERN_INFO "Tostring: read()\n");

	return ( (tostring->tostring_read)(f, buf, len, off) );
}

static ssize_t tostring_read_hexa(struct file *f, char __user *buf, size_t len, loff_t *off) {
	printk(KERN_INFO "Tostring: read_hexa()\n");

	if ( tostring->pointer > 0 )
		return ( snprintf( buf, len, "%16llx\n", tostring->tostring_stack[--(tostring->pointer)] ) );
	else
		return ( 0 );
}

static int tostring_create( int tl ) {
	/*  tostring=kmalloc(sizeof(struct tostring_s), GFP_DMA); */
	taille = tl;
	tostring->tostring_stack = kmalloc((taille * 1024), GFP_DMA);

	if ( unlikely(tostring->tostring_stack == NULL) )
		return( -1 );

	tostring->pointer_max = (taille * 1024) / sizeof(long long int);
	tostring->tostring_read = tostring_read_hexa;

	printk(KERN_INFO "Tostring: Stack size: %dK, locate at %p, max index: %d\n", taille, tostring->tostring_stack, tostring->pointer_max);

	return ( 0 );
}


static ssize_t tostring_read_dec( struct file *f, char __user *buf, size_t len, loff_t *off ) {
	printk(KERN_INFO "Tostring: read_dec()\n");

	if ( tostring->pointer > 0 )
		return ( snprintf( buf, len, "%lld\n", tostring->tostring_stack[--(tostring->pointer)] ) );

	else
		return ( 0 );
}

static ssize_t tostring_write( struct file *f, const char __user *buf, size_t len, loff_t *off ) {
	char *bufk;
	int i, j;

	printk(KERN_INFO "Tostring: write()\n");

	bufk = kmalloc(len + 1, GFP_DMA);

	if ( likely(bufk != NULL) ) {
		if (copy_from_user(bufk, buf, len))
			return -EFAULT;

		bufk[len] = '\0';

		i = 0;

		while ( i < len ) {
			for ( j = 0; ( j < 10 ) && ( bufk[j] == '*' ); j++ );

			if ( j == 10 ) {
				for ( j = i + 10; ( bufk[j] != '\0' ) && ( bufk[j] != '\n' ); j++ );

				bufk[j] = '\0';

				printk(KERN_INFO "Tostring: Cmd %s\n", bufk + i + 10);

				switch ( bufk[i + 10] ) {
				case 'H':
					tostring->tostring_read = tostring_read_hexa;

					break;
				case 'D':
					tostring->tostring_read = tostring_read_dec;

					break;
				case 'S':
					printk(KERN_INFO "Tostring: Delete stack\n");

					kfree(tostring->tostring_stack);

					// this is our target.. :)
					tostring->tostring_stack = NULL;
					tostring->tostring_read = NULL;
					tostring->pointer = 0;
					tostring->pointer_max = 0;

					break;
				case 'N':
					printk(KERN_INFO "Tostring: Stack create with size %ld\n", local_strtoul(bufk + i + 11, NULL, 10));

					if ( unlikely(tostring->tostring_stack == NULL) )
						tostring_create( local_strtoul(bufk + i + 11, NULL, 10) );

					if ( unlikely(tostring->tostring_stack == NULL) )
						printk(KERN_ALERT "Tostring: Error, impossible to create stack\n");

					break;
				}

				i = j + 1;
			}
			else {
				printk(KERN_INFO "tostring: insertion %lld\n", *((long long int *)(bufk + i)));

				if ( tostring->pointer >= tostring->pointer_max )
					printk(KERN_INFO "Tostring: Stack full\n");
				else
					tostring->tostring_stack[(tostring->pointer)++] = *((long long int *)(bufk + i));

				i = i + sizeof(long long int);
			}
		}

		kfree(bufk);
	}

	return ( len );
}

static struct file_operations pugs_fops = {
	.owner = THIS_MODULE,
	.open = tostring_open,
	.release = tostring_close,
	.read = tostring_read,
	.write = tostring_write,
};

static int __init tostring_init( void ) {
	printk(KERN_INFO "Registering /dev/tostring ..\n");

	tostring = kmalloc(sizeof(struct tostring_s), GFP_DMA);

	if ( unlikely(tostring == NULL) ) {
		printk(KERN_ALERT "Registering /dev/tostring failed due to failed allocation..\n");

		return ( -1 );
	}

	tostring_create( taille );

	if ( alloc_chrdev_region(&first, 0, 8, "tostring") < 0 ) {
		printk(KERN_ALERT "Failed to allocate character device region for /dev/tostring ..\n");

		return -1;
	}

	if ( (cl = class_create(THIS_MODULE, "chardrv")) == NULL ) {
		printk(KERN_ALERT "Failed to create character device class on this module..\n");

		unregister_chrdev_region(first, 1);

		return ( -1 );
	}

	if ( device_create(cl, NULL, first, NULL, "tostring") == NULL ) {
		printk(KERN_ALERT "Failed to create /dev/tostring ..\n");

		class_destroy(cl);

		unregister_chrdev_region(first, 1);

		return ( -1 );
	}

	cdev_init(&c_dev, &pugs_fops);

	if ( cdev_add(&c_dev, first, 1) == -1 ) {
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);

		return ( -1 );
	}

	printk(KERN_INFO "<Major, Minor>: <%d, %d>\n", MAJOR(first), MINOR(first));

	return ( 0 );
}

static void __exit tostring_exit(void) {
    printk(KERN_INFO "/dev/tostring unregistered..\n");

    kfree(tostring->tostring_stack);
    unregister_chrdev_region(first, 1);
}

module_init( tostring_init );
module_exit( tostring_exit );

MODULE_LICENSE("GPL");
MODULE_AUTHOR("F.Boisson");
MODULE_DESCRIPTION("Module Tostring Integers Dec/Hex");
