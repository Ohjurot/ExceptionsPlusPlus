# Exceptions Plus Plus
ExceptionsPlusPlus is a free, easy to use, accessible (MIT License or The Unlicense), templated and header only c++ typed exception throwing and handling library. 

This is why you should you this library:

```c++
struct MyData;
typedef unsigned int ErrCode;
bool CreateNewData(MyData**);

// Potentially dangerous function
int myFunction(MyData** ppData) noexcept{
    // Invoke function
    EXPP::InvocationResult<int> invoke = EXPP::invoke<int>(&myFunction_s, *ppData);
    if (invoke.succeeded()) 
        return invoke.returnValue();
    
    // Nullptr error is handled if CreateNewData() succeeds
    invoke.handle<ErrCode>(ERR_CODE_NULLPTR, []() {
		return CreateNewData(ppData);
	});
    
    // If exception was handled return 0
    if (invoke.handled()) {
		return 0;
	} 
    // If exception was not handled return -1
    else {
		return -1;
	}
}

// Safe function
int myFunction_s(MyData* pData){
    // Throw a nullptr exception if pointer is null
    if(!pData){
        throw EXPP_EXCEPTION(
            ErrCode, 						// Type
            ERR_CODE_NULLPTR, 				// Value
            EXPP_TEXT("Invalid pointer")	// Description
        ); 
    }
    
    // ...
    
    return 0;
}
```



> :warning: **WARNING**: Even if thees examples implies the usage of exception as a generic programming pattern you should <u>not</u> use exceptions for normal runtime features! Exception should only catch states which are not resolvable within the current or parent context. Creating `EXPP::InvocationResult` and `EXPP::Exception<T>` is creating additional overhead by calling `malloc`, `free` and `memcpy` so do not recreate them every cycle (Frame, Loop-Iteration, ...)

## Usage

### Including the header

When including the header you are able to define several macros to modify the behavior of the library. Please make sure to use the same macros for every file include in the same build configuration. The only file you need from this repository is `ExceptionsPlusPlus.h`

#### Forcing string type

The library is build to support `char` and `wchar_t` strings for representing the exception line and  description. By the default the library will use `char` and switch to `wchar_t` when the `_UNICODE` macro is set. 

Independent of that functionality you can force the lib to use the character type of your choice. You need to define `EXPP_FORCE_STRING_TYPE_ASCII` for forcing `char` or `EXPP_FORCE_STRING_TYPE_UNICODE` for `wchar_t`  respectively. 

Please keep in mind that when using `wchar_t` an OS dependent conversion function is required since the default `__FILE__` macro will always present the file as `char` and C++ deprecated the `std::codecvt` conversion. This feature is currently only implemented on Windows but feel free to contribute implementations for your preferred OS. Search for `ADD YOUR OS CHAR -> WCHAR CONVERSION HERE` in the header. If you desire to use the without implementing an conversion but using wide char, you can remove the ability to include the file and line of the exception by defining `EXPP_OMMIT_SOURCE`.

#### Omitting the source of the exception

If you desire to omit the source file name and line where the exception occurred you can define `EXPP_OMMIT_SOURCE`. This may be desirable in a production build. 

### Using the library

The library is use in three steps

#### Safely Invoking a method or lambda

In step one you want to invoke a portion of your code via the library. The invocation can be done via calling a existing arbitrary function (Function pointer) or via an in line lambda. Invoking the library is done by calling `EXPP::invoke<T>(function/lamda, argument, ...)` where `T` is the return type (currently `void` is not supported)  and `argument, ...` are zero, one or many argument(s) required by the function. The function returns an object of type `EXPP::InvocationResult<T>` which represents the termination result of the function invocation.

```c++
int myFunction(const wchar_t* string, char** ppConversionResult){
	return 0;
}

std::string str(int stringId, int langId){
    std::wstring localString = getLocalString(stringId, langId);
    
    EXPP::InvocationResult<int> invokeResult = EXPP::invoke<int>(&myFunction, localString.c_str());
}
```

or

```c++
std::wstring str(int stringId, int langId){
	std::wstring localString = getLocalString(stringId, langId);

    EXPP::InvocationResult<int> invokeResult = EXPP::invoke<int>([](){
    	return 0;
    });
}
```

#### Throwing an exception

