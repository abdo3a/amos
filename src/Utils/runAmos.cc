#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <getopt.h>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>




using namespace std;

map<string, string> globals;   // global variables
map<string, string> variables; // conf file variables 
vector<string> cmdLnVars;      // command line variables (numbered)
ofstream logFile;
string logFileName;
string confFile;
time_t allstart; // when program started


string elapsed(time_t time) 
{
  int days;
  int hours;
  int mins;
  int secs;
  int tmp = time;
  ostringstream out;

  days = tmp / 86400; 
  tmp -= days * 86400;
  hours = tmp / 3600;
  tmp -= hours * 3600;
  mins = tmp / 60;
  tmp -= mins * 60;
  secs = tmp;

  out << days << "d " << hours << "h " << mins << "m " << secs << "s" << endl;

  return out.str();
} // elapsed


void finish(int status)
{
  if (logFile.is_open())
    logFile << "!!! END - Elapsed time: " << elapsed(time(NULL) - allstart) << endl;

  exit(status);
} // finish


string timeStr()
{
  char tm[256];
  time_t t = time(NULL);

  strftime(tm, 256, "%F %T  ", localtime(&t));

  return "!!! " + string(tm);
} // timeStr


set<string> splitBlank(string s)
{
  string tmp = s;
  char elem[256];
  int pos;
  set<string> out;

  while (sscanf(tmp.c_str(), " %s%n", elem, &pos) >= 1){ // found a string
    out.insert(elem);
    tmp = tmp.substr(pos);
  }

  return out;
} // splitBlank


void checkFiles()
{
  cerr << "First step: checking input files" << endl;
  logFile << timeStr() << "First step: checking input files" << endl;

  if (variables.find("INPUTS") == variables.end()){
    logFile << timeStr() << "variable INPUTS must be set in order to use -ocd" << endl;
    cerr <<  "variable INPUTS must be set in order to use -ocd" << endl;
    finish(1);
  }

  set<string> inputs = splitBlank(variables["INPUTS"]);
  int numerr = 0;
  for (set<string>::iterator i = inputs.begin() ; i != inputs.end(); i++){
    cerr << "checking " << i->c_str() << endl;
    if (access(i->c_str(), R_OK|F_OK) == -1){
      logFile << timeStr() << "file " << i->c_str() 
	      << " specified in the INPUTS variable cannot be read" << endl;
      numerr++;
    }
  }
  
  if (numerr != 0){
    logFile << timeStr() 
	    << "one or more files specified in INPUTS cannot be read" << endl;
    cerr << "one or more files specified in INPUTS cannot be read" << endl;
    finish(1);
  }
}


