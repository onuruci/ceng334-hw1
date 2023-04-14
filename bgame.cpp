#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logging.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <signal.h>

using namespace std;

struct bomber_data {
  coordinate position;
  int fd0, fd1;
  int arg_count;
  bool alive;
  int pid;
  bool won;
};

struct bomb_struct {
  coordinate position;
  long interval;
  unsigned int radius;
  bool alive;
  int pid;
  int fd0, fd1;
};

typedef struct object_map_data {
  ot type;
  bool alive;
  bool winner;
  int duranz;
} ob;


ob* get_map_item(ob **map, int x, int y) {
  return map[y]+x;
}

void set_map_item(ob **map, int x, int y, ot obj_type, int d = -1) {
  ob new_item;
  new_item.type = obj_type;
  new_item.alive = true;
  new_item.winner = false;
  new_item.duranz = d;
  map[y][x] = new_item;
}

void update_map_item(ob **map, int x, int y, ot obj_type, bool alive) {
  ob new_item;
  new_item.type = obj_type;
  new_item.alive = alive;
  map[y][x] = new_item;
}

void move_map_item(ob **map, int x, int y, ot obj_type, int xn, int yn) {
  map[y][x].alive = false;
  map[y][x].winner = false;
  ob new_item;
  new_item.type = obj_type;
  new_item.alive = true;
  new_item.winner = false;
  map[yn][xn] = new_item;
}

void set_game_winner(ob **map, int w, int h) {
  
  for(int i = 0; i < h; i++) {
    for(int j = 0; j < w; j++) {
      if(map[i][j].alive) {
        map[i][j].winner = true;
      }
    }
  }
}

void kill_bomber(ob ** gamemap, int currx, int curry, int w, int h, int *bomber_count) {
  ob * curr_obj = get_map_item(gamemap, currx, curry);
  
  if(curr_obj->alive) {
    curr_obj->alive = false;
    (*bomber_count)--;

    if((*bomber_count) == 1) {
      set_game_winner(gamemap, w, h);
    }
  }
}

void damage_obstacle(ob **obsmap, int currx, int curry){
  ob * curr_obj = get_map_item(obsmap, currx, curry);

  if(curr_obj->alive) {
    if(curr_obj->duranz > -1) {
      curr_obj->duranz--;
      if(curr_obj->duranz == 0) {
        curr_obj->alive=false;
      }
    }
  } 
}

void get_items_on_map(ob **gamemap, int posx, int posy, od * obj_arr, int *obj_count, int map_w, int map_h, ot obj_type) {
  int miny, minx, maxy, maxx;
  miny = posy - 3;
  minx = posx - 3;
  maxy = posy + 3;
  maxx = posx + 3;

  if(miny < 0) {
    miny = 0;
  }
  
  if(minx < 0) {
    minx = 0;
  }

  if(maxy >= map_h) {
    maxy = map_h -1;
  }

  if(maxx >= map_w) {
    maxx = map_w -1;
  }

  for(int xi = minx; xi <= maxx; xi++){
    ob *map_item = get_map_item(gamemap, xi, posy);
    if(map_item->alive) {
      if(xi != posx) {
        od obj_to_add;
        obj_to_add.type = map_item->type;
        obj_to_add.position.x = xi;
        obj_to_add.position.y = posy;
        obj_arr[*obj_count] = obj_to_add;
        (*obj_count)++;
      }
    }
  }

  for(int yi= miny; yi <= maxy; yi++) {
    ob *map_item = get_map_item(gamemap, posx , yi);
    if(map_item->alive) {
      if(yi != posy) {
        od obj_to_add;
        obj_to_add.type = map_item->type;
        obj_to_add.position.x = posx;
        obj_to_add.position.y = yi;
        obj_arr[*obj_count] = obj_to_add;
        (*obj_count)++;
      }
    }
  }

  if(obj_type == BOMB) {
    ob *map_item = get_map_item(gamemap, posx , posy);
    if(map_item->alive) {
        od obj_to_add;
        obj_to_add.type = map_item->type;
        obj_to_add.position.x = posx;
        obj_to_add.position.y = posy;
        obj_arr[*obj_count] = obj_to_add;
        (*obj_count)++;
    }
  }

}


