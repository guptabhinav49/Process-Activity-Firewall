#include "headers.h"
#include "lwjson/lwjson.h"

static lwjson_token_t tokens[256];
static lwjson_t lwjson;

int ignore_grps, verbose;
char logfile_path[MAX_PATHLEN];
regex_t fpaths_regex[MAX_FILES_PER_TYPE/2];
char mode_regex[MAX_FILES_PER_TYPE/2];
char type_regex[MAX_FILES_PER_TYPE/2];
char fpaths_abs[MAX_FILES_PER_TYPE][MAX_PATHLEN];
char mode_abs[MAX_FILES_PER_TYPE];
char num_absolute, num_regex;

int parse_JSON(const char *c){
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
    return lwjson_parse(&lwjson, c);
}

int read_config(){
    int fd = open(CONFIGFILE_PATH, O_RDONLY);
    if(fd < 0) {
        perror("cannot find the config file, using defaults");
        return -1;
    }
    char buff[MAX_CONFIGSIZE];
    int bytes = read(fd, buff, MAX_CONFIGSIZE-1);

    if(bytes == MAX_CONFIGSIZE-1){
        perror("config size exceeded");
        return -1;
    }
    close(fd);
    buff[bytes] = '\0';

    if(parse_JSON(buff) == lwjsonOK){
        const lwjson_token_t* t;
        printf("Config parsed...\n");

        if((t = lwjson_find(&lwjson, "ignore_groups")) != NULL){
            ignore_grps = (t->type == LWJSON_TYPE_TRUE ? 1 : 0);
        }
        if((t = lwjson_find(&lwjson, "verbose")) != NULL){
            verbose = (t->type == LWJSON_TYPE_TRUE ? 1 : 0);
        }
        if((t = lwjson_find(&lwjson, "logfile_path")) != NULL){
            size_t s;
            sprintf(logfile_path, "%.*s", (int)s, lwjson_get_val_string(t, &s));
        }
        if((t = lwjson_find(&lwjson, "file_groups")) != NULL){
            if(t->type != LWJSON_TYPE_ARRAY){
                return -1;
            }
            int i_abs = 0;
            int i_regex = 0;
            for(const lwjson_token_t* tkn = lwjson_get_first_child(t); tkn!=NULL; tkn = tkn->next){
                if(tkn->type != LWJSON_TYPE_OBJECT){
                    return -1;
                }
                const lwjson_token_t* tkn2;
                int type, mode;
                
                if((tkn2 = lwjson_find_ex(&lwjson, tkn, "type")) != NULL){
                    type = lwjson_get_val_int(tkn2);
                }
                else type = 0;

                if((tkn2 = lwjson_find_ex(&lwjson, tkn, "mode")) != NULL){
                    mode = lwjson_get_val_int(tkn2);
                }
                else mode = 1;

                if((tkn2 = lwjson_find_ex(&lwjson, tkn, "exprs")) != NULL){
                    for(const lwjson_token_t* tkn3 = lwjson_get_first_child(tkn2); tkn3!=NULL; tkn3=tkn3->next){
                        size_t s;
                        if(type > 0){
                            char buffer[MAX_PATHLEN];
                            sprintf(buffer, "%.*s", (int)s, lwjson_get_val_string(tkn3, &s));
                            printf("%s\n",buffer);

                            if(regcomp(&(fpaths_regex[i_regex]), buffer, REG_EXTENDED) != 0) return -1;
                            puts("regex compiled");

                            mode_regex[i_regex] = mode;
                            type_regex[i_regex] = (type == 1 ? '0' : '1');
                            i_regex++;
                            if(i_regex == MAX_FILES_PER_TYPE/2){
                                perror("max file limit reached");
                                break;
                            }
                        }
                        else{
                            sprintf(*(fpaths_abs+i_abs), "%.*s", (int)s, lwjson_get_val_string(tkn3, &s));
                            mode_abs[i_abs] = mode;
                            i_abs++;
                            if(i_abs == MAX_FILES_PER_TYPE){
                                perror("max file limit reached");
                                break;
                            }
                        }
                    }
                }
            }
            num_absolute = i_abs;
            num_regex = i_regex;
            
            // sorting the filenames to falicitate log(n) find queries
            sort(fpaths_abs, num_absolute);
        }
    }
    else{
        printf("JSON not parsed! %d\n", parse_JSON(buff));
    }

    FILE *fp;
    fp = fopen(logfile_path, "w");
    if(!fp){
        perror("Unable to open the logfile");
        exit(0);
    }
    fprintf(fp, "{\n\"config\": %s,\n\"logs\": [\n\t{}\n]\n}", buff);
    fflush(fp);
    fclose(fp);

    lwjson_free(&lwjson);
    return 0;
}

