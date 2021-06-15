#include "headers.h"
#include "lwjson/lwjson.h"

static lwjson_token_t tokens[256];
static lwjson_t lwjson;

int mode, verbose;
char logfile_path[MAX_PATHLEN];
char filepaths[MAX_FILES][MAX_PATHLEN];
char count_files;

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

        if((t = lwjson_find(&lwjson, "mode")) != NULL){
            mode = (int)lwjson_get_val_int(t);
        }
        if((t = lwjson_find(&lwjson, "verbose")) != NULL){
            verbose = (t->type == LWJSON_TYPE_TRUE ? 1 : 0);
        }
        if((t = lwjson_find(&lwjson, "logfile_path")) != NULL){
            size_t s;
            sprintf(logfile_path, "%.*s", (int)s, lwjson_get_val_string(t, &s));
        }
        if((t = lwjson_find(&lwjson, "filepaths")) != NULL){
            assert(t->type == LWJSON_TYPE_ARRAY);
            int i = 0;
            for(const lwjson_token_t* tkn = lwjson_get_first_child(t); tkn!=NULL; tkn = tkn->next){
                size_t s;
                sprintf(*(filepaths+i), "%.*s", (int)s, lwjson_get_val_string(tkn, &s));
                i++;
                if(i == MAX_FILES){
                    perror("max file limit reached");
                    break;
                }
            }
            count_files = i;
            
            // sorting the filenames to falicitate log(n) find queries
            sort(filepaths, count_files);
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

    return 0;
}

int main(int argc, char *argv[]){

    if(argc > 2){
        printf("Usage: %s <substring>", argv[1]);
        return 1;
    }
    // defualt configurations
    mode = 3;
    verbose = 1;
    count_files = 0;
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

    if(verbose){
        printf("mode: %d, verbose: %d, logfile_path: %s. Ignoring/monitoring files\n", mode, verbose, logfile_path);
        for(int i=0; i<count_files; i++){
            printf("\t%d. %s\n", i+1, filepaths[i]);
        }  
    }
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
                int f = 0, m = 1;
                if((t = lwjson_find(&lwjson, "params.file.path")) != NULL){
                    sprintf(filename, "%.*s", (int)s, lwjson_get_val_string(t, &s));
                    f = find(filepaths, filename, count_files);
                }
                if (argc == 2){
                    // printf("matching substring\n");
                    m = match(buf, argv[1]);
                }

                switch (mode)
                {
                    case 0:
                        break;
                    case 1:
                        if(m) {
                            if(verbose) printf(",%s", buf);
                            fseeko(fp, -4, SEEK_END);
                            ftruncate(fileno(fp), ftello(fp));
                            fprintf(fp, ",%s]\n}", buf);
                        }
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
    return 0;
}