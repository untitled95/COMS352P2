#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <sstream>


using namespace std;

struct node;
vector<node*> getNodes(string a);
void initTreeHelper(node *current, node *next);
node* initTree(vector<node*> pre);
void readfile(string path);


 struct node
 {
     int key;
     struct node *left;
     struct node *right;
     struct node *parent;
     bool color;  //false:black true:red
     mutex m;
 };

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

    if (*&current->key!=-1 && *&current->left == nullptr){
        current->left = next;
        next->parent = current;
    }
    else if (*&current->key!=-1 && *&current->left != nullptr && *&current->right == nullptr){
        current->right = next;
        next->parent = current;
    }
    else if(*&current->key==-1 || (*&current->left != nullptr && *&current->right != nullptr)){
        if(*&current->parent!=nullptr){
            *&current = *&current->parent;
        }
        initTreeHelper(*&current,*&next);
    }
}

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

void printTree(node* treeRoot){
    if (treeRoot == NULL){
        return;
    }
    if(treeRoot->key==-1){
        cout << "f" << " ";
    }else{
        cout << treeRoot->key << " ";
    }
    printTree(treeRoot->left);
    printTree(treeRoot->right);
}

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
        node* root = initTree(initNodes);
    printTree(root);
    file.close();
}


int main()
{
    readfile("/Users/lyiming/CLionProjects/cs352/test.txt");

    return 0;

}