#include <assert.h>
#include <stdio.h>

void __assert_fail(const char* __assertion, const char* __file, unsigned int __line) {
	printf("Assertion failed: \"%s\" in file %s at line %d\n", __assertion, __file, __line);

	#warning "Call exit() once implemented!";
	while (1) {
		__asm__ __volatile__ ("hlt");
	}
	
}