When you detect an abnormal unresolvable error and the function should terminate irregularly and immediately you throw an exception of the type `EXPP::Exception<T>` by using the `EXPP_EXCEPTION(T, value, text)` macro. Where `T` is inner exception type (An example would `HRESULT` on windows), `value` is the invalid value / error code of the exception (Needs to be from type `T`) and `text` is a description of the operation trying to archive (An example for a failed DirectX12 `D3D12CreateDevice(...)` call could be "Device creation from IDXGIAdapter*"). Make sure to use the `EXPP_TEXT("...")` macro to get the correct string type. 

```c++
std::wstring str(int stringId, int langId){
    std::wstring localString = getLocalString(stringId, langId);
    std::string result;
    
    EXPP::InvocationResult<int> invokeResult = EXPP::invoke<int>([](){
    	int newLen = CheckConversion(in.c_str());
    	if(newLen == -1){
    		throw EXPP_EXCEPTION(ErrCode_t, ERR_CODE_INVALID_CHAR, EXPP_TEXT("Invalid character"));
    	}
    	
    	// ...
    	
    	return newLen;
    });
}
```

#### Handling the result of the invocation

Handling of the invocation involves two steps: Handling the success case and handling the failed case

Handling of the success case is strait forward. By using`bool EXPP::InvocationResult<T>::succeeded()` and `T EXPP::InvocationResult<T>::returnValue()` you can easily implement the success case (witch should be before the failed case for performance reasons!) 

```c++
std::wstring str(int stringId, int langId){
    std::wstring localString = getLocalString(stringId, langId);
    std::string result;
    
    EXPP::InvocationResult<int> invokeResult = EXPP::invoke<int>([](){
    	int newLen = CheckConversion(in.c_str());
    	if(newLen == -1){
    		throw EXPP_EXCEPTION(ErrCode_t, ERR_CODE_INVALID_CHAR, EXPP_TEXT("Invalid character"));
    	}
    	
    	// ...
    	
    	return newLen;
    });
    
    if(invokeResult.succeeded()){
        return result;
    }
}
```

Handling the failed case is a bit more involved and can be done in several ways! Please consider looking at the API documentation. We will only present one possible solution here. The lambda inside the call `bool EXPP::InvocationResult<T>::handle<ET>(value, lambda)` will only be invoked if the exception type was of type `ET` and has the value `value`. The call itself will only evaluate to true if the lambda was called and returned true. When the lambda was called and yield true all following calls to `handle()` will yield false no matter if matched or not! The exception is marked as handled internally.

```c++
std::wstring str(int stringId, int langId){
    std::wstring localString = getLocalString(stringId, langId);
    std::string result;
    
    EXPP::InvocationResult<int> invokeResult = EXPP::invoke<int>([](){
    	int newLen = CheckConversion(in.c_str());
    	if(newLen == -1){
    		throw EXPP_EXCEPTION(ErrCode_t, ERR_CODE_INVALID_CHAR, EXPP_TEXT("Invalid character"));
    	}
    	
    	// ...
    	
    	return newLen;
    });
    
    if(invokeResult.succeeded()){
        return result;
    }
    
    if(invokeResult.handle<ErrCode_t>(ERR_CODE_INVALID_CHAR), [](){
        return langId != DEFAULT_LANG_ID;
    }){
        return str(stringId, DEFAULT_LANG_ID);
    }
}
```

Finally a default value or invalid value should be returned. Alternatively if another level of exception handling is present the exception can be raised by calling `EXPP::InvocationResult<T>::rais()`. When `...::rais()` is called the lib will do nothing in case the exception never existed or was handled. If an exception is in existence it will throw the original exception (With all the original data). 

```c++
std::wstring str(int stringId, int langId){
    std::wstring localString = getLocalString(stringId, langId);
    std::string result;
    
    EXPP::InvocationResult<int> invokeResult = EXPP::invoke<int>([](){
    	int newLen = CheckConversion(in.c_str());
    	if(newLen == -1){
    		throw EXPP_EXCEPTION(ErrCode_t, ERR_CODE_INVALID_CHAR, EXPP_TEXT("Invalid character"));
    	}
    	
    	// ...
    	
    	return newLen;
    });
    
    if(invokeResult.succeeded()){
        return result;
    }
    
    if(invokeResult.handle<ErrCode_t>(ERR_CODE_INVALID_CHAR), [](){
        return langId != DEFAULT_LANG_ID;
    }){
        return str(stringId, DEFAULT_LANG_ID);
    }
    
    invokeResult.rais();
    return result;
}
```

