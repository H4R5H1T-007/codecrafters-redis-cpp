#include "commands.hpp"
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <functional>

std::vector<std::string> split(std::string & input, const char & delimeter = ' '){
    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string token;

    // Split by given delimeter (only single character) (by default it's space)
    while (std::getline(iss, token, delimeter)) {
        tokens.push_back(token);
    }

    return tokens;
}

void PINGCommand(int client_location){
    auto * clientArgs = &cContext[client_location].readArguments;
    auto * str = &cContext[client_location].writeBeforeRESP;
    if((*clientArgs).size() == 1){
        std::cout<<"PONG"<<std::endl;
        (*str).append("PONG");
    }
    else{
        for(int i = 1; i < (*clientArgs).size(); i++){
            std::cout<<(*clientArgs)[i]<<std::endl;
            (*str).append((*clientArgs)[i]);
            (*str).push_back(' ');
        }
    }
    createBulkRESPString(client_location);
    el.fileSubmit(writeToClientBuffer, client_location);
    return ;
}

void ECHOCommand(int client_location){
    auto * clientArgs = &cContext[client_location].readArguments;
    auto * str = &cContext[client_location].writeBeforeRESP;
    for(int i = 1; i < (*clientArgs).size(); i++){
        std::cout<<(*clientArgs)[i]<<std::endl;
        (*str).append((*clientArgs)[i]);
        (*str).push_back(' ');
    }
    createBulkRESPString(client_location);
    el.fileSubmit(writeToClientBuffer, client_location);
    return ;
}

std::unordered_map<std::string, std::function<void(int)>> commandsTable;

bool cmdTableCreator(){
    try
    {
        commandsTable["PING"] = PINGCommand;
        commandsTable["ECHO"] = ECHOCommand;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
    
    return 1;
}

void commandsHandler(int client_location){
    auto command = cContext[client_location].readArguments[0];
    if (commandsTable.find(command) == commandsTable.end()){
        // Send error reply
        // Need to create reply function in networking.cpp
        std::cout<<"Command not found!\n";
        return ;
    }
    commandsTable[command](client_location);
}

