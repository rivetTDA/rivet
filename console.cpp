
#include "computation.h"
#include "dcel/arrangement.h"
#include "docopt.h"
#include "interface/input_manager.h"
#include "interface/input_parameters.h"
#include <boost/archive/tmpdir.hpp>
#include <boost/multi_array.hpp> // for print_betti
#include <interface/file_writer.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <dcel/grades.h>

#include "dcel/arrangement_message.h"
#include "dcel/serialization.h"

static const char USAGE[] =
    R"(RIVET: Rank Invariant Visualization and Exploration Tool

     The RIVET console  application computes an augmented arrangement for
     2D persistent homology, which can be visualized with the RIVET GUI app.

    Usage:
      rivet_console (-h | --help)
      rivet_console --version
      rivet_console <input_file> --identify
      rivet_console <input_file> --betti [-H <dimension>] [-V <verbosity>] [-x <xbins>] [-y <ybins>]
      rivet_console <input_file> --barcodes <line_file> [-H <dimension>] [-V <verbosity>] [-x <xbins>] [-y <ybins>]
      rivet_console <input_file> <output_file> [-H <dimension>] [-V <verbosity>] [-x <xbins>] [-y <ybins>] [-f <format>] [--binary]

    Options:
      -h --help                                Show this screen
      --version                                Show the version
      --identify                               Parse the file and print filetype information
      --binary                                 Include binary data (used by RIVET viewer only)
      -H <dimension> --homology=<dimension>    Dimension of homology to compute [default: 0]
      -x <xbins> --xbins=<xbins>               Number of bins in the x direction [default: 0]
      -y <ybins> --ybins=<ybins>               Number of bins in the y direction [default: 0]
      -V <verbosity> --verbosity=<verbosity>   Verbosity level: 0 (no console output) to 10 (lots of output) [default: 2]
      -f <format>                              Output format for file [default: R1]
      -b --betti                               Print Betti number information and exit.
      --barcodes <line_file>                   Print barcodes for the line queries in line_file, then exit.
                                               The line_file contains pairs (m, b) where m is the degree (0 to 90)
                                               and b the offset, separated by a space. Each pair should appear on
                                               a line by itself.

                                               If m < 90, then b is a y-intercept; if m = 90, then b is an x-intercept.
                                               Coordinates are assumed to be unnormalized.

                                               Example line_file contents:

                                                    #A line that starts with a # character will be ignored, as will blank lines

                                                    23 0.22
                                                    #67 10.88   <-- will error if uncommented, 10.88 > 1
                                                    67 .88
                                                    10 0.92
                                                    #100 0.92   <-- will error if uncommented, 100 > 90

                                               RIVET will output one line of barcode information for each line
                                               in line_file, beginning by repeating the query. For example:

                                                    23 0.22: 88.1838 inf x1, 88.1838 91.2549 x5, 88.1838 89.7194 x12
                                                    67 0.88: 23.3613 inf x1
                                                    10 0.92: 11.9947 inf x1, 11.9947 19.9461 x2, 11.9947 16.4909 x1, 11.9947 13.0357 x4

)";

unsigned int get_uint_or_die(std::map<std::string, docopt::value>& args, const std::string& key)
{
    try {
        return static_cast<unsigned int>(args[key].asLong());
    } catch (std::exception& e) {
        std::cerr << "Argument " << key << " must be an integer";
        throw std::runtime_error("Failed to parse integer");
        //    exit(1);
    }
}

//TODO: this doesn't really belong here, look for a better place.
void write_boost_file(InputParameters const& params, TemplatePointsMessage const& message, ArrangementMessage const& arrangement)
{
    std::ofstream file(params.outputFile, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open " + params.outputFile + " for writing");
    }
    file << "RIVET_1\n";
    boost::archive::binary_oarchive oarchive(file);
    oarchive& params& message& arrangement;
    file.flush();
}

void print_dims(TemplatePointsMessage const& message, std::ostream& ostream)
{
    assert(message.homology_dimensions.dimensionality == 2);
    auto shape = message.homology_dimensions.shape();
    auto data = message.homology_dimensions.data();
    ostream << "Dimensions > 0:" << std::endl;

    for (unsigned long row = 0; row < shape[0]; row++) {
        for (unsigned long col = 0; col < shape[1]; col++) {
            unsigned dim = data[row * shape[0] + col];
            if (dim > 0) {
                ostream << "(" << col << ", " << row << ", " << dim << ")" << std::endl;
            }
        }
        ostream << std::endl;
    }
}

