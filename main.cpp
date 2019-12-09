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
#include <thread>
#include <shared_mutex>

/**
 * @author yiming ling
 * wrote based on the extra credit request
 */

using namespace std;


struct node;
vector<node*> getNodes(string a);
void initTreeHelper(node *current, node *next);
node* initTree(vector<node*> pre);
void readfile(string path);
void* search(void* key);
void* insert(void* key);
void printTree(node* treeRoot);
void runFunctions();
void run_other_functions();
void leftrotate(node *x);
void rightrotate(node *x);
void* deleteNode(void *key);
void rbTransplant(node* u, node* v);
void delfix(node *x);
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
     recursive_mutex m;
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
//    int number_of_search_thread = 0;
//    int number_of_modify_thread = 0;
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

            // I tried to create all the threads first and found out on pyrite, sometime will cause dead lock.
            // Therefore, I only create threads as needed later.
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
            if(tempCommands[i]=="delete" || tempCommands[i]=="insert") {
                modify_threads.push(tid);
            }
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
    if(tempRoot->key==-1){
        string f = "search(" ;
        string n = to_string(val);
        string f2 = ")-> false, performed by thread ";
        string id = to_string(long(pthread_self()));
        string output = f+n+f2+id;
        char* realOutput = strdup(output.c_str());
        return (realOutput);
    }else{

        tempRoot->m.lock();
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
            parent->m.unlock();
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
    tempRoot->m.unlock();
    string f = "search(" ;
    string n = to_string(val);
    string f2 = ")-> false, performed by thread ";
    string id = to_string(long(pthread_self()));
    string output = f+n+f2+id;
    char* realOutput = strdup(output.c_str());
    return (realOutput);
}


/**
 * Insert function with lock
 * @param key
 * @return
 */
mutex p;
//vector<node*> m;
//vector<node*> n;

