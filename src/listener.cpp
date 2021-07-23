#include <headers.hpp>
#include <nlohmann/json.hpp>
#include <cwalk/cwalk.h>
#include <trie/trie.hpp>

using namespace std;
using json = nlohmann::json;

bool ignore_grps, verbose;
string logfile_path;

//storing (string, permission, mode)
vector<tuple<string,int,int> > fpaths_abs;

// storing (regex, permission, logmode, type(filename/path))
vector<tuple<string,int,int,int> > fpaths_regex;
Trie begp, begf, endp, endf;

// this function reads config.json from the root directory
int read_config(){
    // opening the config file
    ifstream f("../config.json");
    string buff;
    stringstream buffer;
    buffer << f.rdbuf();
    buff = buffer.str();

    json j = json::parse(buff);

    if(!j["ignore_groups"].is_null()){
        ignore_grps = j["ignore_groups"];
    }
    if(!j["logfile_path"].is_null()){
        logfile_path = j["logfile_path"];
    }
    if(!j["verbose"].is_null()){
        verbose = j["verbose"];
    }

    if(!j["file_groups"].is_null()){
        for(json grp : j["file_groups"]){
            int type = grp["type"];
            // cout << "here" << endl;
            vector<string> ps = grp["permission"];
            // cout << "passed" << endl;
            
            int permission;
            sort(ps.begin(), ps.end());

            /*  permission values
                "log" : 0
                "deny": 1
                "allow" : 2
                "log" and "allow" : 3
                otherwise : 0
            */
            if(ps.size() == 1){
                permission = (ps[0]=="allow" ? 2 :(ps[0]=="deny"? 1 : 0));
            }
            else if(ps.size() == 2){
                if(ps[0]=="allow") permission = 3;
                else {
                    cerr << "Invalid format detected. Only logging for now." << endl;
                    permission = 0;
                }
            }
            else {
                cerr << "Too many arguments in the permission list, " << grp[permission] << endl;
                cerr << "Only logging is set." << endl;
                permission = 0;
            }

            int mode = grp["mode"];

            if(!grp["exprs"].is_null()){
                for(string s : grp["exprs"]){
                    if (type==0){
                        fpaths_abs.push_back(make_tuple(s, permission, mode));
                        // cout << "here" << endl;
                    }
                    else if (type == 20){
                        begp.add_string(s, permission, mode, 0);
                    }
                    else if (type == 21){
                        begf.add_string(s, permission, mode, 0);
                    }
                    else if (type == 30){
                        endp.add_string(s, permission, mode, 1);
                    }
                    else if (type == 31){
                        endf.add_string(s, permission, mode, 1);
                    }
                    else if(type == 10){
                        fpaths_regex.push_back(make_tuple(s, permission, mode, 0));
                    }
                    else if(type == 11){
                        fpaths_regex.push_back(make_tuple(s, permission, mode, 1));
                    }
                    else{
                        perror("invalid type encountered");
                    }
                }
            }

        }
    }
    // cout << "here" << endl;
    sort(fpaths_abs.begin(), fpaths_abs.end());
    out(fpaths_abs);
    out(fpaths_regex);

    // logging the config in the log file
    FILE *fp;
    const char* filepath = logfile_path.c_str();
    fp = fopen(filepath, "w");
    if(!fp){
        perror("Unable to open the logfile");
        exit(0);
    }
    fprintf(fp, "{\n\"config\": %s,\n\"logs\": [\n\t{}\n]\n}", buff.c_str());
    fflush(fp);
    fclose(fp);

    return 0;
}