void cleanFiles()
{

  cerr << "Last step: erasing temp files" << endl;
  logFile << timeStr() << "Last step: erasing temp file" << endl;
  
  if (variables.find("TEMPS") != variables.end()){  // the easy way
    set<string> temps = splitBlank(variables["TEMPS"]);

    logFile << timeStr() 
	    << "variable TEMPS is set - ignoring INPUTS and OUTPUTS" << endl;

    for (set<string>::iterator t = temps.begin(); t != temps.end(); t++){
      logFile << timeStr() << "removing " << *t << endl;
      if (unlink((*t).c_str()) < 0){
	logFile << timeStr() << "cannot remove file: " << *t << endl;
	continue;
      }
    }
    return;
  }

  logFile << timeStr() 
	  << "variable TEMPS is not set - no cleaning done" << endl;

//   if (variables.find("INPUTS") == variables.end() ||
//       variables.find("OUTPUTS") == variables.end()){
//     logFile << timeStr() << "variables INPUTS and OUTPUTS must both be set in order to use -clean" << endl;
//     cerr << "variables INPUTS and OUTPUTS must both be set in order to use -clean" << endl;
//     finish(1);
//   }
  
//   set<string> inputs = splitBlank(variables["INPUTS"]);
//   set<string> outputs = splitBlank(variables["OUTPUTS"]);
  
//   if (inputs.size() == 0 || outputs.size() == 0){
//     logFile << timeStr() << "variables INPUTS and OUTPUTS must both be non-empty in order to use -ocd" << endl;
//     cerr << "variables INPUTS and OUTPUTS must both be non-empty in order to use -ocd" << endl;
//     finish(1);
//   }

//   inputs.insert(confFile);     // make sure we don't erase ourselves
//   outputs.insert(logFileName); // or the log file

//   DIR * thisdir = opendir(".");
//   if (thisdir == NULL){
//     logFile << timeStr() << "cannot open local dir - no cleaning will be done" 
// 	    << endl;
//     cerr << "cannot open local dir - no cleaning will be done" << endl;
//     return;
//   }

//   struct dirent * de;
//   struct stat buf;
//   while ((de = readdir(thisdir)) != NULL){
//     if (de->d_name[0] == '.') { // directory or hidden file
//       cerr << "skipping directory " << de->d_name << endl;
//       continue;
//     }

//     if (lstat(de->d_name, &buf) < 0){
//       logFile << timeStr() << "cannot stat file: " << de->d_name << endl;
//       continue;
//     }

//     if (buf.st_ctime < allstart) { // file existed before we started running
//       cerr << "skipping preexisting file " << de->d_name << " (" 
// 	   <<  buf.st_ctime << " < " << allstart << ")" << endl;
//       continue;
//     }

//     if (inputs.find(string(de->d_name)) != inputs.end() ||
// 	outputs.find(string(de->d_name)) != outputs.end()) // file in INPUTS or OUTPUTS
//       continue;

//     logFile << timeStr() << "removing " << de->d_name << endl;

//     // if all else fails, we erase the file
//     if (unlink(de->d_name) < 0){
//       logFile << timeStr() << "cannot remove file: " << de->d_name << endl;
//       continue;
//     }
//   }
}


string substVars(string & in)
{
  string out = "";
  string var;
  int i = 0;
  while (i < in.length()){
    if (in[i] != '$')
      out += in[i];
    else {
      if (in[i + 1] == '('){
	i += 2;
	var = "";
	while (i < in.length() && in[i] != ')')
	  var += in[i++];
	if (var.find_first_not_of("0123456789") == var.size()){
	  // string is a number
	  int n = strtol(var.c_str(), NULL, 10);
	  if (n >= cmdLnVars.size()){
	    cerr << "Cannot find command line parameter " << n << endl;
	    if (logFile.is_open())
	      logFile << timeStr() << "Cannot find command line parameter " 
		      << n << endl;
	    finish(1);
	  }
	  out += cmdLnVars[n];
	} else {
	  if (variables.find(var) == variables.end()){
	    cerr << "Cannot substitute variable " << var << endl;
	    if (logFile.is_open())
	      logFile << timeStr() << "Cannot substitute variable " 
		      << var << endl;
	    finish(1);
	  }
	  out += variables[var];
	}
      } else {
	out += in[i];
      }
    }
    i++;
  }
  return out;
} // substVars


void processDefn(string def)
{
  int i = 0;
  string name = "";
  string value = "";
  while (i < def.length() && def[i] != '=')
    if (def[i] != ' ' && def[i] != '\t')
      name += def[i++];
    else 
      i++;

  i++; //skip the equal sign

  while (i < def.length() && (def[i] == ' ' || def[i] == '\t'))
    i++; // skip white space
  
  while (i < def.length())
    value += def[i++];

  if (variables.find(name) != variables.end())
    return; // do nothing if variable already defined

  variables[name] = substVars(value);
}// processDefn


void printHelpText()
{
  cout << 
    "\n"
    "USAGE:\n"
    "\n"
    "runAmos -C config_file [-D VAR=value] [-s start] [-e end] [-clean] [-ocd] prefix\n"
    "\n"
    "if the config file is not specified we use environment variable AMOSCONF\n"
    "if a start step is specified (-s) starts with that command\n"
    "if an end step is specified (-e) ends with the command prior to the number\n"
    "if -clean is specified, all files except for those listed in the variables INPUTS\n"
    "and OUTPUTS get removed from the current directory\n"
    "if -ocd is specified checks that all files in the INPUTS variable exist\n"
    "\n"
    "e.g.  runAmos -s 1 -e 5   will run steps 1, 2, 3, and 4.\n"
    
    "Special variable PREFIX gets set to the prefix specified in the command line\n"
    "Lines starting with # are comments\n"
    "Lines starting with ## get displayed when next command is run\n"
    "\n"
    "Example config file:\n"
    "\n"
    "PATH = /usr/local/bin  # can also be set with -D command line option\n"
    "PERL = $(PATH)/perl\n"
    "\n"
    "1: $(PERL) $(PREFIX).pl\n"
    "2:\n"
    "$(PERL) $(PREFIX)-1.pl\n"
    "$(PERL) $(PREFIX)-2.pl\n"
       << endl;
} // printHelpText


