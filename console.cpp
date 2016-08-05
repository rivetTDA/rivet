
#include "interface/input_parameters.h"
#include "dcel/mesh.h"
#include "interface/input_manager.h"
#include "computation.h"
#include "debug.h"
#include <interface/file_writer.h>
#include "docopt.h"
#include <boost/archive/tmpdir.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "dcel/serialization.h"
#include "base_64.h"
#include "dcel/mesh_message.h"

static const char USAGE[] =
  R"(RIVET: Rank Invariant Visualization and Exploration Tool

     The RIVET console  application computes an augmented arrangement for
     2D persistent homology, which can be visualized with the RIVET GUI app.

    Usage:
      rivet_console (-h | --help)
      rivet_console --version
      rivet_console <input_file> --identify
      rivet_console <input_file> <output_file> [-H <dimension>] [-V <verbosity>] [-x <xbins>] [-y <ybins>]

    Options:
      -h --help                                Show this screen
      --version                                Show the version
      --identify                               Parse the file and print filetype information
      -H <dimension> --homology=<dimension>    Dimension of homology to compute [default: 0]
      -x <xbins> --xbins=<xbins>               Number of bins in the x direction [default: 0]
      -y <ybins> --ybins=<ybins>               Number of bins in the y direction [default: 0]
      -V <verbosity> --verbosity=<verbosity>   Verbosity level: 0 (no console output) to 10 (lots of output) [default: 2]
)";

unsigned int get_uint_or_die(std::map<std::string, docopt::value> &args, const std::string &key) {
  try {
    return static_cast<unsigned int>(args[key].asLong());
  } catch (std::exception &e) {
    std::cerr << "Argument " << key << " must be an integer" ;
      throw std::runtime_error("Failed to parse integer");
//    exit(1);
  }
}

//http://stackoverflow.com/a/2869667/224186
std::string getcwd()
{
    const size_t chunkSize=255;
    const int maxChunks=10240; // 2550 KiBs of current path are more than enough

    char stackBuffer[chunkSize]; // Stack buffer for the "normal" case
    if(getcwd(stackBuffer,sizeof(stackBuffer))!=NULL)
        return stackBuffer;
    if(errno!=ERANGE)
    {
        // It's not ERANGE, so we don't know how to handle it
        throw std::runtime_error("Cannot determine the current path.");
        // Of course you may choose a different error reporting method
    }
    // Ok, the stack buffer isn't long enough; fallback to heap allocation
    for(int chunks=2; chunks<maxChunks ; chunks++)
    {
        // With boost use scoped_ptr; in C++0x, use unique_ptr
        // If you want to be less C++ but more efficient you may want to use realloc
        std::auto_ptr<char> cwd(new char[chunkSize*chunks]);
        if(getcwd(cwd.get(),chunkSize*chunks)!=NULL)
            return cwd.get();
        if(errno!=ERANGE)
        {
            // It's not ERANGE, so we don't know how to handle it
            throw std::runtime_error("Cannot determine the current path.");
            // Of course you may choose a different error reporting method
        }
    }
    throw std::runtime_error("Cannot determine the current path; the path is apparently unreasonably long");
}

