#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <cstdlib>
#include <string>
#include <fstream>
using namespace std;

char* topology_file_name;
char* message_file_name;
char* changes_file_name;
string output_file_name;
FILE * topology_file, *message_file, *changes_file;
ifstream topology_file_stream, message_file_stream, changes_file_stream;
ofstream output_file_stream;
int output_file_opened=0;
int network_node_num;
int link_start, link_end, link_cost;
int max_dist=INT_MAX/2;

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

void dijkstra(int start){
    int i, cur, temp_next, temp_next_dist, len;
    priority_queue<pair<int,int> > pq;
    vector<int> visited(network_node_num+1, 0);
    for(i=0;i<=network_node_num;i++){
        node_dist[start][i]=max_dist;
    }
    node_dist[start][start]=0;
    prev_link[start][start]=start;
    pq.push(make_pair(0, -start));
    //dist 가장 작은 노드가 여러 개면 id값이 작은 것부터 뺀다.
    while(!pq.empty()){
        cur=-pq.top().second; pq.pop();
        if(visited[cur]){continue;}
        visited[cur]=1;
        len=adj_list[cur].size();
        for(i=0;i<len;i++){
            pair<int,int> p=adj_list[cur][i];
            temp_next=p.first; temp_next_dist=p.second;
            if(node_dist[start][temp_next] > node_dist[start][cur]+temp_next_dist){
                prev_link[start][temp_next]=cur; //temp_next로 가기 위해서는 이전에 cur를 거쳐 와야 한다.
                node_dist[start][temp_next] = node_dist[start][cur]+temp_next_dist;
                pq.push(make_pair(-node_dist[start][temp_next], -temp_next));
            }
            else if(node_dist[start][temp_next] == node_dist[start][cur]+temp_next_dist){
                //tie breaking. dist 같은 parent 여러 개면 id값이 작은 노드를 선택한다.
                prev_link[start][temp_next]=min(prev_link[start][temp_next], cur);
                node_dist[start][temp_next] = node_dist[start][cur]+temp_next_dist;
                pq.push(make_pair(-node_dist[start][temp_next], -temp_next));
            }
        }
    }
}

void make_routing_table(){
    if(output_file_opened==0){
        output_file_stream.open(output_file_name.c_str(), ofstream::out);
        output_file_opened=1;
    }
    else{
        output_file_stream.open(output_file_name.c_str(), ofstream::app);
    }

    for(start_node=0;start_node<network_node_num;start_node++){
        dijkstra(start_node);
        for(goal_node=0;goal_node<network_node_num;goal_node++){
            if(start_node==goal_node){
                routing_table[start_node][goal_node]= make_pair(goal_node,0);
                output_file_stream<<start_node<<" "<<goal_node<<" "<<0<<"\n";
                //start node에서 goal node로 가려면 다음으로 route 노드 지나야 하고 총 거리는 0이다
            }
            else{
                //애초에 도달 불가능한 건 하지 않는다
                if(node_dist[start_node][goal_node]==max_dist){continue;}
                next_node_for_route=goal_node;
                while(prev_link[start_node][next_node_for_route]!=start_node){
                    next_node_for_route=prev_link[start_node][next_node_for_route];
                }
                routing_table[start_node][goal_node]= make_pair(next_node_for_route, node_dist[start_node][goal_node]);
                if(node_dist[start_node][goal_node]!=max_dist){
                    //라우팅 테이블에 경로가 없으면 출력하지 않는다.
                    output_file_stream<<goal_node<<" "<<next_node_for_route<<" "<<node_dist[start_node][goal_node]<<"\n";
                }
            }
        }
        output_file_stream<<"\n";
    }
    output_file_stream.close();
}

void process_message_file(){
    message_file_stream.open(message_file_name, ifstream::in);
    output_file_stream.open(output_file_name.c_str(), ofstream::app);

    while(message_file_stream>>msg_source>>msg_dest){
        getline(message_file_stream, msg_message);
        msg_message.erase(msg_message.begin()); //맨 앞의 띄어쓰기 제거
        //cout<<msg_source<<" "<<msg_dest<<" "<<msg_message<<"\n";

        if(node_dist[msg_source][msg_dest]==max_dist){
            output_file_stream<<"from "<<msg_source<<" to "<<msg_dest<<" cost infinite hops unreachable message "<<msg_message<<"\n";
        }
        else{
            output_file_stream<<"from "<<msg_source<<" to "<<msg_dest<<" cost "<<node_dist[msg_source][msg_dest]<<" hops ";
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
        cout<<"usage: linkstate topologyfile messagesfile changesfile\n";
        exit(1);
    }

    int i;

    topology_file_name=argv[1];
    message_file_name=argv[2];
    changes_file_name=argv[3];
    output_file_name="output_ls.txt";

    topology_file_stream.open(topology_file_name, ifstream::in);
    if(topology_file_stream.fail()){
        cout<<"Error: open input file\n";
        exit(1);
    }
    topology_file_stream>>network_node_num;
    //fscanf(topology_file, "%d", &network_node_num);

    adj_list.resize(network_node_num+1);
    prev_link.resize(network_node_num+1);
    node_dist.resize(network_node_num+1);
    routing_table.resize(network_node_num+1);
    for(i=0;i<=network_node_num;i++){
        prev_link[i].resize(network_node_num+1);
        node_dist[i].resize(network_node_num+1);
        routing_table[i].resize(network_node_num+1);
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