#include<iostream>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<ctime>

/*

run in linux to use fork and getpid() to get process ID
pid > 0 -- parent
pid = 0 -- child
pid < 0 -- fail

*/

using namespace std;
void with_control();
void without_control();
int main()
{
    srand(time(0));
    if(rand()%2 == 0)
    {
        without_control();
    }
    else
    {
        with_control();
    }
}

void with_control()
{
    int prevent;
    pid_t pid;

    pid = fork();
    if(pid > 0)
    {
        wait(&prevent);
        cout<<"Also, my grandpa's name is "<<getpid()<<"\n";
    }
    else
    {
        pid_t pid2 = fork();
        if(pid2 > 0)
        {
            wait(&prevent);
            cout<<getpid()<<".\n";
            pid_t pid3 = fork();
            if(pid3 == 0)
            {
                cout<<"I have a brother, his name is "<<getpid()<<".\n";
                exit(0);
            }
            else if(pid3 > 0)
            {
                wait(&prevent);
            }
            else
            {
                cout<<"Construct Fail!"<<"\n";
            }
        }
        else if(pid2 == 0)
        {
            cout<<"I am a child, my name is "<<getpid()<<",\nand My dad's name is ";
            exit(0);
        }
        else
        {
            cout<<"Construct Fail!"<<"\n";
        }
        exit(0);
    }
}

void without_control()
{
    int prevent;
    pid_t pid;

    pid = fork();
    if(pid > 0)
    {
        sleep(1);
        cout<<"Grandpa's name is "<<getpid()<<"\n";
        exit(0);
    }
    else
    {
        pid_t pid2 = fork();
        if(pid2 > 0)
        {
            sleep(1);
            cout<<"Dad's name is "<<getpid()<<".\n";
            pid_t pid3 = fork();
            if(pid3 == 0)
            {
                cout<<"Brother's name is "<<getpid()<<".\n";
            }
            else if(pid3 > 0)
            {
                sleep(1);
                exit(0);
            }
            else
            {
                cout<<"Construct Fail!"<<"\n";
            }
        }
        else if(pid2 == 0)
        {
            cout<<"My name is "<<getpid()<<".\n";
        }
        else
        {
            cout<<"Construct Fail!"<<"\n";
        }
    }
}
