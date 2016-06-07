#include "exception.h"

Exception::Exception()
{ }

Exception::Exception(QString err_str) :
    error_string(err_str)
{ }

QString Exception::get_error_string()
{
    return error_string;
}
