#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERS 100
#define MAX_NAME_LENGTH 50
#define MAX_LINE_LENGTH 100

// Kullanici yapisi
typedef struct User {
    int id;
    char name[MAX_NAME_LENGTH];
} User;

// Graf yapisi (komsuluk listesi)
typedef struct GraphNode {
    int userId;
    struct GraphNode* next;
} GraphNode;

typedef struct Graph {
    GraphNode* edges[MAX_USERS];
    User users[MAX_USERS];
    int userCount;
} Graph;

// İliski agaci dugumu
typedef struct RelationTreeNode {
    int userId;
    int depth;
    struct RelationTreeNode* firstChild;
    struct RelationTreeNode* nextSibling;
} RelationTreeNode;

// Kirmizi-Siyah Agac dugumu
typedef enum { RED, BLACK } Color;

typedef struct RBTreeNode {
    User user;
    Color color;
    struct RBTreeNode* left;
    struct RBTreeNode* right;
    struct RBTreeNode* parent;
} RBTreeNode;

// Global degiskenler
Graph socialNetwork;
RBTreeNode* rbTreeRoot = NULL;

// Dosyadan veri okuma fonksiyonlari
void trimWhitespace(char *str) {
    int i = strlen(str) - 1;
    while (i >= 0 && isspace(str[i])) {
        str[i--] = '\0';
    }
}

int findUserById(int id) {
    int i;
    for ( i = 0; i < socialNetwork.userCount; i++) {
        if (socialNetwork.users[i].id == id) {
            return i;
        }
    }
    return -1;
}

int addUserFromFile(int id, char* name) {
    if (socialNetwork.userCount >= MAX_USERS) return -1;
    
    // Kullanici zaten var mi kontrol et
    if (findUserById(id) != -1) return -1;
    
    User newUser;
    newUser.id = id;
    strncpy(newUser.name, name, MAX_NAME_LENGTH);
    socialNetwork.users[socialNetwork.userCount] = newUser;
    
    return socialNetwork.userCount++;
}

void addFriendshipFromFile(int userId1, int userId2) {
    int index1 = findUserById(userId1);
    int index2 = findUserById(userId2);
    
    if (index1 == -1 || index2 == -1) return;
    
    // Arkadaslik zaten var mi kontrol et
    GraphNode* node = socialNetwork.edges[index1];
    while (node != NULL) {
        if (node->userId == index2) return;
        node = node->next;
    }
    
    // userId1'in arkadas listesine userId2'yi ekle
    GraphNode* newNode = (GraphNode*)malloc(sizeof(GraphNode));
    newNode->userId = index2;
    newNode->next = socialNetwork.edges[index1];
    socialNetwork.edges[index1] = newNode;
    
    // userId2'nin arkadas listesine userId1'i ekle (cift yonlu)
    newNode = (GraphNode*)malloc(sizeof(GraphNode));
    newNode->userId = index1;
    newNode->next = socialNetwork.edges[index2];
    socialNetwork.edges[index2] = newNode;
}

void readDataFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Dosya acilamadi");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        trimWhitespace(line);
        
        if (strlen(line) == 0) continue;
        
        if (strncmp(line, "USER", 4) == 0) {
            int id;
            char name[MAX_NAME_LENGTH];
            if (sscanf(line, "USER %d %49[^\n]", &id, name) == 2) {
                addUserFromFile(id, name);
            }
        } else if (strncmp(line, "FRIEND", 6) == 0) {
            int id1, id2;
            if (sscanf(line, "FRIEND %d %d", &id1, &id2) == 2) {
                addFriendshipFromFile(id1, id2);
            }
        }
    }
    
    fclose(file);
}

// Graf islemleri
void initializeGraph() {
    int i;
    for (i = 0; i < MAX_USERS; i++) {
        socialNetwork.edges[i] = NULL;
    }
    socialNetwork.userCount = 0;
}

