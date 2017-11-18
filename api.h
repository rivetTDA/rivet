//
// Created by Bryn Keller on 11/16/17.
//

#ifndef RIVET_CONSOLE_API_H
#define RIVET_CONSOLE_API_H

#include "dcel/anchor.h"
#include "dcel/arrangement.h"
#include "dcel/barcode_template.h"
#include "dcel/dcel.h"
#include "type_tag.h"
#include <boost/optional.hpp>
#include <boost/serialization/split_member.hpp>
#include <msgpack.hpp>
#include "dcel/msgpack_adapters.h"
#include "computation.h"
#include "dcel/dcel.h"
#include "dcel/arrangement_message.h"
#include <memory>

std::unique_ptr<ComputationResult> from_messages(
        const TemplatePointsMessage &templatePointsMessage,
        const ArrangementMessage &arrangementMessage);

std::vector<std::unique_ptr<Barcode>> query_barcodes(const ComputationResult &computation,
                                                     const std::vector<std::pair<double, double>> &offset_slopes);

std::unique_ptr<ComputationResult> from_istream(std::istream &file);
#endif //RIVET_CONSOLE_API_H
