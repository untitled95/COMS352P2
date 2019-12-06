#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <pthread.h>
#include <regex>
#include <mutex>
#include <queue>


#include <unistd.h>
/**
 * @author yiming ling
 * wrote based on the original request
 */

using namespace std;


struct node;
vector<node*> getNodes(string a);
void initTreeHelper(node *current, node *next);
node* initTree(vector<node*> pre);
void readfile(string path);
void* search(void* key);
void* insert(void* key);
void* deleteNode(void* key);
void printTree(node* treeRoot);
void runFunctions();
void run_other_functions();


/**
 * node structure
 */
 struct node
 {
     int key;
     struct node *left;
     struct node *right;
     struct node *parent;
     bool color;  //false:black true:red
     mutex m;
 };


 struct Command
 {
     string function;
     int key;
 };

string functions = ""; // For getting all functions
node *root = new node; // For identify the entire tree
queue <pthread_t> search_threads; // For all search threads
queue <pthread_t> modify_threads; // For all modify threads
queue <Command> searches; // For all search commands
queue <Command> modifies; // For all modify commands

/**
 * get all nodes separate from each line and init value for those nodes.
 * @param (string)init nodes read from txt file
 * @return all init nodes
 */
vector<node*> getNodes(string a){
    vector<string> tempNodes;
    vector<node*> nodes;
    stringstream s_stream(a);
    while(s_stream.good()) {
        string substr;
        getline(s_stream, substr, ',');
        tempNodes.push_back(substr);
    }
    for(uint16_t i=0;i<tempNodes.size();++i) {
        if (tempNodes[i][0] == 'f') {
            node *newNode = new node;
            newNode->key = -1;
            newNode->left = NULL;
            newNode->right = NULL;
            newNode->parent = nullptr;
            newNode->color = false;
            nodes.push_back(newNode);
        } else {
            if (tempNodes[i].back() == 'b') {
                node *newNode = new node;
                tempNodes[i].pop_back();
                newNode->key = stoi(tempNodes[i]);
                newNode->left = nullptr;
                newNode->right = nullptr;
                newNode->parent = nullptr;
                newNode->color = false;
                nodes.push_back(newNode);
            } else {
                node *newNode = new node;
                tempNodes[i].pop_back();
                newNode->key = stoi(tempNodes[i]);
                newNode->left = nullptr;
                newNode->right = nullptr;
                newNode->parent = nullptr;
                newNode->color = true;
                nodes.push_back(newNode);
            }
        }
    }
     return nodes;
 }

/**
 * decide add the next node on left or right. or back to parent node
 * @param current node pointer
 * @param next node pointer
 */
void initTreeHelper(node *current, node *next){

    if (current->key!=-1 && current->left == nullptr){
        current->left = next;
        next->parent = current;
    }
    else if (current->key!=-1 && current->left != nullptr && current->right == nullptr){
        current->right = next;
        next->parent = current;
    }
    else if(current->key==-1 || (current->left != nullptr && current->right != nullptr)){
        if(current->parent!=nullptr){
            current = current->parent;
        }
        initTreeHelper(current,next);
    }
}

/**
 * Init the RB-tree based on the init text input
 * @param pre
 * @return the entire init tree
 */
node* initTree(vector<node*> pre){
    int index = 0;
    while(index < pre.size()-1){
        node *current = pre[index];
        node *next = pre[index+1];
        initTreeHelper(current,next);
        index = index+1;
    }

    return pre[0];
}

/**
 * Run all the functions
 * @param (string)a
 */


void runFunctions(){

    while(searches.size()>0) {
        pthread_t pid = search_threads.front();
        int b = pthread_create(&pid, NULL, search, (void *) &searches.front().key);
        search_threads.pop();
        search_threads.push(pid);
        searches.pop();
    }
}

/**
 * run other functions besides the search
 */
