// #include "tests.h"
// #include "x86_desc.h"
// #include "lib.h"
// #include "filesys.h"
// #include "terminal.h"

// /* Implicit declaration fixes: */
// int32_t write_rtc(int32_t fd, const void* buf, int32_t nbytes);
// int32_t read_rtc(int32_t fd, void* buf, int32_t nbytes);
// int32_t open_rtc(const uint8_t *filename);

// #define PASS 1
// #define FAIL 0

// /* format these macros as you see fit */
// #define TEST_HEADER 	\/ /* remove / to compile*/
// 	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
// #define TEST_OUTPUT(name, result)	\/ /* remove / to compile*/
// 	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

// static inline void assertion_failure(){
// 	/* Use exception #15 for assertions, otherwise
// 	   reserved by Intel */
// 	asm volatile("int $15");
// }


// /* Checkpoint 1 tests */

// /* IDT Test - Example
//  * 
//  * Asserts that first 10 IDT entries are not NULL
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: Load IDT, IDT definition
//  * Files: x86_desc.h/S
//  */
// int idt_test() {
// 	TEST_HEADER;

// 	int i;
// 	int result = PASS;
// 	for (i = 0; i < 10; ++i){
// 		if ((idt[i].offset_15_00 == NULL) && 
// 			(idt[i].offset_31_16 == NULL)){
// 			assertion_failure();
// 			result = FAIL;
// 		}
// 	}

// 	return result;
// }

// /* IDT Test 
//  * Inputs: None
//  * Outputs: FAIL if expection doesnt happen and passes if idt print message shown 
//  * Side Effects: causes exception from divide by 0 
//  * Coverage: Load IDT, IDT definition
//  */
// // add more tests here
// int divzTest() {
// 	int result = FAIL;
// 	int i;
// 	i = 0;
// 	int j; 
// 	j = 3; //just a test val 
// 	int k;

// 	k = j/i;

// 	return result;
// }

// /* sysCallTest
//  * 
//  * Asserts that first 10 IDT entries are not NULL
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: Load IDT, IDT definition
//  * Files: x86_desc.h/S
//  */
// //testing system call #INT(x80) 
// int sysCallTest(){
// 	int result = FAIL;

// 	asm volatile ("INT $0x80"); // x80 addr for system call port 
// 	return result;
// }

// /* pageFaultTest
//  * Inputs: None
//  * Outputs: page fault or shows pass if valid 
//  * Side Effects: None
//  * Coverage: paging 
//  */
// int pageFaultTest() {
// 	int result = PASS; 
// 	int * testval;
// 	//outside of vid mem( 0xb8000 ->0xb9000), kernel mem(0x400000 -> 0x800000)
// 	testval = (int *)0xb7999; 			// Out of range - FAULT - prints message of page fault on console
// 	//testval = (int *)0xb8001; 		// Inside Range - PASS - shows as pass in test output
// 	// testval = (int *)0xb8FFF;		// Inside Range - PASS - shows as pass in test output
// 	// testval = (int *)0x3FFFFF;		// Out of range - FAULT - prints message of page fault on console
// 	// testval = (int *)0x400000;		// Inside Range - PASS - shows as pass in test output
// 	// testval = (int *)0x800001;		// Out of range - FAULT - prints message of page fault on console
// 	// testval = (int *)0x0;		// null check Out of range - FAULT - prints message of page fault on console
// 	*testval = 3; //3 is random val to test 
// 	return result;
// }


// /* Checkpoint 2 tests */

// /* testFileDrivers
//  * 
//  * does the ls 
//  * Inputs: None
//  * Outputs:
//  * Side Effects: None
//  * Coverage: 
//  * Files: 
//  */
// int testFileDrivers(){
// 	clear();
// 	set_screen_x(0);
// 	set_screen_y(0);
// 	int i = 0, j = 0;

// 	uint32_t fd_temp = 1;
// 	printf("Printing all files: \n \n");
// 	open_dir((uint8_t *)(".")); // open the directory

