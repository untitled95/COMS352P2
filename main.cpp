#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <thread>

using namespace std;

struct node;
vector<node*> getNodes(string a);
void initTreeHelper(node *current, node *next);
node* initTree(vector<node*> pre);
void readfile(string path);
bool search(int key);
node* insertHelper(node *root, node *a);
void printTree(node* treeRoot);

 struct node
 {
     int key;
     struct node *left;
     struct node *right;
     struct node *parent;
     bool color;  //false:black true:red
     mutex m;
 };

 node *root = new node;


 //get all nodes separate from each line and init value for those nodes.
vector<node*> getNodes(string a){
    vector<string> tempNodes;
    vector<node*> nodes;
    stringstream s_stream(a);
    while(s_stream.good()) {
        string substr;
        getline(s_stream, substr, ',');
        tempNodes.push_back(substr);
    }
    for(uint16_t i=0;i<tempNodes.size();i++) {
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


// decide add the next node on left or right. or back to parent node
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
            current = *&current->parent;
        }
        initTreeHelper(current,next);
    }
}


// init the RB-tree based on the init text input
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



//read the inpur file
void readfile(string path){
    string line;
    string nodeLines;
    ifstream file;
    file.open(path);
    if(file.is_open())
        while (!file.eof())
        {
            getline(file, line);
            if(line!=""){
                if((line[0]<='9' && line[0]>='0' )|| line[0]=='f' ) {
                    nodeLines = nodeLines + line; //read all the init nodes
                }else{
                    if(line.find(',')){
                        //read the executions
                    }else{
                        //create all the threads
                    }
                }
            }
        }
    vector<node*> initNodes = getNodes(nodeLines);
        root = initTree(initNodes);
    file.close();
}

// search the node, if found return true, otherwise return false
// lock necessary nodes and unlock after
bool search(int key) {
    bool found = false;
    node *tempRoot = root;
    tempRoot->m.lock();
    if(tempRoot->key==-1){
        tempRoot->m.unlock();
        return found;
    }
    while(tempRoot->key!=-1 && found==false){
        node *parent = tempRoot;
        if(tempRoot->key==key){
            return found;
        }else if(key>=tempRoot->key){
            tempRoot = tempRoot->right;
        }else{
            tempRoot = tempRoot->left;
        }
        tempRoot->m.lock();
        parent->m.unlock();
    }
    tempRoot->m.unlock();
    return found;
}

// insert function with lock
void insert(int key){
    bool found = false;
    node *tempNode = root;
    tempNode->m.lock();
    node *locked = tempNode;
    while(tempNode->key!=-1 && found==false){
        node *parent = tempNode;
        if(key == tempNode->key){
            found = true;
        }else if(key < tempNode->key){
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

}

//helper function for insert the node as BST
node* insertHelper(node *root, node *a){

    if(root == NULL){
        return a;
    }
    if(a->key < root->key){
        root->left = insertHelper(root->left,a);
        root->left->parent = root;
    }else if(a->key > root->key){
        root->right = insertHelper(root->right,a);
        root->right->parent = root;
    }
    return root;
}

//rotateLeft
void rotateLeft(node *&root, node *&pt)
{
    node *pt_right = pt->right;

    pt->right = pt_right->left;

    if (pt->right != NULL)
        pt->right->parent = pt;

    pt_right->parent = pt->parent;

    if (pt->parent == NULL)
        root = pt_right;

    else if (pt == pt->parent->left)
        pt->parent->left = pt_right;

    else
        pt->parent->right = pt_right;

    pt_right->left = pt;
    pt->parent = pt_right;
}

int main()
{
    string fileName;
    cout << "please enter the file name: ";
    cin >> fileName;
    readfile(fileName);
    printTree(root);
    return 0;
}

//utility function to print the RB-tree in pre-order
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