int main( int argc, char **argv )
{
  int map_w = 0, map_h = 0, obs_count = 0, bomber_count = 0;
  int obs_counter = 0, bomber_counter = 0;
  vector<vector<int>> map2d;
  ob **gamemap;
  ob **bombmap;
  ob **obsmap;
  bool info_read = false;
  
  vector<obsd> obstacles;
  vector<bomber_data> bombers;
  vector<bomb_struct> bombs;
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

        gamemap = (ob **) malloc(map_h * sizeof(ob *));
        for(int i = 0; i < map_h; i++) {
          gamemap[i] = (ob *) malloc(map_w * sizeof(ob));
        }

        bombmap = (ob **) malloc(map_h * sizeof(ob *));
        for(int i = 0; i < map_h; i++) {
          bombmap[i] = (ob *) malloc(map_w * sizeof(ob));
        }

        obsmap = (ob **) malloc(map_h * sizeof(ob *));
        for(int i = 0; i < map_h; i++) {
          obsmap[i] = (ob *) malloc(map_w * sizeof(ob));
        }

      } else if(obs_counter < obs_count) {
        /// Handle obs
        /// add them to map
        istringstream iss ( input );
        obsd new_obstacle;

        iss >> new_obstacle.position.x;
        iss >> new_obstacle.position.y;
        iss >> new_obstacle.remaining_durability;

        set_map_item(obsmap, new_obstacle.position.x, new_obstacle.position.y, OBSTACLE, new_obstacle.remaining_durability);

        obs_counter++;
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
          set_map_item(gamemap, new_bomber.position.x, new_bomber.position.y, BOMBER);
          close(fd[0]);
        } else {
          // child
          // handle bombers
          char **argv = (char **) malloc((new_bomber.arg_count+1) * sizeof(char *));

          for(int i = 0; i < new_bomber.arg_count; i++) {
            argv[i] = (char  * )malloc(4 * sizeof(char));
            iss1 >> argv[i];
          }

          argv[new_bomber.arg_count] = NULL;

          dup2(fd[0], 0);
          dup2(fd[0], 1);
          close(fd[0]);
          close(fd[1]);
         
          if (execvp(argv[0], argv) == -1) {
            char errmsg[64];
            snprintf( errmsg, sizeof(errmsg), "exec failed" );
            perror( errmsg );
          }

          for(int i = 0; i < new_bomber.arg_count; i++) {
            delete [] argv[i];
          }
          delete [] argv;
        }


        bomber_counter++;
      }

    }
  }

  
  while(!bombers.empty() || !bombs.empty() ) {
    for(auto it = bombs.begin(); it != bombs.end(); it++) {
      if(bombs.size() < 1) {
        break;
      }
      im m;
      imp new_imp;
      fd_set rfd;

      FD_ZERO(&rfd);
      FD_SET((*it).fd1, &rfd);

      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 1000;

      int ret = select((*it).fd1 + 1, &rfd, NULL, NULL, &timeout);

      if(ret) {
        if(read_data((*it).fd1, &m) > 0) {
          if(m.type == BOMB_EXPLODE) {

            new_imp.m = &m;
            new_imp.pid = (*it).pid;
            print_output(&new_imp, NULL, NULL, NULL);

            
            
            // Damage others

            int miny = (*it).position.y - (*it).interval;
            int minx = (*it).position.x - (*it).interval;
            int maxy = (*it).position.y + (*it).interval;
            int maxx = (*it).position.x + (*it).interval;

            if(miny < 0) {
              miny = 0;
            }
            
            if(minx < 0) {
              minx = 0;
            }

            if(maxy >= map_h) {
              maxy = map_h -1;
            }

            if(maxx >= map_w) {
              maxx = map_w -1;
            }

            // check for bombers and obstacles
            int currx = (*it).position.x, curry = (*it).position.y;
            kill_bomber(gamemap, currx, curry, map_w, map_h, &bomber_count);
            for(int i = 1; i <= (*it).radius; i++) {

              if(currx + i <= maxx) {
                kill_bomber(gamemap, currx + i, curry, map_w, map_h, &bomber_count);
                damage_obstacle(obsmap, currx + i , curry);
              }

              if(currx - i >= minx) {
                kill_bomber(gamemap, currx - i, curry, map_w, map_h, &bomber_count);
                damage_obstacle(obsmap, currx + i , curry);
              }

              if(curry + i <= maxy) {
                kill_bomber(gamemap, currx, curry + i, map_w, map_h, &bomber_count);
                damage_obstacle(obsmap, currx + i , curry);
              }

              if(curry - i >= miny) {
                kill_bomber(gamemap, currx, curry - i, map_w, map_h, &bomber_count);
                damage_obstacle(obsmap, currx + i , curry);
              }
            }

            // Remove Bomb
            
            update_map_item(bombmap, (*it).position.x, (*it).position.y, BOMB, false);
            
            kill((*it).pid, 9);
            bombs.erase(it);
            it--;
            continue;

          }
        }
      }
      
    }
    
    for( auto it = bombers.begin(); it != bombers.end(); it++) {
      if(bombers.size() < 1) {
        break;
      }
      im m;
      imp new_imp;
      fd_set rfd;

      FD_ZERO(&rfd);
      FD_SET((*it).fd1, &rfd);

      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 1000;

      int ret = select((*it).fd1 + 1, &rfd, NULL, NULL, &timeout);
    
      

      if( ret && read_data((*it).fd1, &m) > 0) {
        new_imp.m = &m;
        new_imp.pid = (*it).pid;
        print_output(&new_imp, NULL, NULL, NULL);
        om message;
        omp sending_message;

        int miny, minx, maxy, maxx;
        ob *target_bomber, *target_bomb;

        int object_count = 0, obj_arr_size = 50, targetx, targety;
        od *object_arr = (od *)malloc(25 * sizeof(od));

        // Check if bomber is dead or won

        ob * bomber_to_get = get_map_item(gamemap, (*it).position.x, (*it).position.y);

        if(bomber_to_get->winner) {
          bomber_to_get->alive = false;

          message.type = BOMBER_WIN;

          sending_message.pid = (*it).pid;
          sending_message.m = &message;

          print_output(NULL, &sending_message, NULL, NULL);
            
          send_message((*it).fd1, &message);

          kill((*it).pid, 9);
          bombers.erase(it);
          it--;
          
          delete [] object_arr;

          continue;
        }

        if(!bomber_to_get->alive) {
          message.type = BOMBER_DIE;

          sending_message.pid = (*it).pid;
          sending_message.m = &message;

          print_output(NULL, &sending_message, NULL, NULL);
            
          send_message((*it).fd1, &message);

          kill((*it).pid, 9);
          bombers.erase(it);
          it--;
          
          delete [] object_arr;

          continue;
        }

        if(bomber_to_get->alive && !bomber_to_get->winner) {
        switch(m.type) {
          case(BOMBER_START):
            message.type = BOMBER_LOCATION;
            message.data.new_position.x = (*it).position.x;
            message.data.new_position.y = (*it).position.y;

            sending_message.pid = (*it).pid;
            sending_message.m = &message;

            print_output(NULL, &sending_message, NULL, NULL);

            send_message((*it).fd1, &message);
            break;
          case(BOMBER_SEE):
            message.type = BOMBER_VISION;
            message.data.object_count = 0;

            sending_message.pid = (*it).pid;
            sending_message.m = &message;

            get_items_on_map(gamemap, (*it).position.x, (*it).position.y, object_arr, &object_count, map_w, map_h, BOMBER);

            get_items_on_map(obsmap, (*it).position.x, (*it).position.y, object_arr, &object_count, map_w, map_h, OBSTACLE);

            get_items_on_map(bombmap, (*it).position.x, (*it).position.y, object_arr, &object_count, map_w, map_h, BOMB);        

            message.data.object_count = object_count;

            print_output(NULL, &sending_message, NULL, object_arr);
            
            send_message((*it).fd1, &message);
            send_object_data((*it).fd1, message.data.object_count, object_arr);

            break;
          case(BOMBER_MOVE) :

            targetx = m.data.target_position.x;
            targety = m.data.target_position.y;

            target_bomber = get_map_item(gamemap, targetx, targety);
            target_bomb = get_map_item(obsmap, targetx, targety);

            message.type = BOMBER_LOCATION;
            message.data.new_position.x = (*it).position.x;
            message.data.new_position.y = (*it).position.y;

            if(!target_bomber->alive && !target_bomb->alive) {
               if(targetx != (*it).position.x && targety != (*it).position.y) {
                message.data.new_position.x = (*it).position.x;
                message.data.new_position.y = (*it).position.y;
              } else if(targetx == (*it).position.x) {
                if(targety - (*it).position.y == 1 || targety - (*it).position.y == -1) {
                  if(targety >= 0 && targety <= map_h-1) {
                    message.data.new_position.y = targety;
                  }
                }
              } else if(targety == (*it).position.y) {
                if(targetx - (*it).position.x == 1 || targetx - (*it).position.x == -1) {
                  if(targetx >= 0 && targetx <= map_w-1) {
                    message.data.new_position.x = targetx;
                  }
                }
              }
            }       

            move_map_item(gamemap, (*it).position.x, (*it).position.y, BOMBER, message.data.new_position.x,  message.data.new_position.y);

            (*it).position.x = message.data.new_position.x;
            (*it).position.y = message.data.new_position.y;

            sending_message.pid = (*it).pid;
            sending_message.m = &message;

            print_output(NULL, &sending_message, NULL, NULL);

            send_message((*it).fd1, &message);

            break;
          case(BOMBER_PLANT):
            // Check if bomber is dead or won

            int p, fd[2];
            PIPE(fd);

            if(p = fork()) {
              // parent
              close(fd[0]);
              bomb_struct new_bomb;
              new_bomb.alive = true;
              new_bomb.fd1 = fd[1];
              new_bomb.fd0 = fd[0];
              new_bomb.interval = m.data.bomb_info.interval;
              new_bomb.radius = m.data.bomb_info.radius;
              new_bomb.position.x = (*it).position.x;
              new_bomb.position.y = (*it).position.y;
              new_bomb.pid = p;
              bombs.push_back(new_bomb);
              set_map_item(bombmap, (*it).position.x, (*it).position.y, BOMB);

              message.type = BOMBER_PLANT_RESULT;
              message.data.planted = 1;

              sending_message.pid = p;
              sending_message.m = &message;

              print_output(NULL, &sending_message, NULL, NULL);

              send_message((*it).fd1, &message);


            } else {
              // child

              char **argv = (char **) malloc((3) * sizeof(char *));
              string s = to_string(m.data.bomb_info.interval);

              string exes = "./bomb";
              argv[0] = (char *) malloc(exes.length() * sizeof(char));
              for(int i=0; i < exes.length(); i++) {
                argv[0][i] = exes.at(i);
              }  

              argv[1] = (char *) malloc(s.length() * sizeof(char));
              for(int i=0; i < s.length(); i++) {
                argv[1][i] = s.at(i);
              }              
              argv[2] = NULL;


              dup2(fd[0], 0);
              dup2(fd[0], 1);
              close(fd[0]);
              close(fd[1]);
              

              execvp(argv[0], argv);

              delete [] argv[0];
              delete [] argv[1];
              delete [] argv[2];
            }


            break;
          default:
            break;
        }}
        delete [] object_arr;

      }
    }

    usleep(1000);
  }
  for(int i = 0; i < map_h; i++) {
    delete [] gamemap[i];
  }
  delete [] gamemap;

  for(int i = 0; i < map_h; i++) {
    delete [] bombmap[i];
  }
  delete [] bombmap;

  for(int i = 0; i < map_h; i++) {
    delete [] obsmap[i];
  }
  delete [] obsmap;
    
  return 0;
}