// Here, we search for the matching instance for the filename that is input through the domain socket to this executable
void search_for_match(string &pathname, int &f, int &mode, int &permission){
    const char *basename;
    char filename[MAX_PATHLEN];
    size_t l;
    cwk_path_get_basename(pathname.c_str(), &basename, &l);
    sprintf(filename, "%.*s", (int)l, basename);
    // cout << filename << endl;
    // the other criterias are matched only if the preceding criterias aren't matched
    
    if(fpaths_abs.size() > 0) { // exact match
        int v = find(fpaths_abs, pathname);
        if(v >=0 ) {
            mode = get<2>(fpaths_abs[v]);
            permission = get<1>(fpaths_abs[v]);
            f = 1;
        }
    }
    if(!f && begf.size()>0){ // beginning of pathname
        f = begf.traverse(filename, permission, mode, 0);
        if(f) mode++;
        // cout << "checking the beginning of the filename" << f << endl;
    }
    if(!f && begp.size()>0){ // beginning of filename
        f = begp.traverse(pathname, permission, mode, 0);
        if(f) mode++;
        // puts("here checking beginning of the path");
    }
    if(!f && endf.size()>0){ // ending of filename
        f = endf.traverse(filename, permission, mode, 1);
        if(f) mode++;
        // puts("here checking ending of the filename");
    }
    if(!f && endp.size()>0){ // ending of pathname
        // puts(pathname);
        f = endp.traverse(pathname, permission, mode, 1);
        if(f) mode++;
        // puts("here checking ending of the path");
    }
    if(!f && fpaths_regex.size()>0){ // matching regex
        int v = match_regex(fpaths_regex, pathname);
        // printf("matching regex %d\n", v);

        if(v>=0){
            mode = get<2>(fpaths_regex[v])+1;
            permission = get<1>(fpaths_regex[v]);
            f = 1;
        }
    }
}

int main(int argc, char *argv[]){
    
    if(argc > 2){
        printf("Usage: %s <substring>", argv[1]);
        return 1;
    }
    // default configurations/initialisation
    ignore_grps = 1;
    verbose = 1;

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
    
    FILE *fp;
    const char* filepath = logfile_path.c_str();
    fp = fopen(filepath, "r+");
    // accepting connections from the active sockets
    printf("\n##########################\nProcess-Activity-Firewall\n##########################\n");
    

    int f, mode = !ignore_grps, permission = 0;
    while(1){
        if((cfd = accept(sfd, NULL, NULL)) == -1){
            perror("accept error");
            continue;
        }

        int done = 0; 
        bool logged = 0;
        do {
            // printing the logged buffer
            if((nbytes=read(cfd, buf, MAX_BUFFLEN)) > 0){
                json j;
                try{
                    j = json::parse(buf);
                }
                catch(nlohmann::detail::exception err){
                    cerr << err.id << '\n';
                    break;
                }
                
                
                if(j["path"].is_null() && (permission == 0 || permission == 3)){
                    size_t s;
                    // printf("f: %d, mode: %d, permission: %d, logged: %d", f, mode, permission, logged);
                    // cout << endl;
                    int m = 1;
                    
                    if (argc == 2){
                        m = match(buf, argv[1]);
                        // cout << mode << ' ' << m << "" << argv[1] << endl;
                    }
                    if(!f) mode = 0;

                    // printing according to the matched string's mode (if the filename/path is not matched anywhere, then the info is logged)
                    switch (mode)
                    {
                        case 0:
                            if(m) {
                                if(verbose) cout << setw(2) << json::parse(buf) << endl;
                                fseeko(fp, -4, SEEK_END);
                                ftruncate(fileno(fp), ftello(fp));
                                fprintf(fp, ",%s]\n}", buf);
                            }
                            break;
                        case 1:
                            break;
                        case 2:
                            if(m && f) {
                                if(verbose) cout << setw(2) << json::parse(buf) << endl;
                                fseeko(fp, -4, SEEK_END);
                                ftruncate(fileno(fp), ftello(fp));
                                fprintf(fp, ",%s]\n}", buf);
                            }
                            break;
                        case 3:
                            if(m && !f) {
                                if(verbose) cout << setw(2) << json::parse(buf) << endl;
                                fseeko(fp, -4, SEEK_END);
                                ftruncate(fileno(fp), ftello(fp));
                                fprintf(fp, ",%s]\n}", buf);
                            }
                            break;
                        default:
                            break;
                    }
                
                    fflush(fp);
                    logged = 1;
                }
                else if(!j["path"].is_null()){
                    f = 0; mode = !ignore_grps; permission = 0; logged = 0;
                    string pathname = j["path"];

                    // matching the filepath/filename using the stored config data
                    if(!ignore_grps){
                        search_for_match(pathname, f, mode, permission);
                    }
                }
            } else{
                if(nbytes < 0) perror("read");
                done = 1;
            }
            if(!done){
                // sending permission (retrieved from the config file) through the domain socket to allow/deny function calls to matched files
                char response[10];

                if(logged) sprintf(response, "logged");
                else if(permission == 1) sprintf(response, "deny");
                else sprintf(response, "allow");

                int t = 10;
                int len = strlen(response);
                while(t){
                    if(write(cfd, response, len+1) == len+1){
                        break;
                    }
                    t--;
                }
            }

        } while(!done);

        close(cfd);
    }

    return 0;
}