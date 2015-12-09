#ifndef __CAM_EXCEPTION_H__
#define __CAM_EXCEPTION_H__

#include <cstring>
#include <exception>
#include <string>

#include <errno.h>


/*
 *	Do not use constructor!! Use below MACRO!!
 */
class cam_exception : public std::exception {
public:
	cam_exception(const char* file, int line, std::string msg) { 
		format_string(file, line, ::strerror(errno), errno, msg);
	}
	cam_exception(const char* file, int line, int err) { 
		format_string(file, line, "", err, ::strerror(err));
	}
	~cam_exception() throw() { }
	virtual const char* what() const throw() { 
		return d_ret.c_str();
	}
private:
	std::string d_ret;

	void format_string(std::string file, int line, std::string errmsg, int err, std::string msg) {
#ifdef MDEBUG
		d_ret = file + "@" + std::to_string(line) + "(" + errmsg + "(" + std::to_string(err * -1) + ")): " + msg;
#else
		d_ret = msg; 
#endif
	}
};

#define cam_except(msg) 	cam_exception(__FILE__, __LINE__, msg)


#endif