#include<iostream>
#include <sstream>     // ← AÑADIR ESTE INCLUDE
#include <string>      // ← TAMBIÉN RECOMENDABLE
#include <cctype>      // ← PARA isdigit()
int main(void){
	std::string temp = "";
    std::string ip = "esnrjinrdif";
	std::istringstream ill(ip);

    for (int i = 0; std::getline(ill, temp, '.'); i++){
        if (i > 3)
			return 0;
        for (int j = 0; j < (int)temp.size(); j++){
            if (!isdigit(temp[j])) return 0;
        }
    }
	std::cout << "OK" << std::endl;
	return 0;
}
