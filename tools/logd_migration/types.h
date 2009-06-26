#include <iostream>
#include <assert.h>


#define ALLOC_STEP 4

enum RequestServiceType {LC_UNIX_WHOIS, LC_WEB_WHOIS, LC_PUBLIC_REQUEST, LC_EPP, LC_WEBADMIN};

struct ccLogProperty {
	char * name;
	char * value;
	bool output;
	bool child;
};

struct ccProperties
{ 
	int _maximum, _length; 
	ccLogProperty* _buffer; 
	bool _release; 
	size_t length() const { return _length; };

	const ccLogProperty &operator [] (int index) const {
		assert(index < _length);
		return _buffer[index];
	}
	~ccProperties() {
		try {
			// delete[] (_buffer);
			free (_buffer);
		} catch(...) {
			std::cout << "Exception in ccProperties destructor" << std::endl;
		}
	}
};


