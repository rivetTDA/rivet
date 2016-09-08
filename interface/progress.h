//
// Created by Bryn Keller on 5/17/16.
//
#pragma once
#include <boost/signals2.hpp>
#include <math/template_point.h>

class Progress {

public:
    boost::signals2::signal<void(unsigned max)> setProgressMaximum;
    boost::signals2::signal<void()> advanceProgressStage;
    boost::signals2::signal<void(unsigned)> progress;
};
