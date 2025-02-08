#include "commands.hpp"
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <memory>
#include <stdexcept>

class valueStruct{
    private:
    TimeStamp ttl;
    bool isttl;
    std::string value;
    public:
    valueStruct(const std::string & v) : value{v} {
        isttl = false;
    }
    valueStruct(const std::string & v, const TimeStamp & exp) : value{v}, ttl{exp} {
        isttl = true;
    }
    void setValue(const std::string & v){
        value = v;
    }
    void setTTL(const TimeStamp & exp){
        isttl = true;
        ttl = exp;
    }
    std::string getValue(){
        return value;
    }
    TimeStamp getTTL(){
        return ttl;
    }
    bool isTTL(){
        return isttl;
    }
};

std::unordered_map<std::string, valueStruct> kvStore;

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

// TODO: Add TTL to keys like redis does.
void SETCommand(std::shared_ptr<clientContext> clientData){
    auto *clientArgs = &(clientData->readArguments);
    auto *write = &(clientData->writeBeforeRESP);
    try
    {
        auto key = (*clientArgs)[1];
        auto value = (*clientArgs)[2];
        auto [it, inserted] = kvStore.insert_or_assign(key, valueStruct{value});
        auto valPtr = &(it->second);
        
        if((*clientArgs).size() > 3){
            for(int arg = 3; arg < (*clientArgs).size(); arg++){
                auto & tempArg = (*clientArgs)[arg];
                for(auto &j : tempArg) j = tolower(j);
                if(tempArg == "px"){
                    auto parameter = std::stoll((*clientArgs)[arg + 1]);
                    if(parameter < 0){
                        throw std::runtime_error("Incorrect Parameter");
                    }
                    // el.timeSubmit()
                    (*valPtr).setTTL(std::chrono::system_clock::now() + std::chrono::milliseconds(parameter));
                    arg++;
                }
            }
        }
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
        auto itr = kvStore.find(key);
        if(itr == kvStore.end()){
            nullBulkRESPString(clientData);
        }
        else if(itr->second.isTTL() && itr->second.getTTL() < std::chrono::system_clock::now()){
            nullBulkRESPString(clientData);
            kvStore.erase(itr);
        }
        else{
            (*write).push_back(itr->second.getValue());
            createBulkRESPString(clientData);
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
    for(auto &i : command){
        i = toupper(i);
    }
    if (commandsTable.find(command) == commandsTable.end()){
        // Send error reply
        // Need to create reply function in networking.cpp
        std::cout<<"Command not found!\n";
        clientData->writeBeforeRESP.push_back("Incorrect Command Entered");
        createSimpleErrorString(clientData);
    }
    else{
        commandsTable[command](clientData);
    }
    clearClientContext(clientData);
    el.fileSubmit(writeToClientBuffer, clientData);
}

