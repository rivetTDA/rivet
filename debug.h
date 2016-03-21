#pragma once

#include <iostream>

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

using Debug = std::ostream&;

Debug debug(bool nospace = false, std::ostream& out = std::clog) {
  return out;
}

#endif
