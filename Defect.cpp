#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <queue>
#include <fstream>
#include <string>
using namespace std;

struct node {
    int v;
    node *next;
    int c, c_ij;                // стоимость пропускания единицы потока
    int f, L, U;
    string type;                // прямая дуга или обратная
    int defect;
    string door;                // закрываем дефектную и обратную ей дуги, когда ищем в графе альтернативный путь
    node (int j, node *t, int l, int u, int temp_c, string t_type) {
        v = j; 
        next = t;
        c = temp_c;
        f = 0;
        c_ij = c;               // так как изначально сделали все двойственные переменные равными нулю
        L = l;
        U = u;
        type = t_type;
        door = "open";
    }
};
typedef node *linkSt;

struct st_vertex {
    int v;          // номер вершины
    int pi;         // двойственная переменная
    string color;   
    int distance;   // длина пути до вершины из стока
    int min_way;    // предок вершины (сам путь)
    int flow;       // метка потока
    linkSt min_way_edge;    // из одной вершины в др могут идти много ребер. Здесь храним адрес ребра, соед данную вершину с ее предком
    
    st_vertex (int number_vertex) {
        v = number_vertex;
        pi = 0;
        flow = 0;
    }
};

int edge_defect(linkSt edge) {          // функция возвращает дефект ребра при создании графа, когда двойственные пер-ые равны нуля
    if (edge->c_ij > 0)
        return edge->f - edge->L;
    if (edge->c_ij == 0) {
         if (edge->f > edge->U)
            return edge->f - edge->U;
        else  if (edge->f < edge->L)
            return edge->f - edge->L;
        else  return 0;
    }
    if (edge->c_ij < 0) 
        return edge->U - edge->f;
    
    return 0;
}
 
int edge_defect_update(linkSt edge, vector<st_vertex> vertexes, int edge_i, int edge_j, int q) {            // обновляем дефект
    edge->c_ij = edge->c + vertexes[edge_i].pi - vertexes[edge_j].pi;
    return edge_defect(edge);
}

void Print_list (linkSt head, int i) {
    while (head != NULL) {
        cout << "(" << i << " - " <<  head->v << " ) type is " << head->type << ", L = " << head->L << ", U = " << head->U << ", c = " << head->c << ", c_ij = " << head->c_ij << ", f = " << head->f << ", defect = " << head->defect << ";\n";
        head = head->next;
    }
    cout << "\n";
}

void Print_list1 (linkSt head, int i) {
    while (head != NULL) {
        cout << "(" << i << " - " <<  head->v << " ) type is " << head->type << ", L = " << head->L << ", U = " << head->U << ", c = " << head->c << ", c_ij = " << head->c_ij << ", f = " << head->f << ", defect = " << head->defect << ";\n";
        cout << "door = " << head->door << "\n";
        head = head->next;
    }
    cout << "\n";
}

void Print_network(vector<linkSt> adj, int v) {
    for (int i = 0; i < v; i++) {
        Print_list(adj[i], i);
        cout << "\n\n";
    }
    printf("\n");
}

void Print_network1(vector<linkSt> adj, int v) {
    for (int i = 0; i < v; i++) { 
        Print_list1(adj[i], i);
        cout << "\n";
    }
    printf("-------------------------------------------------------------------\n");
}

void Print_vertexes(vector<st_vertex> vertexes) {
    for (size_t i = 0; i < vertexes.size(); i++)
        printf("v = %d, color %s, flow = %d, predok = %d, pi = %d\n", vertexes[i].v, (vertexes[i].color).c_str(), vertexes[i].flow, vertexes[i].min_way, vertexes[i].pi);
    printf("\n");
}

bool Find_defect_edge(vector<linkSt> adj, int v, int *edge_i, int *edge_j, linkSt *edge) {          
    linkSt list;
    for (int i = 0; i < v; i++) {
        list = adj[i];
        while (list != NULL) {
            if (list->defect != 0) {
                *edge_i = i;
                *edge_j = list->v;
                *edge = list;  
                return true;
            }
            list = list->next;
        }
    }
    
    return false;
}

