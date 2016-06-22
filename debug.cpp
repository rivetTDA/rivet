//
// Created by Bryn Keller on 5/23/16.
//
#include "debug.h"

#ifdef QT_CORE_LIB

#include <QDebug>
using Debug = QDebug;

Debug debug(bool nospace = false) {
  auto qd = qDebug();
  if (nospace) {
    qd = qd.nospace();
  }
  return qd;
}

#else

Debug debug(bool nospace, std::ostream& out) {
    return Debug(out, !nospace);
}

#endif

