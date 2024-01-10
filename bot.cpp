#include <dpp/dpp.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <iostream>
#include <array>
#include <memory>
#include <cstdio>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}
std::string get_token_from_file(const std::string& filename);
std::string exec(const char* cmd);
std::string getYtDlpUrl(const std::string& youtubeUrl);
std::string getAudioInfo(const std::string& url);
int run_bot();

int main() {
    std::string streamURL = getYtDlpUrl("https://youtu.be/7qFfFVSerQo");
    if (!streamURL.empty()) {
        std::cout << "Streaming URL: " << streamURL << std::endl;
    } else {
        std::cerr << "Failed to get streaming URL." << std::endl;
    }
    return run_bot();
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
std::string exec(const char* cmd){
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if(!pipe){
        throw std::runtime_error("popen() failed!");
    }
    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr){
        result += buffer.data();
    }
    return result;
}
std::string getYtDlpUrl(const std::string& youtubeUrl) {
    std::string command = "yt-dlp -f bestaudio --get-url \"" + youtubeUrl + "\"";
    return exec(command.c_str());
}
std::string getAudioInfo(const std::string& url) {
    std::string command = "ffprobe -v quiet -print_format json -show_streams \"" + url + "\"";
    return exec(command.c_str());
}
int run_bot(){
    std::string token = get_token_from_file("../token.txt");
    if(token.empty()){
        std::cerr << "Token is empty. Ensure the config file is set up correctly.\n";
        return 1;
    }
    dpp::cluster bot(token);
    bot.on_log(dpp::utility::cout_logger());
    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "ping") {
            event.reply("Pong!");
        }
        if (event.command.get_command_name() == "join") {
            dpp::guild* g = dpp::find_guild(event.command.guild_id);
            auto current_vc = event.from->get_voice(event.command.guild_id);
            bool join_vc = true;
            if (current_vc) {
                auto users_vc = g->voice_members.find(event.command.get_issuing_user().id);
                if (users_vc != g->voice_members.end() && current_vc->channel_id == users_vc->second.channel_id) {
                    join_vc = false;
                } else {
                    event.from->disconnect_voice(event.command.guild_id);
                    join_vc = true;
                }
            }
            if (join_vc) {
                if (!g->connect_member_voice(event.command.get_issuing_user().id)) {
                    event.reply("You don't seem to be in a voice channel!");
                    return;
                }
                event.reply("Joined your channel!");
            } else {
                event.reply("Don't need to join your channel as i'm already there with you!");
            }
        }
    });
    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("ping", "Ping pong!", bot.me.id));
            bot.global_command_create(dpp::slashcommand("join", "Joins your voice channel.", bot.me.id));
        }
    });
    bot.start(dpp::st_wait);
    return 0;
}