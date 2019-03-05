#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "filesystem.h"
#include "rtc.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");


static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 20; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	// spot check that the idt table is filled with values we want
	for(i = 0; i < NUM_VEC; i++) {
		if( idt[i].size <= 0x0 || idt[i].present != 0x1 ) {
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/*
 * Tests should interrupt and blue screen with divide by zero error
 * Assert that our IDT table works and can throw errors
 *
 * no output because error is thrown
*/
int divide_by_zero_test() {
	TEST_HEADER;
	int y = 1;

	int x = 0;
	// asm volatile ("int $0x00");
	y = y / x;
	return PASS;
}

/*
 * Checks that page directory exists between 0 and 4MB (4194304 bytes)
 * Asserts that our page directory exists in the range that it should and
 * is not present outside of this
 *
 * output pass or fail on test result
*/

/**
 * Check that the memory locations for paging is valid in the range we want
 * for kernel (4-8MB) by dereferencing memory addresses
 *
 * Returns pass or will cause a memory fault
 */
int paging_dereferencing_valid_range_test() {
	TEST_HEADER;

	int *test_deref_location;
	int *test_deref_pass;

	// kernel at 4-8MB
	test_deref_location = (int *)0x400001;	// memory location at 4MB+1 (4194304 + 1 bytes)
	*test_deref_pass = *test_deref_location;

	test_deref_location = (int *)0x799999;	// memory location at 8MB-1 (8388608 - 1 bytes)
	*test_deref_pass = *test_deref_location;

	return PASS;
}


/**
 * Check that the memory locations for paging cannot access locations that it
 * shouldn't. We should not be able to access 0x01
 *
 * Does cause memory fault
 */
int  paging_dereferencing_invalid_test() {
	TEST_HEADER;

	int *test_deref_location;
	int *test_deref_pass;

	// test things not at 4-8MB
	test_deref_location = (int *)0x800001;	// memory location at 8MB+1 (8388608 + 1 bytes)
	*test_deref_pass = *test_deref_location;
	return PASS;
}
int read_by_index_test(){
	TEST_HEADER;
	int i;
	dentry_t* test;
	read_dentry_by_index(5,test); //should print "."
	for(i=0;i<32;i++){
	printf("%s",test->name[i]);
  }
	return PASS;
}

/* Checkpoint 2 tests */
/**
 * Test that RTC open works
 * RTC open returns 0 when complete
 */
int rtc_open_test() {
	TEST_HEADER;

	if(rtc_open( 0 )) {
		return FAIL;
	}
	return PASS;
}

/**
 * Test that RTC  read works
 * RTC read should return 0 once the interrupt is handled
 */
int rtc_read_test() {
	TEST_HEADER;

	// if we do not get a return of 0 from rtc read, our interrupt flag is not clearing
	if(rtc_read( 0, (void*)0, 4 )) {
		return FAIL;
	}
	return PASS;
}

/**
 * Tests that RTC write works for writing different (valid and non-valid) frequencies
 * Verifies that we can only write frequencies in the range of 2 to 1024 Hz
 */
int rtc_write_freq_test() {
	TEST_HEADER;

	int write_result = 0;
	int pow2_counter = 0;
	int frequency = 1024;
	while(frequency >= 2) {
		write_result = rtc_write( 0, &frequency, 4 );

		if(write_result != 0) {
			return FAIL;
		}

		pow2_counter += 1;
		frequency = frequency >> pow2_counter;
	}

	frequency = 2048;
	if(rtc_write( 0, &frequency, 4 ) != -1) {
		return FAIL;
	}
	frequency = 1;
	if(rtc_write( 0, &frequency, 4 ) != -1) {
		return FAIL;
	}
	return PASS;
}


/**
 * Tests that changing the RTC frequency changes the write speed to the screen
 * Starts at 2Hz for RTC open, and then iterates through the valid frequencies to verify
 * that frequency changes
 */
int rtc_write_freq_print_test(int rate) {
	int max_frequency = 1024;
	int frequency;

	frequency = max_frequency >> rate;

	rtc_write( 0, &frequency, 4 );

	return PASS;
}


/**
 * Test that RTC close works
 * RTC close should return 0 on close
 */
int rtc_close_test() {
	TEST_HEADER;

	if( !rtc_close( 0 ) ) {
		return PASS;
	}
	return FAIL;
}

//TODO Still need to write tests for terminal, keyboard, and the file system open close functions

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//clear();
	clear();
	// checkpoint 1 tests
	//TEST_OUTPUT("idt_test", idt_test());
	//file_read(); //
	//TEST_OUTPUT( "divide_by_zero_test", divide_by_zero_test() );
	//TEST_OUTPUT( "valid page dereferencing test", paging_dereferencing_valid_range_test() );
	//TEST_OUTPUT( "invalid page dereferencing test", paging_dereferencing_invalid_test() );
	//TEST_OUTPUT( "RTC open test", rtc_open_test() );
	//TEST_OUTPUT( "RTC read test", rtc_read_test() );
	//TEST_OUTPUT( "RTC write frequency test", rtc_write_freq_test() );
	//TEST_OUTPUT( "RTC frequency print test", rtc_write_freq_print_test(0) );	// between 0 and 9
	//TEST_OUTPUT( "RTC close test", rtc_close_test() );
}
