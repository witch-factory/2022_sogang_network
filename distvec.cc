#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <queue>
#include <climits>
#include <string>
#include <fstream>
using namespace std;

char* topology_file_name;
char* message_file_name;
char* changes_file_name;
ifstream topology_file_stream, message_file_stream, changes_file_stream;
int network_node_num;
int link_start, link_end, link_cost;
int max_dist=INT_MAX/2;
int path_length;

vector<vector<pair<int,int> > > adj_list;
vector<vector<int> > prev_link;
vector<vector<int> > node_dist;
vector<vector<pair<int,int> > > routing_table;

int msg_source, msg_dest;
string msg_message;

int start_node, goal_node;
int cur_node;
int next_node_for_route;
int change_node_start, change_node_goal, change_node_cost;

int edge_dest, edge_cost;

void add_edge(int s, int e, int c){
    adj_list[s].push_back(make_pair(e,c));
    adj_list[e].push_back(make_pair(s,c));
}

void change_edge(int s, int e, int c){
    //s 에서 e 로 가는 간선을 cost c로 바꾼다.
    int len_s=adj_list[s].size();
    int len_e=adj_list[e].size();
    int i;
    int edge_exist=0;

    for(i=0;i<len_s;i++){
        if(adj_list[s][i].first==e){
            adj_list[s][i].second=c;
            edge_exist=1;
        }
    }
    for(i=0;i<len_e;i++){
        if(adj_list[e][i].first==s){
            adj_list[e][i].second=c;
            edge_exist=1;
        }
    }
    if(edge_exist==0){
        //이 간선이 기존에는 없었다
        add_edge(s,e,c);
    }
}

void erase_edge(int s, int e){
    int i;
    int len_s=adj_list[s].size();
    int len_e=adj_list[e].size();
    int idx;

    for(i=0;i<len_s;i++){
        if(adj_list[s][i].first==e){
            idx=i;
        }
    }
    adj_list[s].erase(adj_list[s].begin()+idx);
    for(i=0;i<len_e;i++){
        if(adj_list[e][i].first==s){
            idx=i;
        }
    }
    adj_list[e].erase(adj_list[e].begin()+idx);
}

void bellman_ford(int start){
    int len,i;
    node_dist[start][start]=0;
    for(path_length=0;path_length<network_node_num-1;path_length++){
        for(cur_node=0;cur_node<network_node_num;cur_node++){
            len=adj_list[cur_node].size();
            for(i=0;i<len;i++){
                edge_dest=adj_list[cur_node][i].first; edge_cost=adj_list[cur_node][i].second;
                if(node_dist[start][cur_node]!=max_dist && node_dist[start][edge_dest] > node_dist[start][cur_node] + edge_cost){
                    node_dist[start][edge_dest]=node_dist[start][cur_node] + edge_cost;
                    prev_link[start][edge_dest]=cur_node;
                }
            }
        }
    }
}

void make_routing_table(){
    int i,j;
    for(i=0;i<network_node_num;i++){
        for(j=0;j<network_node_num;j++){
            if(i==j){node_dist[i][j]=0;}
            else{node_dist[i][j]=max_dist;}
        }
    }

    for(start_node=0;start_node<network_node_num;start_node++){
        bellman_ford(start_node);
        for(goal_node=0;goal_node<network_node_num;goal_node++){
            if(start_node==goal_node){
                routing_table[start_node][goal_node]= make_pair(goal_node,0);
                printf("%d %d %d\n", start_node, goal_node, 0);

                //start node에서 goal node로 가려면 다음으로 route 노드 지나야 하고 총 거리는 0이다
            }
            else{
                next_node_for_route=goal_node;
                while(prev_link[start_node][next_node_for_route]!=start_node){
                    //cout<<next_node_for_route<<"\n";
                    next_node_for_route=prev_link[start_node][next_node_for_route];
                }

                routing_table[start_node][goal_node]= make_pair(next_node_for_route, node_dist[start_node][goal_node]);
                if(node_dist[start_node][goal_node]!=max_dist){
                    //라우팅 테이블에 경로가 없으면 출력하지 않는다.
                    printf("%d %d %d\n", goal_node, next_node_for_route, node_dist[start_node][goal_node]);
                }

            }
        }
        printf("\n");
    }
}

void process_message_file(){
    message_file_stream.open(message_file_name, ifstream::in);
    while(message_file_stream>>msg_source>>msg_dest){
        getline(message_file_stream, msg_message);
        msg_message.erase(msg_message.begin()); //맨 앞의 띄어쓰기 제거
        cout<<msg_source<<" "<<msg_dest<<" "<<msg_message<<"\n";

        if(node_dist[msg_source][msg_dest]==max_dist){
            cout<<"from "<<msg_source<<" to "<<msg_dest<<" cost infinite hops unreachable message "<<msg_message<<"\n";
        }
        else{
            cout<<"from "<<msg_source<<" to "<<msg_dest<<" cost "<<node_dist[msg_source][msg_dest]<<" hops ";
            cur_node=msg_source;
            while(cur_node!=msg_dest){
                cout<<cur_node<<" ";
                cur_node=routing_table[cur_node][msg_dest].first;
            }
            cout<<"message "<<msg_message<<"\n";
        }

    }
    message_file_stream.close();
}

int main(int argc, char** argv){
    if(argc!=4){
        printf("usage: distvec topologyfile messagesfile changesfile\n");
        exit(1);
    }

    int i,j;

    topology_file_name=argv[1];
    message_file_name=argv[2];
    changes_file_name=argv[3];

    /* topology 입력받기 */
    topology_file_stream.open(topology_file_name, ifstream::in);
    if(topology_file_stream.fail()){
        cout<<"Error: open input file\n";
        exit(1);
    }
    topology_file_stream>>network_node_num;

    adj_list.resize(network_node_num+1);
    prev_link.resize(network_node_num+1);
    node_dist.resize(network_node_num+1);
    routing_table.resize(network_node_num+1);
    for(i=0;i<=network_node_num;i++){
        prev_link[i].resize(network_node_num+1);
        node_dist[i].resize(network_node_num+1);
        routing_table[i].resize(network_node_num+1);
    }

    for(i=0;i<network_node_num;i++){
        for(j=0;j<network_node_num;j++){
            if(i==j){node_dist[i][j]=0;}
            else{node_dist[i][j]=max_dist;}
        }
    }

    while(topology_file_stream>>link_start>>link_end>>link_cost){
        add_edge(link_start, link_end, link_cost);
    }
    topology_file_stream.close();

    make_routing_table();

    process_message_file();

    changes_file_stream.open(changes_file_name, ifstream::in);

    while(changes_file_stream>>change_node_start>>change_node_goal>>change_node_cost){
        if(change_node_cost==-999){
            erase_edge(change_node_start, change_node_goal);
        }
        else{
            change_edge(change_node_start, change_node_goal, change_node_cost);
        }
        make_routing_table();

        process_message_file();

    }
    changes_file_stream.close();

    return 0;
}