#ifndef __exception__
#define __exception__

#include <QException>
#include <QString>

class Exception : public QException
{
public:
    Exception();
    Exception(QString err_str);
//	~Exception();

    QString get_error_string();

//  const char* what() const throw();

//  void set_error_string(QString str) {error_str = str;}
//  void raise() const { throw *this; }

private:
    QString error_string;

};

#endif // __exception__