void run_other_functions(){
        while(modifies.size()>0){
            pthread_t pid = modify_threads.front();
            if(modifies.front().function=="delete"){
                int b = pthread_create(&pid, NULL, deleteNode, (void *) &modifies.front().key);
            }else{
                int * arg = (int *)malloc(sizeof(*arg));
                *arg = modifies.front().key;
                int b = pthread_create(&pid, NULL, insert, (void *) arg);
            }
            modify_threads.pop();
            modify_threads.push(pid);
            modifies.pop();
        }
}



/**
 * Read the input file
 * @param path
 */
void readfile(string path){
    string line;
    string nodeLines;
    ifstream file;
    file.open(path);
    int count = 0;
    int number_of_search_thread = 0;
    int number_of_modify_thread = 0;
    if(file.is_open())
        while (!file.eof())
        {
            getline(file, line);
            if(line!="" && count ==0){
                nodeLines = nodeLines + line; //read all the init nodes
            }
            if(line==""){
                count += 1;
                continue;
            }
            if(line!="" && count == 1){
               if(line[0]=='S'){
//                   smatch m;
//                   regex_search(line, m, regex("[0-9]+"));
//                   string temp = m.str(0);
//                   number_of_search_thread = stoi(temp);
//                   pthread_t tid[number_of_search_thread];
//                   for(int i=0;i<number_of_search_thread;++i){
//                       search_threads.push(tid[i]);
//                   }
               }else{
//                   smatch m;
//                   regex_search(line, m, regex("[0-9]+"));
//                   string temp = m.str(0);
//                   number_of_modify_thread = stoi(temp);
//                   pthread_t tid[number_of_modify_thread];
//                   for(int i=0;i<number_of_modify_thread;++i){
//                       modify_threads.push(tid[i]);
//                   }
               }
            }
            if(line!="" && count == 2){
                functions = functions + line;  // read all the functions
                std::string::iterator end_pos = std::remove(functions.begin(), functions.end(), ' ');
                functions.erase(end_pos, functions.end());
            }
        }
    vector<string> tempCommands;
    while(functions.size()>0) {
        smatch m;
        regex_search(functions, m, regex("[a-z0-9]+"));
        string temp = m.str(0);

        functions = functions.substr(temp.size()+1,functions.size());
        char temp2 = functions[0];
        if(functions[0]=='|'){
            functions = functions.substr(2,functions.size());
        }
        tempCommands.push_back(temp);
    }

    for(int i=0;i<tempCommands.size();i+=2){
        Command c;
        c.function = tempCommands[i];
        c.key = stoi(tempCommands[i+1]);
        if(tempCommands[i]=="search"){
            searches.push(c);
            pthread_t tid;
            search_threads.push(tid);
        }else{
            modifies.push(c);
            pthread_t tid;
            modify_threads.push(tid);
        }
    }

    vector<node*> initNodes = getNodes(nodeLines);
    root = initTree(initNodes);

    file.close();
}

/**
 * Search the node
 * Lock necessary nodes and unlock after
 * @param key
 * @return if found return true, otherwise return false
 */
void* search(void* key) {
    int val = *(int*)key;
    bool found = false;
    node *tempRoot = root;
    tempRoot->m.lock();
    if(tempRoot->key==-1){
        tempRoot->m.unlock();
        string f = "search(" ;
        string n = to_string(val);
        string f2 = ")-> false, performed by thread ";
        string id = to_string(long(pthread_self()));
        string output = f+n+f2+id;
        char* realOutput = strdup(output.c_str());
        return (realOutput);
    }else{
    while(tempRoot->key!=-1 && found==false){
        node *parent = tempRoot;
        if(tempRoot->key==val){
            string f = "search(" ;
            string n = to_string(val);
            string f2 = ")-> true, performed by thread ";
            string id = to_string(long(pthread_self()));
            string output = f+n+f2+id;
            char* realOutput = strdup(output.c_str());
            found = true;
            return (realOutput);
        }else if(val>=tempRoot->key){
            tempRoot = tempRoot->right;
        }else{
            tempRoot = tempRoot->left;
        }

        tempRoot->m.lock();
        parent->m.unlock();
    }
    }
    string f = "search(" ;
    string n = to_string(val);
    string f2 = ")-> false, performed by thread ";
    string id = to_string(long(pthread_self()));
    string output = f+n+f2+id;
    char* realOutput = strdup(output.c_str());
    tempRoot->m.unlock();
    return (realOutput);
}