bool Augmenting_path (vector<linkSt> adj, int v, int edge_i, int edge_j, linkSt defect_edge, linkSt back_d_edge, vector<int> *way, vector<linkSt> *edge_way, vector<st_vertex> *vertexes) {
    int source, sink;
    if (defect_edge->type == "direct") {            // получили Дефектное ребро (i - j) 
        if (defect_edge->defect < 0) {              // существует 2 случая для поиска циркуляции при прямом и при обраном ребре
            source = defect_edge->v;
            sink = edge_i;
        }
        if (defect_edge->defect > 0) {
            source = edge_i;
            sink = defect_edge->v;
        }
    }
    if (defect_edge->type == "reverse") {
        if (defect_edge->defect < 0) {
            source = edge_i;
            sink = defect_edge->v;
        }
        if (defect_edge->defect > 0) {
            source = defect_edge->v;
            sink = edge_i;
        }
    }
    // закрываем исходную найденную дефектную дугу, ее же обратную и ищем альтернативый путь(без ее участия)
    linkSt temp;
    temp = adj[edge_i];
    while (temp != NULL) {
        if (temp == defect_edge)
            temp->door = "close";
        temp = temp->next;
    }
    temp = adj[edge_j];
    while (temp != NULL) {
        if (temp == back_d_edge)
            temp->door = "close";
        temp = temp->next;
    }
    (*vertexes)[source].flow = abs(defect_edge->defect);
    Print_network1(adj, v);
    // Готовим граф к поиску в ширину
    queue<int> myqueue;
    int u = -1;
    for (int i = 0; i < v; i++) {
        (*vertexes)[i].color = "White";
        (*vertexes)[i].distance = -1;
        (*vertexes)[i].min_way = -1;
    }
    printf("source = %d; sink = %d\n", source, sink);
    (*vertexes)[source].color = "Gray";
    (*vertexes)[source].distance = 0;
    myqueue.push(source);
    
    // Граф готов!
    
    while (!myqueue.empty()) {
        printf("queue_size = %d\n", myqueue.size());
        Print_vertexes(*vertexes);
        u = myqueue.front();
        myqueue.pop();
            
        for (linkSt t = adj[u]; t != NULL; t = t->next) {
            if (((*vertexes)[t->v].color == "White") && (t->type == "direct") && (t->door == "open")) {
                if ((t->c_ij > 0) && (t->defect < 0)) {
                    (*vertexes)[t->v].flow = min((*vertexes)[u].flow, abs(t->defect));
                    (*vertexes)[t->v].color = "Gray";
                    (*vertexes)[t->v].distance = (*vertexes)[t->v].distance + 1;
                    (*vertexes)[t->v].min_way = u;
                    (*vertexes)[u].min_way_edge = t;
                    myqueue.push(t->v);
                }
                if ((t->c_ij == 0) && (t->defect == 0) && (t->f < t->U)) {                  // дуга без деффекта, но мы может изменить поток, не увелич дефектра
                    (*vertexes)[t->v].flow = min((*vertexes)[u].flow, t->U - t->f);
                    (*vertexes)[t->v].color = "Gray";
                    (*vertexes)[t->v].distance = (*vertexes)[t->v].distance + 1;
                    (*vertexes)[t->v].min_way = u;
                    (*vertexes)[u].min_way_edge = t;
                    myqueue.push(t->v);
                }
                if ((t->c_ij < 0) && (t->defect < 0)) {
                    (*vertexes)[t->v].flow = min((*vertexes)[u].flow, abs(t->defect));
                    (*vertexes)[t->v].color = "Gray";
                    (*vertexes)[t->v].distance = (*vertexes)[t->v].distance + 1;
                    (*vertexes)[t->v].min_way = u;
                    (*vertexes)[u].min_way_edge = t;
                    myqueue.push(t->v);
                }
            }
            if (((*vertexes)[t->v].color == "White") && (t->type == "reverse") && (t->door == "open")) {
                if ((t->c_ij > 0) && (t->defect > 0)) {
                    (*vertexes)[t->v].flow = min((*vertexes)[u].flow, abs(t->defect));
                    (*vertexes)[t->v].color = "Gray";
                    (*vertexes)[t->v].distance = (*vertexes)[t->v].distance + 1;
                    (*vertexes)[t->v].min_way = u;
                    (*vertexes)[u].min_way_edge = t;
                    myqueue.push(t->v);
                }
                if ((t->c_ij == 0) && (t->defect == 0) && (t->f > t->L)) {
                    (*vertexes)[t->v].flow = min((*vertexes)[u].flow, t->f - t->L);
                    (*vertexes)[t->v].color = "Gray";
                    (*vertexes)[t->v].distance = (*vertexes)[t->v].distance + 1;
                    (*vertexes)[t->v].min_way = u;
                    (*vertexes)[u].min_way_edge = t;
                    myqueue.push(t->v);
                }
                if ((t->c_ij < 0) && (t->defect > 0)) {
                    (*vertexes)[t->v].flow = min((*vertexes)[u].flow, abs(t->defect));
                    (*vertexes)[t->v].color = "Gray";
                    (*vertexes)[t->v].distance = (*vertexes)[t->v].distance + 1;
                    (*vertexes)[t->v].min_way = u;
                    (*vertexes)[u].min_way_edge = t;
                    myqueue.push(t->v);
                }
            }
        }
        (*vertexes)[u].color = "BLACK";
    }
    
    printf("queue_size = %d\n", myqueue.size());
    Print_vertexes(*vertexes);
    
    // представляю увелич путь в виде массива(обратный порядок)
    u = (*vertexes)[sink].min_way;
    way->push_back(sink);
    while (u != -1) {
        way->push_back(u);
        edge_way->push_back((*vertexes)[u].min_way_edge);
        u = (*vertexes)[u].min_way;
    }
    
    // меняю порядок следования элементов на прямой
    int temp_swap;
    linkSt temp_swap1;
    for (size_t i = 0; i < way->size() / 2; i++) {
        temp_swap = (*way)[i];
        (*way)[i] = (*way)[way->size() - i - 1];
        (*way)[way->size() - i - 1] = temp_swap;
    }
    for (size_t i = 0; i < edge_way->size() / 2; i++) {
        temp_swap1 = (*edge_way)[i];
        (*edge_way)[i] = (*edge_way)[edge_way->size() - i - 1];
        (*edge_way)[edge_way->size() - i - 1] = temp_swap1;
    }
/*    for (size_t i = 0; i < edge_way->size(); i++)
        printf("v = %d, ", (*edge_way)[i]->v);
    printf("\n");
*/    
    // в конце работы открываю ребра
    temp = adj[edge_i];
    while (temp != NULL) {
        if (temp == defect_edge)
            temp->door = "open";
        temp = temp->next;
    }
    temp = adj[edge_j];
    while (temp != NULL) {
        if (temp == back_d_edge)
            temp->door = "open";
        temp = temp->next;
    }
/*    
    printf("min way: ");
    for (size_t i = 0; i < way->size(); i++)
        printf("%d ", (*way)[i]);
    printf("\n");
    Print_vertexes(*vertexes);
*/    
    if ((*vertexes)[sink].distance != -1)
        return true;
    else
        return false;
}