void* insert(void* key){
    int val = *(int*)key;
    bool found = false;
    node *tempNode = root;

    tempNode->m.lock();

//    m.push_back(tempNode);
    node *locked = tempNode;

    while(tempNode->key!=-1 && !found){
        node *parent = tempNode;
        if(tempNode->key==val){
            found = true;
        }
        else if(val < tempNode->key){
            tempNode = tempNode->left;

        }else{
            tempNode = tempNode->right;
        }
        if((tempNode->color==false) && (parent->color ==false) && (parent!=locked)){
            parent->m.lock();
//            m.push_back(parent);
            locked->m.unlock();
//            n.push_back(locked);
            locked = parent;
        }
    }

    if(found==false){
        tempNode->m.lock();
//        m.push_back(tempNode);

        tempNode->color = true;
        tempNode->key = val;
        node *leftNode = new node;

        leftNode->key = -1;
        leftNode->left = nullptr;
        leftNode->right = nullptr;
        leftNode->parent = tempNode;
        leftNode->color = false;

        node *rightNode = new node;

        rightNode->key = -1;
        rightNode->left = nullptr;
        rightNode->right = nullptr;
        rightNode->parent = tempNode;
        rightNode->color = false;

        tempNode->left = leftNode;
        leftNode->parent = tempNode;

        tempNode->right = rightNode;
        rightNode->parent=tempNode;

        tempNode->m.unlock();
//        n.push_back(tempNode);

        while(tempNode!=root && tempNode->parent->color == true){

//            vector<node*> aaa = m;
//            vector<node*> bbb = n;

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
//                        m.push_back(grandparent);
//                        m.push_back(parent);
//                        m.push_back(sister);
                        root = parent;
                        parent->right = grandparent;
                        grandparent->parent = parent;

                        grandparent->left = sister;
                        sister->parent=grandparent;

                        sister->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
//                        n.push_back(sister);
//                        n.push_back(parent);
//                        n.push_back(grandparent);

                    }else{
                        node *grandgrandparent = grandparent->parent;

                        grandgrandparent->m.lock();
                        grandparent->m.lock();
                        parent->m.lock();
                        sister->m.lock();

//                        m.push_back(grandgrandparent);
//                        m.push_back(grandparent);
//                        m.push_back(parent);
//                        m.push_back(sister);

                        if(grandgrandparent->left == grandparent){
                            grandgrandparent->left = parent;
                            parent->parent = grandgrandparent;
                        }else{
                            grandgrandparent->right = parent;
                            parent->parent = grandgrandparent;
                        }
                        parent->right = grandparent;
                        grandparent->parent = parent;

                        grandparent->left = sister;
                        sister->parent = grandparent;

                        sister->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                        grandgrandparent->m.unlock();

//                        n.push_back(sister);
//                        n.push_back(parent);
//                        n.push_back(grandparent);
//                        n.push_back(grandgrandparent);

                    }
                }
                else {
                    tempNode->color = false;
                    grandparent->color = true;
                    leftNode = tempNode->left;
                    rightNode = tempNode->right;
                    if (grandparent == root) {

                        grandparent->m.lock();
                        parent->m.lock();
                        tempNode->m.lock();
                        leftNode->m.lock();
                        rightNode->m.lock();

//                        m.push_back(grandparent);
//                        m.push_back(parent);
//                        m.push_back(tempNode);
//                        m.push_back(leftNode);
//                        m.push_back(rightNode);

                        root = tempNode;
                        tempNode->left = parent;
                        parent->parent = tempNode;

                        tempNode->right = grandparent;
                        grandparent->parent = tempNode;

                        parent->right = leftNode;
                        leftNode->parent = parent;

                        grandparent->left = rightNode;
                        rightNode->parent = grandparent;

                        rightNode->m.unlock();
                        leftNode->m.unlock();
                        tempNode->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();

//                        n.push_back(rightNode);
//                        n.push_back(leftNode);
//                        n.push_back(tempNode);
//                        n.push_back(parent);
//                        n.push_back(grandparent);

                    } else {
                        node *grandgrandparent = grandparent->parent;

                        grandgrandparent->m.lock();
                        grandparent->m.lock();
                        parent->m.lock();
                        tempNode->m.lock();
                        leftNode->m.lock();
                        rightNode->m.lock();

//                        m.push_back(grandgrandparent);
//                        m.push_back(grandparent);
//                        m.push_back(parent);
//                        m.push_back(tempNode);
//                        m.push_back(leftNode);
//                        m.push_back(rightNode);

                        if (grandgrandparent->left == grandparent) {
                            grandgrandparent->left = tempNode;
                            tempNode->parent = grandgrandparent;
                        } else {
                            grandgrandparent->right = tempNode;
                            tempNode->parent = grandgrandparent;
                        }
                        tempNode->left = parent;
                        parent->parent = tempNode;

                        tempNode->right = grandparent;
                        grandparent->parent = tempNode;

                        parent->right = leftNode;
                        leftNode->parent = parent;

                        grandparent->left = rightNode;
                        rightNode->parent = grandparent;

                        rightNode->m.unlock();
                        leftNode->m.unlock();
                        tempNode->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                        grandgrandparent->m.unlock();

//                        n.push_back(rightNode);
//                        n.push_back(leftNode);
//                        n.push_back(tempNode);
//                        n.push_back(parent);
//                        n.push_back(grandparent);
//                        n.push_back(grandgrandparent);
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

//                        m.push_back(grandparent);
//                        m.push_back(parent);
//                        m.push_back(sister);

                        root = parent;
                        parent->left = grandparent;
                        grandparent->parent = parent;

                        grandparent->right = sister;
                        sister->parent = grandparent;

                        sister->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();

//                        n.push_back(sister);
//                        n.push_back(parent);
//                        n.push_back(grandparent);
                    }
                    else{
                        node *grandgrandparent = grandparent->parent;

                        grandgrandparent->m.lock();
                        grandparent->m.lock();
                        parent->m.lock();
                        sister->m.lock();

//                        m.push_back(grandgrandparent);
//                        m.push_back(grandparent);
//                        m.push_back(parent);
//                        m.push_back(sister);

                        if(grandgrandparent->left==grandparent){
                            grandgrandparent->left = parent;
                            parent->parent = grandgrandparent;
                        }else{
                            grandgrandparent->right = parent;
                            parent->parent = grandgrandparent;
                        }
                        parent->left = grandparent;
                        grandparent->parent = parent;

                        grandparent->right = sister;
                        sister->parent = grandparent;

                        sister->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                        grandgrandparent->m.unlock();

//                        n.push_back(sister);
//                        n.push_back(parent);
//                        n.push_back(grandparent);
//                        n.push_back(grandgrandparent);

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

//                        m.push_back(grandparent);
//                        m.push_back(parent);
//                        m.push_back(tempNode);
//                        m.push_back(leftNode);
//                        m.push_back(rightNode);

                        root = tempNode;
                        tempNode->right = parent;
                        parent->parent = tempNode;

                        tempNode->left = grandparent;
                        grandparent->parent = tempNode;

                        parent->left = rightNode;
                        rightNode->parent = parent;

                        grandparent->right = leftNode;
                        leftNode->parent = grandparent;

                        rightNode->m.unlock();
                        leftNode->m.unlock();
                        tempNode->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();

//                        n.push_back(rightNode);
//                        n.push_back(leftNode);
//                        n.push_back(tempNode);
//                        n.push_back(parent);
//                        n.push_back(grandparent);

                    }else{
                        node *grandgrandparent = grandparent->parent;
                        grandgrandparent->m.lock();
                        grandparent->m.lock();
                        parent->m.lock();
                        tempNode->m.lock();
                        leftNode->m.lock();
                        rightNode->m.lock();

//                        m.push_back(grandgrandparent);
//                        m.push_back(grandparent);
//                        m.push_back(parent);
//                        m.push_back(tempNode);
//                        m.push_back(leftNode);
//                        m.push_back(rightNode);

                        if(grandgrandparent->left == grandparent){
                            grandgrandparent->left = tempNode;
                            tempNode->parent = grandgrandparent;
                        }else{
                            grandgrandparent->right = tempNode;
                            tempNode->parent = grandgrandparent;
                        }
                        tempNode->right = parent;
                        parent->parent = tempNode;

                        tempNode->left = grandparent;
                        grandparent->parent = tempNode;

                        parent->left = rightNode;
                        rightNode->parent = parent;

                        grandparent->right = leftNode;
                        leftNode->parent = grandparent;

                        rightNode->m.unlock();
                        leftNode->m.unlock();
                        tempNode->m.unlock();
                        parent->m.unlock();
                        grandparent->m.unlock();
                        grandgrandparent->m.unlock();

//                        n.push_back(rightNode);
//                        n.push_back(leftNode);
//                        n.push_back(tempNode);
//                        n.push_back(parent);
//                        n.push_back(grandparent);
//                        n.push_back(grandgrandparent);

                    }
                }
            }
        }
    }
    root->color = false;
    locked->m.unlock();