bool GetOptions(int argc, char ** argv)
{
  int option_index = 0;
  static struct option long_options[] = {
    {"conf",  1, 0, 'c'},
    {"C",     1, 0, 'c'},
    {"help",  0, 0, 'h'},
    {"h",     0, 0, 'h'},
    {"start", 1, 0, 's'},
    {"s",     1, 0, 's'},
    {"end",   1, 0, 'e'},
    {"e",     1, 0, 'e'},
    {"clean", 0, 0, 'x'},
    {"ocd",   0, 0, 'o'},
    {"D",     1, 0, 'D'},
    {0, 0, 0, 0}
  };

  bool helpRequested = false;
  bool confFile = false;
  int c;
  while ((c = getopt_long_only(argc, argv, "", long_options, &option_index))!= -1){
    switch (c){
    case 'c':
      confFile = true;
      globals["conffile"] = string(optarg);
      break;
    case 'h':
      //      printHelpText();
      helpRequested = true;
      break;
    case 's':
      globals["start"] = string(optarg);
      break;
    case 'e':
      globals["end"] = string(optarg);
      break;
    case 'x':
      globals["clean"] = "";
      break;
    case 'o':
      globals["ocd"] = "";
      break;
    case 'D':
      processDefn(string(optarg));
      break;
    case '?':
      return false;
    }
  }

  if (helpRequested && confFile){
    ifstream conf(confFile.c_str());
    if (! conf.is_open()){
      cerr << "Could not open config file " << confFile << endl;
      finish(1);
    }
    
    string line;
    while (getline(conf, line))
      if (line.length() > 1 && line[0] == '#' && line[1] == '?') //help
	cout << line.substr(2) << endl;;

    exit(0);
  }
  
  if (helpRequested){
    printHelpText();
    exit(0);
  }

  if (optind < argc){
    variables["PREFIX"] = string(argv[optind]);
    for (;optind < argc; optind++)
      cmdLnVars.push_back(string(argv[optind]));
  } else 
    return false;
  return true;
} // GetOptions


