#include <dpp/dpp.h>
#include <fstream>
#include <string>
#include <iostream>
#include <array>
#include <oggz/oggz.h>
#include <sys/wait.h>
#include <memory>
#include <cstdio>
#include <unistd.h>
#include <dpp/discordclient.h>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}

std::string get_token_from_file(const std::string& filename);
std::string exec(const char* cmd);
std::string get_yt_dlp_url(const std::string& youtubeUrl);
int run_bot();
void ffprobe_stream(const std::string& stream_url);
void pipe_audio_to_discord(const std::string& stream_url, dpp::voiceconn* voice_connection);

int main(){
    return run_bot();
}

std::string get_token_from_file(const std::string& filename){
    std::ifstream configuration_file(filename);
    std::string token;
    if(configuration_file){
        std::getline(configuration_file, token);
        size_t pos = token.find('=');
        if(pos != std::string::npos){
            token = token.substr(pos + 1);
        }
    }
    else{
        std::cerr << "Unable to open configuration file!\n";
    }
    return token;
}

std::string exec(const char* cmd){
    std::array<char, 128> buffer = {};
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

std::string get_yt_dlp_url(const std::string& youtubeUrl){
    std::string command = "yt-dlp -f bestaudio --get-url \"" + youtubeUrl + "\"";
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
    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event){
        if (event.command.get_command_name() == "ping"){
            event.reply("Pong!");
        }
        else if (event.command.get_command_name() == "join"){
            dpp::guild* guild = dpp::find_guild(event.command.guild_id);
            if(!guild->connect_member_voice(event.command.get_issuing_user().id)){
                event.reply("You don't seem to be in a voice channel!");
                return;
            }
            event.reply("Joined your channel!");
        }
        else if (event.command.get_command_name() == "play"){
            std::string stream_url = get_yt_dlp_url("https://youtu.be/dQw4w9WgXcQ");
            dpp::voiceconn* voice_connection = event.from->get_voice(event.command.guild_id);
            if(!voice_connection || !voice_connection->voiceclient || !voice_connection->voiceclient->is_ready()){
                event.reply("There was an issue with getting the voice channel.");
                return;
            }
            if (!stream_url.empty()){
                if (voice_connection && voice_connection->voiceclient && voice_connection->voiceclient->is_ready()){
                    pipe_audio_to_discord(stream_url, voice_connection);
                    event.reply("Playing music!");
                }
                else{
                    event.reply("Bot is not in a voice channel.");
                }
            }
            else{
                event.reply("Failed to get the streaming URL.");
            }
        }
    });
    bot.on_ready([&bot](const dpp::ready_t& event){
        if (dpp::run_once<struct register_bot_commands>()){
            bot.global_command_create(dpp::slashcommand("ping", "Ping pong!", bot.me.id));
            bot.global_command_create(dpp::slashcommand("join", "Joins your voice channel.", bot.me.id));
            bot.global_command_create(dpp::slashcommand("play", "Plays music in your voice channel.", bot.me.id));
        }
    });
    bot.start(dpp::st_wait);
    return 0;
}
void ffprobe_stream(const std::string& stream_url) {
    std::string ffprobe_command = "ffprobe -v error -show_format -show_streams \"" + stream_url + "\"";
    std::string ffprobe_result = exec(ffprobe_command.c_str());
    std::cout << "FFprobe analysis:\n" << ffprobe_result << std::endl;
}

void pipe_audio_to_discord(const std::string& stream_url, dpp::voiceconn* voice_connection) {
    ffprobe_stream(stream_url);
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        std::cerr << "Failed to create pipe." << std::endl;
        return;
    }
    std::cout << "Forking process for FFmpeg..." << std::endl;
    pid_t ffmpeg_pid = fork();
    if (ffmpeg_pid == -1) {
        std::cerr << "Failed to fork process." << std::endl;
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return;
    } else if (ffmpeg_pid == 0) {
        close(pipe_fd[0]);
        int stderr_fd = open("ffmpeg_stderr.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(stderr_fd, STDERR_FILENO);
        execlp("ffmpeg", "ffmpeg", "-i", stream_url.c_str(), "-map", "0:a", "-c:a", "libopus", "-ar", "48000", "-ac", "2", "-f", "ogg", "-", "-progress", "-", "-nostats", (char*)nullptr);
        _exit(EXIT_FAILURE);
    }
    close(pipe_fd[1]);

    FILE* oggz_stream = fdopen(pipe_fd[0], "r");
    if (!oggz_stream) {
        std::cerr << "Failed to open pipe stream." << std::endl;
        close(pipe_fd[0]);
        return;
    }
    std::cout << "Opening OGGZ stream..." << std::endl;
    OGGZ* oggz = oggz_open_stdio(oggz_stream, OGGZ_READ);
    if (!oggz) {
        std::cerr << "Failed to open OGGZ stream." << std::endl;
        fclose(oggz_stream);
        close(pipe_fd[0]);
        return;
    }
    oggz_set_read_callback(oggz, -1, [] (OGGZ *oggz1, oggz_packet *packet, long serial_number, void* user_data) {
        auto *voice_connection2 = static_cast<dpp::voiceconn*>(user_data);
        if (!voice_connection2 || !voice_connection2->voiceclient) {
            std::cerr << "Voice connection is not valid." << std::endl;
            return -1;
        }
        voice_connection2->voiceclient->send_audio_opus(packet->op.packet, packet->op.bytes);
        return 0;
    }, (void*)voice_connection);
    std::cout << "Reading from OGGZ stream..." << std::endl;
    while (voice_connection && voice_connection->voiceclient && !voice_connection->voiceclient->terminating) {
        try {
            static const constexpr long CHUNK_READ = 9223372036854775807;
            const long read_bytes = oggz_read(oggz, CHUNK_READ);
            if (!read_bytes) {
                std::cout << "End of OGGZ stream or error encountered." << std::endl;
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception caught during audio streaming: " << e.what() << std::endl;
            break;
        }
    }
    int status;
    waitpid(ffmpeg_pid, &status, 0);
    if(WIFEXITED(status) != 0 || status != 0){
        int exit_status = WEXITSTATUS(status);
        std::cerr << "FFmpeg process exited with status " << exit_status << " " << status << std::endl;
    }
    oggz_close(oggz);
    fclose(oggz_stream);
    close(pipe_fd[0]);
    waitpid(ffmpeg_pid, nullptr, 0);
    std::cout << "Audio streaming finished." << std::endl;
}




