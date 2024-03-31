#include "test_runner.h"
#include <cassert>
#include <deque>
#include <iostream>

using namespace std;


struct Node {
    Node(int v, Node* p)
        : value(v)
        , parent(p)
    {}

    int value;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* parent;
};


class NodeBuilder {
public:
    Node* CreateRoot(int value) {
        nodes.emplace_back(value, nullptr);
        return &nodes.back();
    }

    Node* CreateLeftSon(Node* me, int value) {
        assert(me->left == nullptr);
        nodes.emplace_back(value, me);
        me->left = &nodes.back();
        return me->left;
    }

    Node* CreateRightSon(Node* me, int value) {
        assert(me->right == nullptr);
        nodes.emplace_back(value, me);
        me->right = &nodes.back();
        return me->right;
    }

private:
    deque<Node> nodes;
};


Node* Next(Node* me) {
    assert(me);

    if (me->right) {
        Node* temp = me->right;
        while (temp->left != nullptr)
            temp = temp->left;
        return temp;
    }
    else if (me->parent) {
        Node* temp = me->parent;
        while (temp != nullptr && temp->value < me->value)
            temp = temp->parent;
        return temp;
    }
    else
        return nullptr;
}


void Test1() {
    NodeBuilder nb;

    Node* root = nb.CreateRoot(50);
    ASSERT_EQUAL(root->value, 50);

    Node* l = nb.CreateLeftSon(root, 2);
    Node* min = nb.CreateLeftSon(l, 1);
    Node* r = nb.CreateRightSon(l, 4);
    ASSERT_EQUAL(min->value, 1);
    ASSERT_EQUAL(r->parent->value, 2);

    nb.CreateLeftSon(r, 3);
    Node* max = nb.CreateRightSon(r, 5);
    ASSERT_EQUAL(Next(max)->value, 50);

    r = nb.CreateRightSon(root, 100);
    l = nb.CreateLeftSon(r, 90);
    max = nb.CreateRightSon(r, 101);

    nb.CreateLeftSon(l, 89);
    r = nb.CreateRightSon(l, 91);

    ASSERT_EQUAL(Next(l)->value, 91);
    ASSERT_EQUAL(Next(root)->value, 89);
    ASSERT_EQUAL(Next(min)->value, 2);
    ASSERT_EQUAL(Next(r)->value, 100);

    ASSERT(Next(max) == nullptr);

    while (min) {
        cout << min->value << '\n';
        min = Next(min);
    }
}

void TestRootOnly() {
    NodeBuilder nb;
    Node* root = nb.CreateRoot(42);
    ASSERT(Next(root) == nullptr);
};


int main() {
    TestRunner tr;
    RUN_TEST(tr, Test1);
    RUN_TEST(tr, TestRootOnly);
    return 0;
}