//    n.push_back(locked);
    return (NULL);
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
//        free(msg);
    }
    run_other_functions();
    int size2 = modify_threads.size();
    for(int j=0;j<size2;++j){
        pthread_t pid = modify_threads.front();
        pthread_join(pid,NULL);
        modify_threads.pop();
    }

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


mutex modifyMutex;
/**
 * delete Node main function
 * @param key
 * @return
 */
void *deleteNode(void *key)
{
    int val =  *(int*)key;
    if(root->key==-1) //root is nil
    {
        return (NULL);
    }
    node *z = root;
    node *x = nullptr;
    bool found = false;
    z->m.lock();
    while(z->key!=-1 && found == false)
    {
        node* parentNode = z;
        if(z->key == val)
            found = true;
        if(found == false)
        {
            if(z->key < val)
                z = z->right;
            else
                z = z->left;
        }
        if (parentNode->key!=-1) {
            parentNode->m.unlock();
        }
        if (!found && z->key!=-1) {
            z->m.lock();
        }
    }
    if (z->key!=-1 && !found) {
        z->m.unlock();
    }
    if(found == false)
    {
        return (NULL);
    }
    else
    {
        modifyMutex.lock();
        node* y = z;
        node* node5 = z->parent;
        if (node5 != nullptr && node5->key!=-1) {
            node5->m.lock();
        }
        node* node1 = y->parent->left;
        if (node1 != nullptr && node1->key!=-1) {
            node1->m.lock();
        }
        node* node2 = y->parent->right;
        if (node2 != nullptr && node2->key!=-1) {
            node2->m.lock();
        }
        node* node3 = z->right;
        if (node3 != nullptr && node3->key!=-1) {
            node3->m.lock();
        }
        node* node4 = z->left;
        if (node4 != nullptr && node4->key!=-1){
            node4->m.lock();
        }
        //delMutex.unlock();
        modifyMutex.unlock();
        node* node6 = nullptr;
        node* node7 = nullptr;
        node* node8 = nullptr;
        node* node9 = nullptr;
        // old function
        bool y_original_color = y->color;
        if (z->left->key==-1){
            x = z->right;
            rbTransplant(z,z->right);

        }
        else if (z->right->key==-1){
            x = z->left;
            rbTransplant(z,z->left);
        }
        else{
            node* p = z->right; // p is node3
            while (p->left->key!=-1){
                node* parentNode = p;
                p = p->left;
                if (parentNode->key!=-1 && parentNode != node3) {
                    parentNode->m.unlock();
                }
                if(p->key!=-1) {
                    p->m.lock();
                }
            }

            y = p; // has locked
            node9 = p;
            node6 = y->left;
            if (node6 != nullptr  && node6->key!=-1) {
                node6->m.lock();
            }
            node7 = y->right; // = x
            if (node7 != nullptr && node7->key!=-1) {
                node7->m.lock();
            }
            bool unlockMutex = false;
            y_original_color = y->color;
            x = y->right;
            if (y->parent == z)
                x->parent = y;
            else{
                node8 = y->parent;
                if (node8 != nullptr && node8->key!=-1 && node8 != node3) {
                    node8->m.lock();

                }

                rbTransplant(y,y->right);
                y->right = z->right;
                y->right->parent = y;
            }

            rbTransplant(z,y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }
        if(y_original_color == false)
            delfix(x); // x has lock
        //delMutex.lock();
        //printMutex.lock();
        if (node5 != nullptr && node5->key!=-1) {
            node5->m.unlock();
        }
        if (node1 != nullptr && node1->key!=-1){
            node1->m.unlock();
        }
        if (node2 != nullptr && node2->key!=-1) {
            node2->m.unlock();
        }
        if (node3 != nullptr && node3->key!=-1) {
            node3->m.unlock();
        }
        if (node4 != nullptr && node4->key!=-1) {
            node4->m.unlock();
        }
        if (node6 != nullptr && node6->key!=-1) {
            node6->m.unlock();
        }
        if (node7 != nullptr && node7->key!=-1) {
           node7->m.unlock();
        }
        if (node8 != nullptr && node8->key!=-1){
           node8->m.unlock();

        }
        if (node9 != nullptr && node9->key!=-1){
           node9->m.unlock();
        }
    }

    return (NULL);

}

/**
 * delete helper function
 * @param x
 */
void leftrotate(node *x)
{
    if(x->right->key==-1)
        return ;
    else
    {
        node *y = x->right;
        x->right = y->left;
        if(y->left->key!=-1)
        {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if(x->parent->key==-1)
            root = y;
        else
        {
            if(x == x->parent->left)
                x->parent->left = y;
            else
                x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }
}

/**
 * delete helper function
 * @param x
 */
void rightrotate(node *x)
{
    if(x->left->key==-1)
        return ;
    else
    {
        node *y = x->left;
        x->left = y->right;
        if(y->right->key!=-1)
        {
            y->right->parent = x;
        }
        y->parent=x->parent;
        if(x->parent->key==-1)
            root = y;
        else
        {
            if(x == x->parent->left)
                x->parent->left = y;
            else
                x->parent->right = y;
        }
        y->right = x;
        x->parent = y;
    }
}

/**
 * delete help function
 * @param u
 * @param v
 */
void rbTransplant(node* u, node* v){
    if (u->parent->key==-1){
        root = v;
    }
    else if (u == u->parent->left){
        u->parent->left = v;
    }
    else{
        u->parent->right = v;
    }
    v->parent = u->parent;
}


/**
 * delete helper function
 * @param x
 */
void delfix(node *x)
{
    node *w;
    node* node1 = nullptr;
    node* node2 = nullptr;
    while(x != root && x->color == false)
    {
        if(x->parent->left == x)
        {
            w=x->parent->right;
            //delMutex.lock();
            node1 = w->left;
            if (node1 != nullptr && node1->key!=-1) {
               node1->m.lock();

                node2 = w->left->right;
                if (node2 != nullptr && node2->key!=-1) {
                    node2->m.lock();
                }
            }

            if(w->color== true)
            {
                w->color = false;
                x->parent->color = true;
                leftrotate(x->parent);
                w = x->parent->right;
            }
            if(w->left != nullptr && w->right != nullptr && w->left->color ==false && w->right->color == false )
            {
                w->color = true;
                x = x->parent;
            }
            else
            {
                if(w->right != nullptr && w->right->color == false)
                {
                    w->left->color = false;
                    w->color = true;
                    rightrotate(w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = false;
                if (w->right != nullptr)
                    w->right->color = false;
                leftrotate(x->parent);
                x=root;
            }
        }
        else
        {
            w=x->parent->left;
            //delMutex.lock();
            node1 = w->right;
            if (node1 != nullptr && node1->key!=-1) {
                node1->m.lock();

                node2 = w->right->left;
                if (node2 != nullptr && node2->key!=-1) {
                    node2->m.lock();
                }
            }
            //delMutex.unlock();
            if(w->color == true)
            {
                w->color = false;
                x->parent->color = true;
                rightrotate(x->parent);
                w=x->parent->left;
            }
            if(w->left != nullptr && w->right != nullptr && w->left->color == false && w->right->color == false)
            {
                w->color = true;
                x = x->parent;
            }
            else
            {
                if(w->left != nullptr && w->left->color == false)
                {
                    w->right->color = false;
                    w->color = true;
                    leftrotate(w);
                    w=x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = false;
                if (w->left != nullptr)
                    w->left->color = false;
                rightrotate(x->parent);
                x=root;
            }
        }
        x->color = false;
        root->color= false;
        //delMutex.lock();
        if (node1 != nullptr && node1->key!=-1) {
            node1->m.unlock();
        }
        if (node2 != nullptr && node2->key!=-1) {
           node2->m.unlock();

        }
        //delMutex.unlock();
    }
}