int main(int argc, char *argv[])
{
//        debug() << "CONSOLE RIVET" ;

        InputParameters params;   //parameter values stored here


        std::map<std::string, docopt::value> args = docopt::docopt(USAGE, {argv + 1, argv + argc}, true,
                                                                   "RIVET Console 0.4");

        // for (auto const &arg : args) {
        //   std::cout << arg.first << ":" << arg.second ;
        // }

        params.fileName = args["<input_file>"].asString();
    docopt::value &out_file_name = args["<output_file>"];
    if (out_file_name.isString()) {
        params.outputFile = out_file_name.asString();
    }
        params.dim = get_uint_or_die(args, "--homology");
        params.x_bins = get_uint_or_die(args, "--xbins");
        params.y_bins = get_uint_or_die(args, "--ybins");
        params.verbosity = get_uint_or_die(args, "--verbosity");
    bool identify = args["--identify"].isBool() && args["--identify"].asBool();
    if (identify) {
        params.verbosity = 0;
    }

//        debug() << "X bins: " << params.x_bins ;
//        debug() << "Y bins: " << params.y_bins ;
//        debug() << "Verbosity: " << params.verbosity ;

        InputManager inputManager(params);
        Progress progress;
        Computation computation(params, progress);
        progress.advanceProgressStage.connect([]{
            std::cout << "STAGE" << std::endl;

        });
        progress.progress.connect([](int amount) {
            std::cout << "PROGRESS " << amount << std::endl;
        });
        computation.arrangementReady.connect([](std::shared_ptr<Mesh> mesh){
            //TODO: won't work on windows, need xplatform separator.
            std::string file_name(getcwd() + "/rivet_arrangement_temp");
            std::ofstream output(file_name);

            MeshMessage arrangement(*mesh);

            {
//                boost::archive::text_oarchive archive(output);
                boost::archive::binary_oarchive archive(output);
                archive << arrangement;
            }
            std::cout.flush();
            {
                //TODO: this should become a system test with a known dataset
                std::stringstream ss(std::ios_base::binary | std::ios_base::out | std::ios_base::in);
                {
//                cereal::JSONOutputArchive archive(std::cout);
//                cereal::BinaryOutputArchive archive(ss);
//                cereal::XMLOutputArchive archive(std::cout);
                    boost::archive::binary_oarchive archive(ss);
                    archive << arrangement;

                }
                std::cerr << "Testing deserialization locally..." << std::endl;
                std::string original = ss.str();
                boost::archive::binary_iarchive inarch(ss);
                MeshMessage test;
                inarch >> test;
                std::cerr << "Deserialized!";
                if (!(arrangement == test)) {
                    throw std::runtime_error("Original and deserialized don't match!");
                }
                Mesh reconstituted = arrangement.to_mesh();
                MeshMessage round_trip(reconstituted);
                if (!(round_trip == arrangement)) {
                    throw std::runtime_error("Original and reconstituted don't match!");
                }
            }
            std::cout << "ARRANGEMENT: " << file_name << std::endl;
        });
        computation.xiSupportReady.connect([](XiSupportMessage message){
            std::cout << "XI" << std::endl;
//            cereal::JSONOutputArchive archive(std::cout);
//            cereal::BinaryOutputArchive archive(std::cout);
            {
//                cereal::XMLOutputArchive archive(std::cout);
                boost::archive::text_oarchive archive(std::cout);
                archive << message;
            }
            std::cout << "END XI" << std::endl;
            std::cout.flush();
            {
                std::cerr << "Local deserialization test" << std::endl;
                std::stringstream ss;
                boost::archive::text_oarchive out(ss);
                out << message;
                boost::archive::text_iarchive in(ss);
                XiSupportMessage result;
                in >> result;
                if (!(message == result)) {
                    throw std::runtime_error("Original XiSupportMessage and reconstituted don't match!");
                }
            }
            std::cerr << "xi support received: " << message.xi_support.size();
        });
        //TODO: input is a simple pointer, switch to unique_ptr
    std::unique_ptr<InputData> input;
    try {
        input = inputManager.start(progress);
    } catch (const std::exception &e) {
        std::cerr << "INPUT ERROR: " << e.what() << std::endl;
        return 1;
    }
        if (identify) {
            std::cout << "FILE TYPE: " << input->file_type.identifier << std::endl;
            std::cout << "FILE TYPE DESCRIPTION: " << input->file_type.description << std::endl;
            std::cout << "RAW DATA: " << input->is_data << std::endl;
            return 0;
        }
        if (params.verbosity >= 2) { debug() << "Input processed"; }
        auto result = computation.compute(*input);
        if (params.verbosity >= 2) { debug() << "Computation complete"; }
        auto arrangement = result->arrangement;
        //TESTING: print arrangement info and verify consistency
        arrangement->print_stats();
        arrangement->test_consistency();

        if (params.verbosity >= 2) { debug() << "COMPUTATION FINISHED." ; }

        //if an output file has been specified, then save the arrangement
        if (!params.outputFile.empty()) {
            std::ofstream file(params.outputFile);
            if (file.is_open()) {
                debug() << "Writing file:" << params.outputFile ;

                FileWriter fw(params, *input, *(arrangement), result->xi_support);
                fw.write_augmented_arrangement(file);
            }
            else {
                std::stringstream ss;
                ss << "Error: Unable to write file:" << params.outputFile ;
                throw std::runtime_error(ss.str());
            }
        }
        debug() << "CONSOLE RIVET: Goodbye" ;
        return 0;
}

