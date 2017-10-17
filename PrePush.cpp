#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <queue>
#include <fstream>
#include <string>
using namespace std;

struct st_vertex {
    int v;
    int e;
    int h;
    st_vertex (int number_vertex) {
        v = number_vertex;
        e = 0;
        h = 0;
    }
};

struct node {
    int v;
    node *next;
    int c, f, cf;
    node (int j, node *t, int temp_c) {
        v = j; 
        next = t;
        c = temp_c;
        f = 0;
        cf = c - f;
    }
};
typedef node *linkSt;

void Print_vertexes (int v, vector<st_vertex> vertexes) {
    for (int i = 0; i < v; i++)
        printf("v = %d,  e = %d,  h = %d\n", vertexes[i].v, vertexes[i].e, vertexes[i].h);
}

void Print_list (linkSt head) {
    while (head != NULL) {
        cout << "link with: " << head->v << " c = " << head->c << " f = " << head->f << " cf = " << head->cf << "; ";
        head = head->next;
    }
    cout << "\n";
}

void Print_residual_network(vector<linkSt> adj, int v) {
    for (int i = 0; i < v; i++) {
        cout << i << ": "; 
        Print_list(adj[i]);
        cout << "\n";
    }
}

bool Find_adj_vertex(int i, int j, vector<linkSt> *adj) {
    linkSt vertex_view = (*adj)[j];
    
    while (vertex_view != NULL) {
        if (vertex_view->v == i)
            return true;
        vertex_view = vertex_view->next;
    }
    
    return false;
}
    
int min(int a, int b) {
    if (a < b) return a;
    else return b;
}

int min(vector<int> array) {
    int min = array[0];
    for (size_t i = 1; i < array.size(); i++) 
        if (array[i] < min)
            min = array[i];
     
    return min;
}

int main() {
    ifstream File("test1.txt");
    int j, i, c;
    int v;
    vector<linkSt> adj;
    File >> v; 
    vector<st_vertex> vertexes;
    for (i = 0; i < v; i++) {
        adj.push_back(NULL);
        vertexes.push_back(i);
    }    
    
    // Создание исходного графа
    while (true) {
        File >> i >> j >> c;
        if (i == -1)
            break;
        adj[i] = new node (j, adj[i], c);
    }
    
    for (i = 0; i < v; i++) {
        linkSt vertex_view = adj[i];
        while (vertex_view != NULL) {       
            if (Find_adj_vertex(i, vertex_view->v, &adj) == false)
                adj[vertex_view->v] = new node (i, adj[vertex_view->v], 0);
            vertex_view = vertex_view->next;
        }
    }
    
    int source, sink;
    File >> source >> sink;
    vertexes[source].h = v;
    
    // Пускаю начальные поток
    
    linkSt listSource = adj[source], list_back = NULL;
    while (listSource != NULL) {
        listSource->f = listSource->c;
        listSource->cf = listSource->c - listSource->f;
        vertexes[listSource->v].e = listSource->c;
        vertexes[source].e -= listSource->c;
        // необходимо еще сделать поток антисимметричным
        list_back = adj[listSource->v];
        while (list_back != NULL) {
            if (list_back->v == source) {
                list_back->f = (-1) * listSource->c;
                list_back->cf = list_back->c - list_back->f;
                break;
            }
            list_back = list_back->next;
        }
        listSource = listSource->next;
    }
    // начальные поток пущен
    
    linkSt list_f = NULL;
    list_back = NULL;
    bool operation_exist = true;
    
    while (operation_exist) {
        printf("go\n");
        operation_exist = false;
        
        // проверяем применима ли операция проталкивания
        #if 1
        for (int i = 0; i < v; i++) {
            if ((i == sink) || (i == source))
                continue;
            if (vertexes[i].e > 0) {
                list_f = adj[i];
                printf("tolkaem1\n");
                while (list_f != NULL) {
                    if ((list_f->cf > 0) && (vertexes[i].h == vertexes[list_f->v].h + 1)) {
                        printf("tolkaem2\n");
                        int df = min(vertexes[i].h, list_f->cf);
                        list_f->f += df;
                        list_f->cf = list_f->c - list_f->f;
                        
                        list_back = adj[list_f->v];
                        while (list_back != NULL) {
                            if (list_back->v == i) {
                                list_back->f = (-1) * list_f->f;
                                list_back->cf = list_back->c - list_back->f;
                                break;
                            }
                            list_back = list_back->next;
                        }
                        
                        vertexes[i].e -= df;
                        vertexes[list_f->v].e += df;
                        operation_exist = true;
                        break;
                    }
                    list_f = list_f->next;
                }
            }
        }
        #endif
        
        #if 1
        bool condition;
        vector<int> min_h;
        for (int i = 0; i < v; i++) {
            if ((i == sink) || (i == source))
                continue;
            if (vertexes[i].e > 0) {
                condition = true;
                min_h.clear();
                for (list_f = adj[i]; list_f != NULL; list_f = list_f->next) {
                    if (list_f->cf > 0) {
                        min_h.push_back(vertexes[list_f->v].h);
                        if (vertexes[i].h > vertexes[list_f->v].h) {
                            condition = false;
                            break;
                        }
                    }
                }
                if (condition == true) {
                    printf("поднимаем\n");
                    vertexes[i].h = 1 + min(min_h);
                    operation_exist = true;
                }
            }
        }
        #endif
    }
    
    printf("\n\n\n");
    Print_residual_network(adj, v);         
    printf("\n");
    Print_vertexes(v, vertexes);
    
    printf("MAX FLOW = %d\n", vertexes[sink].e);
    
    return 0;
}

