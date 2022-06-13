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
string output_file_name;
ifstream topology_file_stream, message_file_stream, changes_file_stream;
ofstream output_file_stream;
int network_node_num;
int link_start, link_end, link_cost;
int max_dist=INT_MAX/2;
int path_length;
int file_opened=0;

vector<vector<pair<int,int> > > adj_list;
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

void bellman_ford(){
    int i,neighbor_num;
    int sending_node;
    int neighbor_route;

    for(path_length=0;path_length<network_node_num-1;path_length++){
        //path_length만큼의 경로를 가지는 최단 경로를 라우팅 테이블에 모두 업데이트한다.

        for(sending_node=0;sending_node<network_node_num;sending_node++){
            //sending_node의 라우팅 테이블을 이웃들에게 뿌린다
            neighbor_num=adj_list[sending_node].size();
            for(i=0;i<neighbor_num;i++){
                edge_dest=adj_list[sending_node][i].first;
                edge_cost=adj_list[sending_node][i].second;
                //cout<<edge_dest<<" "<<edge_cost<<"\n";
                //sending 노드의 이웃의 라우팅 테이블 업데이트
                for(neighbor_route=0;neighbor_route<network_node_num;neighbor_route++){
                    //edge_dest의 라우팅 테이블 업데이트
                    if(routing_table[edge_dest][neighbor_route].second > edge_cost + routing_table[sending_node][neighbor_route].second){
                        //sending node를 거쳐 가면 더 빠르게 갈 수 있는 경우
                        routing_table[edge_dest][neighbor_route].second=routing_table[sending_node][neighbor_route].second + edge_cost;
                        routing_table[edge_dest][neighbor_route].first=sending_node;
                    }
                    else if(routing_table[edge_dest][neighbor_route].second == routing_table[sending_node][neighbor_route].second + edge_cost){
                        //tie breaking
                        routing_table[edge_dest][neighbor_route].first=min(routing_table[edge_dest][neighbor_route].first, sending_node);
                    }
                }

            }
        }
    }

}

void make_routing_table(){
    int i,j;
    int neighbor_num;
    int sending_node;
    int neighbor_route;

    for(i=0;i<network_node_num;i++){
        for(j=0;j<network_node_num;j++){
            if(i==j){
                routing_table[i][j]= make_pair(i,0);
            }
            else{
                routing_table[i][j]= make_pair(max_dist,max_dist);
            }

        }
    }

    if(file_opened==0){
        output_file_stream.open(output_file_name, ofstream::out);
        file_opened=1;
    }
    else{
        output_file_stream.open(output_file_name, ofstream::app);
    }

    for(path_length=0;path_length<network_node_num-1;path_length++){
        //path_length만큼의 경로를 가지는 최단 경로를 라우팅 테이블에 모두 업데이트한다.

        for(sending_node=0;sending_node<network_node_num;sending_node++){
            //sending_node의 라우팅 테이블을 이웃들에게 뿌린다
            neighbor_num=adj_list[sending_node].size();
            for(i=0;i<neighbor_num;i++){
                edge_dest=adj_list[sending_node][i].first;
                edge_cost=adj_list[sending_node][i].second;
                //cout<<edge_dest<<" "<<edge_cost<<"\n";
                //sending 노드의 이웃의 라우팅 테이블 업데이트
                for(neighbor_route=0;neighbor_route<network_node_num;neighbor_route++){
                    //edge_dest의 라우팅 테이블 업데이트
                    if(routing_table[edge_dest][neighbor_route].second > edge_cost + routing_table[sending_node][neighbor_route].second){
                        //sending node를 거쳐 가면 더 빠르게 갈 수 있는 경우
                        routing_table[edge_dest][neighbor_route].second=routing_table[sending_node][neighbor_route].second + edge_cost;
                        routing_table[edge_dest][neighbor_route].first=sending_node;
                    }
                    else if(routing_table[edge_dest][neighbor_route].second == routing_table[sending_node][neighbor_route].second + edge_cost){
                        //for the tie breaking
                        //더 id값이 작은 노드를 다음 노드로 선택한다.
                        routing_table[edge_dest][neighbor_route].first=min(routing_table[edge_dest][neighbor_route].first, sending_node);
                    }
                }

            }
        }
    }

    for(i=0;i<network_node_num;i++){
        for(j=0;j<network_node_num;j++){
            output_file_stream<<j<<" "<<routing_table[i][j].first<<" "<<routing_table[i][j].second<<"\n";
        }
        output_file_stream<<"\n";
    }
    output_file_stream.close();
}

void process_message_file(){
    message_file_stream.open(message_file_name, ifstream::in);
    output_file_stream.open(output_file_name, ofstream::app);
    while(message_file_stream>>msg_source>>msg_dest){
        getline(message_file_stream, msg_message);
        msg_message.erase(msg_message.begin()); //맨 앞의 띄어쓰기 제거
        cout<<msg_source<<" "<<msg_dest<<" "<<msg_message<<"\n";

        if(routing_table[msg_source][msg_dest].second==max_dist){
            output_file_stream<<"from "<<msg_source<<" to "<<msg_dest<<" cost infinite hops unreachable message "<<msg_message<<"\n";
        }
        else{
            output_file_stream<<"from "<<msg_source<<" to "<<msg_dest<<" cost "<<routing_table[msg_source][msg_dest].second<<" hops ";
            cur_node=msg_source;
            while(cur_node!=msg_dest){
                output_file_stream<<cur_node<<" ";
                cur_node=routing_table[cur_node][msg_dest].first;
            }
            output_file_stream<<"message "<<msg_message<<"\n";
        }
    }
    output_file_stream<<"\n";
    message_file_stream.close();
    output_file_stream.close();
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
    output_file_name="output_dv1.txt";

    /* topology 입력받기 */
    topology_file_stream.open(topology_file_name, ifstream::in);
    if(topology_file_stream.fail()){
        cout<<"Error: open input file\n";
        exit(1);
    }
    topology_file_stream>>network_node_num;

    adj_list.resize(network_node_num+1);
    routing_table.resize(network_node_num+1);
    for(i=0;i<=network_node_num;i++){
        routing_table[i].resize(network_node_num+1);
    }

    for(i=0;i<network_node_num;i++){
        for(j=0;j<network_node_num;j++){
            if(i==j){
                routing_table[i][j]= make_pair(max_dist,0);
            }
            else{
                routing_table[i][j]= make_pair(max_dist,max_dist);
            }

        }
    }

    while(topology_file_stream>>link_start>>link_end>>link_cost){
        add_edge(link_start, link_end, link_cost);

        //각 정점의 이웃 정점까지만 라우팅 테이블 업데이트
        routing_table[link_start][link_end]=make_pair(link_end, link_cost); //next, cost
        routing_table[link_end][link_start]=make_pair(link_start, link_cost);

    }

    //토폴로지는 정상적으로 읽어지고 direct neighbor와의 거리도 라우팅 테이블에 잘 갱신된다.


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