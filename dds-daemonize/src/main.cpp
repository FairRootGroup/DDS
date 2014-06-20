// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// API
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
// STD
#include <iostream>
#include <string>
// DDS
#include "version.h"
#include "Res.h"

using namespace std;
using namespace MiscCommon;

void printVersion()
{
    cout << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
         << "DDS configuration"
         << " v" << USER_DEFAULTS_CFG_VERSION << "\n" << g_cszReportBugsAddr << endl;
}

int main(int argc, char* argv[])
{

    // process ID and Session ID
    pid_t pid;
    pid_t sid;

    // Fork off the parent process
    pid = ::fork();
    if (pid < 0)
        return EXIT_FAILURE;

    // If we got a good PID, then we can exit the parent process.
    if (pid > 0)
        return EXIT_SUCCESS;

    // Change the file mode mask
    ::umask(0);

    // Create a new SID for the child process
    sid = ::setsid();
    if (sid < 0) // TODO:  Log the failure
        return EXIT_FAILURE;

    // Change the current working directory
    // chdir("/") to ensure that our process doesn't keep any directory
    // in use. Failure to do this could make it so that an administrator
    // couldn't unmount a file system, because it was our current directory.
    if (::chdir("/") < 0) // TODO: Log the failure
        return EXIT_FAILURE;

    // Close out the standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Establish new open descriptors for stdin, stdout, and stderr. Even if
    // we don't plan to use them, it is still a good idea to have them open.
    int fd = open("/dev/null", O_RDWR); // stdin - file handle 0.
    // stdout - file handle 1.
    if (dup(fd) < 0)
        cerr << "Error occurred while duplicating stdout descriptor" << endl;
    // stderr - file handle 2.
    if (dup(fd) < 0)
        cerr << "Error occurred while duplicating stderr descriptor" << endl;

    char** cmd_arg = &argv[1];
    if (argc > 2)
        execvp(argv[1], cmd_arg);

    return 0;
}