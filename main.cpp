#include <dpp/dpp.h>

int main() {
    dpp::cluster bot("MTE5NDIxODk3NDA0OTczNDY4Ng.GhzvV7.3AskQGZ45oBur8S6JvzCNXvtwIXFCXHtKULBC8");

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