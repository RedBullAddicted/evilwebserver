#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
 //JSON parser library
#include "../libraries/jsmn/jsmn.h"
#include "webserver.h"
#include "evilnetlib.h"
#include "config.h"

int loadConfig(){

    //Read config file
    char * json; /* Contents of json file */
    json = readFile(DIR_CONFIG"config.json");
    if(json == NULL){
        perror("Error reading config file");
        return -1;
    }

    int r; /* Return value */
    jsmn_parser parser; /* Parser */
    jsmntok_t tokens[256]; /* Objects parsed from the json file */

    //initialise the parser
    jsmn_init(&parser);

    //Parse the json string, returns jsmnerr_t
    r = jsmn_parse(&parser, json, tokens, 256);
    if(r != JSMN_SUCCESS){
        printf("%s\n", json);
        return -1;
    }

    int cparent = -1;   /* current parent node */
    int nservers = 0;   /* number of server configs */
    int nmodules = 0;
    int i = 1;          /* array iterator */
    int config = CONFIG_NONE; /* Current config array */    
    jsmntok_t ctoken;   /* current token holder*/

    //The first token MUST be a object
    if(tokens[0].type != JSMN_OBJECT)
        return -1;

    //Servers config parser
    while((ctoken = tokens[i]).end != 0){
        if(ctoken.type == JSMN_OBJECT){
            if(cparent == -1 || ctoken.start > tokens[cparent].end){
                cparent = -1;   
                if(tokens[i - 1].type != JSMN_STRING){
                    i++;
                    continue;
                }
                cparent = i;
                if(config != CONFIG_SERVER 
                    && strncasecmp(&json[tokens[i - 1].start], "servers", 6) == 0){
                    config = CONFIG_SERVER;
                } else if(config != CONFIG_MODULE 
                    && strncasecmp(&json[tokens[i - 1].start], "modules", 7) == 0){
                    config = CONFIG_MODULE;
                    //Handle the modules array
                }
                if(config == CONFIG_SERVER){                    
                    //Handle the server array
                    config_servers = realloc(config_servers,
                     (nservers + 1 * sizeof(config_server_t *)));
                    config_servers[nservers] = malloc(sizeof(config_server_t)); 
                    config_servers[nservers]->port = 0;
                    config_servers[nservers]->name = NULL;
                    nservers++;
                } else if(config == CONFIG_MODULE){
                   config_modules = realloc(config_modules,
                    (nmodules + 1 * sizeof(config_module_t *)));
                   config_modules[nmodules] = malloc(sizeof(config_module_t));
                   config_modules[nmodules]->name = strndup(&json[tokens[i - 1].start],
                    tokens[i - 1].end - tokens[i - 1].start);
                   config_modules[nmodules]->command = NULL;
                   config_modules[nmodules]->method = NULL;
                   printf("%s\n", config_modules[nmodules]->name);
                }
                continue;
            } else {
                if(config == CONFIG_SERVER){                    
                    //Handle the server array
                    config_servers = realloc(config_servers,
                     (nservers + 1 * sizeof(config_server_t *)));
                    config_servers[nservers] = malloc(sizeof(config_server_t)); 
                    config_servers[nservers]->port = 0;
                    config_servers[nservers]->name = NULL;
                    nservers++;
                } else if(config == CONFIG_MODULE){
                   config_modules = realloc(config_modules,
                    (nmodules + 1 * sizeof(config_module_t *)));
                   config_modules[nmodules] = malloc(sizeof(config_module_t));
                   config_modules[nmodules]->name = strndup(&json[tokens[i - 1].start],
                    tokens[i - 1].end - tokens[i - 1].start);
                   config_modules[nmodules]->command = NULL;
                   config_modules[nmodules]->method = NULL;
                   printf("%s\n", config_modules[nmodules]->name);
                }
            }
        } else if(cparent == -1){
            i++;
            continue;
        }
        
        if(ctoken.type != JSMN_STRING && tokens[i+1].type != JSMN_STRING || config == CONFIG_NONE){
            i++;
            continue;
        }


        //Process key(ctoken):val(vtoken) pair
        jsmntok_t vtoken = tokens[i+1];

        //Abstract the value
        unsigned int length = vtoken.end - vtoken.start;
        char value[length + 10];
        memcpy(value, &json[vtoken.start], length);
        value[length] = '\0';

        if(config == CONFIG_SERVER){
            //Current server to be configured
            config_server_t *cserver = config_servers[nservers - 1];    
            if(strncasecmp(&json[ctoken.start], "port", 4) == 0){    
                if((cserver->port = atoi(value)) > 65535 || cserver->port < 1){
                    perror("Invalid port number");
                    return -1;
                }
            } else if(strncasecmp(&json[ctoken.start], "name", 4) == 0){
                cserver->name = strdup(value);                
            }
        } else if(config == CONFIG_MODULE) {
            if(strncasecmp(&json[ctoken.start], "command", 4) == 0){    
                printf("command %s\n", value);
            } else if(strncasecmp(&json[ctoken.start], "name", 4) == 0){
                            
            }
        }
        i++;
    }

    return 0;
}