## API Reference 

In the following section we will present you a reference for all the Types and Function you should and can use. If you are interested in the internal types and architecture we recommend to go through the commented header file on your own. 

### `EXPP::BaseException<T>`

`T` - String type (will be set automatically)

#### `size_t dynamicType()`

Will return the hash of the type this exception is currently holding.

#### `const T* what()`

Will return the description string of the exception.

#### `const T* file()`

Will return the file string of the exception.

#### `int line()`

Will return the line number of the exception.

------

### `EXPP::Exception<T, ST> : public BaseException<ST>` 

`T` - Return type of the invocation (currently void is not supported!)
`ST` - String type (will be set automatically)

#### `T& get()`

Will return a reference to the inner exception.

#### `static constexpr size_t staticType()`

Will return the hash value of the exception type.

------

### `EXPP::ExceptionAccessObject<ST>`

`ST` - String type (will be set automatically)

#### `bool testExceptionType<T>()`

`T` - Type of exception to test for

Will return `true` if the inner exception type is the same type than `T` is.

#### `T* getExceptionType<T>()`

`T` - Type of exception to test for

Will return an `EXPP::Exception<T, ST>` pointer if exception type and `T` match. Else it will return `nullptr`

------

### `EXPP::InvocationResult<T, ST> : public EXPP::ExceptionAccessObject<ST>`

`T` - Return type of the invocation (currently void is not supported!)

#### `bool succeeded()`

Will return `true` if the invocation succeeded. This means no exception occurred or was handled and resolved in a lower exception handler of any kind.

#### `bool failed()`

Will return `true` if the invocation failed. This means an exception of any kind occurred and was catch by the library's invoke function.

#### `EXPP::BaseException<ST>& getException()`

`ST` - String Type (will be automatically set)

Will return a reference to a `BaseException<T>` holding all non typed data of the exception.

#### `void handle()`

Will mark the exception (in case one exists) as handled without allying any kinds of constrains.

#### `bool handle<ET>()`

`ET` - Type of exception

Will mark the exception as handled if `ET ` matches the inner exception type.
Will return `true` only if this call marked the exception as handled.

#### `bool handle<ET, AT>(AT value)`

`ET` - Type of the exception
`AT` - Type of the argument needs to be cast able to `ET` (will be deduced automatically)

Will mark the exception as handle if `ET ` matches the inner exception type and the value of the inner exception is equal to `value`
Will return `true` only if this call marked the exception as handled.

#### `bool handle<ET, FT, AT>(FT functionPointer, AT... args)`

`ET` - Type of the exception
`FT` - Type of function pointer or lambda (will be deduced automatically)
`AT` - Variadic types of the function or lambda arguments (will be deduced automatically)

The last argument of `functionPointer` has to be `ET*`  no matter if any or how many `args` are required.
Will call the lambda `functionPointer` with it's `args` only if `ET ` matches the inner exception type and exception was not handled.
Will return `true `if the lambda was called and returned true.

#### `bool handle<ET, FT, AT>(ET value, FT functionPointer, AT... args)`

`ET` - Type of the exception
`FT` - Type of function pointer or lambda (will be deduced automatically)
`AT` - Variadic types of the function or lambda arguments (will be deduced automatically)

The last argument of `functionPointer` has to be `ET*`  no matter if any or how many `args` are required.
Will call the lambda `functionPointer` with it's `args` only if `ET ` matches the inner exception type, value of the inner exception is equal to `value` and exception was not handled.
Will return `true` if the lambda was called and returned true.

#### `bool handled()`

Will return `true` if no exception existed or the exception was already handled.

#### `void rais()`

Will throw the inner exception if it exist and and was not handled. 

#### `T returnValue()`

Will return the return value when the invocation succeeded. Will return `T()` if the invocation failed.

------

### `EXPP::invoke<T, ST, FT, FA>(FT functionPtr, FA... args)`

`T` - Return type of the function to be invoked
`ST` - String type (will be set automatically)
`FT` - Type of function pointer or lambda (will be deduced automatically)
`AT` - Variadic types of the function or lambda arguments (will be deduced automatically)

Will invoke the function `functionPtr` with it's arguments `args` (if any) in an exception safe manor.
Will return an `EXPP::InvocationResult<T, ST>` which holds the result of the invocation.