int main(int argc, char *argv[]){

    if(argc > 2){
        printf("Usage: %s <substring>", argv[1]);
        return 1;
    }
    // defualt configurations
    ignore_grps = 1;
    verbose = 1;
    num_absolute = num_regex = 0;
    // reading the configuration file


    if(read_config() < 0){
        perror("error while reading config file, using defaults");
    }

    struct sockaddr_un addr;
    char buf[MAX_BUFFLEN];
    int sfd, cfd, nbytes;

    // Creating a server socket, this is where this application listens for incoming log buffers
    if((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(-1);
    }

    // Setting the family and path, this path is available to the logger function as well
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path));

    // unlinking the path before binding it to the socket
    unlink(SV_SOCK_PATH);

    // binding the socket with a known address
    if(bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind error");
        exit(-1);
    }

    // making the socket passive *listener*
    if(listen(sfd, 100) == -1) {
        perror("listener error");
        exit(-1);
    }

    // if(verbose){
    //     printf("mode: %d, verbose: %d, logfile_path: %s. Ignoring/monitoring files\n", mode, verbose, logfile_path);
    //     for(int i=0; i<count_files; i++){
    //         printf("\t%d. %s\n", i+1, filepaths[i]);
    //     }  
    // }
    
    FILE *fp;
    fp = fopen(logfile_path, "r+");
    // accepting connections from the active sockets
    printf("\n##########################\nProcess-Activity-Firewall\n##########################\n");
    // fprintf(fp, "\n##########################\nProcess-Activity-Firewall\n##########################\n");

    while(1){
        if((cfd = accept(sfd, NULL, NULL)) == -1){
            perror("accept error");
            continue;
        }

        while((nbytes=read(cfd, buf, MAX_BUFFLEN)) > 0) {
            // printing the logged buffer
            if(parse_JSON(buf) == lwjsonOK){
                const lwjson_token_t* t;
                char filename[MAX_PATHLEN];
                size_t s;
                int f = 0, m = 1, mode = !ignore_grps;

                if(!ignore_grps && (t = lwjson_find(&lwjson, "params.file.path")) != NULL){
                    sprintf(filename, "%.*s", (int)s, lwjson_get_val_string(t, &s));
                    if(num_absolute) {
                        int v = find(fpaths_abs, filename, num_absolute);
                        if(v >=0) {
                            mode = mode_abs[v]+1;
                            f = 1;
                        }
                    }
                    if(!f && num_regex){
                        int v = match_regex(fpaths_regex, type_regex, filename, num_regex);
                        if(v>=0){
                            mode = mode_regex[v]+1;
                            f = 1;
                            // printf("matching regex %d\n", v);
                        }
                    }
                }
                if (argc == 2){
                    // printf("matching substring\n");
                    m = match(buf, argv[1]);
                }

                switch (mode)
                {
                    case 0:
                        if(m) {
                            if(verbose) printf(",%s", buf);
                            fseeko(fp, -4, SEEK_END);
                            ftruncate(fileno(fp), ftello(fp));
                            fprintf(fp, ",%s]\n}", buf);
                        }
                        break;
                    case 1:
                        break;
                    case 2:
                        if(m && f) {
                            if(verbose) printf(",%s", buf);
                            fseeko(fp, -4, SEEK_END);
                            ftruncate(fileno(fp), ftello(fp));
                            fprintf(fp, ",%s]\n}", buf);
                        }
                        break;
                    case 3:
                        if(m && !f) {
                            if(verbose) printf(",%s", buf);
                            fseeko(fp, -4, SEEK_END);
                            ftruncate(fileno(fp), ftello(fp));
                            fprintf(fp, ",%s]\n}", buf);
                        }
                        break;
                    default:
                        break;
                }
            }
            else{
                perror("wrong format logs");
                exit(0);
            }
            fflush(fp);
        }
        close(cfd);
    }

    lwjson_free(&lwjson);
    return 0;
}