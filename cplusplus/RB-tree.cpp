#include <iostream>
using namespace std;



typedef bool _rb_tree_color_type;

const _rb_tree_color_type _rb_tree_red = false;
const _rb_tree_color_type _rb_tree_black = true;



struct _rb_tree_node_base
{
    typedef _rb_tree_color_type color_type;
    typedef _rb_tree_node_base* base_ptr;

    color_type color;           // 节点颜色
    base_ptr parent;            // 父节点
    base_ptr left;              // 左节点
    base_ptr right;             // 右节点

    static base_ptr minimum(base_ptr x) { // 一路向左，找最小值
        while(x->left != 0) x = x->left;
        return x;
    }
    static base_ptr maximum(base_ptr x) { // 一路向右，找最大值
        while(x->right != 0) x = x->right;
        return x;
    }
};

template<class Value>
struct _rb_tree_node : public _rb_tree_node_base
{
    typedef _rb_tree_node<Value> *link_type;
    Value Value_field;          // 节点值
};


struct _rb_tree_base_iterator
{
    typedef _rb_tree_node_base::base_ptr base_ptr;
    typedef bidirectional_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;

    base_ptr node;

    void increment() {          // 自加
        if(node->right != 0) {
            node = node->right;
            while(node->left != 0)
                node = node->left;
        }
        else {
            base_ptr y = node->parent;
            while(node == y->right) {
                node = y;
                y = y->parent;
            }
            if(node->right != y)
                node = y;
        }
    }
    void decrement() {          // 自减
        if(node->color == _rb_tree_red &&
           node->parent->parent == node)
            node = node->right;
        else if(node->left != 0) {
            base_ptr y = node->left;
            while(y->right != 0) {
                y = y->right;
                node = y;
            }
        }
        else {
            base_ptr y = node->parent;
            while(node == y->left) {
                node = y;
                y = y->parent;
            }
            node = y;
        }
    }
};

template <class Value, class Ref, class Ptr>
struct _rb_tree_iterator : public _rb_tree_base_iterator
{
    typedef Value value_type;
    typedef Ref reference;
    typedef Ptr pointer;
    typedef _rb_tree_iterator<Value, Value&, Value*> iterator;
    typedef _rb_tree_iterator<Value, const Value&, const Value*> const_iterator;
    typedef _rb_tree_iterator<Value, Ref, Ptr> self;
    typedef _rb_tree_node<Value>* link_type;

    _rb_tree_iterator() {};
    _rb_tree_iterator(link_type x) {node = x;}
    _rb_tree_iterator(const iterator& it) {node = it.node;}

    reference operator*() const {return link_type(node)->Value_field;}
    pointer operator->() const {return &(operator*());}
    self& operator++() {increment(); return *this;}
    self operator++(int) {
        self temp = *this;
        increment();
        return temp;
    }
    self& operator--() {decrement(); return *this;}
    self operator--(int) {
        self temp = *this;
        decrement();
        return temp;
    }
};


// 未完待续



























int main()
{
    return 0;
}
