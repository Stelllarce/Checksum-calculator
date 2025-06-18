#include <tclap/CmdLine.h>

int main(int argc, char** argv) {
    try{
        TCLAP::CmdLine cmd("Checksum calculator", ' ', "0.1");
        

    } catch(TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    return 0;
}