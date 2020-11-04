#include <iostream>
#include <string>
#include "displayer.hpp"
#include <yaml.h>
#include <cstring>

//#define DEBUG

#ifdef DEBUG
    #define ConfigPath "grzelock.yml"
#else
    #define ConfigPath  &("/home/" + std::string(std::getenv("USER")) + "/.config/grzelock.yml")[0]
#endif


enum parserState
{
    SeekingKey,
    SeekingKeyField,
    SeekingValue,
    SeekingValueField
};

int main (int argc, char** argv)
{   
    yaml_parser_t parser;
    yaml_token_t token;
    FILE *config = fopen(ConfigPath, "rb");

    if(!yaml_parser_initialize(&parser))
    {
        std::cerr << "Failed to initialize parser" << std::endl;
        return 1;
    }
    if(config == NULL)
    {
        std::cerr << "Config file can not be opened, please make sure that " << ConfigPath << " exists!" << std::endl;
        return 1;
    }
    yaml_parser_set_input_file(&parser, config);

    std::string lastKey;
    parserState state;
    state = SeekingKey;
    Settings settings;

    do {
        yaml_parser_scan(&parser, &token);
        switch(token.type)
        {   
            case YAML_KEY_TOKEN:
                if (state == SeekingKey)
                    state = SeekingKeyField;
                break;
            case YAML_VALUE_TOKEN:
                if (state == SeekingValue)
                    state = SeekingValueField;
                break;
            case YAML_SCALAR_TOKEN:
                if (state == SeekingKeyField)
                {
                    const char* key = (char*) token.data.scalar.value;
                    lastKey = std::string(key);
                    state = SeekingValue;
                }
                else if (state == SeekingValueField)
                {
                    const char* val = (char*) token.data.scalar.value;
                    if (lastKey == "ResX") settings.resX = std::stoi(val);
                    else if (lastKey == "ResY") settings.resY = std::stoi(val);
                    else if (lastKey == "Background") settings.background = strdup(val);
                    else if (lastKey == "Font") settings.font = strdup(val);
                    state = SeekingKey;
                }


            default:
                break;
        }
    } while (token.type != YAML_STREAM_END_TOKEN);
    std::cout << "Setting resolution to " << settings.resX << "x" << settings.resY << std::endl;
    lockScreen(settings);
    yaml_parser_delete(&parser);
    fclose(config);
    return 0;
}
