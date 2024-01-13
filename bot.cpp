#include <dpp/dpp.h>
#include <fstream>
#include <string>
#include <iostream>
#include <memory>
#include <cstdio>
#include <dpp/discordclient.h>
#include <oggz/oggz.h>
extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}
std::map<dpp::snowflake, dpp::discord_client*> discord_clients;
std::mutex discord_clients_m;

int execute_client();
std::string get_token_from_file(const std::string& filename);
void handle_ready(const dpp::ready_t &event);
void handle_slash_command(const dpp::slashcommand_t &event);
void handle_streaming(const dpp::voice_ready_t event);
void handle_voice_ready(const dpp::voice_ready_t& event);
void handle_voice_ready(const dpp::voice_ready_t& event);
void handle_voice_track_marker(const dpp::voice_track_marker_t &event);
void handle_message(const dpp::message_create_t& event);
std::string youtube_url;

int main()
{
    return execute_client();
}

std::string get_token_from_file(const std::string& filename)
{
    std::ifstream configuration_file(filename);
    std::string token;
    if(configuration_file)
    {
        std::getline(configuration_file, token);
        size_t pos = token.find('=');
        if(pos != std::string::npos)
        {
            token = token.substr(pos + 1);
        }
    }
    else
    {
        std::cerr << "Unable to open configuration file!\n";
    }
    return token;
}

int execute_client()
{
    const std::string bot_token = get_token_from_file("../token.txt");
    if(bot_token.empty())
    {
        std::cerr << "Token is empty. Ensure the config file is set up correctly.\n";
        return 1;
    }
    dpp::cluster client(bot_token, dpp::i_default_intents | dpp::i_message_content);
    client.on_message_create(handle_message);
    client.on_log(dpp::utility::cout_logger());
    client.on_ready(handle_ready);
    client.on_slashcommand(handle_slash_command);
    client.on_voice_ready(handle_voice_ready);
    client.on_voice_track_marker(handle_voice_track_marker);
    client.start(false);
    return 0;
}

void handle_ready(const dpp::ready_t &event)
{
    fprintf(stderr, "Bot ready\n");
    if(dpp::run_once<struct register_bot_commands>())
    {
        dpp::slashcommand play_command
        (
            "play",
            "Play music",
            event.from->creator->me.id
       );

        play_command.add_option
        (
            dpp::command_option
            (
                dpp::co_sub_command, "youtube", "Play from youtube URL", true
            ).add_option(dpp::command_option(dpp::co_string, "URL", "Youtube URL", true))
        );
        dpp::slashcommand pong_command
        (
            "pong",
            "test",
            event.from->creator->me.id
        );
        event.from->creator->global_command_create(play_command);
        event.from->creator->global_command_create(pong_command);
    }
}
void handle_slash_command(const dpp::slashcommand_t &event)
{
    const std::string operation = event.command.get_command_name();
    if(operation == "pong")
    {
        event.reply("pong");
    }
    if(operation == "play")
    {
        dpp::guild *guild = dpp::find_guild(event.command.guild_id);
        if(!guild)
        {
            event.reply("Guild not found!!");
            return;
        }
        if(!guild->connect_member_voice(event.command.usr.id))
        {
            event.reply("Please join a voice channel first!");
            return;
        }
        dpp::command_interaction cmd_data = event.command.get_command_interaction();
        if(!cmd_data.options.empty())
        {
            auto option = cmd_data.options[0];
            if(option.name == "youtube")
            {
                youtube_url = std::get<std::string>(option.value);
                event.reply("youtube URL streaming: " + youtube_url);
                std::lock_guard lock(discord_clients_m);
                discord_clients.insert_or_assign(event.command.guild_id, event.from);
                event.reply("Joining and playing: " + youtube_url);
            }
        }
        event.reply("youtube URL is required...");
    }
}

void handle_streaming(const dpp::voice_ready_t event)
{
    std::string _command_ = "yt-dlp -f bestaudio -o - \"" + youtube_url + "\" | ffmpeg -acodec opus -i pipe:0 -f s16le -ar 48000 -ac 2 -";

    FILE *read_stream = popen(_command_.c_str(), "r");

    constexpr size_t buffer_size = dpp::send_audio_raw_max_length;

    char buffer[buffer_size];
    ssize_t buf_read = 0;
    ssize_t current_read = 0;

    while((current_read = fread(buffer + buf_read, 1, buffer_size - buf_read, read_stream)) > 0)
    {
        buf_read += current_read;
        if(buf_read == buffer_size)
        {
            event.voice_client->send_audio_raw((uint16_t *)buffer, buf_read);
            buf_read = 0;
        }
    }
    if(buf_read > 0)
    {
        event.voice_client->send_audio_raw((uint16_t *)buffer, buf_read);
        buf_read = 0;
    }
    pclose(read_stream);
    read_stream = nullptr;
    while(event.voice_client->is_playing())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    event.voice_client->insert_marker("disconnect");
}
void handle_message(const dpp::message_create_t& event) {
    if (event.msg.content.starts_with("!play ")) {
        youtube_url = event.msg.content.substr(6);
        dpp::guild* guild = dpp::find_guild(event.msg.guild_id);
        if (!guild) {
            event.reply("Guild not found!");
            return;
        }
        if (!guild->connect_member_voice(event.msg.author.id)) {
            event.reply("Please join a voice channel first!");
            return;
        }
        std::lock_guard<std::mutex> lock(discord_clients_m);
        discord_clients[event.msg.guild_id] = event.from;
        // The actual streaming will be started in handle_voice_ready
        event.reply("Joining and preparing to play: " + youtube_url);
    }
}
void handle_voice_ready(const dpp::voice_ready_t& event)
{
    std::thread thr(handle_streaming, event);
    thr.detach();
}
void handle_voice_track_marker(const dpp::voice_track_marker_t &event)
{
    if(event.track_meta == "disconnect")
    {
        std::thread thr([event](){
          std::lock_guard lock(discord_clients_m);
          auto it = discord_clients.find(event.voice_client->server_id);

          if(it == discord_clients.end())
          {
             return;
          }
          it->second->disconnect_voice(event.voice_client->server_id);
          discord_clients.erase(it);
        });
        thr.detach();
    }
}
