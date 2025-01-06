#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class RedirStdOut {
    public:
        RedirStdOut(std::string filename) {
            info_file = output_folder_name + "/statistics/" + filename;
            buffer.str("");
            old_cout_buf = std::cout.rdbuf();
            cout_buf = std::cout.rdbuf(buffer.rdbuf());
            printf("Saving %s\n", filename.c_str());
        }
        ~RedirStdOut() {
            std::ofstream fout(info_file.c_str());
            fout << buffer.str();
            fout.close();
            std::cout.rdbuf(old_cout_buf);
        }
    private:
        std::string info_file;
        std::stringstream buffer;
        std::streambuf *old_cout_buf;
        std::streambuf *cout_buf;
};