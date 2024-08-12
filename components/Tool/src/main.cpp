#include <iostream>
#include <stddef.h>
#include <iostream>
#include "algorithm.h"

int main(int argc, char* argv[])  
{  
    if (argc < 3) {  
        std::cerr << "Usage: " << argv[0] << " <output_file> <input_file1> <input_file2> ... <input_fileN>\n";  
        return 1;  
    }  
  
    std::list<std::string> filenames;  
    for (int i = 2; i < argc; ++i) {  
        filenames.push_back(argv[i]);  
    }  
  
    Algorithm::merge(argv[1], filenames);  
  
    return 0;  
}