// İliski agaci olusturma
RelationTreeNode* buildRelationTree(int rootUserId, int maxDepth) {
    int rootIndex = findUserById(rootUserId);
    if (rootIndex == -1) return NULL;
    
    bool visited[MAX_USERS] = {false};
    RelationTreeNode* root = (RelationTreeNode*)malloc(sizeof(RelationTreeNode));
    root->userId = rootIndex;
    root->depth = 0;
    root->firstChild = NULL;
    root->nextSibling = NULL;
    
    RelationTreeNode* queue[MAX_USERS];
    int front = 0, rear = 0;
    queue[rear++] = root;
    visited[rootIndex] = true;
    
    while (front < rear) {
        RelationTreeNode* current = queue[front++];
        
        if (current->depth >= maxDepth) continue;
        
        GraphNode* friendNode = socialNetwork.edges[current->userId];
        RelationTreeNode* lastChild = NULL;
        
        while (friendNode != NULL) {
            if (!visited[friendNode->userId]) {
                RelationTreeNode* child = (RelationTreeNode*)malloc(sizeof(RelationTreeNode));
                child->userId = friendNode->userId;
                child->depth = current->depth + 1;
                child->firstChild = NULL;
                child->nextSibling = NULL;
                
                if (lastChild == NULL) {
                    current->firstChild = child;
                } else {
                    lastChild->nextSibling = child;
                }
                lastChild = child;
                
                queue[rear++] = child;
                visited[friendNode->userId] = true;
            }
            friendNode = friendNode->next;
        }
    }
    
    return root;
}

// Depth-first arama ile belirli mesafedeki arkadaslari bulma
void findFriendsAtDepthDFS(RelationTreeNode* root, int targetDepth, int currentDepth, int* friends, int* count) {
    if (root == NULL) return;
    
    if (currentDepth == targetDepth) {
        friends[(*count)++] = socialNetwork.users[root->userId].id;
        return;
    }
    
    RelationTreeNode* child = root->firstChild;
    while (child != NULL) {
        findFriendsAtDepthDFS(child, targetDepth, currentDepth + 1, friends, count);
        child = child->nextSibling;
    }
}

// Ortak arkadas analizi
void findCommonFriends(int userId1, int userId2, int* commonFriends, int* count) {
    *count = 0;
    int index1 = findUserById(userId1);
    int index2 = findUserById(userId2);
    
    if (index1 == -1 || index2 == -1) return;
    
    bool friendsOf1[MAX_USERS] = {false};
    
    // userId1'in arkadaslarini isaretle
    GraphNode* friendNode = socialNetwork.edges[index1];
    while (friendNode != NULL) {
        friendsOf1[friendNode->userId] = true;
        friendNode = friendNode->next;
    }
    
    // userId2'nin arkadaslarinda işaretli olanlari bul
    friendNode = socialNetwork.edges[index2];
    while (friendNode != NULL) {
        if (friendsOf1[friendNode->userId]) {
            commonFriends[(*count)++] = socialNetwork.users[friendNode->userId].id;
        }
        friendNode = friendNode->next;
    }
}

// Topluluk tespiti (basitlestirilmiş BFS tabanli)
void detectCommunities() {
    bool visited[MAX_USERS] = {false};
    int communityId = 1;
    int i;
    for ( i = 0; i < socialNetwork.userCount; i++) {
        if (!visited[i]) {
            printf("Topluluk %d: ", communityId++);
            
            int queue[MAX_USERS];
            int front = 0, rear = 0;
            queue[rear++] = i;
            visited[i] = true;
            
            while (front < rear) {
                int current = queue[front++];
                printf("%s ", socialNetwork.users[current].name);
                
                GraphNode* friendNode = socialNetwork.edges[current];
                while (friendNode != NULL) {
                    if (!visited[friendNode->userId]) {
                        visited[friendNode->userId] = true;
                        queue[rear++] = friendNode->userId;
                    }
                    friendNode = friendNode->next;
                }
            }
            printf("\n");
        }
    }
}

// Etki alani hesaplama (bir kullanicinin ag üzerindeki etkisi)
float calculateInfluence(int userId) {
    int index = findUserById(userId);
    if (index == -1) return 0.0f;
    
    int totalFriends = 0;
    GraphNode* friendNode = socialNetwork.edges[index];
    while (friendNode != NULL) {
        totalFriends++;
        friendNode = friendNode->next;
    }
    
    if (totalFriends == 0) return 0.0f;
    
    float influence = 0.0f;
    friendNode = socialNetwork.edges[index];
    while (friendNode != NULL) {
        int friendId = friendNode->userId;
        int friendsOfFriend = 0;
        
        GraphNode* fofNode = socialNetwork.edges[friendId];
        while (fofNode != NULL) {
            friendsOfFriend++;
            fofNode = fofNode->next;
        }
        
        influence += 1.0f / (friendsOfFriend + 1);
        friendNode = friendNode->next;
    }
    
    return influence / totalFriends;
}