// returns the stdout of the command
string doCommandStr(string command)
{
  ostringstream out;

  command = substVars(command);

  logFile << timeStr() << "Running: " << command << endl;

  int fd[2];
  
  if (pipe(fd) < 0){
    logFile << timeStr() << "Failed to create pipe" << endl;
    finish(1);
  }

  int status;

  pid_t process = fork();

  if (process == -1){
    logFile << timeStr() << "Could not fork!" << endl;
    finish(1);
  }

  if (process == 0){ // child
    close(fd[0]); // don't need to read

    if (dup2(fd[1], fileno(stdout)) == -1){// redirect stdout
       logFile << timeStr() << "Could not redirect stdout" << endl;
       finish(1);
    }
    execlp("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);

    logFile << timeStr() << "Could not exec command: " << command << endl;
    finish(1);
  } else {
    close(fd[1]);
    char buf[256];
    int nread;
    while ((nread = read(fd[0], buf, 256)) > 0){
      buf[nread] = 0;
      //      printf("got %s\n", buf);
      out.write(buf, nread);
    }
    if (nread == -1)
      logFile << timeStr() << "Could not read from pipe" << endl;

    //    logFile << "waiting " << endl;

    close(fd[0]);
    waitpid(process, & status, 0);
  }
  
#ifdef WCOREDUMP
  if (WCOREDUMP(status)){
    logFile << timeStr() << "Command: " << command << " exited with a coredump"
	    << endl;
    cerr << "Command: " << command << " exited with a coredump" << endl;
    finish(1);
  }
#endif

  if (WIFEXITED(status) == 0){
    logFile << timeStr() << "Command: " << command << " failed: likely through abort() or coredump" << endl;
    cerr << "Command: " << command << " failed: likely through abort() or coredump" << endl;
    finish(1);
  }

  // here we assume that WIFEXITED(status) is non null(should have been caught)

  if (WEXITSTATUS(status) != 0){
    logFile << timeStr() << "Command: " << command << " exited with status: " 
	    << WEXITSTATUS(status) << endl;
    cerr << "Command: " << command << " exited with status: " << WEXITSTATUS(status) << endl;
    finish(1);
  }

  return out.str();
} // doCommandStr

bool doCommand(string command, bool noop)
{
  // if outside of start/end range, return without doing the work

  if (noop) 
    return true;

  command = substVars(command);

  logFile << timeStr() << "Running: " << command << endl;

  int fd[2];
  
  if (pipe(fd) < 0){
    logFile << timeStr() << "Failed to create pipe" << endl;
    finish(1);
  }

  int status;

  pid_t process = fork();

  if (process == -1){
    logFile << timeStr() << "Could not fork!" << endl;
    finish(1);
  }

  if (process == 0){ // child
    close(fd[0]); // don't need to read

    if (dup2(fd[1], fileno(stdout)) == -1){// redirect stdout
       logFile << timeStr() << "Could not redirect stdout" << endl;
       finish(1);
    }
    
    if (dup2(fd[1], fileno(stderr)) == -1){ // redirect stderr
      logFile << timeStr() << "Could not redirect stderr" << endl;
      finish(1);
    }

    execlp("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);

    logFile << timeStr() << "Could not exec command: " << command << endl;
    finish(1);
  } else {
    close(fd[1]);
    char buf[256];
    int nread;
    while ((nread = read(fd[0], buf, 256)) > 0){
      buf[nread] = 0;
      //      printf("got %s\n", buf);
      logFile.write(buf, nread);
    }
    if (nread == -1)
      logFile << timeStr() << "Could not read from pipe" << endl;

    //    logFile << "waiting " << endl;

    close(fd[0]);
    waitpid(process, & status, 0);
  }
  
#ifdef WCOREDUMP
  if (WCOREDUMP(status)){
    logFile << timeStr() << "Command: " << command << " exited with a coredump"
	    << endl;
    cerr << "Command: " << command << " exited with a coredump" << endl;
    finish(1);
  }
#endif

  if (WIFEXITED(status) == 0){
    logFile << timeStr() << "Command: " << command << " failed: likely through abort() or coredump" << endl;
    cerr << "Command: " << command << " failed: likely through abort() or coredump" << endl;
    finish(1);
  }

  // here we assume that WIFEXITED(status) is non null(should have been caught)

  if (WEXITSTATUS(status) != 0){
    logFile << timeStr() << "Command: " << command << " exited with status: " 
	    << WEXITSTATUS(status) << endl;
    cerr << "Command: " << command << " exited with status: " << WEXITSTATUS(status) << endl;
    finish(1);
  }
} // doCommand



//--------------------------------------------------------
int main(int argc, char ** argv)
{

  if (!GetOptions(argc, argv)){
    cerr << "Command line parsing failed, use -h option for usage info" 
	 << endl;
    //printHelpText();
    finish(1);
  }

  char * amosConf;
  if (globals.find("conffile") == globals.end()){
    amosConf = getenv("AMOSCONF");
    if (amosConf == NULL){
      cerr << "A configuration file must be passed either through command line\n"
	"option -C or through environment variable AMOSCONF" << endl;
      finish(1);
    }
    confFile = string(amosConf);
  } else 
    confFile = globals["conffile"];


  vector<pair<string, vector<string>::iterator> > steps; //steps that will be executed
  vector<string> commands; // commands that will be executed

  ifstream conf(confFile.c_str());
  if (! conf.is_open()){
    cerr << "Could not open config file " << confFile << endl;
    finish(1);
  }

  if (variables.find("PREFIX") != variables.end()){
    logFile.open((variables["PREFIX"] + ".runAmos.log").c_str(), ios::out);
    logFileName = variables["PREFIX"] + ".runAmos.log";
  } else {
    logFile.open("runAmos.log", ios::out);
    logFileName = "runAmos.log";
  }

  if (!logFile.is_open()){
    cerr << "Cannot open logfile!" << endl;
    finish(1);
  }

  logFile.setf(ios::unitbuf);  // make sure buffer flushes on endls

  if (globals.find("start") != globals.end() &&
      globals.find("end") != globals.end()){

      if (strtol(globals["start"].c_str(), NULL, 10) > 
	  strtol(globals["end"].c_str(), NULL, 10)){
	logFile << timeStr() << "Start must be less than end\n" << endl;
	finish(1);
      }
  }

  string line;
  int lineno = 0;
  string message;
  bool multiline = false;
  int currstep = -1;
  int step;
  int noscan;// where scanf stops
  time_t start;
  bool noop = false;

  allstart = time(NULL);
  while (getline(conf, line)){
    lineno++;
    if (line.length() > 1 && line[0] == '#' && line[1] == '#'){ // message - will be saved
      message = line.substr(2);
      continue;
    } 
    if (line.length() > 0 && line[0] == '#')
      continue; // comment
    
    bool space = true;
    for (int i = 0; i < line.length(); i++)
      if (! isspace(line[i])){
	space = false;
	break;
      }

    if (space)
      continue; // skip empty lines

    if (multiline){ // part of a multi-line command
      if (line.length() == 1 && line[0] == '.'){ // end multiline
	multiline = false;
	if (! noop)
	  logFile << timeStr() << "Elapsed: " 
		  << elapsed(time(NULL) - start) << endl;
	noop = false;
	continue;
      }

      doCommand(line, noop);

      continue;
    } // multiline command

    

    if (sscanf(line.c_str(), "%d : %n", &step, &noscan) >= 1 ) {// numbered command
      if (step <= currstep){
	logFile << timeStr() << "Steps appear out of order at line " << lineno 
		<< " in " << confFile << endl;
	cerr << "Steps appear out of order at line " << lineno 
	     << " in " << confFile << endl;
	finish(1);
      } else 
	currstep = step;

      if (globals.find("ocd") != globals.end()){
	globals.erase("ocd");
	checkFiles();
      }

      if (globals.find("start") != globals.end() &&
	  strtol(globals["start"].c_str(), NULL, 10) > currstep)
	noop =  true;
      if (globals.find("end") != globals.end() &&
	  strtol(globals["end"].c_str(), NULL, 10) < currstep)
	noop = true;  
      

      if (! noop) {
	cout << "Doing step " << step;
	logFile << timeStr() << "Doing step " << step;
	if (message.length() != 0){
	  cout << ": " << message;
	  logFile << ": " << message;
	  message = "";
	}
	cout << endl;
	logFile << endl;
      }

      if (line.substr(noscan).length() == 0) {// multiline command
	multiline = true;
	start = time(NULL);
	continue;
      } else { 
	start = time(NULL);
	doCommand(line.substr(noscan), noop);
	
	if (! noop)
	  logFile << timeStr() << "Elapsed: " << elapsed(time(NULL) - start) 
		  << endl;
	noop = false;

      }

      continue;
    } // numbered commands 


    char varname[256];
    if (sscanf(line.c_str(), "%[a-zA-Z0-9_-] = %n", varname, &noscan) >= 1){
      processDefn(line); // variable definition
      continue;
    }

    int numvars;
    if (sscanf(line.c_str(), "EXPECT %d", &numvars) == 1){
      if (cmdLnVars.size() < numvars){
	cerr << "Expecting " << numvars 
	     << " parameters. Try -h for usage info" <<endl;
	finish(1);
      }
    } 

    logFile << timeStr() << "Cannot parse line " << lineno << " in " 
	    << confFile << ":" << endl << line << endl;
    cerr << "Cannot parse line " << lineno << " in " 
	 << confFile << endl;
    finish(1);
  }


  if (globals.find("clean") != globals.end())
    cleanFiles();

  // successful end
  finish(0);
}// main