void* deleteNode(void* key){
    return NULL;
}

/**
 * Insert function with lock
 * @param key
 * @return
 */
void* insert(void* key){
    int val = *(int*)key;
    free(key);
    bool found = false;
    node *tempNode = root;
    tempNode->m.lock();
    node *locked = tempNode;
    while(tempNode->key!=-1 && found==false){
        node *parent = tempNode;
        if(val == tempNode->key){
            found = true;
        }else if(val < tempNode->key){
            tempNode = tempNode->left;
        }else{
            tempNode = tempNode->right;
        }
        if(tempNode->color==false && parent->color ==false && parent!=locked){
            parent->m.lock();
            locked->m.unlock();
            locked = parent;
        }
    }
    if(found==false){
        tempNode->m.lock();
        tempNode->color = true;
        tempNode->key = val;
        node *leftNode = new node;

        leftNode->key = -1;
        leftNode->left = NULL;
        leftNode->right = NULL;
        leftNode->parent = tempNode;
        leftNode->color = false;

        tempNode->left = leftNode;

        node *rightNode = new node;

        rightNode->key = -1;
        rightNode->left = NULL;
        rightNode->right = NULL;
        rightNode->parent = tempNode;
        rightNode->color = false;

        tempNode->right = rightNode;
        tempNode->m.unlock();

        while(tempNode!=root && tempNode->parent->color == true){
            node *parent = tempNode->parent;
            node *grandparent = parent->parent;
            if(parent == grandparent->left){
                node *aunt = grandparent->right;
                if(aunt->color == true){
                    aunt->color = false;
                    parent->color = false;
                    grandparent->color = true;
                    tempNode = grandparent;
                }
                else if(tempNode == parent->left){
                    parent->color = false;
                    grandparent->color = true;
                    node *sister = parent->right;
                    if(grandparent==root){
                        grandparent->m.lock();
                        parent->m.lock();
                        sister->m.lock();
                        root = parent;
                        parent->right = grandparent;
                        parent->left = sister;
                        sister->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                    }else{
                        node *grandgrandparent = grandparent->parent;
                        grandgrandparent->m.lock();
                        grandparent->m.lock();
                        parent->m.lock();
                        sister->m.lock();
                        if(grandgrandparent->left == grandparent){
                            grandgrandparent->left = grandparent;
                        }else{
                            grandgrandparent->right = grandparent;
                        }
                        parent->right = grandparent;
                        grandparent->left = sister;
                        sister->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                        grandgrandparent->m.unlock();
                    }
                }
                else{
                    tempNode->color = false;
                    grandparent->color = true;
                    leftNode = tempNode->left;
                    rightNode = tempNode->right;
                    if(grandparent==root){
                        grandparent->m.lock();
                        parent->m.lock();
                        tempNode->m.lock();
                        leftNode->m.lock();
                        rightNode->m.lock();
                        root = tempNode;
                        tempNode->left = parent;
                        tempNode->right = grandparent;
                        parent->right = leftNode;
                        grandparent->left=rightNode;
                        rightNode->m.unlock();
                        leftNode->m.unlock();
                        tempNode->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                    }else{
                        node *grandgrandparent = grandparent->parent;
                        grandgrandparent->m.lock();
                        grandparent->m.lock();
                        parent->m.lock();
                        tempNode->m.lock();
                        leftNode->m.lock();
                        rightNode->m.lock();
                        if(grandgrandparent->left == grandparent){
                            grandgrandparent->left = tempNode;
                        }else{
                            grandgrandparent->right = tempNode;
                        }
                        tempNode->left = parent;
                        tempNode->right = grandparent;
                        parent->right = leftNode;
                        grandparent->left = rightNode;
                        rightNode->m.unlock();
                        leftNode->m.unlock();
                        tempNode->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                    }
                }
            }else{
                node *aunt = grandparent->left;
                if(aunt->color == true){
                    aunt->color = false;
                    parent->color = false;
                    grandparent->color = true;
                    tempNode = grandparent;
                }
                else if(parent->right == tempNode){
                    parent->color = false;
                    grandparent->color = true;
                    node *sister = parent->left;
                    if(grandparent==root){
                        grandparent->m.lock();
                        parent->m.lock();
                        sister->m.lock();
                        root = parent;
                        parent->left = grandparent;
                        grandparent->right = sister;
                        sister->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                    }
                    else{
                        node *grandgrandparent = grandparent->parent;
                        grandgrandparent->m.lock();
                        grandparent->m.lock();
                        parent->m.lock();
                        sister->m.lock();
                        if(grandgrandparent->left==grandparent){
                            grandgrandparent->left = parent;
                        }else{
                            grandgrandparent->right = parent;
                        }
                        parent->left = grandparent;
                        grandparent->right = sister;
                        sister->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                        grandgrandparent->m.unlock();
                    }
                }
                else{
                    tempNode->color = false;
                    grandparent->color = true;
                    leftNode = tempNode->left;
                    rightNode = tempNode->right;
                    if(grandparent == root){
                        grandparent->m.lock();
                        parent->m.lock();
                        tempNode->m.lock();
                        leftNode->m.lock();
                        rightNode->m.lock();
                        root = tempNode;
                        tempNode->right = parent;
                        tempNode->left = grandparent;
                        parent->left = rightNode;
                        grandparent->right = leftNode;
                        rightNode->m.unlock();
                        leftNode->m.unlock();
                        tempNode->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                    }else{
                        node *grandgrandparent = grandparent->parent;
                        grandgrandparent->m.lock();
                        grandparent->m.lock();
                        parent->m.lock();
                        tempNode->m.lock();
                        leftNode->m.lock();
                        rightNode->m.lock();
                        if(grandgrandparent->left == grandparent){
                            grandgrandparent->left = tempNode;
                        }else{
                            grandgrandparent->right = tempNode;
                        }
                        tempNode->right = parent;
                        tempNode->left = grandparent;
                        parent->left = rightNode;
                        grandparent->right = leftNode;
                        rightNode->m.unlock();
                        leftNode->m.unlock();
                        tempNode->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                        grandgrandparent->m.unlock();
                    }
                }
            }
        }
    }
    root->color = false;
    locked->m.unlock();
    return ((void*)&found);
}