void print_betti(TemplatePointsMessage const& message, std::ostream& ostream)
{
    ostream << "Betti numbers:" << std::endl;
    for (int xi = 0; xi < 3; xi++) {
        ostream << "xi_" << xi << ":" << std::endl;
        for (auto point : message.template_points) {
            auto value = 0;
            if (xi == 0)
                value = point.zero;
            else if (xi == 1)
                value = point.one;
            else if (xi == 2)
                value = point.two;
            if (value > 0) {
                ostream << "(" << point.x << ", " << point.y << ", " << value << ")" << std::endl;
            }
        }
    }
}

void process_barcode_queries(std::string query_file_name, const ComputationResult& computation_result)
{
    std::ifstream query_file(query_file_name);
    if (!query_file.is_open()) {
        std::clog << "Could not open " << query_file_name << " for reading";
        return;
    }
    std::string line;
    std::vector<std::pair<int, double>> queries;
    int line_number = 0;
    while (std::getline(query_file, line)) {
        line_number++;
        line.erase(0, line.find_first_not_of(" \t"));
        if (line.empty() || line[0] == '#') {
            std::clog << "Skipped line " << line_number << ", comment or empty" << std::endl;
            continue;
        }
        std::istringstream iss(line);
        int angle;
        double offset;

        if (iss >> angle >> offset) {
            if (angle < 0 || angle > 90) {
                std::clog << "Angle on line " << line_number << " must be between 0 and 90" << std::endl;
                return;
            }

            if (offset < 0 || offset > 1) {
                std::clog << "Offset on line " << line_number << " must be between 0 and 1" << std::endl;
                return;
            }
            queries.push_back(std::pair<int, double>(angle, offset));
        } else {
            std::clog << "Parse error on line " << line_number << std::endl;
            return;
        }
    }
    Grades grades(computation_result.arrangement->x_exact, computation_result.arrangement->y_exact);

    for (auto query : queries) {
        std::cout << query.first << " " << query.second << ": ";
        auto absolute = grades.relative_offset_to_absolute(query.second);
        auto templ = computation_result.arrangement->get_barcode_template(query.first, absolute);
        auto barcode = templ.rescale(query.first, absolute, computation_result.template_points, grades);
        for (auto it = barcode->begin(); it != barcode->end(); it++) {
            auto bar = *it;
            std::cout << bar.birth << " ";

            if (bar.death == rivet::numeric::INFTY) {
                std::cout << "inf";
            } else {
                std::cout << bar.death;
            }
            std::cout << " x" << bar.multiplicity;
            if (std::next(it) != barcode->end()) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }
}
//

int main(int argc, char* argv[])
{
    //        debug() << "CONSOLE RIVET" ;

    InputParameters params; //parameter values stored here

    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, { argv + 1, argv + argc }, true,
        "RIVET Console 0.4");

    // for (auto const &arg : args) {
    //   std::cout << arg.first << ":" << arg.second ;
    // }

    ArrangementMessage* arrangement_message = nullptr;
    TemplatePointsMessage* points_message = nullptr;

    params.fileName = args["<input_file>"].asString();
    docopt::value& out_file_name = args["<output_file>"];
    if (out_file_name.isString()) {
        params.outputFile = out_file_name.asString();
    }
    params.dim = get_uint_or_die(args, "--homology");
    params.x_bins = get_uint_or_die(args, "--xbins");
    params.y_bins = get_uint_or_die(args, "--ybins");
    params.verbosity = get_uint_or_die(args, "--verbosity");
    params.outputFormat = args["-f"].asString();
    bool betti_only = args["--betti"].isBool() && args["--betti"].asBool();
    bool binary = args["--binary"].isBool() && args["--binary"].asBool();
    bool identify = args["--identify"].isBool() && args["--identify"].asBool();
    std::string slices;
    if (args["--barcodes"].isString()) {
        slices = args["--barcodes"].asString();
    }
    if (identify) {
        params.verbosity = 0;
    }
    int verbosity = params.verbosity;

    //        debug() << "X bins: " << params.x_bins ;
    //        debug() << "Y bins: " << params.y_bins ;
    //        debug() << "Verbosity: " << params.verbosity ;

    InputManager inputManager(params);
    Progress progress;
    Computation computation(params, progress);
    progress.advanceProgressStage.connect([] {
        std::cout << "STAGE" << std::endl;

    });
    progress.progress.connect([](int amount) {
        std::cout << "PROGRESS " << amount << std::endl;
    });
    computation.arrangement_ready.connect([&arrangement_message, &params, binary](std::shared_ptr<Arrangement> arrangement) {
        arrangement_message = new ArrangementMessage(*arrangement);
        //TODO: this should become a system test with a known dataset
        //Note we no longer write the arrangement to stdout, it goes to a file at the end
        //of the run. This message just announces the absolute path of the file.
        //The viewer should capture the file name from the stdout stream, and
        //then wait for the console program to finish before attempting to read the file.
        std::stringstream ss(std::ios_base::binary | std::ios_base::out | std::ios_base::in);
        {
            boost::archive::binary_oarchive archive(ss);
            archive << *arrangement_message;
        }
        std::clog << "Testing deserialization locally..." << std::endl;
        std::string original = ss.str();
        ArrangementMessage test;
        {
            boost::archive::binary_iarchive inarch(ss);
            inarch >> test;
            std::clog << "Deserialized!";
        }
        if (!(*arrangement_message == test)) {
            throw std::runtime_error("Original and deserialized don't match!");
        }
        Arrangement reconstituted = arrangement_message->to_arrangement();
        ArrangementMessage round_trip(reconstituted);
        if (!(round_trip == *arrangement_message)) {
            throw std::runtime_error("Original and reconstituted don't match!");
        }
        if (binary) {
            std::cout << "ARRANGEMENT: " << params.outputFile << std::endl;
        } else {
            std::cout << "Wrote arrangement to " << params.outputFile << std::endl;
        }
    });
    computation.template_points_ready.connect([&points_message, binary, betti_only, verbosity](TemplatePointsMessage message) {
        points_message = new TemplatePointsMessage(message);

        if (binary) {
            std::cout << "XI" << std::endl;
            {
                boost::archive::text_oarchive archive(std::cout);
                archive << message;
            }
            std::cout << "END XI" << std::endl;
            std::cout.flush();
        }

        if (verbosity >= 4 || betti_only) {
            FileWriter::write_grades(std::cout, message.x_exact, message.y_exact);
        }
        //TODO: Add a flag to re-enable this code?
        //        std::stringstream ss;
        //        {
        //            std::cerr << "Local deserialization test" << std::endl;
        //            boost::archive::text_oarchive out(ss);
        //            out << message;
        //        }
        //        {
        //            boost::archive::text_iarchive in(ss);
        //            TemplatePointsMessage result;
        //            in >> result;
        //            if (!(message == result)) {
        //                throw std::runtime_error("Original TemplatePointsMessage and reconstituted don't match!");
        //            }
        //        }
        if (betti_only) {
            print_dims(message, std::cout);
            std::cout << std::endl;
            print_betti(message, std::cout);
            std::cout.flush();
            //TODO: this seems a little abrupt...
            exit(0);
        }
    });

    std::unique_ptr<InputData> input;
    try {
        input = inputManager.start(progress);
    } catch (const std::exception& e) {
        std::cerr << "INPUT ERROR: " << e.what() << std::endl;
        std::cerr.flush();
        return 1;
    }
    if (identify) {
        std::cout << "FILE TYPE: " << input->file_type.identifier << std::endl;
        std::cout << "FILE TYPE DESCRIPTION: " << input->file_type.description << std::endl;
        std::cout << "RAW DATA: " << input->is_data << std::endl;
        std::cout.flush();
        return 0;
    }
    if (params.verbosity >= 2) {
        debug() << "Input processed";
    }
    auto result = computation.compute(*input);
    if (params.verbosity >= 2) {
        debug() << "Computation complete";
    }
    auto arrangement = result->arrangement;
    //TESTING: print arrangement info and verify consistency
    arrangement->print_stats();
    arrangement->test_consistency();

    if (params.verbosity >= 2) {
        debug() << "COMPUTATION FINISHED.";
    }

    if (!slices.empty()) {
        process_barcode_queries(slices, *result);
        return 0;
    }
    //if an output file has been specified, then save the arrangement
    if (!params.outputFile.empty()) {
        std::ofstream file(params.outputFile);
        if (file.is_open()) {
            debug() << "Writing file:" << params.outputFile;

            if (params.outputFormat == "R0") {
                FileWriter fw(params, *input, *(arrangement), result->template_points);
                fw.write_augmented_arrangement(file);
            } else if (params.outputFormat == "R1") {
                write_boost_file(params, *points_message, *arrangement_message);
            } else {
                throw std::runtime_error("Unsupported output format: " + params.outputFormat);
            }
        } else {
            std::stringstream ss;
            ss << "Error: Unable to write file:" << params.outputFile;
            throw std::runtime_error(ss.str());
        }
    }
    debug() << "CONSOLE RIVET: Goodbye";
    return 0;
}
