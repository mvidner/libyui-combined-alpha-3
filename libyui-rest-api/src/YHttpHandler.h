/*
  Copyright (C) 2017 SUSE LLC

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) version 3.0 of the License. This library
  is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details. You should have received a copy of the GNU
  Lesser General Public License along with this library; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
  Floor, Boston, MA 02110-1301 USA
*/

#ifndef YHttpHandler_h
#define YHttpHandler_h

#include <microhttpd.h>

#if MHD_VERSION >= 0x00097002
/**
 * Data type to use for functions return an "MHD result".
 */
#define MHD_RESULT enum MHD_Result
#else
/**
 * Data type to use for functions return an "MHD result".
 */
#define MHD_RESULT int
#endif

#include <string>
#include <iostream>

struct MHD_Connection;

class YHttpHandler
{

public:

    YHttpHandler() {}
    virtual ~YHttpHandler() {}

    virtual MHD_RESULT handle(struct MHD_Connection* connection,
        const char* url, const char* method, const char* upload_data,
        size_t* upload_data_size, bool *redraw = nullptr);


protected:

    virtual void process_request(struct MHD_Connection* connection,
        const char* url, const char* method, const char* upload_data,
        size_t* upload_data_size, std::ostream& body, int& error_code,
        std::string& content_type, bool *redraw) = 0;

    int handle_error(std::ostream& body, std::string error, int error_code);
};

#endif // YHttpHandler_h
