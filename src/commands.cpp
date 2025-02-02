#include "commands.hpp"
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <memory>

std::unordered_map<std::string, std::string> kvStore;

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

void PINGCommand(std::shared_ptr<clientContext> clientData){
    // auto * clientArgs = &cContext[client_location].readArguments;
    // auto * str = &cContext[client_location].writeBeforeRESP;
    if(clientData->readArguments.size() == 1){
        // std::cout<<"PONG"<<std::endl;
        clientData->writeBeforeRESP.push_back("PONG");
    }
    else{
        for(int i = 1; i < clientData->readArguments.size(); i++){
            // std::cout<<clientData->readArguments[i]<<std::endl;
            clientData->writeBeforeRESP.push_back(clientData->readArguments[i]);
            // clientData->writeBeforeRESP.push_back(' ');
        }
    }
    // createBulkRESPString(clientData);
    createSimpleRESPString(clientData);
    return ;
}

void ECHOCommand(std::shared_ptr<clientContext> clientData){
    auto * clientArgs = &(clientData->readArguments);
    auto * str = &(clientData->writeBeforeRESP);
    for(int i = 1; i < (*clientArgs).size(); i++){
        // std::cout<<(*clientArgs)[i]<<std::endl;
        (*str).push_back((*clientArgs)[i]);
        // (*str).push_back(' ');
    }
    // createBulkRESPString(clientData);
    createSimpleRESPString(clientData);
    return ;
}

void SETCommand(std::shared_ptr<clientContext> clientData){
    auto *clientArgs = &(clientData->readArguments);
    auto *write = &(clientData->writeBeforeRESP);
    try
    {
        auto key = (*clientArgs)[1];
        auto value = (*clientArgs)[2];
        kvStore[key] = value;
        (*write).push_back("OK");
        createSimpleRESPString(clientData);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        (*write).push_back("Incorrect Command Arguments");
        createSimpleErrorString(clientData);
    }
}

void GETCommand(std::shared_ptr<clientContext> clientData){
    auto *clientArgs = &(clientData->readArguments);
    auto *write = &(clientData->writeBeforeRESP);
    try
    {
        auto key = (*clientArgs)[1];
        if(kvStore.find(key) != kvStore.end()){
            (*write).push_back(kvStore[key]);
            createBulkRESPString(clientData);
        }
        else{
            nullBulkRESPString(clientData);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        (*write).push_back("Incorrect Command Arguments");
        createSimpleErrorString(clientData);
    }
}

std::unordered_map<std::string, std::function<void(std::shared_ptr<clientContext>)>> commandsTable;

bool cmdTableCreator(){
    try
    {
        commandsTable["PING"] = PINGCommand;
        commandsTable["ECHO"] = ECHOCommand;
        commandsTable["SET"] = SETCommand;
        commandsTable["GET"] = GETCommand;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
    
    return 1;
}

void commandsHandler(std::shared_ptr<clientContext> clientData){
    auto command = clientData->readArguments[0];
    if (commandsTable.find(command) == commandsTable.end()){
        // Send error reply
        // Need to create reply function in networking.cpp
        std::cout<<"Command not found!\n";
        return ;
    }
    commandsTable[command](clientData);
    clearClientContext(clientData);
    el.fileSubmit(writeToClientBuffer, clientData);
}