int main() {
    ifstream File("testGarsia.txt");
    const int INFINITY = 10000000;
    int j, i, c, l, u;
    int v;                          // кол-во вершин
    vector<linkSt> adj;             // список смежности графа
    File >> v; 
    int product;
    File >> product;
    vector<st_vertex> vertexes;   // описание вершин
    for (i = 0; i < v; i++) {
        adj.push_back(NULL);
        vertexes.push_back(i);
    }    
    // Создание исходного графа
    while (true) {
        File >> i >> j >> l >> u >> c;    // вершина i соединена с j ребром с пропускной способностью c
        if (i == -1)            // условие окончания считывания информации из файла
            break;
        adj[i] = new node (j, adj[i], l, u, c, "direct");
        adj[i]->defect = edge_defect(adj[i]);
        adj[j] = new node (i, adj[j], l, u, c, "reverse");
        adj[j]->defect = edge_defect(adj[j]);
    }
    
    int source, sink;
    File >> source >> sink;
    // создадим возвратную дугу
    adj[sink] = new node (source, adj[sink], product, product, 0, "direct");
    adj[sink]->defect = edge_defect(adj[sink]);
    adj[source] = new node (sink, adj[source], product, product, 0, "reverse");
    adj[source]->defect = edge_defect(adj[source]);
    
    // Создали начальную сеть с нулевыми потоками и метками 
    
    Print_network1(adj, v);
    
    bool There_is_an_edge_defect;
    int edge_i, edge_j;                 // инфо о дефектном ребре, edge_i это индекс массива adj или вершины из которой исходит ребро
    linkSt defect_edge;                 // инфо о дефектном ребре
    linkSt back_d_edge, temp;
    linkSt list_f = NULL, list_back = NULL;
    vector<int> way;
    vector<linkSt> edge_way;            // массив указателей на ребра из увелич пути
    way.clear();
    int flow_go_go_go;                  // поток, который будем пускать при циркуляции
    int z;
    int q1 = INFINITY, q2 = INFINITY, q;                        // переменные кси1, кси2 и кси
    
    while ((There_is_an_edge_defect = Find_defect_edge(adj, v, &edge_i, &edge_j, &defect_edge)) == true) {
        
        printf ("defect edge: (%d - %d) %s, defect = %d\n", edge_i, edge_j, defect_edge->type.c_str(), defect_edge->defect);
        
        // найдем обратное к найденному дефектному ребру
        temp = adj[defect_edge->v];
        while (temp != NULL) {
            if (defect_edge->type == "direct") {
                if ((temp->v == edge_i) && (defect_edge->L == temp->L) && (defect_edge->U == temp->U) && (defect_edge->c == temp->c) && (temp->type == "reverse") && (defect_edge->f == temp->f) && (defect_edge->defect == temp->defect) && (defect_edge->c_ij == temp->c_ij)) {
                    back_d_edge = temp;
                    break;
                }
            }
            else {
                if ((temp->v == edge_i) && (defect_edge->L == temp->L) && (defect_edge->U == temp->U) && (defect_edge->c == temp->c) && (temp->type == "direct") && (defect_edge->f == temp->f) && (defect_edge->defect == temp->defect) && (defect_edge->c_ij == temp->c_ij)) {
                    back_d_edge = temp;
                    break;
                }
            }
            temp = temp->next;
        }
        printf ("back_defect edge: (%d - %d) %s, defect = %d\n\n", defect_edge->v, back_d_edge->v, back_d_edge->type.c_str(), back_d_edge->defect);
        if (Augmenting_path (adj, v, edge_i, edge_j, defect_edge, back_d_edge, &way, &edge_way, &vertexes) == true) {
    
            flow_go_go_go = vertexes[way[way.size()-1]].flow;       //допустимый поток окажется записан а стоке увелич пути
            printf("flow_go = %d\n", flow_go_go_go);
    
            printf("min way: ");
            for (size_t i = 0; i < way.size(); i++)
                printf("%d ", way[i]);
            printf("\n");
            // пускаю поток по найденному пути и обновляю дефекты
    
            list_f = NULL;
            list_back = NULL;
            for (size_t i = 0; i + 1 < way.size(); i++) {           //прохожусь по всем ребрам увел пути и обновляю поток
                list_f = adj[way[i]];
                while (list_f != NULL) {                            //ищу в списке смежности для вершины way[i] ребро,связывающее ее с way[i+1]
                    if ((list_f->v == way[i+1]) && (list_f == edge_way[i])) {
                        
                        list_back = adj[way[i+1]];                  //список смежности для вершины way[i+1]
                        while (list_back != NULL) {                     //ищу в списке смежности для вершины way[i+1] ребро,связывающее ее с way[i]
                            if (list_f->type == "direct") { 
                                if ((list_back->v == way[i]) && (list_back->L == list_f->L) && (list_back->U == list_f->U) && (list_back->f == list_f->f) && (list_back->c == list_f->c) && (list_back->c_ij == list_f->c_ij) && (list_back->defect == list_f->defect) && (list_back->type == "reverse")) {
                                    list_back->f += flow_go_go_go;          
                                    list_back->defect = edge_defect(list_back);
                                    break;
                                }
                            }
                            else {  // если обратное, то поток нужно уменьшить
                                if ((list_back->v == way[i]) && (list_back->L == list_f->L) && (list_back->U == list_f->U) && (list_back->f == list_f->f) && (list_back->c == list_f->c) && (list_back->c_ij == list_f->c_ij) && (list_back->defect == list_f->defect) && (list_back->type == "direct")) {
                                    list_back->f -= flow_go_go_go;
                                    list_back->defect = edge_defect(list_back);
                                    break;
                                }
                            }
                            list_back = list_back->next;
                        }
                        
                        
                        if (list_f->type == "direct") 
                            list_f->f += flow_go_go_go;
                        else
                            list_f->f -= flow_go_go_go;
                        list_f->defect = edge_defect(list_f);
                        break;
                    }
                    list_f = list_f->next;
                }     
            }
            // еще осталось пустить поток из стока в исток для циркуляции
            list_f = adj[way[way.size()-1]];
            while (list_f != NULL) {
                if ((list_f->v == way[0]) && (list_f == back_d_edge)) {
                    defect_edge->f += flow_go_go_go;
                    defect_edge->defect = edge_defect(defect_edge);
                    
                    list_f->f += flow_go_go_go;
                    list_f->defect = edge_defect(list_f);
                    break;
                }
                list_f = list_f->next;
            }
            // пустил
            
            way.clear();
            edge_way.clear();
            Print_network1(adj, v);
        }
        else {
        // если циркуляцию найти не удалось, то работаем с двойственными переменными.  
            printf("Way does not exist\n");
            Print_vertexes(vertexes);
            q1 = INFINITY;
            q2 = INFINITY;
            for (int i = 0; i < v; i++) {
                if (vertexes[i].color == "BLACK") {
                    list_f = adj[i];
                    while (list_f != NULL) {
                        if ((vertexes[list_f->v].color == "White") && (list_f->c_ij > 0) && (list_f->f <= list_f->U) && (list_f->type == "direct")) 
                            if (list_f->c_ij < q1)
                                q1 = list_f->c_ij;
                        list_f = list_f->next;
                    }
                }
            }
            for (int i = 0; i < v; i++) {
                if (vertexes[i].color == "White") {
                    list_f = adj[i];
                    while (list_f != NULL) {
                        if ((vertexes[list_f->v].color == "BLACK") && (list_f->c_ij < 0) && (list_f->f >= list_f->L) && (list_f->type == "direct"))
                            if (abs(list_f->c_ij) < q2)
                                q2 = abs(list_f->c_ij);
                        list_f = list_f->next;
                    }
                }
            }
            q = min(q1, q2);
            printf("q1 = %d, q2 = %d, q = %d\n", q1, q2, q);
            if (q == INFINITY) {                    // условие на окончание раоты. Потока не существует
                printf("Flow does not exist\n");
                break;
            }
            // увелич значение двойственных переменных для вершин из множества непомеченных вершин 
            for (int i = 0; i < v; i++)
                if (vertexes[i].color == "White")
                    vertexes[i].pi += q;
            for (int i = 0; i < v; i++) {
                list_f = adj[i];
                while (list_f != NULL) {
                    if (list_f->type == "direct")
                        list_f->defect = edge_defect_update(list_f, vertexes, i, list_f->v, q);
                    else
                        list_f->defect = edge_defect_update(list_f, vertexes, list_f->v, i, q);
                    list_f = list_f->next;
                }
            }
            Print_vertexes(vertexes);
            Print_network1(adj, v);
            way.clear();
            edge_way.clear();
        }
        
        scanf("%d", &z);
    }
    
    return 0;
}
