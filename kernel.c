#if 1  // PREAMBLE
/**
** @file	kernel.c
**
** @author	Numerous CSCI-452 classes
**
** @brief	Miscellaneous OS support routines.
*/

#define SP_KERNEL_SRC

#include "common.h"

#include "kernel.h"
#include "queues.h"
#include "procs.h"
#include "users.h"

#include "bootstrap.h"
#include "cio.h"
#include "clock.h"
#include "kmem.h"
#include "sched.h"
#include "sio.h"
#include "support.h"
#include "syscalls.h"
#include "filesystem.h"
#include "fshelper.h"
#include "ramdisk.h"

// need address of the init() function
USERMAIN( init );

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

/*
** PUBLIC GLOBAL VARIABLES
*/

// character buffers, usable throughout the OS
// not guaranteed to retain their contents across an exception return
char _b256[256];
char _b512[512];

// Other kernel variables that could be defined here:
//
//	   system time
//	   pointer to the current process
//	   information about the initial process
//		   pid, PCB pointer
//	   information about the idle process (if there is one)
//		   pid, PCB pointer
//	   information about active processes
//		   static array of PCBs, active count, next available PID
//	   queue variables
//	   OS stack & stack pointer
//

// table of state name strings
// [0]: long name, [1]: short name
const char *_state_str[N_STATES][2] = {
	[ Unused   ] = { "Unused",	 "Unu" },
	[ New	   ] = { "New",		 "New" },
	[ Ready	   ] = { "Ready",	 "Rdy" },
	[ Running  ] = { "Running",	 "Run" },
	[ Sleeping ] = { "Sleeping", "Slp" },
	[ Blocked  ] = { "Blocked",	 "Blk" },
	[ Waiting  ] = { "Waiting",	 "Wtg" },
	[ Killed   ] = { "Killed"	 "Kil" },
	[ Zombie   ] = { "Zombie",	 "Zom" }
};

// table of priority name strings
const char *_prio_str[N_PRIOS] = {
	[ SysPrio      ] = "Sys",
	[ UserPrio	   ] = "Usr",
	[ DeferredPrio ] = "Def"
};

/*
** PRIVATE FUNCTIONS
*/

/**
** _kreport - report the system configuration
**
** Prints configuration information about the OS on the console monitor.
**
** @param dtrace  Decode the TRACE options
*/
static void _kreport( bool_t dtrace ) {

	__cio_puts( "\n-------------------------------\n" );
	__cio_printf( "Config:	N_PROCS = %d", N_PROCS );
	__cio_printf( " N_PRIOS = %d", N_PRIOS );
	__cio_printf( " N_STATES = %d", N_STATES );
	__cio_printf( " CLOCK = %dHz\n", CLOCK_FREQUENCY );

	// This code is ugly, but it's the simplest way to
	// print out the values of compile-time options
	// without spending a lot of execution time at it.

	__cio_puts( "Options: " );
#ifdef RPT_INT_UNEXP
	__cio_puts( " U_Ints" );
#endif
#ifdef RPT_INT_MYSTERY
	__cio_puts( " M_Ints" );
#endif
#ifdef TRACE_CX
	__cio_puts( " CX" );
#endif
#ifdef SANITY
	__cio_printf( " SANITY = %d", SANITY );
#endif
#ifdef CONSOLE_STATS
	__cio_puts( " Cstats" );
#endif
#ifdef STATUS
	__cio_printf( " STATUS = %d", STATUS );
#endif

#if TRACE > 0
	__cio_printf( " TRACE = 0x%04x\n", TRACE );

	// decode the trace settings if that was requested
	if( TRACING_SOMETHING && dtrace ) {

		// this one is simpler - we rely on string literal
		// concatenation in the C compiler to create one
		// long string to print out

		__cio_puts( "Tracing:"
#	if TRACING_PCB
			" PCB"
#	endif
#	if TRACING_STACK
			 " STK"
#	endif
#	if TRACING_QUEUE
			 " QUE"
#	endif
#	if TRACING_SCHED
			 " SCHED"
#	endif
#	if TRACING_SYSCALLS
			 " SYSCALL"
#	endif
#	if TRACING_SYSRETS
			 " SYSRET"
#	endif
#	if TRACING_EXIT
			 " EXIT"
#	endif
#	if TRACING_DISPATCH
			 " DISPATCH"
#	endif
#	if TRACING_CONSOLE
			 " CONS"
#	endif
#	if TRACING_KMEM
			 " KM"
#	endif
#	if TRACING_KMEM_FREE
			 " KMFL"
#	endif
#	if TRACING_SPAWN
			 " SPAWN"
#	endif
#	if TRACING_SIO_STAT
			 " S_STAT"
#	endif
#	if TRACING_SIO_ISR
			 " S_ISR"
#	endif
#	if TRACING_SIO_RD
			 " S_RD"
#	endif
#	if TRACING_SIO_WR
			 " S_WR"
#	endif
			 ); // end of __cio_puts() call
	}
#endif

	__cio_puts( "\n-------------------------------\n" );
}


