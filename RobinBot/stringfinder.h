#ifndef STRINGFINDER_H
#define STRINGFINDER_H

class StringFinder
{
public:
	void clear();
	void add(const char* the_string, int the_number);
	void erase( const char* str );
	bool find(const char* str);
	int          num;
	const char*  str;
	void it_start  ();
	bool it_running();
	void it_next   ();
private:
	void* data;
public:
	StringFinder();
	~StringFinder();
};

#endif