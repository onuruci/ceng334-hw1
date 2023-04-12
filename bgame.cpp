#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logging.h"
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

struct bomber_data {
  coordinate position;
  int fd0, fd1;
  int arg_count;
  bool alive;
  int pid;
};


int main( int argc, char **argv )
{
  int map_w = 0, map_h = 0, obs_count = 0, bomber_count = 0;
  int obs_counter = 0, bomber_counter = 0;
  vector<vector<int>> map2d;
  bool info_read = false;
  
  vector<obsd> obstacles;
  vector<bomber_data> bombers;
  vector<int[2]> pipes;
  
  printf("\n\n\n");

  // Read input
  while ( true )
  {
    string input;
    getline( cin, input );

    if ( input.size() == 0 ) // empty input string
    {
      break;
    }
    else
    {
      // Read first line
      if(!info_read) {
        int value;
        istringstream iss ( input );
        iss >> map_w;
        iss >> map_h;
        iss >> obs_count;
        iss >> bomber_count;
        info_read = true;

        printf("Values read for initial update:  %d %d %d %d\n", map_w, map_h, obs_count, bomber_count);
      } else if(obs_counter < obs_count) {
        istringstream iss ( input );
        obsd new_obstacle;

        iss >> new_obstacle.position.x;
        iss >> new_obstacle.position.y;
        iss >> new_obstacle.remaining_durability;

        obs_counter++;
        printf("obs entered\n");
      } else if(bomber_counter < bomber_count) {
        // Read bomber values
        istringstream iss ( input );
        

        int p, fd[2];
        PIPE(fd);

        bomber_data new_bomber;
        iss >> new_bomber.position.x;
        iss >> new_bomber.position.y;
        iss >> new_bomber.arg_count;
        new_bomber.fd0 = fd[0];
        new_bomber.fd1 = fd[1];
        new_bomber.alive = true;

        


        getline( cin, input );
        istringstream iss1 ( input );

        if(p = fork()) {
          // parent
          new_bomber.pid = p;
          bombers.push_back(new_bomber);
          printf("parent\n");
        } else {
          // child
          char **argv = (char **) malloc(new_bomber.arg_count * sizeof(char *));

          argv[0] = (char  * )malloc(4 * sizeof(char));
          argv[1] = (char  * )malloc(4 * sizeof(char));
          argv[2] = (char  * )malloc(4 * sizeof(char));
          argv[3] = (char  * )malloc(4 * sizeof(char));
          iss1 >> argv[0];
          iss1 >> argv[1];
          iss1 >> argv[2];
          iss1 >> argv[3];
          argv[4] = NULL;

          new_bomber.pid = p;
          bombers.push_back(new_bomber);
          

          printf("child \n" );
          

          dup2(fd[0], 0);
          close(fd[0]);
          dup2(fd[1], 1);
          
          close(fd[1]);


          
          printf("asdd\n");


          // char *pa;
          // iss1 >> pa;
          // iss1 >> argss[0];
          // iss1 >> argss[1];
          // iss1 >> argss[2];
          printf("asdasdasdasd \n");
          if (execvp("./bomber_inek", argv) == -1) {
            char errmsg[64];
            snprintf( errmsg, sizeof(errmsg), "exec failed" );
            perror( errmsg );
          }
        }


        bomber_counter++;
      }

    }
  }

  
  while(true) {
    
    for( auto it = bombers.begin(); it != bombers.end(); it++) {
      im m;
      imp new_imp;

      printf("%d\n", (*it).fd0);
      if( read_data((*it).fd0, &m) > 0) {
        new_imp.m = &m;
        new_imp.pid = (*it).pid;
        print_output(&new_imp, NULL, NULL, NULL);
        if(m.type == BOMBER_START ) {
          printf("Bomber started \n");
        }
        printf("%d\n", m.type);
        printf("asd\n");
      }
      
    }
    printf("uncaut\n");

    usleep(100);
  }
}
