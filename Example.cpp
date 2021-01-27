#define EXPP_FORCE_STRING_TYPE_ASCII
#include "ExceptionsPlusPlus.h"

int main(int argc, char** argv) {
	// Invoke lambda functian of return type int
	EXPP::InvocationResult<int> ir = EXPP::invoke<int>([]() {
		// Throw exeception
		throw EXPP_EXCEPTION(unsigned int, 10, EXPP_TEXT("Demo application! Always throwing!"));
		return 0;
	});

	// Check if invocation failed
	if (ir.failed()) {
		// Print generic exception details
		printf("Invoation Failed!\nFile: %s \nLine: %i \nExeption: %s\n\n", 
			ir.getException().file(), ir.getException().line(), ir.getException().what());

		// Handle all exeption of type with lambda function
		ir.handle<unsigned int>([](unsigned int* ptrValue) {
			// Print UINT value
			printf("UINT Exception: %i\n", *ptrValue);
			//Exeption handled if value was 10
			return *ptrValue == 10;
		});

		// Check if exception was handled
		if (ir.handled()) {
			printf("Exception handled!\n");
		}
		else {
			printf("Exception NOT handled!\n");
		}

		// Return -1
		return -1;
	}

	// If not failed return invocation return value
	return ir.returnValue();
}