#if defined(CONSOLE_STATS)
/**
** _kshell - callback routine for console input
**
** Called by the CIO module when a key is pressed on the
** console keyboard.  Depending on the key, it will print
** statistics on the console display, or will cause the
** user shell process to be dispatched.
**
** This code runs as part of the CIO ISR.
*/
static void _kshell( int code ) {

	switch( code ) {

	case 'a':  // dump the active table
		_ptable_dump( "\nActive processes", false );
		break;

	case 'c':  // dump context info for all active PCBs
		_ctx_dump_all( "\nContext dump" );
		break;

	case 'p':  // dump the active table and all PCBs
		_ptable_dump( "\nActive processes", true );
		break;

	case 'q':  // dump the queues
		// code to dump out any/all queues
		_que_dump( "Sleep", _sleeping );
#ifdef QNAME
		_que_dump( "SIO", QNAME );
#endif
		_que_dump( "R[S]", _ready[SysPrio] );
		_que_dump( "R[U]", _ready[UserPrio] );
		_que_dump( "R[D]", _ready[DeferredPrio] );
		break;

	case 'r':  // print system configuration information
		_kreport( true );
		break;

	case 's':  // dump stack info for all active PCBS
		__cio_puts( "\nActive stacks (w/5-sec. delays):\n" );
		register pcb_t *pcb = _processes;
		for( register int i = 0; i < N_PROCS; ++i, ++pcb ) {
			if( pcb->state != Unused ) {
				__cio_printf( "pid %5u: ", pcb->pid );
				__cio_printf( "EIP %08x, ", pcb->context->eip );
				_stk_dump( NULL, pcb->stack, 12 );
			}
		}
		break;

		// ignore CR and LF
	case '\r': // FALL THROUGH
	case '\n':
		break;
 
	default:
		__cio_printf( "shell: unknown request '0x%02x'\n", code );
		// FALL THROUGH

	case 'h':  // help message
		__cio_puts( "\nCommands:\n"
					 "	 a	-- dump the active table\n"
					 "	 c	-- dump contexts for active processes\n"
					 "	 h	-- this message\n"
					 "	 p	-- dump the active table and all PCBs\n"
					 "	 q	-- dump the queues\n"
					 "	 r	-- print system configuration\n"
					 "	 s	-- dump stacks for active processes\n"
					);
		break;
	}
}
#endif

/*
** PUBLIC FUNCTIONS
*/

