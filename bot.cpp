#include <dpp/dpp.h>
#include <fstream>
#include <string>
#include <iostream>

std::string get_token_from_file(const std::string& filename);

int main() {

    std::string token = get_token_from_file("../token.txt");
    if(token.empty()){
        std::cerr << "Token is empty. Ensure the config file is set up correctly.\n";
        return 1;
    }

    dpp::cluster bot(token);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "ping") {
            event.reply("Pong!");
        }
    });

    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("ping", "Ping pong!", bot.me.id));
        }
    });

    bot.start(dpp::st_wait);
}

std::string get_token_from_file(const std::string& filename){
    std::ifstream stream(filename);
    std::string token;
    if(stream){
        std::getline(stream, token);
        size_t pos = token.find('=');
        if(pos != std::string::npos){
            token = token.substr(pos + 1);
        }
    }else{
        std::cerr << "Unable to open config file!\n";
    }
    return token;
}