// Kirmizi-Siyah Agac Islemleri
RBTreeNode* createRBTreeNode(User user) {
    RBTreeNode* newNode = (RBTreeNode*)malloc(sizeof(RBTreeNode));
    newNode->user = user;
    newNode->color = RED;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->parent = NULL;
    return newNode;
}

void leftRotate(RBTreeNode** root, RBTreeNode* x) {
    RBTreeNode* y = x->right;
    x->right = y->left;
    
    if (y->left != NULL) {
        y->left->parent = x;
    }
    
    y->parent = x->parent;
    
    if (x->parent == NULL) {
        *root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    
    y->left = x;
    x->parent = y;
}

void rightRotate(RBTreeNode** root, RBTreeNode* y) {
    RBTreeNode* x = y->left;
    y->left = x->right;
    
    if (x->right != NULL) {
        x->right->parent = y;
    }
    
    x->parent = y->parent;
    
    if (y->parent == NULL) {
        *root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    
    x->right = y;
    y->parent = x;
}

void rbInsertFixup(RBTreeNode** root, RBTreeNode* z) {
    while (z->parent != NULL && z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            RBTreeNode* y = z->parent->parent->right;
            
            if (y != NULL && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    leftRotate(root, z);
                }
                
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rightRotate(root, z->parent->parent);
            }
        } else {
            RBTreeNode* y = z->parent->parent->left;
            
            if (y != NULL && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(root, z);
                }
                
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                leftRotate(root, z->parent->parent);
            }
        }
    }
    
    (*root)->color = BLACK;
}