/**
** _kinit - system initialization routine
**
** Called by the startup code immediately before returning into the
** first user process.
*/
void _kinit( void ) {

	/*
	** BOILERPLATE CODE - taken from basic framework
	**
	** Initialize interrupt stuff.
	*/

	__init_interrupts();  // IDT and PIC initialization

	/*
	** Console I/O system.
	**
	** Does not depend on the other kernel modules, so we can
	** initialize it before we initialize the kernel memory
	** and queue modules.
	*/

#if defined(CONSOLE_STATS) 
	__cio_init( _kshell );
#else
	__cio_init( NULL );	   // no console callback routine
#endif

#ifdef TRACE_CX
	// define a scrolling region in the top 7 lines of the screen
	__cio_setscroll( 0, 7, 99, 99 );
	// clear the top line
	__cio_puts_at( 0, 0, "*																				  " );
	// separator
	__cio_puts_at( 0, 6, "================================================================================" );
#endif

	/*
	** TERM-SPECIFIC CODE STARTS HERE
	*/

	/*
	** Initialize various OS modules
	**
	** Other modules (clock, SIO, syscall, etc.) are expected to
	** install their own ISRs in their initialization routines.
	*/

	__cio_puts( "System initialization starting.\n" );
	__cio_puts( "-------------------------------\n" );

	__cio_puts( "Modules:" );

	// call the module initialization functions, being
	// careful to follow any module precedence requirements
	//
	// classic order:  kmem; queue; everything else

	_km_init();		// MUST BE FIRST
#if TRACING_KMEM || TRACING_KMEM_FREE
	__delay(50);	// about 1.25 seconds
#endif

	_que_init();	// MUST BE SECOND
#if TRACING_QUEUE
	__delay(50);
#endif

	// other module initialization calls here
	_clk_init();
	_pcb_init();
#if TRACING_PCB
	__delay(50);
#endif
	_sch_init();
	_stk_init();
#if TRACING_STACK
	__delay(50);
#endif
	_sio_init();
	_sys_init();
#if TRACING_SYSCALLS || TRACING_SYSRETS
	__delay(50);
#endif
#endif // PREAMBLE
	int result = -1;
  	result = _fs_init();
	if (result != 0) {
        __cio_printf("Error initializing filesystem\n");
        __panic("Failed to init FS\n");
    }
// #if TRACING_FS TODO uncomment
	__cio_putchar('\n');
	// __delay(50);
	phn("INITIALIZED", 0);
#if 0  // BEG Disk driver testing
	// Initialize storage interface with RAM disk backend
    StorageInterface storage;
    result = _storage_init(&storage);
    if (result != 0) __panic("Failed to init Storage!\n");
	
	storage.init(DISK_SIZE);
    if (result != 0) __panic("Failed to init RAMdisk!\n");

    // Test storage operations
    uint32_t block;
    char data1[] = "Hello_, RAM disk!";
    char data2[] = "Goodbye_, RAM disk!";
    char data3[] = "Goodbye, RAM diskKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKk!";
    char buffer[100];

    // Write data to RAM disk
    result = storage.write(data1, sizeof(data1));
    if (result <= 0) {
        __cio_printf("Error writing data to RAM disk\n");
        return;
    }

	result = storage.write(data2, sizeof(data2));
    if (result <= 0) {
        __cio_printf("Error writing data to RAM disk\n");
        return;
    }

    result = storage.write(data3, sizeof(data3));
    if (result <= 0) {
        __cio_printf("Error writing data to RAM disk\n");
        return;
    }

	// ramdisk_print();

    // Read data from RAM disk
    result = storage.read(3, buffer, sizeof(buffer));
    if (result != 0) {
        __cio_printf("Error reading data from RAM disk\n");
        return;
    }
    // Print the read data
    __cio_printf("Data read from RAM disk: %s\n", buffer);


	result = ramdisk_request_space(16);
	if (result <= 0){
		__cio_printf("Error requesting space on RAM disk\n");
		return;
	}

	result = ramdisk_release_space(2);
	if (result != 0){
		__cio_printf("Error releasing space on RAM disk\n");
		return;
	}

	ramdisk_print();

    char data4[] = "Goodbye, RAM disk!";

	result = storage.write(data4, sizeof(data4));
    if (result <= 0) {
        __cio_printf("Error writing data to RAM disk\n");
        return;
    }

	ramdisk_print();

	__cio_printf("Filesystem implementation complete\n");
#endif // END Disk driver testing

#if 0 //  BEG FS init
	ph("FILESYSTEM");

	const char *path = "";
	const char *filename = "";
	// DirectoryEntry *root = _fs_find_entry("/");
	DirectoryEntry *de = NULL;
	show_header_info(true);

	// __cio_printf("I hate unused warnings %s %d %s %s %s:\n", root->filename, result, path, filename, de->filename);

#endif // END FS init
	// ph("post init");
#if 0  // BEG Simple filesystem tests
	
#if 1 // Data polulation
	filename = "/test/file";
	DirectoryEntry *entry = _fs_create_file(filename);
	filename = "/test/file2";
	DirectoryEntry *entry2 = _fs_create_file(filename);
	filename = "/you/will/be/impressed";
	DirectoryEntry *entry3 = _fs_create_file(filename);
	filename = "/told/you/so";
	DirectoryEntry *entry4 = _fs_create_dir(filename);
	filename = "/told/you/LOL";
	DirectoryEntry *entry5 = _fs_create_file(filename);
	filename = "/told/myself/nothing";
	DirectoryEntry *entry6 = _fs_create_dir(filename);
	plnn();
	dr();
	pln();
	filename = "/told/you/LOL";
	DirectoryEntry *found = _fs_find_entry(filename);
	_fs_print_entry(found);
	pln();
	
	plw('@');
	__delay(STEP);
#endif

	get_path();

# if 0
	/** Working in CWD **/
	plnn();
	change_dir(path);
	pwd();
	__delay(DEBUG_DELAY);

	plnn();
	change_dir("../");
	__delay(DEBUG_DELAY);
	// _fs_print_entry(root);
	// dr();
	// plnn();
	// list_dir_contents("/newdir", false);
	// list_dir_contents("/newdir", true);
	plnn();
	result = _fs_create_dir("/new");
	// void *buffer = _km_page_alloc(1);
	plnn();
	result = _fs_create_dir("/new/newer");
	// dr();

	plnn();
	path = "/rootdir/otherdir";
	result = change_dir(path);
	plnn();
	// get_path();
	result = _fs_print_entry(de);
	dr();

	// change_dir("/");

	// list_dir_contents("/new");
	// _fs_create_entry_from_path("/newdir/sub2");
	// list_dir_contents("dir");
#endif

#endif // END Simple filesystem tests


#if 0  // BEG FS / DD Integration
	ph("FILESYSTEM / DISK DRIVER INTEGRATION");
	

#endif // END FS / DD Integration

	plw('#');
	// __delay( INF_PAUSE );
// #endif // TRACING end

#if 1  // POSTAMBLE
	__cio_puts( "\nModule initialization complete.\n" );
	__cio_puts( "-------------------------------\n" );

	// report our configuration options
	_kreport( false );


	/*
	** Other tasks typically performed here:
	**
	**	Enabling any I/O devices (e.g., SIO xmit/rcv)
	*/

	/*
	** Create the initial user process
	** 
	** This code is largely stolen from the fork() and exec()
	** implementations in syscalls.c; if those change, this must
	** also change.
	*/

    // allocate the necessary data structures
    pcb_t *pcb = _pcb_alloc();
    assert( pcb != NULL );

    pcb->stack = _stk_alloc();
    assert( pcb->stack != NULL );

    // fill in the PCB
    pcb->pid = pcb->ppid = PID_INIT;
    pcb->state = New;
    pcb->priority = SysPrio;

    // process context area and initial stack contents
	char *args[] = { "init", "+", NULL };
    context_t *ctx = _stk_setup( pcb->stack, (uint32_t) init, args );
    assert( ctx != NULL );

    // remember where the context area is
    pcb->context = ctx;

	// remember which PCB is 'init'
	_init_pcb = pcb;

    // schedule and dispatch it
    assert( _schedule(pcb) == S_OK );
    _dispatch();

	/*
	** END OF TERM-SPECIFIC CODE
	**
	** Finally, report that we're all done.
	*/

	__cio_puts( "System initialization complete.\n" );
	__cio_puts( "-------------------------------\n" );

	_sio_enable( SIO_RX );
}

/**
** _kpanic - kernel-level panic routine
**
** usage:  _kpanic( msg )
**
** Prefix routine for __panic() - can be expanded to do other things
** (e.g., printing a stack traceback)
**
** @param msg  String containing a relevant message to be printed if
**			   it is non-NULL
*/
void _kpanic( const char *msg ) {

	__cio_puts( "\n\n***** KERNEL PANIC *****\n\n" );
	__cio_printf( "Msg: %s\n", msg ? msg : "(none)" );

	// dump a bunch of potentially useful information

	// EXAMPLES

	// dump the contents of the current PCB
	_pcb_dump( "Current", _current );

	// dump the contents of the process table
	_ptable_dump( "Processes", false );

#if PANIC_DUMPS_QUEUES
	// dump the entire contents of the queues
	// _queue_dump( "Sleep queue", _sleeping );
	// etc.
#else
	// just dump the queue sizes
	// __cio_printf( "Queue sizes:	sleep %d", _q_length(_sleeping) );
	// etc.
#endif

	// could dump other stuff here, too

	// could do a stack trace for the kernel, or for the current process

   __panic( "KERNEL PANIC" );
}
#endif // POSTAMBLE