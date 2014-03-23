#include "data_utils.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <SDL2/SDL.h>

using namespace std;
//using namespace glw;
namespace glTools {

//void File2Str(string &str, const string &file_name)
//{
//    ifstream t(file_name);
//    if(!t.is_open()){
//        str.clear();
//        return;
//    }
//    t.seekg(0, ios::end);
//    str.resize((long)t.tellg() + 1);
//    t.seekg(0, ios::beg);
//
//    str.assign((istreambuf_iterator<char>(t)),
//            istreambuf_iterator<char>());
//    //str.at(str.size()) = '\0';
//    return;
//}
//
//void File2Str(string &str, FILE * fp)
//{
//    assert(fp);
//    fseek(fp, 0, SEEK_END);
//    long int fsize = ftell(fp);
//    rewind(fp);
//    str.resize(fsize + 1);
//    fread(&str[0], 1, fsize, fp);
//    str.at(fsize) = '\0';
//}

string fileToString(string fileName){
	//in android, SDL does not handle "//" ot "/" automatically
	string target("//");
	for (int i = 0; i < 1000; ++i){
		size_t pos = fileName.find("//");
		if (string::npos == pos){
			break;
		}
		fileName.replace(pos, 2, "/");
	}

	SDL_RWops *file = SDL_RWFromFile(fileName.c_str(), "r");
	size_t len = SDL_RWseek(file, 0, SEEK_END);
	SDL_RWseek(file, 0, SEEK_SET);
	string fileString;
	fileString.resize(len + 1);
	SDL_RWread(file,  &fileString[0], sizeof(char), len);
	fileString[len] = '\0';
	SDL_RWclose(file);
	return fileString;
}

string fileToString(const char *fileName){
	return fileToString(string(fileName));
}

}