// 	for(i = 0; i<17;i++){
// 		int8_t curfname[32];
// 		read_dir(fd_temp,curfname,32);
// 		printf("FILE NAME: ");

// 		for(j = 0; j <32; j++){
// 			printf("%c",curfname[j]);
// 		}

// 		printf("\n");
// 	}

// 	close_dir(fd_temp);


// 	return 0;
// }

// /* testRTC
//  * print a char at set freq below 
//  * Inputs: None
//  * Outputs:
//  * Side Effects: None
//  * Coverage: 
//  * Files: 
//  */
// int testRTC(int frequency){
// 	clear();
// 	set_screen_x(0);
// 	set_screen_y(0);
// 	uint32_t fd_temp = 1;
// 	uint8_t testName[6] = "testF";
// 	//need to uncomment putc2 line in rtc.c
// 	open_rtc(testName);
// 	int checkfreq = 0;
// 	checkfreq = write_rtc(fd_temp, &(frequency), 4);
// 	if (checkfreq < 0 ) {
// 		printf("Invalid freq");
// 		return -1;
// 	}
// 	while (1) {
// 		checkfreq = read_rtc(fd_temp, &(frequency), 4);
// 		if (checkfreq < 0 ) {
// 			printf("Could not read");
// 			return -1;
// 		}
// 		putc2('a');
// 	}
// 	return 0;
// }

// /* terminalTest()
//  * Enables user input to a terminal
//  * Inputs: None
//  * Outputs: None
//  * Side Effects: None
//  * Coverage:
//  * Files: 
//  */
// void terminalTest(){
// 	clear();
// 	set_screen_x(0);
// 	set_screen_y(0);

// 	uint8_t buf[128];
// 	uint32_t fd = 1;
// 	int32_t nbytes;
// 	nbytes = 128;
// 	terminal_open(buf);
// 	while(1){
// 		terminal_read(fd, buf, nbytes);
// 		terminal_write(fd, buf, nbytes);
// 	}
// 	terminal_close(fd);
// 	// uint8_t* text;
// 	// text = "Rope may be constructed of any long, stringy, fibrous material, but generally is constructed of certain natural or synthetic fibres.[1][2][3] Synthetic fibre ropes are significantly stronger than their natural fibre counterparts, they have a higher tensile strength, they are more resistant to rotting than ropes created from natural fibres, and they can be made to float on water.[4] But synthetic ropes also possess certain disadvantages, including slipperiness, and some can be damaged more easily by UV light.[5]Common natural fibres for rope are Manila hemp, hemp, linen, cotton, coir, jute, straw, and sisal. Synthetic fibres in use for rope-making include polypropylene, nylon, polyesters (e.g. PET, LCP, Vectran), polyethylene (e.g. Dyneema and Spectra), Aramids (e.g. Twaron, Technora and Kevlar) and acrylics (e.g. Dralon). Some ropes are constructed of mixtures of several fibres or use co-polymer fibres. Wire rope is made of steel or other metal alloys. Ropes have been constructed of other fibrous materials such as silk, wool, and hair, but such ropes are not generally available. Rayon is a regenerated fibre used to make decorative rope.The twist of the strands in a twisted or braided rope serves not only to keep a rope together, but enables the rope to more evenly distribute tension among the individual strands. Without any twist in the rope, the shortest strand(s) would always be supporting a much higher proportion of the total load.";
// 	// terminal_write(fd, text, 1000);
// }


// /* testFilesys
//  * 
//  * tests reading file data 
//  * Inputs: None
//  * Outputs:
//  * Side Effects: None
//  * Coverage: 
//  * Files: 
//  */
// int testFilesys(uint8_t *testfname){
// 	clear();
// 	set_screen_x(0);
// 	set_screen_y(0);
	