/**
 * Main funciton
 * @return
 */
int main()
{
    string fileName;
    cout << "please enter the file name: ";
    cin >> fileName;
    readfile(fileName);  // read the init tree and all actions
    cout << "Original tree (in order) : " << endl;
    printTree(root); // for init tree
    cout << endl;
    clock_t begin = clock(); // clock begin
    runFunctions(); // run all the functions
    int size = search_threads.size();
    for(int i=0;i<size;++i){
        char* msg=(char*) malloc(sizeof(char)*200);
        pthread_t pid = search_threads.front();
        pthread_join(pid,(void**)&msg);
        if(strncmp(msg,"",1)){
            cout << msg << endl;
        }
        search_threads.pop();
        free(msg);
    }
//    run_other_functions();
//    int size2 = modify_threads.size();
//    for(int j=0;j<size2;++j){
//        pthread_t pid = modify_threads.front();
//        pthread_join(pid,NULL);
//        cout << j <<endl;
//        modify_threads.pop();
//    }

    clock_t end = clock(); // clock stop
    cout << "Final tree (in order) : " << endl;
    printTree(root);
    cout << endl;
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout << "Execution time: " << elapsed_secs << endl; //print out the execution time
    return 0;
}

/**
 * utility function to print the RB-tree in pre-order
 * @param treeRoot
 */
void printTree(node* treeRoot){
    if (treeRoot == NULL){
        return;
    }
    if(treeRoot->key==-1){
        cout << "f" << " ";
    }
    else{
        char c;
        if(treeRoot->color==true){
            c = 'r';
        }else{
            c = 'b';
        }
        cout << treeRoot->key<< c << " ";
    }
    printTree(treeRoot->left);
    printTree(treeRoot->right);
}