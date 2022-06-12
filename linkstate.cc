#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <queue>
#include <climits>
using namespace std;

char* topology_file_name;
char* message_file_name;
char* changes_file_name;
FILE * topology_file, *message_file, *changes_file;
int network_node_num;
int link_start, link_end, link_cost;
int max_dist=INT_MAX;

vector<vector<pair<int,int>> > adj_list;
vector<vector<int> > prev_link;
vector<vector<int> > node_dist;

void add_edge(int s, int e, int c){
    adj_list[s].push_back({e,c});
    adj_list[e].push_back({s,c});
}

void dijkstra(int start){
    int i, cur, temp_next, temp_next_dist;
    priority_queue<pair<int,int>> pq;
    vector<int> visited(network_node_num+1, 0);
    for(i=0;i<=network_node_num;i++){
        node_dist[start][i]=max_dist;
    }
    node_dist[start][start]=0;
    pq.push({0, start});
    while(!pq.empty()){
        cur=pq.top().second; pq.pop();
        if(visited[cur]){continue;}
        visited[cur]=1;
        for(pair<int,int> p:adj_list[cur]){
            temp_next=p.first; temp_next_dist=p.second;
            if(node_dist[start][temp_next] > node_dist[start][cur]+temp_next_dist){
                prev_link[start][temp_next]=cur; //temp_next로 가기 위해서는 이전에 cur를 거쳐 와야 한다.
                node_dist[start][temp_next] = node_dist[start][cur]+temp_next_dist;
                pq.push({-node_dist[start][temp_next], temp_next});
            }
        }
    }
}

int main(int argc, char** argv){
    if(argc!=4){
        printf("usage: linkstate topologyfile messagesfile changesfile\n");
        exit(1);
    }

    int i,j;
    int start_node, goal_node;
    int next_node_for_route;

    topology_file_name=argv[1];
    message_file_name=argv[2];
    changes_file_name=argv[3];

    topology_file= fopen(topology_file_name, "r");
    if(topology_file==NULL){
        printf("Error: open input file\n");
        exit(1);
    }
    fscanf(topology_file, "%d", &network_node_num);

    adj_list.resize(network_node_num+1);
    prev_link.resize(network_node_num+1);
    node_dist.resize(network_node_num+1);
    for(i=0;i<=network_node_num;i++){
        prev_link[i].resize(network_node_num+1);
        node_dist[i].resize(network_node_num+1);
    }
    while(fscanf(topology_file, "%d %d %d", &link_start, &link_end, &link_cost)!=EOF){
        add_edge(link_start, link_end, link_cost);
    }

    for(start_node=0;start_node<network_node_num;start_node++){
        dijkstra(start_node);
        for(goal_node=0;goal_node<network_node_num;goal_node++){
            if(start_node==goal_node){
                printf("%d %d %d\n", start_node, goal_node, 0);
            }
            else{
                next_node_for_route=goal_node;
                while(prev_link[start_node][next_node_for_route]!=start_node){
                    next_node_for_route=prev_link[start_node][next_node_for_route];
                }
                printf("%d %d %d\n", goal_node, next_node_for_route, node_dist[start_node][goal_node]);
            }
        }
        printf("\n");
    }
    
    return 0;
}