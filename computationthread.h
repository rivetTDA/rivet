/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef COMPUTATIONTHREAD_H
#define COMPUTATIONTHREAD_H

//forward declarations
class InputManager;
struct InputParameters;
class Arrangement;

#include "dcel/barcode_template.h"
#include "math/template_point.h"

#include <QObject>
#include <QThread>

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;
#include "boost/multi_array.hpp"
typedef boost::multi_array<unsigned, 2> unsigned_matrix;

#include "dcel/arrangement_message.h"
#include <vector>

class ComputationThread : public QThread {
    Q_OBJECT

    friend class InputManager; //so that we don't have to pass all of the data structures from ComputationThread to InputManager

public:
    ComputationThread(InputParameters& params,
        QObject* parent = 0);
    ~ComputationThread();

    void compute();

    std::shared_ptr<TemplatePointsMessage> message;

signals:
    void advanceProgressStage();
    void setProgressMaximum(unsigned max);
    void setCurrentProgress(unsigned current);
    void templatePointsReady(std::shared_ptr<TemplatePointsMessage> template_points);
    void arrangementReady(std::shared_ptr<ArrangementMessage> arrangement);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    InputParameters& params;

    std::shared_ptr<ArrangementMessage> arrangement;

    void compute_from_file();
    void unpack_message_fields();
    bool is_precomputed(const std::string &file_name);
    void load_template_points_from_file(const std::string &file_name);
    void load_from_file(const std::string &file_name);
};

//TODO: Move this somewhere. See comments on implementation for details.
void write_msgpack_file(QString file_name, InputParameters const& params, TemplatePointsMessage const& message, ArrangementMessage const& arrangement);
#endif // COMPUTATIONTHREAD_H