// 	int32_t fd = 1;
// 	int i = 0;
// 	uint8_t buf[180000];
// 	for (i = 0; i<180000; i++) {
// 		buf[i] = '\0';
// 	}
// 	int numb = -2;
// 	// numb = read_dentry_by_name(testfname, (dentry_t *)(&testdir));
// 	numb = open_file(testfname);
// 	if (numb < 0 ) {
// 		printf("INVALID FILENAME: %s",testfname);
// 		return -1;
// 	}
	
// 	printf("FILENAME: %s",testfname);
// 	printf("\n");

// 	numb = read_file(fd, buf,180000);
// 	if (numb < 0 ) {
// 		printf("INVALID FILENAME: %s",testfname);
// 		return -1;
// 	}
// 	terminal_write(fd, buf, 180000);
	
	

// 	close_file(fd);
// 	//terminal_write(fd, testfname, 5);
// 	//printf("\nFILE NAME: %s",testfname);

// 	// for (i = 0; i<strlen(testfname); i++) {
// 	// 	putc2(testfname[i]);
// 	//  }
// 	// printf("num bytes = %u", numb);
// 	// printf("%s", buf);
// 	return 0;
	
// }

// int testReadData()
// {
// 	clear();
// 	set_screen_x(0);
// 	set_screen_y(0);
// 	int i;

// 	uint8_t buf[200000];
	
// 	for (i = 0; i <200000; i++)
// 		buf[i] = NULL;

// 	int fd = 1;
// 	dentry_t tdentry;

// 	read_dentry_by_name((const uint8_t *)"ls", &tdentry);
	

// 	// printf("%d", tdentry->inode);

// 	read_data(tdentry.inode, 0, buf, 200000);
// 	//printf("%s", buf);
// 	// puts2(buf);
// 	terminal_write(fd, buf, 200000);


// 	//printf(fd, buf, 180000);

// 	return 0;
// }


// /* Checkpoint 3 tests */
// /* Checkpoint 4 tests */
// /* Checkpoint 5 tests */

// /* Test suite entry point */
// void launch_tests() {
// 	// launch your tests here 
// /* Checkpoint 1 */
// 	// 	TEST_OUTPUT("idt_test", idt_test());						// Given IDT Test
// 	//	TEST_OUTPUT("divz_test", divzTest());					// Divide by 0 test
// 	//	TEST_OUTPUT("Page Fault Test", pageFaultTest());			// Page Fault Test
// 	//	TEST_OUTPUT("System Call Test", sysCallTest());				// System Call Test
// 	// Our RTC Test is checked through rtc.c where we call test_interrupts() to check frequency.
// /* Checkpoint 2 */
// 	testReadData();
// 	/* Test 1: List all files: */
// 	//testFileDrivers();

// 	/* Test 2: List file by name: */
// 			// Normal files:
// 	// testFilesys((uint8_t *)"frame0.txt");
// 	// testFilesys((uint8_t *)"frame1.txt");
	
// 	// 		Executables:	
// 	// testFilesys((uint8_t *)"cat");
// 	//testFilesys((uint8_t *)"ls");
// 	// testFilesys((uint8_t *)"grep");

// 	// 		Large files:
// 	// testFilesys((uint8_t *)"verylargetextwithverylongname.txt");
// 	// testFilesys((uint8_t *)"verylargetextwithverylongname.tx");
// 	// testFilesys((uint8_t *)"fish");

// 	// 		Others (don't use)
// 	// testFilesys((uint8_t *)"hello");
// 	// testFilesys((uint8_t *)"shell");
// 	// testFilesys((uint8_t *)"sigtest");
// 	// testFilesys((uint8_t *)"syserr");
// 	// testFilesys((uint8_t *)"testprint");

// 	// 		Bad files
// 	// testFilesys((uint8_t *)"mystery.txt");


// 	/* Test 3: RTC Test (make sure you uncomment putc2 in rtc.c) */
// 	// testRTC(1024);
// 	// testRTC(2);
// 	// testRTC(128);
// 	// testRTC(123);

// 	/* Test 4: Terminal Tests */
// 	// terminalTest();
// }


