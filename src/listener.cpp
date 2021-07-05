#include "headers.h"
#include "lwjson/lwjson.h"
#include "cwalk/cwalk.h"
#include "trie/trie.h"

static lwjson_token_t tokens[256];
static lwjson_t lwjson;

int ignore_grps, verbose;
char logfile_path[MAX_PATHLEN];
regex_t fpaths_regex[MAX_FILES_PER_TYPE/2];
char mode_regex[MAX_FILES_PER_TYPE/2];
char type_regex[MAX_FILES_PER_TYPE/2];
char fpaths_abs[MAX_FILES_PER_TYPE][MAX_PATHLEN];
int mode_abs;
int num_absolute, num_regex, num_begf, num_begp, num_endf, num_endp;
vector trie_begf, trie_endp, trie_begp, trie_endf;

// here we store the configuration file as a lwjson_t object
int parse_JSON(const char *c){
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
    return lwjson_parse(&lwjson, c);
}

// to add the root vertex to the trie datastructure
void init_vertex(vector *v){
    trie_v *vertex = (trie_v *)malloc(sizeof(trie_v));
    trie_v_init(vertex);
    vector_add(v, vertex);
}

// this function reads config.json from the root directory
int read_config(){
    // opening the config file
    int fd = open(CONFIGFILE_PATH, O_RDONLY);
    if(fd < 0) {
        perror("cannot find the config file, using defaults");
        return -1;
    }
    
    char buff[MAX_CONFIGSIZE]; // this buffer stores the whole file
    int bytes = read(fd, buff, MAX_CONFIGSIZE-1);

    if(bytes == MAX_CONFIGSIZE-1){
        perror("config size exceeded");
        return -1;
    }
    close(fd);
    buff[bytes] = '\0';

    // here we store parsed buffer into varianble lwjson 
    if(parse_JSON(buff) == lwjsonOK){
        const lwjson_token_t* t;
        printf("Config parsed...\n");

        // from here onwards we find values of various fields present in config file

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
        // the "file_groups" field
        if((t = lwjson_find(&lwjson, "file_groups")) != NULL){
            if(t->type != LWJSON_TYPE_ARRAY){
                return -1;
            }
            int i_abs = 0;
            int i_regex = 0;
            // iterating through file groups
            for(const lwjson_token_t* tkn = lwjson_get_first_child(t); tkn!=NULL; tkn = tkn->next){
                if(tkn->type != LWJSON_TYPE_OBJECT){
                    return -1;
                }
                const lwjson_token_t* tkn2;
                int type, mode;
                
                // parsing values of fields within the filegroup

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
                        char buffer[MAX_PATHLEN];
                        
                        // regex match type
                        if(type/10 == 1){
                            sprintf(buffer, "%.*s", (int)s, lwjson_get_val_string(tkn3, &s));
                            printf("regex: %s\n",buffer);

                            if(regcomp(&(fpaths_regex[i_regex]), buffer, REG_EXTENDED) != 0) return -1;

                            // storing mode/type for each regex
                            mode_regex[i_regex] = mode;
                            type_regex[i_regex] = (type%10 == 0 ? '0' : '1');
                            i_regex++;
                            if(i_regex == MAX_FILES_PER_TYPE/2){
                                perror("max file limit reached");
                                break;
                            }
                        }

                        // exact match type
                        else if(type/10 == 0){
                            sprintf(*(fpaths_abs+i_abs), "%.*s", (int)s, lwjson_get_val_string(tkn3, &s));
                            mode_abs = mode;
                            i_abs++;
                            if(i_abs == MAX_FILES_PER_TYPE){
                                perror("max file limit reached");
                                break;
                            }
                        }
                        // beginning type
                        else if(type/10 == 2){
                            sprintf(buffer, "%.*s", (int)s, lwjson_get_val_string(tkn3, &s));
                            printf("beginning matching: %s\n",buffer);

                            // path
                            if(type%10 == 0){
                                trie_add_string(&trie_begp, buffer, (int)s, 0, mode);
                                num_begp++;
                            }
                            // filename
                            else if (type%10 == 1){
                                trie_add_string(&trie_begf, buffer, (int)s, 0, mode);
                                num_begf++;
                            }
                            else{
                                perror("Invalid type encountered");
                                break;
                            }                            
                        }
                        // ending type
                        else if(type/10 == 3){
                            sprintf(buffer, "%.*s", (int)s, lwjson_get_val_string(tkn3, &s));
                            printf("end matching: %s\n",buffer);

                            // path
                            if(type%10 == 0){
                                trie_add_string(&trie_endp, buffer, (int)s, 1, mode);
                                num_endp++;
                            }
                            // filename
                            else if (type%10 == 1){
                                trie_add_string(&trie_endf, buffer, (int)s, 1, mode);
                                num_endf++;
                            }
                            else{
                                perror("Invalid type encountered");
                                break;
                            }
                        }
                        else {
                            perror("Invalid type encountered");
                            break;
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

    // logging the config in the log file
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
    // default configurations/initialisation
    ignore_grps = 1;
    verbose = 1;
    num_absolute = num_regex = num_begf = num_begp = num_endp = num_endf = 0;
    init_vertex(&trie_begf);
    init_vertex(&trie_begp);
    init_vertex(&trie_endf);
    init_vertex(&trie_endp);

    // reading the configuration file
    if(read_config() < 0){
        perror("error while reading config file, using defaults");
    }
    // if(((trie_v *)vector_get(&trie_endp, 4))->leaf == false) puts("hehe");
    printf("size of 4 trie data structures: %lu %lu %lu %lu\n", vector_size(&trie_begp), vector_size(&trie_begf), vector_size(&trie_endp), vector_size(&trie_endf));
    
    struct sockaddr_un addr;
    char buf[MAX_BUFFLEN];
    int sfd, cfd, nbytes;

    // if(((trie_v *)vector_get(&trie_endp, 4))->leaf == false) puts("hehe");
     
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
    
    FILE *fp;
    fp = fopen(logfile_path, "r+");
    // accepting connections from the active sockets
    printf("\n##########################\nProcess-Activity-Firewall\n##########################\n");
    
    // if(((trie_v *)vector_get(&trie_endp, 4))->leaf == false) puts("hehe");
    while(1){
        if((cfd = accept(sfd, NULL, NULL)) == -1){
            perror("accept error");
            continue;
        }

        while((nbytes=read(cfd, buf, MAX_BUFFLEN)) > 0) {
            // printing the logged buffer
            if(parse_JSON(buf) == lwjsonOK){
                const lwjson_token_t* t;
                char pathname[MAX_PATHLEN], filename[MAX_PATHLEN];
                size_t s;
                int f = 0, m = 1, mode = !ignore_grps;

                // matching the filepath/filename using the stored config data
                if(!ignore_grps && (t = lwjson_find(&lwjson, "params.file.path")) != NULL){
                    sprintf(pathname, "%.*s", (int)s, lwjson_get_val_string(t, &s));
                    const char *basename;
                    size_t l;
                    cwk_path_get_basename(pathname, &basename, &l);
                    sprintf(filename, "%.*s", (int)l, basename);

                    // the other criterias are matched only if the preceding criterias aren't matched
                    
                    if(num_absolute) { // exact match
                        int v = find(fpaths_abs, pathname, num_absolute);
                        if(v >=0) {
                            mode = mode_abs+1;
                            f = 1;
                        }
                    }
                    if(!f && num_begf){ // beginning of pathname
                        int status = trie_traverse(&trie_begf, filename, strlen(filename), 0);
                        if(status >= 0) f = 1;
                        mode = status + 1 ;
                        puts("here checking beginning of the filename");
                    }
                    if(!f && num_begp){ // beginning of filename
                        int status = trie_traverse(&trie_begp, pathname, strlen(pathname), 0);
                        if(status >= 0) f = 1;
                        mode = status + 1 ;
                        puts("here checking beginning of the path");
                    }
                    if(!f && num_endf){ // ending of filename
                        int status = trie_traverse(&trie_endf, filename, strlen(filename), 1);
                        if(status >= 0) f = 1;
                        mode = status + 1 ;
                        puts("here checking ending of the filename");
                    }
                    if(!f && num_endp){ // ending of pathname
                        // puts(pathname);
                        if(((trie_v *)vector_get(&trie_endp, 4))->leaf == false) puts("hehe");
                        int status = trie_traverse(&trie_endp, pathname, strlen(pathname), 1);
                        if(status >= 0) f = 1;
                        mode = status + 1 ;
                        puts("here checking ending of the path");
                    }
                    if(!f && num_regex){ // matching regex
                        int v = match_regex(fpaths_regex, type_regex, pathname, num_regex);
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

                // printing according to the matched string's mode (if the filename/path is not matched anywhere, then the info is logged)
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