void rbInsert(RBTreeNode** root, User user) {
    RBTreeNode* z = createRBTreeNode(user);
    RBTreeNode* y = NULL;
    RBTreeNode* x = *root;
    
    while (x != NULL) {
        y = x;
        if (z->user.id < x->user.id) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    
    z->parent = y;
    
    if (y == NULL) {
        *root = z;
    } else if (z->user.id < y->user.id) {
        y->left = z;
    } else {
        y->right = z;
    }
    
    rbInsertFixup(root, z);
}

RBTreeNode* rbSearch(RBTreeNode* root, int userId) {
    if (root == NULL || root->user.id == userId) {
        return root;
    }
    
    if (userId < root->user.id) {
        return rbSearch(root->left, userId);
    } else {
        return rbSearch(root->right, userId);
    }
}

// Yardimci fonksiyonlar
void printRelationTree(RelationTreeNode* root) {
    if (root == NULL) return;
    
    printf("%*s%s (ID: %d, Depth: %d)\n", root->depth * 2, "", 
           socialNetwork.users[root->userId].name, 
           socialNetwork.users[root->userId].id,
           root->depth);
    
    RelationTreeNode* child = root->firstChild;
    while (child != NULL) {
        printRelationTree(child);
        child = child->nextSibling;
    }
}

void freeRelationTree(RelationTreeNode* root) {
    if (root == NULL) return;
    
    RelationTreeNode* child = root->firstChild;
    while (child != NULL) {
        RelationTreeNode* next = child->nextSibling;
        freeRelationTree(child);
        child = next;
    }
    
    free(root);
}

void freeRBTree(RBTreeNode* root) {
    if (root == NULL) return;
    
    freeRBTree(root->left);
    freeRBTree(root->right);
    free(root);
}

void freeGraph() {
    int i;
    for (i = 0; i < socialNetwork.userCount; i++) {
        GraphNode* current = socialNetwork.edges[i];
        while (current != NULL) {
            GraphNode* temp = current;
            current = current->next;
            free(temp);
        }
    }
}

// Kullanici arayuzu fonksiyonlari
void printMenu() {
    printf("\nSosyal Ag Analiz Sistemi\n");
    printf("1. Iliski agaci olustur\n");
    printf("2. Belirli mesafedeki arkadaslari bul\n");
    printf("3. Ortak arkadas analizi yap\n");
    printf("4. Topluluk tespiti yap\n");
    printf("5. Kullanici etki alani hesapla\n");
    printf("6. Kirmizi-Siyah agacta kullanici ara\n");
    printf("7. Cikis\n");
    printf("Seciminiz: ");
}

int main() {
    initializeGraph();
    
    // Veri dosyasını oku
    readDataFromFile("veriseti.txt");
    
    // Kirmizi-Siyah agaca kullanicilari ekle
    int i;
    for ( i = 0; i < socialNetwork.userCount; i++) {
        rbInsert(&rbTreeRoot, socialNetwork.users[i]);
    }
    
    int choice;
    do {
        printMenu();
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                int userId, depth;
                printf("Iliski agaci olusturulacak kullanici ID: ");
                scanf("%d", &userId);
                printf("Maksimum derinlik: ");
                scanf("%d", &depth);
                
                RelationTreeNode* tree = buildRelationTree(userId, depth);
                if (tree == NULL) {
                    printf("Kullanici bulunamadi!\n");
                } else {
                    printf("\nIliski Agaci:\n");
                    printRelationTree(tree);
                    freeRelationTree(tree);
                }
                break;
            }
            case 2: {
                int userId, depth;
                printf("Kullanici ID: ");
                scanf("%d", &userId);
                printf("Aranacak arkadas derinligi: ");
                scanf("%d", &depth);
                
                RelationTreeNode* tree = buildRelationTree(userId, depth);
                if (tree == NULL) {
                    printf("Kullanici bulunamadi!\n");
                } else {
                    int friends[MAX_USERS];
                    int count = 0;
                    findFriendsAtDepthDFS(tree, depth, 0, friends, &count);
                    
                    if (count == 0) {
                        printf("Belirtilen derinlikte arkadas bulunamadi.\n");
                    } else {
                        printf("%d. dereceden arkadaslar:\n", depth);
                        int i;
                        for ( i = 0; i < count; i++) {
                            int index = findUserById(friends[i]);
                            printf("- %s (ID: %d)\n", socialNetwork.users[index].name, friends[i]);
                        }
                    }
                    freeRelationTree(tree);
                }
                break;
            }
            case 3: {
                int userId1, userId2;
                printf("Birinci kullanici ID: ");
                scanf("%d", &userId1);
                printf("İkinci kullanici ID: ");
                scanf("%d", &userId2);
                
                int commonFriends[MAX_USERS];
                int count = 0;
                findCommonFriends(userId1, userId2, commonFriends, &count);
                
                if (count == 0) {
                    printf("Ortak arkadas bulunamadi.\n");
                } else {
                    printf("Ortak arkadaslar:\n");
                    int i;
                    for (i = 0; i < count; i++) {
                        int index = findUserById(commonFriends[i]);
                        printf("- %s (ID: %d)\n", socialNetwork.users[index].name, commonFriends[i]);
                    }
                }
                break;
            }
            case 4: {
                printf("\nTopluluk Tespiti:\n");
                detectCommunities();
                break;
            }
            case 5: {
                int userId;
                printf("Etki alani hesaplanacak kullanici ID: ");
                scanf("%d", &userId);
                
                float influence = calculateInfluence(userId);
                int index = findUserById(userId);
                if (index == -1) {
                    printf("Kullanici bulunamadi!\n");
                } else {
                    printf("%s (ID: %d) kullanicisinin etki alani: %.2f\n",
                           socialNetwork.users[index].name, userId, influence);
                }
                break;
            }
            case 6: {
                int userId;
                printf("Aranacak kullanici ID: ");
                scanf("%d", &userId);
                
                RBTreeNode* found = rbSearch(rbTreeRoot, userId);
                if (found != NULL) {
                    printf("Kullanici bulundu: %s (ID: %d)\n", found->user.name, found->user.id);
                } else {
                    printf("Kullanici bulunamadi.\n");
                }
                break;
            }
            case 7:
                printf("Program sonlandiriliyor...\n");
                break;
            default:
                printf("Gecersiz secim!\n");
        }
    } while (choice != 7);
    
    // Bellegi temizle
    freeRBTree(rbTreeRoot);
    freeGraph();
    
    return 0;
}